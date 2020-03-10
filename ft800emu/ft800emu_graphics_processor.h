/*
FT800 Emulator Library
FT810 Emulator Library
Copyright (C) 2013-2016  Future Technology Devices International Ltd
BT815 Emulator Library
Copyright (C) 2016-2019  Bridgetek Pte Lte
Author: Jan Boon <jan@no-break.space>
*/

#ifndef FT800EMU_GRAPHICS_PROCESSOR_H
#define FT800EMU_GRAPHICS_PROCESSOR_H
// #include <...>

#ifdef BT815EMU_MODE
// Select only one or no cache mechanism
#define BT815EMU_ASTC_CONCURRENT_MAP_CACHE 0
#define BT815EMU_ASTC_LAST_CACHE 0
#define BT815EMU_ASTC_THREAD_LOCAL_CACHE 0
#define BT815EMU_ASTC_CONCURRENT_BUCKET_MAP_CACHE 1
#define BT812EMU_ASTC_SEPARATE_CBMAP_CACHE 0
#endif

// Select only one or no thread spreading preference
#define FT800EMU_SPREAD_RENDER_THREADS 0
#define FT800EMU_SPREAD_RENDER_THREADS_FAIR 0
#define FT800EMU_SPREAD_RENDER_THREADS_SNAP 0

// System includes
#include <vector>
#include <memory>

#ifdef BT815EMU_MODE
#	if BT815EMU_ASTC_CONCURRENT_MAP_CACHE
#pragma warning(push)
#pragma warning(disable : 26495)
#		include <astc_codec_internals.h>
#pragma warning(pop)
#		undef IGNORE
#		include <concurrent_unordered_map.h>
#	endif
#	if BT815EMU_ASTC_THREAD_LOCAL_CACHE
#pragma warning(push)
#pragma warning(disable : 26495)
#		include <astc_codec_internals.h>
#pragma warning(pop)
#		undef IGNORE
#		include <unordered_map>
#		include <shared_mutex>
#	endif
#	if BT815EMU_ASTC_CONCURRENT_BUCKET_MAP_CACHE || BT812EMU_ASTC_SEPARATE_CBMAP_CACHE
#pragma warning(push)
#pragma warning(disable : 26495)
#		include <astc_codec_internals.h>
#pragma warning(pop)
#		undef IGNORE
#		include "concurrent_bucket_map.h"
#	endif
#endif

// Project includes
#include "bt8xxemu_inttypes.h"

#ifdef FT810EMU_MODE
#define FT800EMU_SCREEN_WIDTH_MAX 2048
#define FT800EMU_SCREEN_HEIGHT_MAX 2048
#else
#define FT800EMU_SCREEN_WIDTH_MAX 512
#define FT800EMU_SCREEN_HEIGHT_MAX 512
#endif
#define FT800EMU_SCREEN_HEIGHT_MASK (FT800EMU_SCREEN_HEIGHT_MAX - 1)

#define FT800EMU_BITMAP_HANDLE_NB 32

#define FT800EMU_DEBUGMODE_NONE 0
#define FT800EMU_DEBUGMODE_ALPHA 1
#define FT800EMU_DEBUGMODE_TAG 2
#define FT800EMU_DEBUGMODE_STENCIL 3
#define FT800EMU_DEBUGMODE_COUNT 4

namespace FT8XXEMU {
	class System;
}

