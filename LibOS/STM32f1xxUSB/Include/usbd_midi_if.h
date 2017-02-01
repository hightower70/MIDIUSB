/**
  ******************************************************************************
  * @file           : usbd_cdc_if.h
  * @brief          : Header for usbd_cdc_if file.
  ******************************************************************************
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  * 1. Redistributions of source code must retain the above copyright notice,
  * this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  * this list of conditions and the following disclaimer in the documentation
  * and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of its contributors
  * may be used to endorse or promote products derived from this software
  * without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_MIDI_IF_H
#define __USBD_MIDI_IF_H

#ifdef __cplusplus
 extern "C" {
#endif
/* Includes ------------------------------------------------------------------*/
#include "usbd_midi.h"
/* USER CODE BEGIN INCLUDE */
/* USER CODE END INCLUDE */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */
  
/** @defgroup USBD_MIDI_IF
  * @brief header 
  * @{
  */ 
	 
	 typedef enum {
		 NoteOff       = 0x8,
		 NoteOn        = 0x9,
		 PolyPressure  = 0xa,
		 CC            = 0xb,
		 ProgramChange = 0xc,
		 Aftertouch    = 0xd,
		 PitchBend     = 0xe
	 } midi_event_t;


	 typedef union {
		 struct {
			 uint32_t ALL;
		 };
		 struct {
			 uint8_t cin_cable;
			 uint8_t evnt0;
			 uint8_t evnt1;
			 uint8_t evnt2;
		 };
		 struct {
			 uint8_t type : 4;
			 uint8_t cable : 4;
			 uint8_t chn : 4; 
			 uint8_t event : 4; 
			 uint8_t value1;
			 uint8_t value2;
		 };
		 struct {
			 uint8_t cin : 4;
			 uint8_t dummy1_cable : 4;
			 uint8_t dummy1_chn : 4;  
			 uint8_t dummy1_event : 4; 
			 uint8_t note : 8;
			 uint8_t velocity : 8;
		 };
		 struct {
			 uint8_t dummy2_cin : 4;
			 uint8_t dummy2_cable : 4;
			 uint8_t dummy2_chn : 4; 
			 uint8_t dummy2_event : 4; 
			 uint8_t cc_number : 8;
			 uint8_t value : 8;
		 };
	 } midi_package_t; 
	 

/** @defgroup USBD_MIDI_IF_Exported_Types
  * @{
  */  
/* USER CODE BEGIN EXPORTED_TYPES  */
/* USER CODE END  EXPORTED_TYPES */

/**
  * @}
  */ 

/** @defgroup USBD_CDC_IF_Exported_Macros
  * @{
  */ 
/* USER CODE BEGIN EXPORTED_MACRO  */
/* USER CODE END  EXPORTED_MACRO */

/**
  * @}
  */ 

/** @defgroup USBD_AUDIO_IF_Exported_Variables
  * @{
  */ 
//extern USBD_MIDI_ItfTypeDef  USBD_Interface_fops_FS;

/* USER CODE BEGIN EXPORTED_VARIABLES  */
/* USER CODE END  EXPORTED_VARIABLES */

/**
  * @}
  */ 

/** @defgroup USBD_MIDI_IF_Exported_FunctionsPrototype
  * @{
  */ 
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);

/* USER CODE BEGIN EXPORTED_FUNCTIONS  */
/* USER CODE END  EXPORTED_FUNCTIONS */
/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 
  
#ifdef __cplusplus
}
#endif
  
#endif /* __USBD_CDC_IF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
