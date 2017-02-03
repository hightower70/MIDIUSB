/*****************************************************************************/
/* Main Entry Function                                                       */
/*                                                                           */
/* Copyright (C) 2016 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/
#ifndef __sysMain_h
#define __sysMain_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTypes.h>

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void sysInitialization(void);
void appMainTask(void);
void appInitialization(void);
#endif
