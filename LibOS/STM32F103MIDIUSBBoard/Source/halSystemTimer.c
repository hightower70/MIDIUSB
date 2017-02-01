/*****************************************************************************/
/* System timer (1ms) driver                                                 */
/* (emulated on Win32)                                                       */
/*                                                                           */
/* Copyright (C) 2014-2015 Laszlo Arvai                                      */
/* All rights reserved.                                                      */
/*                                                                           */
/* This software may be modified and distributed under the terms             */
/* of the GNU General Public License.  See the LICENSE file for details.     */
/*****************************************************************************/

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include <sysTimer.h>
#include <sysConfig.h>

/*****************************************************************************/
/* Function implementation                                                   */
/*****************************************************************************/

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes system timer
void halSystemTimerInitialize(void)
{
	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

	 /* SysTick_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Gets system timer current value (current timestamp)
/// @return System timer value
sysTick sysTimerGetTimestamp(void)
{
	return HAL_GetTick();
}

///////////////////////////////////////////////////////////////////////////////
/// @ brief This function handles System tick timer.
void SysTick_Handler(void)
{
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
}