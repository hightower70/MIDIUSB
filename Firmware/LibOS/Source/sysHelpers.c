/*****************************************************************************/
/* Collection of various system helper functions                             */
/*                                                                           */
/* Copyright (C) 2017 Laszlo Arvai                                           */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <string.h>

///////////////////////////////////////////////////////////////////////////////
/// @brief Fills the given memory region with zeros
/// @param in_destination memory address to fill with zeros
/// @param in_size Number of bytes to fill
void sysMemZero(void* in_destination, size_t in_size)  
{
	memset(in_destination, 0, in_size);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Copies the given memory region 
/// @param in_destination Address where the memory if copied to
/// @param in_source Address copy memory content from
/// @param in_size Number of bytes to copy
void sysMemCopy(void* in_destination, void* in_source, size_t in_size)
{
	memcpy(in_destination, in_source, in_size);
}