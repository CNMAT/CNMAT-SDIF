/* ISO_HEADER_START */

/* 
 * $Id: port.h,v 1.3 2001/11/27 22:23:53 matt Exp $
 *
 * Copyright (C) 1997, ISO/IEC
 * 
 * The software module contained in this file was originally developed by
 * Prof. Alexandros Eleftheriadis and Yihan Fang (Columbia University) as part
 * of the Flavor run-time library (see http://www.ee.columbia.edu/flavor).
 * Copyright has been transferred to ISO/IEC for inclusion in the MPEG-4
 * Reference Software (ISO/IEC 14496-5) distribution. ISO/IEC gives users of
 * this code free license to this software or modifications thereof for use in
 * hardware or software products claiming conformance to one or more parts of
 * the MPEG-4 (ISO/IEC 14496) series of specifications. Those intending to use
 * this Reference Software in hardware or software products are advised that
 * its use may infringe existing patents. The original developer of this
 * software module and his company, the subsequent editors and their
 * companies, and ISO/IEC have no liability for use of this software or
 * modifications thereof in an implementation. Copyright is not released for
 * imlementations that do not comply to one or more parts of the MPEG-4
 * (ISO/IEC 14496) series of specifications. Columbia University retains full
 * right to use the code for its own purpose, including assignment or
 * donatation of this code to a third party.
 * 
 * This copyright notice must be included in all copies or derivative works.
 * 
 * For more information or to receive updated versions of this module, contact
 * Prof. Alexandros Eleftheriadis at eleft@ee.columbia.edu.
 */

/* ISO_HEADER_END */

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

