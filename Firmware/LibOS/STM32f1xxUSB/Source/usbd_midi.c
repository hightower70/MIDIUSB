/**
  ******************************************************************************
  * @file    USBD_MIDI.c
  * @author  MCD Application Team
  * @version V2.4.1
  * @date    19-June-2015
  * @brief   This file provides the high layer firmware functions to manage the 
  *          following functionalities of the USB CDC Class:
  *           - Initialization and Configuration of high and low layer
  *           - Enumeration as CDC Device (and enumeration for each implemented memory interface)
  *           - OUT/IN data transfer
  *           - Command IN transfer (class requests management)
  *           - Error management
  *           
  *  @verbatim
  *      
  *          ===================================================================      
  *                                CDC Class Driver Description
  *          =================================================================== 
  *           This driver manages the "Universal Serial Bus Class Definitions for Communications Devices
  *           Revision 1.2 November 16, 2007" and the sub-protocol specification of "Universal Serial Bus 
  *           Communications Class Subclass Specification for PSTN Devices Revision 1.2 February 9, 2007"
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Enumeration as CDC device with 2 data endpoints (IN and OUT) and 1 command endpoint (IN)
  *             - Requests management (as described in section 6.2 in specification)
  *             - Abstract Control Model compliant
  *             - Union Functional collection (using 1 IN endpoint for control)
  *             - Data interface class
  * 
  *           These aspects may be enriched or modified for a specific user application.
  *          
  *            This driver doesn't implement the following aspects of the specification 
  *            (but it is possible to manage these features with some modifications on this driver):
  *             - Any class-specific aspect relative to communication classes should be managed by user application.
  *             - All communication classes other than PSTN are not managed
  *      
  *  @endverbatim
  *                                  
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

/* Includes ------------------------------------------------------------------*/
#include "USBD_MIDI.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_MIDI 
  * @brief usbd core module
  * @{
  */ 

/** @defgroup USBD_MIDI_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup USBD_MIDI_Private_Defines
  * @{
  */ 

#define USB_LEN_MIDI_CLASS_IF_DESC 7
#define USB_LEN_MIDI_IN_JACK_DESC 6
#define USB_LEN_MIDI_OUT_JACK_DESC 9
#define USB_LEN_BULK_EP_DESC 9
#define USB_LEN_CLASS_EP_DESC 4

#define USB_MIDI_NUM_INTERFACES        2
#define USB_LEN_MIDI_CLASS_DESC (USB_LEN_MIDI_CLASS_IF_DESC+2*USB_MIDI_NUM_PORTS*USB_LEN_MIDI_IN_JACK_DESC+2*USB_MIDI_NUM_PORTS*USB_LEN_MIDI_OUT_JACK_DESC+USB_LEN_BULK_EP_DESC+(USB_LEN_CLASS_EP_DESC + USB_MIDI_NUM_PORTS)+USB_LEN_BULK_EP_DESC+(USB_LEN_CLASS_EP_DESC + USB_MIDI_NUM_PORTS))
#define USB_LEN_MIDI_CFG_DESC (USB_LEN_CFG_DESC+USB_LEN_IF_DESC+USB_LEN_IF_DESC+USB_LEN_MIDI_CLASS_DESC+USB_LEN_IF_DESC)

// MIDI Class-Specific Descriptor Types
#define CS_INTERFACE	0x24	// Class-specific type: Interface
#define CS_ENDPOINT	0x25	// Class-specific type: Endpoint

#define MS_HEADER 0x01	// Class-specific descriptor subtype

/**
  * @}
  */ 


/** @defgroup USBD_MIDI_Private_Macros
  * @{
  */ 

/**
  * @}
  */ 


/** @defgroup USBD_MIDI_Private_FunctionPrototypes
  * @{
  */


static uint8_t  USBD_MIDI_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx);

static uint8_t  USBD_MIDI_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx);

static uint8_t  USBD_MIDI_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req);

static uint8_t  USBD_MIDI_DataIn (USBD_HandleTypeDef *pdev, 
                                 uint8_t epnum);

static uint8_t  USBD_MIDI_DataOut (USBD_HandleTypeDef *pdev, 
                                 uint8_t epnum);

static uint8_t  USBD_MIDI_EP0_RxReady (USBD_HandleTypeDef *pdev);

static uint8_t  *USBD_MIDI_GetFSCfgDesc (uint16_t *length);

static uint8_t  *USBD_MIDI_GetHSCfgDesc (uint16_t *length);

static uint8_t  *USBD_MIDI_GetOtherSpeedCfgDesc (uint16_t *length);

static uint8_t  *USBD_MIDI_GetOtherSpeedCfgDesc (uint16_t *length);

uint8_t  *USBD_MIDI_GetDeviceQualifierDescriptor (uint16_t *length);

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_MIDI_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/**
  * @}
  */ 

/** @defgroup USBD_MIDI_Private_Variables
  * @{
  */ 


/* MIDI interface class callbacks structure */
USBD_ClassTypeDef  USBD_MIDI = 
{
  USBD_MIDI_Init,
  USBD_MIDI_DeInit,
  USBD_MIDI_Setup,
  NULL,                 /* EP0_TxSent, */
  USBD_MIDI_EP0_RxReady,
  USBD_MIDI_DataIn,
  USBD_MIDI_DataOut,
  NULL,
  NULL,
  NULL,     
  USBD_MIDI_GetHSCfgDesc,  
  USBD_MIDI_GetFSCfgDesc,    
  USBD_MIDI_GetOtherSpeedCfgDesc, 
  USBD_MIDI_GetDeviceQualifierDescriptor,
};

// This deviceDescrMIDI[] is based on 
// http://www.usb.org/developers/devclass_docs/midi10.pdf
// Appendix B. Example: Simple MIDI Adapter (Informative)

