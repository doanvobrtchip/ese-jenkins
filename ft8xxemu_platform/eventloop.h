/*

Copyright (C) 2016  by authors
Author: Jan Boon <jan.boon@kaetemi.be>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef THREADUTIL_EVENTLOOP_H
#define THREADUTIL_EVENTLOOP_H

#define EVENTLOOP_CONCURRENT_QUEUE
#define EVENTLOOP_WIN32_EVENT

#include <thread>
#include <mutex>
#include <condition_variable>

#include <functional>

#include <queue>
#include <set>

#ifdef EVENTLOOP_CONCURRENT_QUEUE
#include <concurrent_queue.h>
#include <concurrent_priority_queue.h>
#endif

#ifdef EVENTLOOP_WIN32_EVENT
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

class EventLoop
{
public:
	EventLoop() : m_Running(false), m_Cancel(false)
	{
#ifdef EVENTLOOP_WIN32_EVENT
		m_PokeEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
#endif
	}

	~EventLoop()
	{
		stop();
		clear();
#ifdef EVENTLOOP_WIN32_EVENT
		CloseHandle(m_PokeEvent);
#endif
	}

	void run()
	{
		stop();
		m_Running = true;
		m_Thread = std::move(std::thread(&EventLoop::loop, this));
	}

	void runSync()
	{
		stop();
		m_Running = true;
		loop();
	}

	void stop() // thread-safe
	{
		m_Running = false;
		poke();
		if (m_Thread.joinable())
			m_Thread.join();
	}

	void clear() // thread-safe
	{
#ifdef EVENTLOOP_CONCURRENT_QUEUE
		m_ImmediateConcurrent.clear();
		m_TimeoutConcurrent.clear();
#else
		std::unique_lock<std::mutex> lock(m_QueueLock);
		std::unique_lock<std::mutex> tlock(m_QueueTimeoutLock);
		m_Immediate = std::move(std::queue<std::function<void()>>());
		m_Timeout = std::move(std::priority_queue<timeout_func>());
#endif
	}

	//! Call from inside an interval function to prevent it from being called again
	void cancel()
	{
		m_Cancel = true;
	}

	//! Block call until the queued functions  finished processing. Set empty to repeat the wait until the queue is empty
	void join(bool empty = false) // thread-safe
	{
		std::mutex syncLock;
		std::condition_variable syncCond;
		std::unique_lock<std::mutex> lock(syncLock);
		std::function<void()> syncFunc = [this, &syncLock, &syncCond, &syncFunc, empty]() -> void {
			std::unique_lock<std::mutex> lock(syncLock);
#ifdef EVENTLOOP_CONCURRENT_QUEUE
			if (empty && !m_ImmediateConcurrent.empty())
#else
			bool immediateEmpty;
			; {
				std::unique_lock<std::mutex> lock(m_QueueLock);
				immediateEmpty = m_Immediate.empty();
			}
			if (empty && !immediateEmpty)
#endif
			{
				immediate(syncFunc);
			}
			else
			{
				syncCond.notify_one();
			}
		};
		immediate(syncFunc);
		syncCond.wait(lock);
	}

public:
	void immediate(std::function<void()> f) // thread-safe
	{
#ifdef EVENTLOOP_CONCURRENT_QUEUE
		m_ImmediateConcurrent.push(f);
#else
		std::unique_lock<std::mutex> lock(m_QueueLock);
		m_Immediate.push(f);
#endif
		poke();
	}

	template<class rep, class period> void timeout(std::function<void()> f, const std::chrono::duration<rep, period>& delta) // thread-safe
	{
		timeout_func tf;
		tf.f = f;
		tf.time = std::chrono::steady_clock::now() + delta;
		tf.interval = std::chrono::nanoseconds::zero();
		; {
#ifdef EVENTLOOP_CONCURRENT_QUEUE
			m_TimeoutConcurrent.push(std::move(tf));
#else
			std::unique_lock<std::mutex> lock(m_QueueTimeoutLock);
			m_Timeout.push(std::move(tf));
#endif
		}
		poke();
	}

	template<class rep, class period> void interval(std::function<void()> f, const std::chrono::duration<rep, period>& interval) // thread-safe
	{
		timeout_func tf;
		tf.f = f;
		tf.time = std::chrono::steady_clock::now() + interval;
		tf.interval = interval;
		; {
#ifdef EVENTLOOP_CONCURRENT_QUEUE
			m_TimeoutConcurrent.push(std::move(tf));
#else
			std::unique_lock<std::mutex> lock(m_QueueTimeoutLock);
			m_Timeout.push(std::move(tf));
#endif
		}
		poke();
	}


	void timed(std::function<void()> f, const std::chrono::steady_clock::time_point &point) // thread-safe
	{
		timeout_func tf;
		tf.f = f;
		tf.time = point;
		tf.interval = std::chrono::steady_clock::duration::zero();
		; {
#ifdef EVENTLOOP_CONCURRENT_QUEUE
			m_TimeoutConcurrent.push(std::move(tf));
#else
			std::unique_lock<std::mutex> lock(m_QueueTimeoutLock);
			m_Timeout.push(std::move(tf));
#endif
		}
		poke();
	}

public:
	void thread(std::function<void()> f, std::function<void()> callback)
	{
		std::thread t([this, f, callback]() -> void {
			f();
			immediate(callback);
		});
		t.detach();
	}

private:
	void loop()
	{
		while (m_Running)
		{
#ifndef EVENTLOOP_WIN32_EVENT
			m_Poked = false;
#endif

			for (;;)
			{
#ifdef EVENTLOOP_CONCURRENT_QUEUE
				std::function<void()> f;
				if (!m_ImmediateConcurrent.try_pop(f))
					break;
#else
				m_QueueLock.lock();
				if (!m_Immediate.size())
				{
					m_QueueLock.unlock();
					break;
				}
				std::function<void()> f = m_Immediate.front();
				m_Immediate.pop();
				m_QueueLock.unlock();
#endif
				f();
			}

			bool poked = false;
			for (;;)
			{
#ifdef EVENTLOOP_CONCURRENT_QUEUE
				timeout_func tf;
				if (!m_TimeoutConcurrent.try_pop(tf))
					break;
				const timeout_func &tfr = tf;
#else
				m_QueueTimeoutLock.lock();
				if (!m_Timeout.size())
				{
					m_QueueTimeoutLock.unlock();
					break;
				}
				const timeout_func &tfr = m_Timeout.top();
#endif
				std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
#ifdef EVENTLOOP_WIN32_EVENT
				DWORD wt = (DWORD)std::chrono::duration_cast<std::chrono::milliseconds>(tfr.time - now).count();
				if (tfr.time > now && wt > 0)
#else
				if (tfr.time > now) // wait
#endif
				{
#ifdef EVENTLOOP_CONCURRENT_QUEUE
					m_TimeoutConcurrent.push(tf);
#else
					m_QueueTimeoutLock.unlock();
#endif
#ifdef EVENTLOOP_WIN32_EVENT
					WaitForSingleObject(m_PokeEvent, wt);
#else
					; {
						std::unique_lock<std::mutex> lock(m_PokeLock);
						if (!m_Poked)
							m_PokeCond.wait_until(lock, tfr.time);
					}
#endif
					poked = true;
					break;
				}
#ifndef EVENTLOOP_CONCURRENT_QUEUE
				timeout_func tf = tfr;
				m_Timeout.pop();
				m_QueueTimeoutLock.unlock();
#endif
				m_Cancel = false;
				tf.f(); // call
				if (!m_Cancel && (tf.interval > std::chrono::nanoseconds::zero())) // repeat
				{
					tf.time += tf.interval;
					; {
#ifdef EVENTLOOP_CONCURRENT_QUEUE
						m_TimeoutConcurrent.push(std::move(tf));
#else
						std::unique_lock<std::mutex> lock(m_QueueTimeoutLock);
						m_Timeout.push(std::move(tf));
#endif
					}
				}
			}

			if (!poked)
			{
#ifdef EVENTLOOP_WIN32_EVENT
				WaitForSingleObject(m_PokeEvent, INFINITE);
#else
				std::unique_lock<std::mutex> lock(m_PokeLock);
				if (!m_Poked)
					m_PokeCond.wait(lock);
#endif
			}
		}
	}

	void poke() // private
	{
#ifdef EVENTLOOP_WIN32_EVENT
		SetEvent(m_PokeEvent);
#else
		std::unique_lock<std::mutex> lock(m_PokeLock);
		m_PokeCond.notify_one();
		m_Poked = true;
#endif
	}

private:
	struct timeout_func
	{
		std::function<void()> f;
		std::chrono::steady_clock::time_point time;
		std::chrono::steady_clock::duration interval;

		bool operator <(const timeout_func &o) const
		{
			return time > o.time;
		}

	};

private:
	bool m_Running;
#ifndef EVENTLOOP_WIN32_EVENT
	volatile bool m_Poked;
#endif
	std::thread m_Thread;
#ifndef EVENTLOOP_WIN32_EVENT
	std::mutex m_PokeLock;
	std::condition_variable m_PokeCond;
#else
	HANDLE m_PokeEvent;
#endif

#ifdef EVENTLOOP_CONCURRENT_QUEUE
	concurrency::concurrent_queue<std::function<void()>> m_ImmediateConcurrent;
	concurrency::concurrent_priority_queue<timeout_func> m_TimeoutConcurrent;
#else
	std::mutex m_QueueLock;
	std::queue<std::function<void()>> m_Immediate;
	std::mutex m_QueueTimeoutLock;
	std::priority_queue<timeout_func> m_Timeout;
#endif
	bool m_Cancel;

};

#endif /* THREADUTIL_EVENTLOOP_H */

/* end of file */