namespace FT800EMU {
	class Memory;
	class Touch;

#pragma warning(push)
#pragma warning(disable : 26495)

struct BitmapInfo
{
	uint32_t Source;
	int LayoutFormat;
	int LayoutPixelWidth;
	int LayoutPixelHeight;
	int LayoutStride;
	int LayoutLines;
	int SizeFilter;
	int SizeWrapX;
	int SizeWrapY;
	int SizeWidth;
	int SizeHeight;
#ifdef BT815EMU_MODE
	int ExtFormat;
	union
	{
		uint32_t U;
		struct {
			uint8_t B;
			uint8_t G;
			uint8_t R;
			uint8_t A;
		};
	} Swizzle;
#endif
	
};

#ifdef FT810EMU_MODE
#define FT810EMU_SWAPXY_PARAM , const bool swapXY
#define FT810EMU_SWAPXY , swapXY
#define FT810EMU_SWAPXY_FALSE , false
#else
#define FT810EMU_SWAPXY_PARAM
#define FT810EMU_SWAPXY
#define FT810EMU_SWAPXY_FALSE
#endif

#ifndef FTEMU_GRAPHICS_PROCESSOR_SEMI_PRIVATE
#define FTEMU_GRAPHICS_PROCESSOR_SEMI_PRIVATE private
#endif

#ifdef BT815EMU_MODE
#	if BT815EMU_ASTC_CONCURRENT_MAP_CACHE
struct AstcCacheEntry
{
#pragma warning(push)
#pragma warning(disable: 26495) // C not initialized on purpose
	AstcCacheEntry() : Ok(false) { }
#pragma warning(pop)
	physical_compressed_block PhysicalBlock;
	argb8888 C[12 * 12]; // Not MAX_TEXELS_PER_BLOCK
	volatile bool Ok;
};
typedef concurrency::concurrent_unordered_map<ptrdiff_t, AstcCacheEntry> AstcCache;
#	endif
#	if BT815EMU_ASTC_THREAD_LOCAL_CACHE
struct AstcCacheEntry
{
	physical_compressed_block PhysicalBlock;
	argb8888 Color[MAX_TEXELS_PER_BLOCK];
};
typedef std::unordered_map<ptrdiff_t, AstcCacheEntry> AstcCache;
#	endif
#	if BT815EMU_ASTC_CONCURRENT_BUCKET_MAP_CACHE
struct AstcCacheEntry
{
#pragma warning(push)
#pragma warning(disable: 26495) // C not initialized on purpose
	AstcCacheEntry() : Ok(false) { }
#pragma warning(pop)
	physical_compressed_block PhysicalBlock;
	argb8888 C[12 * 12]; // Not MAX_TEXELS_PER_BLOCK
	volatile bool Ok;
};
typedef concurrent_bucket_map<size_t, AstcCacheEntry> AstcCache;
#	endif
#	if BT812EMU_ASTC_SEPARATE_CBMAP_CACHE
struct AstcCacheEntry
{
#pragma warning(push)
#pragma warning(disable: 26495) // C not initialized on purpose
	AstcCacheEntry() : Ok(false) { }
#pragma warning(pop)
	volatile bool Ok;
	physical_compressed_block PhysicalBlock;
	argb8888 C[MAX_TEXELS_PER_BLOCK];
};
template <size_t tSize>
struct AstcCacheEntrySub
{
	static_assert(tSize <= MAX_TEXELS_PER_BLOCK, "Texels per block exceeds known value");
#pragma warning(push)
#pragma warning(disable: 26495) // C not initialized on purpose
	AstcCacheEntrySub() : Ok(false) { }
#pragma warning(pop)
	volatile bool Ok;
	physical_compressed_block PhysicalBlock;
	argb8888 C[tSize];
};
template <size_t tSize>
using AstcCache = concurrent_bucket_map<size_t, AstcCacheEntrySub<tSize>>;
#	endif
#endif

/**
 * GraphicsProcessor
 * \brief GraphicsProcessor
 * \date 2013-06-22 09:29GMT
 * \author Jan Boon (Kaetemi)
 */
class GraphicsProcessor
{
public:
	GraphicsProcessor(FT8XXEMU::System *system, Memory *memory, Touch *touch, bool backgroundPerformance);
	~GraphicsProcessor();

	void setThreadPriority(bool realtime);

	void process(
		argb8888 *screenArgb8888, 
		bool upsideDown, 
		bool mirrored, 
#ifdef FT810EMU_MODE
		bool swapXY,
#endif
		uint32_t hsize, 
		uint32_t yTop, 
		uint32_t yBottom, 
		uint32_t yInc = 1);
	void processBlank();