__ALIGN_BEGIN uint8_t USBD_MIDI_CfgFSDesc[USB_LEN_MIDI_CFG_DESC] __ALIGN_END =
{ 	
	// B.2 Configuration Descriptor
	USB_LEN_CFG_DESC,									// Descriptor length
	USB_DESC_TYPE_CONFIGURATION,			// Descriptor type
	LOBYTE(USB_LEN_MIDI_CFG_DESC),		// Total length
	HIBYTE(USB_LEN_MIDI_CFG_DESC),
	USB_MIDI_NUM_INTERFACES,					// Number of interfaces
	0x01,															// Configuration value
	0x00,															// Configuration string index
	USB_DSC_CNFG_ATR_BASE | USB_DSC_CNFG_ATR_SELF_POWERED | USB_DSC_CNFG_ATR_REMOTEWAKEUP, // Attributes
	USB_DSC_CNFG_MAXPOWER(100),				// Power requirement
	
	// B.3 AudioControl Interface Descriptors
	// The AudioControl interface describes the device structure (audio function topology) 
	// and is used to manipulate the Audio Controls. This device has no audio function 
	// incorporated. However, the AudioControl interface is mandatory and therefore both 
	// the standard AC interface descriptor and the classspecific AC interface descriptor 
	// must be present. The class-specific AC interface descriptor only contains the header 
	// descriptor.

	//////////////
	// Interface 0
	// -----------
	// B.3.1 Standard AC Interface Descriptor
	// The AudioControl interface has no dedicated endpoints associated with it. It uses the 
	// default pipe (endpoint 0) for all communication purposes. Class-specific AudioControl 
	// Requests are sent using the default pipe. There is no Status Interrupt endpoint provided.
  // Standard MIDI Streaming Interface Descriptor
  USB_LEN_IF_DESC,					// Descriptor length
  USB_DESC_TYPE_INTERFACE,	// Descriptor type
  0x00,											// Zero-based index of this interface
  0x00,											// Alternate setting
  0x00,											// Number of end points 
  0x01,											// Interface class  (AUDIO)
  0x01,											// Interface sub class  (AUDIO_CONTROL)
  0x00,											// Interface sub sub class
  0x00,											// Interface descriptor string index

	// B.3.2 Class-specific AC Interface Descriptor
	// The Class-specific AC interface descriptor is always headed by a Header descriptor 
	// that contains general information about the AudioControl interface. It contains all 
	// the pointers needed to describe the Audio Interface Collection, associated with the 
	// described audio function. Only the Header descriptor is present in this device 
	// because it does not contain any audio functionality as such.
  USB_LEN_IF_DESC,					// Descriptor length
  CS_INTERFACE,							// Descriptor type
  MS_HEADER,								// Header subtype
  0x00,											// Revision of class specification - 1.0 (LSB)
  0x01,											// Revision of class specification (MSB)
  LOBYTE(USB_LEN_IF_DESC),	// Total size of class-specific descriptors (LSB)
  HIBYTE(USB_LEN_IF_DESC),	// Total size of class-specific descriptors (MSB)
  0x01,											// Number of streaming interfaces
  0x01,											// MIDI Streaming Interface 1 belongs to this AudioControl Interface
	
	//////////////
	// Interface 1
	// -----------
	// B.4 MIDIStreaming Interface Descriptors

	// B.4.1 Standard MS Interface Descriptor	
  USB_LEN_IF_DESC,					// Descriptor length
  USB_DESC_TYPE_INTERFACE,	// Descriptor type
  0x01,											// Zero-based index of this interface
  0x00,											// Alternate setting
  0x02,											// Number of end points 
  0x01,											// Interface class  (AUDIO)
  0x03,											// Interface sub class  (MIDISTREAMING)
  0x00,											// Interface sub sub class
  0x00,											// Interface descriptor string index

	// B.4.2 Class-specific MS Interface Descriptor
	USB_LEN_MIDI_CLASS_IF_DESC,				// Descriptor length
	CS_INTERFACE,											// Descriptor type
	MS_HEADER,												// Header subtype
	0x00,															// Revision of class specification - 1.0 (LSB)
	0x01,															// Revision of class specification (MSB)
	LOBYTE(USB_LEN_MIDI_CLASS_DESC),	// Total size of class-specific descriptors (LSB)
	HIBYTE(USB_LEN_MIDI_CLASS_DESC),	// Total size of class-specific descriptors (MSB)

	// B.4.3 MIDI IN Jack Descriptor
	
#if USB_MIDI_NUM_PORTS >= 1
	// MIDI IN Jack Descriptor (Embedded)
	USB_LEN_MIDI_IN_JACK_DESC,	// Descriptor length
	CS_INTERFACE,								// Descriptor type (CS_INTERFACE)
	0x02,												// MIDI_IN_JACK subtype
	0x01,												// EMBEDDED
	0x01,												// ID of this jack
	0x00,												// unused

	// MIDI Adapter MIDI IN Jack Descriptor (External)
	USB_LEN_MIDI_IN_JACK_DESC,	// Descriptor length
	CS_INTERFACE,								// Descriptor type (CS_INTERFACE)
	0x02,												// MIDI_IN_JACK subtype
	0x02,												// EXTERNAL
	0x02,												// ID of this jack
	0x00,												// unused

	// MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	USB_LEN_MIDI_OUT_JACK_DESC,	// Descriptor length
	CS_INTERFACE,								// Descriptor type (CS_INTERFACE)
	0x03,												// MIDI_OUT_JACK subtype
	0x01,												// EMBEDDED
	0x03,												// ID of this jack
	0x01,												// number of input pins of this jack
	0x02,												// ID of the entity to which this pin is connected
	0x01,												// Output Pin number of the entity to which this input pin is connected
	0x00,												// unused

	// MIDI Adapter MIDI OUT Jack Descriptor (External)
	USB_LEN_MIDI_OUT_JACK_DESC,	// Descriptor length
	CS_INTERFACE,								// Descriptor type (CS_INTERFACE)
	0x03,												// MIDI_OUT_JACK subtype
	0x02,												// EXTERNAL
	0x04,												// ID of this jack
	0x01,												// number of input pins of this jack
	0x01,												// ID of the entity to which this pin is connected
	0x01,												// Output Pin number of the entity to which this input pin is connected
	0x00,												// unused
#endif


#if USB_MIDI_NUM_PORTS >= 2
	  // Second MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x05,				// ID of this jack
	0x00,				// unused

	  // Second MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x06,				// ID of this jack
	0x00,				// unused

	  // Second MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x07,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x06,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused

	  // Second MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x08,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x05,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused
#endif


#if USB_MIDI_NUM_PORTS >= 3
	  // Third MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x09,				// ID of this jack
	0x00,				// unused

	  // Third MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x0a,				// ID of this jack
	0x00,				// unused

	  // Third MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x0b,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x0a,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused

	  // Third MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x0c,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x09,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused
#endif


#if USB_MIDI_NUM_PORTS >= 4
	  // Fourth MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x0d,				// ID of this jack
	0x00,				// unused

	  // Fourth MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x0e,				// ID of this jack
	0x00,				// unused

	  // Fourth MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x0f,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x0e,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused

	  // Fourth MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x10,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x0d,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused
#endif


#if USB_MIDI_NUM_PORTS >= 5
	  // Fifth MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x11,				// ID of this jack
	0x00,				// unused

	  // Fifth MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x12,				// ID of this jack
	0x00,				// unused

	  // Fifth MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x13,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x12,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused

	  // Fifth MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x14,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x11,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused
#endif


#if USB_MIDI_NUM_PORTS >= 6
	  // Sixth MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x15,				// ID of this jack
	0x00,				// unused

	  // Sixth MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x16,				// ID of this jack
	0x00,				// unused

	  // Sixth MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x17,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x16,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused

	  // Sixth MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x18,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x15,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused
#endif


#if USB_MIDI_NUM_PORTS >= 7
	  // Seventh MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x19,				// ID of this jack
	0x00,				// unused

	  // Seventh MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x1a,				// ID of this jack
	0x00,				// unused

	  // Seventh MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x1b,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x1a,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused

	  // Seventh MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x1c,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x19,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused
#endif


#if USB_MIDI_NUM_PORTS >= 8
	  // Eighth MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x1d,				// ID of this jack
	0x00,				// unused

	  // Eighth MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x1e,				// ID of this jack
	0x00,				// unused

	  // Eighth MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x1f,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x1e,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused

	  // Eighth MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x20,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x1d,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused
#endif

	// B.5 Bulk OUT Endpoint Descriptors

	//B.5.1 Standard Bulk OUT Endpoint Descriptor
	USB_LEN_BULK_EP_DESC,				// Descriptor length
	USB_DESC_TYPE_ENDPOINT,			// Descriptor type
	USB_MIDI_DATA_OUT_EP,				// Out Endpoint 1
	0x02,												// Bulk, not shared
	LOBYTE(USB_MIDI_DATA_FS_OUT_SIZE),	// num of bytes per packet (LSB)
	HIBYTE(USB_MIDI_DATA_FS_OUT_SIZE),	// num of bytes per packet (MSB)
	0x00,												// ignore for bulk
	0x00,												// unused
	0x00,												// unused

	// B.5.2 Class-specific MS Bulk OUT Endpoint Descriptor
	USB_LEN_CLASS_EP_DESC + USB_MIDI_NUM_PORTS,			// Descriptor length
	CS_ENDPOINT,								// Descriptor type (CS_ENDPOINT)
	0x01,												// MS_GENERAL
	USB_MIDI_NUM_PORTS,					// number of embedded MIDI IN Jacks
	0x01,												// ID of embedded MIDI In Jack
#if USB_MIDI_NUM_PORTS >= 2
	0x05,												// ID of embedded MIDI In Jack
#endif
#if USB_MIDI_NUM_PORTS >= 3
	0x09,												// ID of embedded MIDI In Jack
#endif
#if USB_MIDI_NUM_PORTS >= 4
	0x0d,												// ID of embedded MIDI In Jack
#endif
#if USB_MIDI_NUM_PORTS >= 5
	0x11,												// ID of embedded MIDI In Jack
#endif
#if USB_MIDI_NUM_PORTS >= 6
	0x15,												// ID of embedded MIDI In Jack
#endif
#if USB_MIDI_NUM_PORTS >= 7
	0x19,												// ID of embedded MIDI In Jack
#endif
#if USB_MIDI_NUM_PORTS >= 8
	0x1d,												// ID of embedded MIDI In Jack
#endif

	//B.6 Bulk IN Endpoint Descriptors

	//B.6.1 Standard Bulk IN Endpoint Descriptor
	USB_LEN_BULK_EP_DESC,				// Descriptor length
	USB_DESC_TYPE_ENDPOINT,			// Descriptor type
	USB_MIDI_DATA_IN_EP,				// In Endpoint 1
	0x02,												// Bulk, not shared
	LOBYTE(USB_MIDI_DATA_FS_IN_SIZE),	// num of bytes per packet (LSB)
	HIBYTE(USB_MIDI_DATA_FS_IN_SIZE),	// num of bytes per packet (MSB)
	0x00,												// ignore for bulk
	0x00,												// unused
	0x00,												// unused

	// B.6.2 Class-specific MS Bulk IN Endpoint Descriptor
	USB_LEN_CLASS_EP_DESC + USB_MIDI_NUM_PORTS,	// Descriptor length
	CS_ENDPOINT,								// Descriptor type (CS_ENDPOINT)
	0x01,												// MS_GENERAL
	USB_MIDI_NUM_PORTS,					// number of embedded MIDI Out Jacks
	0x03,												// ID of embedded MIDI Out Jack
#if USB_MIDI_NUM_PORTS >= 2
	0x07,												// ID of embedded MIDI Out Jack
#endif
#if USB_MIDI_NUM_PORTS >= 3
	0x0b,												// ID of embedded MIDI Out Jack
#endif
#if USB_MIDI_NUM_PORTS >= 4
	0x0f,												// ID of embedded MIDI Out Jack
#endif
#if USB_MIDI_NUM_PORTS >= 5
	0x13,												// ID of embedded MIDI Out Jack
#endif
#if USB_MIDI_NUM_PORTS >= 6
	0x17,												// ID of embedded MIDI Out Jack
#endif
#if USB_MIDI_NUM_PORTS >= 7
	0x1b,												// ID of embedded MIDI Out Jack
#endif
#if USB_MIDI_NUM_PORTS >= 8
	0x1f,												// ID of embedded MIDI Out Jack
#endif

};

