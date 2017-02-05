/**
  ******************************************************************************
  * @file    usbd_midi.h
  * @author  MCD Application Team
  * @version V2.4.1
  * @date    19-June-2015
  * @brief   header file for the usbd_midi.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_MIDI_H
#define __USB_MIDI_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"
#include  "usbd_def.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */
  
/** @defgroup usbd_cdc
  * @brief This file is the Header file for usbd_midi.c
  * @{
  */ 


/** @defgroup usbd_midi_Exported_Defines
  * @{
  */ 
	 
// Configuration descriptor: bmAttributes
#define USB_DSC_CNFG_ATR_BASE           0x80    // D7: base attribute, always 1
#define USB_DSC_CNFG_ATR_SELF_POWERED   0x40    // D6: bus-powered: 0, self-powered: 1, both: 1
#define USB_DSC_CNFG_ATR_BUS_POWERED    0x00
#define USB_DSC_CNFG_ATR_REMOTEWAKEUP   0x20    // D5: remote-wakeup disabled: 0, enabled: 1

// Configuration descriptor: bMaxPower
#define USB_DSC_CNFG_MAXPOWER( x )      (((x) + 1) / 2)     // 1 unit = 2mA	 
	 
// allowed numbers: 1..8
#ifndef USB_MIDI_NUM_PORTS
#define USB_MIDI_NUM_PORTS 1
#endif
	 
// size of IN/OUT pipe (FS configuration)
#ifndef USB_MIDI_DATA_FS_IN_SIZE
#define USB_MIDI_DATA_FS_IN_SIZE           64
#endif
#ifndef USB_MIDI_DATA_FS_OUT_SIZE
#define USB_MIDI_DATA_FS_OUT_SIZE          64
#endif
// size of IN/OUT pipe (HS configuration)
#ifndef USB_MIDI_DATA_HS_IN_SIZE
#define USB_MIDI_DATA_HS_IN_SIZE           64
#endif
#ifndef USB_MIDI_DATA_HS_OUT_SIZE
#define USB_MIDI_DATA_HS_OUT_SIZE          64
#endif

// endpoint assignments (don't change!)
#define USB_MIDI_DATA_OUT_EP 0x01
#define USB_MIDI_DATA_IN_EP  0x81
	 
/**
  * @}
  */ 



/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */ 

	 typedef struct
	 {
		 int8_t(*Init)(void);
		 int8_t(*DeInit)(void);
		 int8_t(*Receive)(uint8_t *, uint32_t *);  

	 } USBD_MIDI_ItfTypeDef;
	 
	 typedef struct
	 {
		 uint8_t  *RxBuffer;  
		 uint8_t  *TxBuffer;   
		 uint32_t RxLength;
		 uint32_t TxLength;    
  
		 __IO uint32_t TxState;     
		 __IO uint32_t RxState;    
	 } USBD_MIDI_HandleTypeDef; 	 

	 extern USBD_ClassTypeDef  USBD_MIDI;
	 
	 /**
	   * @}
	   */
	 
/** @defgroup USB_CORE_Exported_Functions
  * @{
  */
	 uint8_t  USBD_MIDI_RegisterInterface(USBD_HandleTypeDef   *pdev, 
		 USBD_MIDI_ItfTypeDef *fops);

	 uint8_t  USBD_MIDI_SetTxBuffer(USBD_HandleTypeDef   *pdev,
		 uint8_t  *pbuff,
		 uint16_t length);

	 uint8_t  USBD_MIDI_SetRxBuffer(USBD_HandleTypeDef   *pdev,
		 uint8_t  *pbuff);
	 
	 uint8_t  USBD_MIDI_TransmitPacket(USBD_HandleTypeDef *pdev);

	 uint8_t  USBD_MIDI_ReceivePacket(USBD_HandleTypeDef *pdev);
	 
	 uint8_t  USBD_MIDI_IsTransmitterBusy(USBD_HandleTypeDef *pdev);
	 uint8_t USBD_MIDI_IsReceiverBusy(USBD_HandleTypeDef *pdev);
	 /**
  * @}
  */ 

#ifdef __cplusplus
}
#endif

#endif  /* __USB_MIDI_H */
/**
  * @}
  */ 

/**
  * @}
  */ 
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