	void processTrace(int *result, int *size, uint32_t x, uint32_t y, uint32_t hsize);

	// Enables or disables emuating REG_PWM_DUTY by fading to black
	void enableRegPwmDutyEmulation(bool enabled = true);

	// Enables multithreaded rendering, sets thread count to number of available CPU cores
	void enableMultithread(bool enabled = true);
	// Reduces the number of used threads with the specified amount
	void reduceThreads(int nb);

public:
	void setDebugMode(int debugMode);
	inline int getDebugMode() { return m_DebugMode; }
	void setDebugMultiplier(int debugMultiplier);
	inline int getDebugMultiplier() { return m_DebugMultiplier; }
	void setDebugLimiter(int debugLimiter);
	inline int getDebugLimiter() { return m_DebugLimiter; }

	inline bool getDebugLimiterEffective() { return m_DebugLimiterEffective; }
	inline int getDebugLimiterIndex() { return m_DebugLimiterIndex; }

public:
	inline bool getRegPwmDutyEmulation() { return m_RegPwmDutyEmulation; }

public:
	inline FT8XXEMU::System *system() { return m_System; }
	inline Memory *memory() { return m_Memory; }
	inline Touch *touch() { return m_Touch; }

private:
	struct ThreadInfo;
	std::vector<std::unique_ptr<ThreadInfo> > m_ThreadInfos;

	void resizeThreadInfos(int size);
	void launchGraphicsProcessorThread(ThreadInfo *li);

private:
	FT8XXEMU::System *m_System;
	Memory *m_Memory;
	Touch *m_Touch;