#if 0
/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN uint8_t USBD_MIDI_CfgHSDesc[USB_MIDI_CONFIG_DESC_SIZ] __ALIGN_END =
{ 	
  ///////////////////////////////////////////////////////////////////////////
  // USB MIDI
  ///////////////////////////////////////////////////////////////////////////

#if USB_MIDI_USE_AC_INTERFACE
  // Standard AC Interface Descriptor
	9,				// Descriptor length
	USB_DESC_TYPE_INTERFACE,			// Descriptor type
	USB_MIDI_AC_INTERFACE_IX, // Zero-based index of this interface
	0x00,				// Alternate setting
	0x00,				// Number of end points 
	0x01,				// Interface class  (AUDIO)
	0x01,				// Interface sub class  (AUDIO_CONTROL)
	0x00,				// Interface sub sub class
	0x05,				// Interface descriptor string index

	  // MIDI Adapter Class-specific AC Interface Descriptor
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type
	0x01,				// Header subtype
	0x00,				// Revision of class specification - 1.0 (LSB)
	0x01,				// Revision of class specification (MSB)
	9,				// Total size of class-specific descriptors (LSB)
	0,				// Total size of class-specific descriptors (MSB)
	0x01,				// Number of streaming interfaces
	0x01,				// MIDI Streaming Interface 1 belongs to this AudioControl Interface
#endif

	  // Standard MS Interface Descriptor
	9,				// Descriptor length
	USB_DESC_TYPE_INTERFACE,			// Descriptor type
	USB_MIDI_AS_INTERFACE_IX, // Zero-based index of this interface
	0x00,				// Alternate setting
	0x02,				// Number of end points 
	0x01,				// Interface class  (AUDIO)
	0x03,				// Interface sub class  (MIDISTREAMING)
	0x00,				// Interface sub sub class
	0x05,				// Interface descriptor string index

	  // Class-specific MS Interface Descriptor
	7,				// Descriptor length
	CS_INTERFACE,			// Descriptor type
#if USB_MIDI_USE_AC_INTERFACE
	0x01,				// Zero-based index of this interface
#else
	0x00,				// Zero-based index of this interface
#endif
	0x00,				// revision of this class specification (LSB)
	0x01,				// revision of this class specification (MSB)
	LOBYTE(USB_MIDI_SIZ_CLASS_DESC), // Total size of class-specific descriptors (LSB)
	HIBYTE(USB_MIDI_SIZ_CLASS_DESC),   // Total size of class-specific descriptors (MSB)


#if USB_MIDI_NUM_PORTS >= 1
	  // MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x01,				// ID of this jack
	0x06,				// unused

	  // MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x02,				// ID of this jack
	0x07,				// unused

	  // MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x03,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x02,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x08,				// unused

	  // MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x04,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x01,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x09,				// unused
#endif


#if USB_MIDI_NUM_PORTS >= 2
	  // Second MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x05,				// ID of this jack
	0x00,				// unused

	  // Second MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x06,				// ID of this jack
	0x00,				// unused

	  // Second MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x07,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x06,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused

	  // Second MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x08,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x05,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused
#endif


#if USB_MIDI_NUM_PORTS >= 3
	  // Third MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x09,				// ID of this jack
	0x00,				// unused

	  // Third MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x0a,				// ID of this jack
	0x00,				// unused

	  // Third MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x0b,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x0a,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused

	  // Third MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x0c,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x09,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused
#endif


#if USB_MIDI_NUM_PORTS >= 4
	  // Fourth MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x0d,				// ID of this jack
	0x00,				// unused

	  // Fourth MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x0e,				// ID of this jack
	0x00,				// unused

	  // Fourth MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x0f,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x0e,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused

	  // Fourth MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x10,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x0d,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused
#endif


#if USB_MIDI_NUM_PORTS >= 5
	  // Fifth MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x11,				// ID of this jack
	0x00,				// unused

	  // Fifth MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x12,				// ID of this jack
	0x00,				// unused

	  // Fifth MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x13,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x12,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused

	  // Fifth MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x14,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x11,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused
#endif


#if USB_MIDI_NUM_PORTS >= 6
	  // Sixth MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x15,				// ID of this jack
	0x00,				// unused

	  // Sixth MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x16,				// ID of this jack
	0x00,				// unused

	  // Sixth MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x17,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x16,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused

	  // Sixth MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x18,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x15,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused
#endif


#if USB_MIDI_NUM_PORTS >= 7
	  // Seventh MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x19,				// ID of this jack
	0x00,				// unused

	  // Seventh MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x1a,				// ID of this jack
	0x00,				// unused

	  // Seventh MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x1b,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x1a,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused

	  // Seventh MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x1c,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x19,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused
#endif


#if USB_MIDI_NUM_PORTS >= 8
	  // Eighth MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x1d,				// ID of this jack
	0x00,				// unused

	  // Eighth MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x1e,				// ID of this jack
	0x00,				// unused

	  // Eighth MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x1f,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x1e,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused

	  // Eighth MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x20,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x1d,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused
#endif


	  // Standard Bulk OUT Endpoint Descriptor
	9,				// Descriptor length
	USB_DESC_TYPE_ENDPOINT,			// Descriptor type
	0x02,				// Out Endpoint 2
	0x02,				// Bulk, not shared
	LOBYTE(USB_MIDI_DATA_HS_IN_SIZE),	// num of bytes per packet (LSB)
	HIBYTE(USB_MIDI_DATA_HS_IN_SIZE),	// num of bytes per packet (MSB)
	0x00,				// ignore for bulk
	0x00,				// unused
	0x00,				// unused

	  // Class-specific MS Bulk Out Endpoint Descriptor
	4 + USB_MIDI_NUM_PORTS,	// Descriptor length
	CS_ENDPOINT,			// Descriptor type (CS_ENDPOINT)
	0x01,				// MS_GENERAL
	USB_MIDI_NUM_PORTS,	// number of embedded MIDI IN Jacks
	0x01,				// ID of embedded MIDI In Jack
#if USB_MIDI_NUM_PORTS >= 2
	0x05,				// ID of embedded MIDI In Jack
#endif
#if USB_MIDI_NUM_PORTS >= 3
	0x09,				// ID of embedded MIDI In Jack
#endif
#if USB_MIDI_NUM_PORTS >= 4
	0x0d,				// ID of embedded MIDI In Jack
#endif
#if USB_MIDI_NUM_PORTS >= 5
	0x11,				// ID of embedded MIDI In Jack
#endif
#if USB_MIDI_NUM_PORTS >= 6
	0x15,				// ID of embedded MIDI In Jack
#endif
#if USB_MIDI_NUM_PORTS >= 7
	0x19,				// ID of embedded MIDI In Jack
#endif
#if USB_MIDI_NUM_PORTS >= 8
	0x1d,				// ID of embedded MIDI In Jack
#endif

	  // Standard Bulk IN Endpoint Descriptor
	9,				// Descriptor length
	USB_DESC_TYPE_ENDPOINT,			// Descriptor type
	USB_MIDI_DATA_IN_EP,	// In Endpoint 1
	0x02,				// Bulk, not shared
	LOBYTE(USB_MIDI_DATA_HS_OUT_SIZE),	// num of bytes per packet (LSB)
	HIBYTE(USB_MIDI_DATA_HS_OUT_SIZE),	// num of bytes per packet (MSB)
	0x00,				// ignore for bulk
	0x00,				// unused
	0x00,				// unused

	  // Class-specific MS Bulk In Endpoint Descriptor
	4 + USB_MIDI_NUM_PORTS,	// Descriptor length
	CS_ENDPOINT,			// Descriptor type (CS_ENDPOINT)
	0x01,				// MS_GENERAL
	USB_MIDI_NUM_PORTS,	// number of embedded MIDI Out Jacks
	0x03,				// ID of embedded MIDI Out Jack
#if USB_MIDI_NUM_PORTS >= 2
	0x07,				// ID of embedded MIDI Out Jack
#endif
#if USB_MIDI_NUM_PORTS >= 3
	0x0b,				// ID of embedded MIDI Out Jack
#endif
#if USB_MIDI_NUM_PORTS >= 4
	0x0f,				// ID of embedded MIDI Out Jack
#endif
#if USB_MIDI_NUM_PORTS >= 5
	0x13,				// ID of embedded MIDI Out Jack
#endif
#if USB_MIDI_NUM_PORTS >= 6
	0x17,				// ID of embedded MIDI Out Jack
#endif
#if USB_MIDI_NUM_PORTS >= 7
	0x1b,				// ID of embedded MIDI Out Jack
#endif
#if USB_MIDI_NUM_PORTS >= 8
	0x1f,				// ID of embedded MIDI Out Jack
#endif

};
#endif

