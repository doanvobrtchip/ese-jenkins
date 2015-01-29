/*
 * Test notification definitions
 *
 * These are used by common tests to show test progress.
 * A test executable calls NOTIFY_STARTED() with its test
 * name at the start of the test.
 *
 * At the end of the test it calls either NOTIFY_FAILED()
 * or NOTIFY_PASSED(). At this point the test is complete
 * and execution can stop.
 */

#if !defined(FT900_TEST_NOTIFY)
#define FT900_TEST_NOTIFY

#include <stdio.h>

// Use stringification to avoid pulling in printf() just for
// reporting FAIL line numbers.
// Details on this particular trick at
//   http://gcc.gnu.org/onlinedocs/cpp/Stringification.html

#define __XSTR(X) __STR(X)
#define __STR(X)  #X

#define NOTIFY_STARTED(nm) puts("TEST " #nm " STARTED")
#define NOTIFY_READY()     ((*(int volatile *)0x18000) = 1)
#define NOTIFY_FAILED()    puts("TEST FAILED AT LINE " __XSTR(__LINE__))
#define NOTIFY_PASSED()    puts("TEST PASSED")

#endif
