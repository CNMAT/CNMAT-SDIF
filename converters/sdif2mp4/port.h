/* Original author: Eric D. Scheirer, MIT Media Laboratory
 *
 * This source file has been placed in the public domain by its author(s).
 */

#ifndef _port_h_
#define _port_h_

/* WIN32 automatically defined by the Visual C++ build environment */
#if defined(_WIN32_) || defined(WIN32)
#include <stdlib.h>
#include <stdarg.h>
#include <malloc.h>
#include <io.h>

/* max length for filenames */
#define MAX_LEN _MAX_PATH

/* Visual C++ supports exception handling */
/* #define USE_EXCEPTION */

#else /* ! WIN32 */

/*
 *  Most Unix C++ compilers do not support exceptions; if yours does,
 *  uncomment the following line
 */
/* #define USE_EXCEPTION */

#if HAVE_UNISTD_H
# include <sys/types.h>
# include <unistd.h>
#endif

#if HAVE_LIMITS_H
# include <limits.h>
#endif

#ifdef PATH_MAX
# define MAX_LEN PATH_MAX
#else
# define MAX_LEN 1024
#endif

#if HAVE_BSTRING_H
# include <bstring.h>
#endif

#if HAVE_STRINGS_H
# include <strings.h>
#endif

#if HAVE_STRING_H
# include <string.h>
#endif

#if HAVE_MALLOC_H
# include <malloc.h>
#endif

#if HAVE_STDLIB_H
# include <stdlib.h>
#endif

#endif  /* ! WIN32 */

#endif /* ! _port_h_ */