#if 0
__ALIGN_BEGIN uint8_t USBD_MIDI_OtherSpeedCfgDesc[USB_MIDI_CONFIG_DESC_SIZ] __ALIGN_END =
{ 	
  ///////////////////////////////////////////////////////////////////////////
  // USB MIDI
  ///////////////////////////////////////////////////////////////////////////

#if USB_MIDI_USE_AC_INTERFACE
  // Standard AC Interface Descriptor
	9,				// Descriptor length
	USB_DESC_TYPE_INTERFACE,			// Descriptor type
	USB_MIDI_AC_INTERFACE_IX, // Zero-based index of this interface
	0x00,				// Alternate setting
	0x00,				// Number of end points 
	0x01,				// Interface class  (AUDIO)
	0x01,				// Interface sub class  (AUDIO_CONTROL)
	0x00,				// Interface sub sub class
	0x05,				// Interface descriptor string index

	  // MIDI Adapter Class-specific AC Interface Descriptor
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type
	0x01,				// Header subtype
	0x00,				// Revision of class specification - 1.0 (LSB)
	0x01,				// Revision of class specification (MSB)
	9,				// Total size of class-specific descriptors (LSB)
	0,				// Total size of class-specific descriptors (MSB)
	0x01,				// Number of streaming interfaces
	0x01,				// MIDI Streaming Interface 1 belongs to this AudioControl Interface
#endif

	  // Standard MS Interface Descriptor
	9,				// Descriptor length
	USB_DESC_TYPE_INTERFACE,			// Descriptor type
	USB_MIDI_AS_INTERFACE_IX, // Zero-based index of this interface
	0x00,				// Alternate setting
	0x02,				// Number of end points 
	0x01,				// Interface class  (AUDIO)
	0x03,				// Interface sub class  (MIDISTREAMING)
	0x00,				// Interface sub sub class
	0x05,				// Interface descriptor string index

	  // Class-specific MS Interface Descriptor
	7,				// Descriptor length
	CS_INTERFACE,			// Descriptor type
#if USB_MIDI_USE_AC_INTERFACE
	0x01,				// Zero-based index of this interface
#else
	0x00,				// Zero-based index of this interface
#endif
	0x00,				// revision of this class specification (LSB)
	0x01,				// revision of this class specification (MSB)
	LOBYTE(USB_MIDI_SIZ_CLASS_DESC), // Total size of class-specific descriptors (LSB)
	HIBYTE(USB_MIDI_SIZ_CLASS_DESC),   // Total size of class-specific descriptors (MSB)


#if USB_MIDI_NUM_PORTS >= 1
	  // MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x01,				// ID of this jack
	0x06,				// unused

	  // MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x02,				// ID of this jack
	0x07,				// unused

	  // MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x03,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x02,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x08,				// unused

	  // MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x04,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x01,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x09,				// unused
#endif


#if USB_MIDI_NUM_PORTS >= 2
	  // Second MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x05,				// ID of this jack
	0x00,				// unused

	  // Second MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x06,				// ID of this jack
	0x00,				// unused

	  // Second MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x07,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x06,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused

	  // Second MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x08,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x05,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused
#endif


#if USB_MIDI_NUM_PORTS >= 3
	  // Third MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x09,				// ID of this jack
	0x00,				// unused

	  // Third MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x0a,				// ID of this jack
	0x00,				// unused

	  // Third MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x0b,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x0a,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused

	  // Third MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x0c,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x09,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused
#endif


#if USB_MIDI_NUM_PORTS >= 4
	  // Fourth MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x0d,				// ID of this jack
	0x00,				// unused

	  // Fourth MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x0e,				// ID of this jack
	0x00,				// unused

	  // Fourth MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x0f,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x0e,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused

	  // Fourth MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x10,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x0d,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused
#endif


#if USB_MIDI_NUM_PORTS >= 5
	  // Fifth MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x11,				// ID of this jack
	0x00,				// unused

	  // Fifth MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x12,				// ID of this jack
	0x00,				// unused

	  // Fifth MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x13,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x12,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused

	  // Fifth MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x14,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x11,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused
#endif


#if USB_MIDI_NUM_PORTS >= 6
	  // Sixth MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x15,				// ID of this jack
	0x00,				// unused

	  // Sixth MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x16,				// ID of this jack
	0x00,				// unused

	  // Sixth MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x17,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x16,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused

	  // Sixth MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x18,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x15,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused
#endif


#if USB_MIDI_NUM_PORTS >= 7
	  // Seventh MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x19,				// ID of this jack
	0x00,				// unused

	  // Seventh MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x1a,				// ID of this jack
	0x00,				// unused

	  // Seventh MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x1b,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x1a,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused

	  // Seventh MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x1c,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x19,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused
#endif


#if USB_MIDI_NUM_PORTS >= 8
	  // Eighth MIDI IN Jack Descriptor (Embedded)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x01,				// EMBEDDED
	0x1d,				// ID of this jack
	0x00,				// unused

	  // Eighth MIDI Adapter MIDI IN Jack Descriptor (External)
	6,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x02,				// MIDI_IN_JACK subtype
	0x02,				// EXTERNAL
	0x1e,				// ID of this jack
	0x00,				// unused

	  // Eighth MIDI Adapter MIDI OUT Jack Descriptor (Embedded)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x01,				// EMBEDDED
	0x1f,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x1e,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused

	  // Eighth MIDI Adapter MIDI OUT Jack Descriptor (External)
	9,				// Descriptor length
	CS_INTERFACE,			// Descriptor type (CS_INTERFACE)
	0x03,				// MIDI_OUT_JACK subtype
	0x02,				// EXTERNAL
	0x20,				// ID of this jack
	0x01,				// number of input pins of this jack
	0x1d,				// ID of the entity to which this pin is connected
	0x01,				// Output Pin number of the entity to which this input pin is connected
	0x00,				// unused
#endif


	  // Standard Bulk OUT Endpoint Descriptor
	9,				// Descriptor length
	USB_DESC_TYPE_ENDPOINT,			// Descriptor type
	0x02,				// Out Endpoint 2
	0x02,				// Bulk, not shared
	0x40,				// num of bytes per packet (LSB)
	0x00,				// num of bytes per packet (MSB)
	0x00,				// ignore for bulk
	0x00,				// unused
	0x00,				// unused

	  // Class-specific MS Bulk Out Endpoint Descriptor
	4 + USB_MIDI_NUM_PORTS,	// Descriptor length
	CS_ENDPOINT,			// Descriptor type (CS_ENDPOINT)
	0x01,				// MS_GENERAL
	USB_MIDI_NUM_PORTS,	// number of embedded MIDI IN Jacks
	0x01,				// ID of embedded MIDI In Jack
#if USB_MIDI_NUM_PORTS >= 2
	0x05,				// ID of embedded MIDI In Jack
#endif
#if USB_MIDI_NUM_PORTS >= 3
	0x09,				// ID of embedded MIDI In Jack
#endif
#if USB_MIDI_NUM_PORTS >= 4
	0x0d,				// ID of embedded MIDI In Jack
#endif
#if USB_MIDI_NUM_PORTS >= 5
	0x11,				// ID of embedded MIDI In Jack
#endif
#if USB_MIDI_NUM_PORTS >= 6
	0x15,				// ID of embedded MIDI In Jack
#endif
#if USB_MIDI_NUM_PORTS >= 7
	0x19,				// ID of embedded MIDI In Jack
#endif
#if USB_MIDI_NUM_PORTS >= 8
	0x1d,				// ID of embedded MIDI In Jack
#endif

	  // Standard Bulk IN Endpoint Descriptor
	9,				// Descriptor length
	USB_DESC_TYPE_ENDPOINT,			// Descriptor type
	USB_MIDI_DATA_IN_EP,	// In Endpoint 1
	0x02,				// Bulk, not shared
	0x40,				// num of bytes per packet (LSB)
	0x00,				// num of bytes per packet (MSB)
	0x00,				// ignore for bulk
	0x00,				// unused
	0x00,				// unused

	  // Class-specific MS Bulk In Endpoint Descriptor
	4 + USB_MIDI_NUM_PORTS,	// Descriptor length
	CS_ENDPOINT,			// Descriptor type (CS_ENDPOINT)
	0x01,				// MS_GENERAL
	USB_MIDI_NUM_PORTS,	// number of embedded MIDI Out Jacks
	0x03,				// ID of embedded MIDI Out Jack
#if USB_MIDI_NUM_PORTS >= 2
	0x07,				// ID of embedded MIDI Out Jack
#endif
#if USB_MIDI_NUM_PORTS >= 3
	0x0b,				// ID of embedded MIDI Out Jack
#endif
#if USB_MIDI_NUM_PORTS >= 4
	0x0f,				// ID of embedded MIDI Out Jack
#endif
#if USB_MIDI_NUM_PORTS >= 5
	0x13,				// ID of embedded MIDI Out Jack
#endif
#if USB_MIDI_NUM_PORTS >= 6
	0x17,				// ID of embedded MIDI Out Jack
#endif
#if USB_MIDI_NUM_PORTS >= 7
	0x1b,				// ID of embedded MIDI Out Jack
#endif
#if USB_MIDI_NUM_PORTS >= 8
	0x1f,				// ID of embedded MIDI Out Jack
#endif

};
#endif

