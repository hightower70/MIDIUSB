/*****************************************************************************/
/* Collection of various system helper functions                             */
/*                                                                           */
/* Copyright (C) 2017 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

#ifndef __sysHelpers_h
#define __sysHelpers_h

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <stddef.h>

/*****************************************************************************/
/* Function prototypes                                                       */
/*****************************************************************************/
void sysMemZero(void* in_destination, size_t in_size);
void sysMemCopy(void* in_destination, void* in_source, size_t in_size);

#endif