	// Master copy of bitmap
	BitmapInfo m_BitmapInfoMaster[FT800EMU_BITMAP_HANDLE_NB];
FTEMU_GRAPHICS_PROCESSOR_SEMI_PRIVATE:
#ifdef BT815EMU_MODE
#	if BT812EMU_ASTC_SEPARATE_CBMAP_CACHE
	// ASTC Cache per format
	AstcCache<4 * 4> m_AstcCache4x4;
	AstcCache<5 * 4> m_AstcCache5x4;
	AstcCache<5 * 5> m_AstcCache5x5;
	AstcCache<6 * 5> m_AstcCache6x5;
	AstcCache<6 * 6> m_AstcCache6x6;
	AstcCache<8 * 5> m_AstcCache8x5;
	AstcCache<8 * 6> m_AstcCache8x6;
	AstcCache<8 * 8> m_AstcCache8x8;
	AstcCache<10 * 5> m_AstcCache10x5;
	AstcCache<10 * 6> m_AstcCache10x6;
	AstcCache<10 * 8> m_AstcCache10x8;
	AstcCache<10 * 10> m_AstcCache10x10;
	AstcCache<12 * 10> m_AstcCache12x10;
	AstcCache<12 * 12> m_AstcCache12x12;
	AstcCache<sizeof(AstcCacheEntry().C) / sizeof(AstcCacheEntry().C[0])> m_AstcCacheMAX;
	BT8XXEMU_FORCE_INLINE AstcCacheEntry &getAstcCacheEntry(int format, AstcCache<1>::key_t key)
	{
		switch (format)
		{
		case 0: return reinterpret_cast<AstcCacheEntry &>(m_AstcCache4x4.find_or_emplace(key));
		case 1: return reinterpret_cast<AstcCacheEntry &>(m_AstcCache5x4.find_or_emplace(key));
		case 2: return reinterpret_cast<AstcCacheEntry &>(m_AstcCache5x5.find_or_emplace(key));
		case 3: return reinterpret_cast<AstcCacheEntry &>(m_AstcCache6x5.find_or_emplace(key));
		case 4: return reinterpret_cast<AstcCacheEntry &>(m_AstcCache6x6.find_or_emplace(key));
		case 5: return reinterpret_cast<AstcCacheEntry &>(m_AstcCache8x5.find_or_emplace(key));
		case 6: return reinterpret_cast<AstcCacheEntry &>(m_AstcCache8x6.find_or_emplace(key));
		case 7: return reinterpret_cast<AstcCacheEntry &>(m_AstcCache8x8.find_or_emplace(key));
		case 8: return reinterpret_cast<AstcCacheEntry &>(m_AstcCache10x5.find_or_emplace(key));
		case 9: return reinterpret_cast<AstcCacheEntry &>(m_AstcCache10x6.find_or_emplace(key));
		case 10: return reinterpret_cast<AstcCacheEntry &>(m_AstcCache10x8.find_or_emplace(key));
		case 11: return reinterpret_cast<AstcCacheEntry &>(m_AstcCache10x10.find_or_emplace(key));
		case 12: return reinterpret_cast<AstcCacheEntry &>(m_AstcCache12x10.find_or_emplace(key));
		case 13: return reinterpret_cast<AstcCacheEntry &>(m_AstcCache12x12.find_or_emplace(key));
		default: return reinterpret_cast<AstcCacheEntry &>(m_AstcCacheMAX.find_or_emplace(key)); // Should never reach here
		}
	}
	BT8XXEMU_FORCE_INLINE AstcCacheEntry &clearAstcCache(int format, AstcCache<1>::key_t key)
	{
		m_AstcCache4x4.clear();
		m_AstcCache5x4.clear();
		m_AstcCache5x5.clear();
		m_AstcCache6x5.clear();
		m_AstcCache6x6.clear();
		m_AstcCache8x5.clear();
		m_AstcCache8x6.clear();
		m_AstcCache8x8.clear();
		m_AstcCache10x5.clear();
		m_AstcCache10x6.clear();
		m_AstcCache10x8.clear();
		m_AstcCache10x10.clear();
		m_AstcCache12x10.clear();
		m_AstcCache12x12.clear();
	}
#	endif
#	if BT815EMU_ASTC_CONCURRENT_MAP_CACHE || BT815EMU_ASTC_CONCURRENT_BUCKET_MAP_CACHE
	// ASTC Cache
	AstcCache m_AstcCache;
#	endif
#	if BT815EMU_ASTC_THREAD_LOCAL_CACHE
	AstcCache m_CachedAstc;
#ifdef _USING_V110_SDK71_
	std::shared_timed_mutex m_CachedAstcMutex;
#else
	std::shared_mutex m_CachedAstcMutex;
#endif
#	endif
#endif

private:
	bool m_RegPwmDutyEmulation;

	// Threading options
	int m_ThreadCount;
	bool m_ThreadPriorityRealtime;
	bool m_BackgroundPerformance;

	// Visual debugging options
	int m_DebugMode;
	int m_DebugMultiplier;
	int m_DebugLimiter;
	bool m_DebugLimiterEffective;
	int m_DebugLimiterIndex;

FTEMU_GRAPHICS_PROCESSOR_SEMI_PRIVATE:
	// Cursor tracing
	uint32_t m_DebugTraceX;
	uint32_t m_DebugTraceLine;
	int *m_DebugTraceStack;
	int m_DebugTraceStackMax;
	int *m_DebugTraceStackSize;

private:
	template <bool debugTrace>
	void processPart(argb8888 *const screenArgb8888, const bool upsideDown, const bool mirrored FT810EMU_SWAPXY_PARAM, const uint32_t hsize, const uint32_t yTop, const uint32_t yBottom, const uint32_t yStart, const uint32_t yInc, const uint32_t yNum, BitmapInfo *const bitmapInfo);
	void processBlankDL(BitmapInfo *const bitmapInfo);

private:
	GraphicsProcessor(const GraphicsProcessor &) = delete;
	GraphicsProcessor &operator=(const GraphicsProcessor &) = delete;

}; /* class GraphicsProcessor */

#pragma warning(pop)

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_GRAPHICS_PROCESSOR_H */

/* end of file */