#if 0

{
  /*Configuration Descriptor*/
  0x09,   /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,      /* bDescriptorType: Configuration */
  USB_CDC_CONFIG_DESC_SIZ,                /* wTotalLength:no of returned bytes */
  0x00,
  0x02,   /* bNumInterfaces: 2 interface */
  0x01,   /* bConfigurationValue: Configuration value */
  0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,   /* bmAttributes: self powered */
  0x32,   /* MaxPower 0 mA */
  
  /*---------------------------------------------------------------------------*/
  
  /*Interface Descriptor */
  0x09,   /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0x00,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x01,   /* bNumEndpoints: One endpoints used */
  0x02,   /* bInterfaceClass: Communication Interface Class */
  0x02,   /* bInterfaceSubClass: Abstract Control Model */
  0x01,   /* bInterfaceProtocol: Common AT commands */
  0x00,   /* iInterface: */
  
  /*Header Functional Descriptor*/
  0x05,   /* bLength: Endpoint Descriptor size */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x00,   /* bDescriptorSubtype: Header Func Desc */
  0x10,   /* bcdCDC: spec release number */
  0x01,
  
  /*Call Management Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x01,   /* bDescriptorSubtype: Call Management Func Desc */
  0x00,   /* bmCapabilities: D0+D1 */
  0x01,   /* bDataInterface: 1 */
  
  /*ACM Functional Descriptor*/
  0x04,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,   /* bmCapabilities */
  
  /*Union Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x06,   /* bDescriptorSubtype: Union func desc */
  0x00,   /* bMasterInterface: Communication class interface */
  0x01,   /* bSlaveInterface0: Data Class Interface */
  
  /*Endpoint 2 Descriptor*/
  0x07,                           /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,   /* bDescriptorType: Endpoint */
  CDC_CMD_EP,                     /* bEndpointAddress */
  0x03,                           /* bmAttributes: Interrupt */
  LOBYTE(CDC_CMD_PACKET_SIZE),     /* wMaxPacketSize: */
  HIBYTE(CDC_CMD_PACKET_SIZE),
  0x10,                           /* bInterval: */ 
  /*---------------------------------------------------------------------------*/
  
  /*Data class interface descriptor*/
  0x09,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: */
  0x01,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x02,   /* bNumEndpoints: Two endpoints used */
  0x0A,   /* bInterfaceClass: CDC */
  0x00,   /* bInterfaceSubClass: */
  0x00,   /* bInterfaceProtocol: */
  0x00,   /* iInterface: */
  
  /*Endpoint OUT Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
  CDC_OUT_EP,                        /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),
  0x00,                              /* bInterval: ignore for Bulk transfer */
  
  /*Endpoint IN Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
  CDC_IN_EP,                         /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_HS_MAX_PACKET_SIZE),
  0x00                               /* bInterval: ignore for Bulk transfer */
} ;


