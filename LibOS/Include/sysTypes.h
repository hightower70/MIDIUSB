/*****************************************************************************/
/* System Type Definitions                                                   */
/*                                                                           */
/* Copyright (C) 2014 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __sysTypes_h
#define __sysTypes_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>


/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define sysSTRING_MAX_LENGTH 0xfffe
#define sysSTRING_INVALID_POS 0xffff

/*****************************************************************************/
/* Common types                                                              */
/*****************************************************************************/
typedef uint16_t sysStringLength;

typedef char sysChar;
typedef char sysASCIIChar;
typedef uint16_t sysUnicodeChar;
typedef sysChar*	sysString;
typedef const sysChar* sysConstString;

/*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/
#ifndef LOW
#define LOW(x) ((x)&0xff)
#endif

#ifndef HIGH
#define HIGH(x) ((x)>>8)
#endif

#ifndef BV
#define BV(x) (1<<(x))
#endif

#define STRINGIZE(x) ___STRINGIZE(x)
#define ___STRINGIZE(x) #x

#define sysNULL 0

#define sysUNUSED(x) (void)(x)

#define sysASSERT(x) assert_param(x)

#endif
