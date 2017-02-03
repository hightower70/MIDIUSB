/*****************************************************************************/
/* System timer (1ms) routines                                               */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __sysTimer_h
#define __sysTimer_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Constants                                                                 */
/*****************************************************************************/
#define sysTIMER_TICKS_PER_SECOND 1000 // number of tick in one second

/*****************************************************************************/
/* Types                                                                     */
/*****************************************************************************/
#ifndef drvSYSTEMTIMER_USE16BIT
typedef uint32_t sysTick;
#else
typedef uint16_t sysTick;
#endif

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void sysTimerInitialize(void);
void halSystemTimerInitialize(void);
sysTick sysTimerGetTimestamp(void);
void sysTimerDelay(sysTick in_delay_ms);
sysTick sysTimerGetTimeSince(sysTick in_start_time_ms);

#endif