/* USB CDC device Configuration Descriptor */
__ALIGN_BEGIN uint8_t USBD_MIDI_CfgFSDesc[USB_CDC_CONFIG_DESC_SIZ] __ALIGN_END =
{
  /*Configuration Descriptor*/
  0x09,   /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,      /* bDescriptorType: Configuration */
  USB_CDC_CONFIG_DESC_SIZ,                /* wTotalLength:no of returned bytes */
  0x00,
  0x02,   /* bNumInterfaces: 2 interface */
  0x01,   /* bConfigurationValue: Configuration value */
  0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,   /* bmAttributes: self powered */
  0x32,   /* MaxPower 0 mA */
  
  /*---------------------------------------------------------------------------*/
  
  /*Interface Descriptor */
  0x09,   /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0x00,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x01,   /* bNumEndpoints: One endpoints used */
  0x02,   /* bInterfaceClass: Communication Interface Class */
  0x02,   /* bInterfaceSubClass: Abstract Control Model */
  0x01,   /* bInterfaceProtocol: Common AT commands */
  0x00,   /* iInterface: */
  
  /*Header Functional Descriptor*/
  0x05,   /* bLength: Endpoint Descriptor size */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x00,   /* bDescriptorSubtype: Header Func Desc */
  0x10,   /* bcdCDC: spec release number */
  0x01,
  
  /*Call Management Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x01,   /* bDescriptorSubtype: Call Management Func Desc */
  0x00,   /* bmCapabilities: D0+D1 */
  0x01,   /* bDataInterface: 1 */
  
  /*ACM Functional Descriptor*/
  0x04,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,   /* bmCapabilities */
  
  /*Union Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x06,   /* bDescriptorSubtype: Union func desc */
  0x00,   /* bMasterInterface: Communication class interface */
  0x01,   /* bSlaveInterface0: Data Class Interface */
  
  /*Endpoint 2 Descriptor*/
  0x07,                           /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,   /* bDescriptorType: Endpoint */
  CDC_CMD_EP,                     /* bEndpointAddress */
  0x03,                           /* bmAttributes: Interrupt */
  LOBYTE(CDC_CMD_PACKET_SIZE),     /* wMaxPacketSize: */
  HIBYTE(CDC_CMD_PACKET_SIZE),
  0x10,                           /* bInterval: */ 
  /*---------------------------------------------------------------------------*/
  
  /*Data class interface descriptor*/
  0x09,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: */
  0x01,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x02,   /* bNumEndpoints: Two endpoints used */
  0x0A,   /* bInterfaceClass: CDC */
  0x00,   /* bInterfaceSubClass: */
  0x00,   /* bInterfaceProtocol: */
  0x00,   /* iInterface: */
  
  /*Endpoint OUT Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
  CDC_OUT_EP,                        /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),
  0x00,                              /* bInterval: ignore for Bulk transfer */
  
  /*Endpoint IN Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
  CDC_IN_EP,                         /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),  /* wMaxPacketSize: */
  HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),
  0x00                               /* bInterval: ignore for Bulk transfer */
} ;

__ALIGN_BEGIN uint8_t USBD_MIDI_OtherSpeedCfgDesc[USB_CDC_CONFIG_DESC_SIZ] __ALIGN_END =
{ 
  0x09,   /* bLength: Configuation Descriptor size */
  USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION,   
  USB_CDC_CONFIG_DESC_SIZ,
  0x00,
  0x02,   /* bNumInterfaces: 2 interfaces */
  0x01,   /* bConfigurationValue: */
  0x04,   /* iConfiguration: */
  0xC0,   /* bmAttributes: */
  0x32,   /* MaxPower 100 mA */  
  
  /*Interface Descriptor */
  0x09,   /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0x00,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x01,   /* bNumEndpoints: One endpoints used */
  0x02,   /* bInterfaceClass: Communication Interface Class */
  0x02,   /* bInterfaceSubClass: Abstract Control Model */
  0x01,   /* bInterfaceProtocol: Common AT commands */
  0x00,   /* iInterface: */
  
  /*Header Functional Descriptor*/
  0x05,   /* bLength: Endpoint Descriptor size */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x00,   /* bDescriptorSubtype: Header Func Desc */
  0x10,   /* bcdCDC: spec release number */
  0x01,
  
  /*Call Management Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x01,   /* bDescriptorSubtype: Call Management Func Desc */
  0x00,   /* bmCapabilities: D0+D1 */
  0x01,   /* bDataInterface: 1 */
  
  /*ACM Functional Descriptor*/
  0x04,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x02,   /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,   /* bmCapabilities */
  
  /*Union Functional Descriptor*/
  0x05,   /* bFunctionLength */
  0x24,   /* bDescriptorType: CS_INTERFACE */
  0x06,   /* bDescriptorSubtype: Union func desc */
  0x00,   /* bMasterInterface: Communication class interface */
  0x01,   /* bSlaveInterface0: Data Class Interface */
  
  /*Endpoint 2 Descriptor*/
  0x07,                           /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT      ,   /* bDescriptorType: Endpoint */
  CDC_CMD_EP,                     /* bEndpointAddress */
  0x03,                           /* bmAttributes: Interrupt */
  LOBYTE(CDC_CMD_PACKET_SIZE),     /* wMaxPacketSize: */
  HIBYTE(CDC_CMD_PACKET_SIZE),
  0xFF,                           /* bInterval: */
  
  /*---------------------------------------------------------------------------*/
  
  /*Data class interface descriptor*/
  0x09,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: */
  0x01,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x02,   /* bNumEndpoints: Two endpoints used */
  0x0A,   /* bInterfaceClass: CDC */
  0x00,   /* bInterfaceSubClass: */
  0x00,   /* bInterfaceProtocol: */
  0x00,   /* iInterface: */
  
  /*Endpoint OUT Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,      /* bDescriptorType: Endpoint */
  CDC_OUT_EP,                        /* bEndpointAddress */
  0x02,                              /* bmAttributes: Bulk */
  0x40,                              /* wMaxPacketSize: */
  0x00,
  0x00,                              /* bInterval: ignore for Bulk transfer */
  
  /*Endpoint IN Descriptor*/
  0x07,   /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,     /* bDescriptorType: Endpoint */
  CDC_IN_EP,                        /* bEndpointAddress */
  0x02,                             /* bmAttributes: Bulk */
  0x40,                             /* wMaxPacketSize: */
  0x00,
  0x00                              /* bInterval */
};

#endif
/**
  * @}
  */ 

/** @defgroup USBD_MIDI_Private_Functions
  * @{
  */ 

/**
  * @brief  USBD_MIDI_Init
  *         Initialize the MIDI interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_MIDI_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx)
{
  uint8_t ret = 0;
  USBD_MIDI_HandleTypeDef   *hcdc;
	
	if (pdev->dev_speed == USBD_SPEED_HIGH) 
	{  
    /* Open EP IN */
		USBD_LL_OpenEP(pdev,
			USB_MIDI_DATA_IN_EP,
			USBD_EP_TYPE_BULK,
			USB_MIDI_DATA_HS_IN_SIZE);
    
		/* Open EP OUT */
		USBD_LL_OpenEP(pdev,
			USB_MIDI_DATA_OUT_EP,
			USBD_EP_TYPE_BULK,
			USB_MIDI_DATA_HS_OUT_SIZE);
	}
	else
	{
    /* Open EP IN */
		USBD_LL_OpenEP(pdev,
			USB_MIDI_DATA_IN_EP,
			USBD_EP_TYPE_BULK,
			USB_MIDI_DATA_FS_IN_SIZE);
    
		/* Open EP OUT */
		USBD_LL_OpenEP(pdev,
			USB_MIDI_DATA_OUT_EP,
			USBD_EP_TYPE_BULK,
			USB_MIDI_DATA_FS_OUT_SIZE);
	}
	
  pdev->pClassData = USBD_malloc(sizeof (USBD_MIDI_HandleTypeDef));
  
  if(pdev->pClassData == NULL)
  {
    ret = 1; 
  }
  else
  {
    hcdc = (USBD_MIDI_HandleTypeDef*) pdev->pClassData;
    
    /* Init  physical Interface components */
    ((USBD_MIDI_ItfTypeDef*)pdev->pUserData)->Init();
    
    /* Init Xfer states */
    hcdc->TxState =0;
    hcdc->RxState =0;
       
	  if (pdev->dev_speed == USBD_SPEED_HIGH) 
	  {      
	    /* Prepare Out endpoint to receive next packet */
		  USBD_LL_PrepareReceive(pdev,
			  USB_MIDI_DATA_OUT_EP,
			  hcdc->RxBuffer,
			  USB_MIDI_DATA_HS_OUT_SIZE);
	  }
	  else
	  {
	    /* Prepare Out endpoint to receive next packet */
		  USBD_LL_PrepareReceive(pdev,
			  USB_MIDI_DATA_OUT_EP,
			  hcdc->RxBuffer,
			  USB_MIDI_DATA_FS_OUT_SIZE);
	  }
  }
	
  return ret;
}

/**
  * @brief  USBD_MIDI_Init
  *         DeInitialize the MIDI layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_MIDI_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx)
{
  uint8_t ret = 0;
  
  /* Open EP IN */
	USBD_LL_CloseEP(pdev, USB_MIDI_DATA_IN_EP);
  
  /* Open EP OUT */
	USBD_LL_CloseEP(pdev, USB_MIDI_DATA_OUT_EP);
  
  /* DeInit  physical Interface components */
  if(pdev->pClassData != NULL)
  {
    ((USBD_MIDI_ItfTypeDef *)pdev->pUserData)->DeInit();
    USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }
  
  return ret;
}

/**
  * @brief  USBD_MIDI_Setup
  *         Handle the CDC specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_MIDI_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req)
{
	// not relevant for USB MIDI
  return USBD_OK;
}

/**
  * @brief  USBD_MIDI_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_MIDI_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_MIDI_HandleTypeDef   *hcdc = (USBD_MIDI_HandleTypeDef*) pdev->pClassData;
  
  if(pdev->pClassData != NULL)
  {
    
    hcdc->TxState = 0;

    return USBD_OK;
  }
  else
  {
    return USBD_FAIL;
  }
}

/**
  * @brief  USBD_MIDI_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_MIDI_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum)
{      
  USBD_MIDI_HandleTypeDef   *hmidi = (USBD_MIDI_HandleTypeDef*) pdev->pClassData;
  
  /* Get the received data length */
  hmidi->RxLength = USBD_LL_GetRxDataSize (pdev, epnum);
  hmidi->RxState = 0;
  
  /* USB data will be immediately processed, this allow next USB traffic being 
  NAKed till the end of the application Xfer */
  if(pdev->pClassData != NULL)
  {
    ((USBD_MIDI_ItfTypeDef *)pdev->pUserData)->Receive(hmidi->RxBuffer, &hmidi->RxLength);

    return USBD_OK;
  }
  else
  {
    return USBD_FAIL;
  }
}

/**
  * @brief  USBD_MIDI_DataOut
  *         Data received on control endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_MIDI_EP0_RxReady (USBD_HandleTypeDef *pdev)
{ 
	// not relevant for USB MIDI
  return USBD_OK;
}

/**
  * @brief  USBD_MIDI_GetFSCfgDesc 
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_MIDI_GetFSCfgDesc (uint16_t *length)
{
  *length = sizeof (USBD_MIDI_CfgFSDesc);
	return USBD_MIDI_CfgFSDesc;
}

/**
  * @brief  USBD_MIDI_GetHSCfgDesc 
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_MIDI_GetHSCfgDesc (uint16_t *length)
{
  //*length = sizeof (USBD_MIDI_CfgHSDesc);
  //return USBD_MIDI_CfgHSDesc;
	*length = sizeof(USBD_MIDI_CfgFSDesc);
	return USBD_MIDI_CfgFSDesc;
}

/**
  * @brief  USBD_MIDI_GetCfgDesc 
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_MIDI_GetOtherSpeedCfgDesc (uint16_t *length)
{
  //*length = sizeof (USBD_MIDI_OtherSpeedCfgDesc);
  //return USBD_MIDI_OtherSpeedCfgDesc;
	*length = sizeof(USBD_MIDI_CfgFSDesc);
	return USBD_MIDI_CfgFSDesc;
}

/**
* @brief  DeviceQualifierDescriptor 
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
uint8_t  *USBD_MIDI_GetDeviceQualifierDescriptor (uint16_t *length)
{
  *length = sizeof (USBD_MIDI_DeviceQualifierDesc);
  return USBD_MIDI_DeviceQualifierDesc;
}

/**
* @brief  USBD_MIDI_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: CD  Interface callback
  * @retval status
  */
uint8_t  USBD_MIDI_RegisterInterface  (USBD_HandleTypeDef   *pdev, 
                                      USBD_MIDI_ItfTypeDef *fops)
{
  uint8_t  ret = USBD_FAIL;
  
  if(fops != NULL)
  {
    pdev->pUserData= fops;
    ret = USBD_OK;    
  }
  
  return ret;
}

/**
  * @brief  USBD_MIDI_SetTxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Tx Buffer
  * @retval status
  */
uint8_t  USBD_MIDI_SetTxBuffer  (USBD_HandleTypeDef   *pdev,
                                uint8_t  *pbuff,
                                uint16_t length)
{
  USBD_MIDI_HandleTypeDef   *hcdc = (USBD_MIDI_HandleTypeDef*) pdev->pClassData;
  
	if (hcdc == NULL)
		return USBD_FAIL;
	
  hcdc->TxBuffer = pbuff;
  hcdc->TxLength = length;  
  
  return USBD_OK;  
}


/**
  * @brief  USBD_MIDI_SetRxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Rx Buffer
  * @retval status
  */
uint8_t  USBD_MIDI_SetRxBuffer(USBD_HandleTypeDef   *pdev,
	uint8_t  *pbuff)
{
	USBD_MIDI_HandleTypeDef   *hcdc = (USBD_MIDI_HandleTypeDef*) pdev->pClassData;
  
	if (hcdc == NULL)
		return USBD_FAIL;

	hcdc->RxBuffer = pbuff;
  
	return USBD_OK;
}

/**
  * @brief  USBD_MIDI_IsTransmitterBusy
  *         Return true when transmitter (USB IN) is busy
  * @param  pdev: device instance
  * @retval busy status
  */
uint8_t  USBD_MIDI_IsTransmitterBusy(USBD_HandleTypeDef *pdev)
{
	USBD_MIDI_HandleTypeDef   *hcdc = (USBD_MIDI_HandleTypeDef*) pdev->pClassData;

	if (pdev->pClassData != NULL)
	{
		return (hcdc->TxState != 0);
	}
	else
	{
		return 1;
	}
}

/**
  * @brief  USBD_MIDI_IsReceiverBusy
  *         Checks USB receiver (OUT endpoint) status
  * @param  pdev: device instance
  * @retval true if receiver is waiting for packet, false when receiver is not active endpoint sends NAK
  */
uint8_t USBD_MIDI_IsReceiverBusy(USBD_HandleTypeDef *pdev)
{
  return (PCD_GET_EP_RX_STATUS((PCD_TypeDef*)((PCD_HandleTypeDef*)(pdev->pData))->Instance, USB_MIDI_DATA_OUT_EP) == USB_EP_RX_VALID);
}

/**
  * @brief  USBD_MIDI_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
uint8_t  USBD_MIDI_TransmitPacket(USBD_HandleTypeDef *pdev)
{      
	USBD_MIDI_HandleTypeDef   *hmidi = (USBD_MIDI_HandleTypeDef*) pdev->pClassData;
  
	if (pdev->pClassData != NULL)
	{
		if (hmidi->TxState == 0)
		{
		  /* Tx Transfer in progress */
			hmidi->TxState = 1;
      
			/* Transmit next packet */
			USBD_LL_Transmit(pdev,
				USB_MIDI_DATA_IN_EP,
				hmidi->TxBuffer,
				hmidi->TxLength);
      
			return USBD_OK;
		}
		else
		{
			return USBD_BUSY;
		}
	}
	else
	{
		return USBD_FAIL;
	}
}


/**
  * @brief  USBD_MIDI_ReceivePacket
  *         prepare OUT Endpoint for reception
  * @param  pdev: device instance
  * @retval status
  */
	  uint8_t  USBD_MIDI_ReceivePacket(USBD_HandleTypeDef *pdev)
	  {      
		  USBD_MIDI_HandleTypeDef   *hmidi = (USBD_MIDI_HandleTypeDef*) pdev->pClassData;
  
		  /* Suspend or Resume USB Out process */
		  if (hmidi != NULL)
		  {
		  	hmidi->RxState = 1;
			  if (pdev->dev_speed == USBD_SPEED_HIGH) 
			  {      
			    /* Prepare Out endpoint to receive next packet */
				  USBD_LL_PrepareReceive(pdev,
					  USB_MIDI_DATA_OUT_EP,
					  hmidi->RxBuffer,
					  USB_MIDI_DATA_HS_OUT_SIZE);
			  }
			  else
			  {
			    /* Prepare Out endpoint to receive next packet */
				  USBD_LL_PrepareReceive(pdev,
					  USB_MIDI_DATA_OUT_EP,
					  hmidi->RxBuffer,
					  USB_MIDI_DATA_FS_OUT_SIZE);
			  }
			  return USBD_OK;
		  }
		  else
		  {
			  return USBD_FAIL;
		  }
	  }
/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
