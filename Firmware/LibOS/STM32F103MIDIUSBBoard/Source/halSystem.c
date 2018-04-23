/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "stm32f1xx_hal.h"
#include <mxconstants.h>
#include <sysTimer.h>
#include <usb_device.h>
#include <halMIDIPort.h>
#include <midiUSB.h>
#include <midiInput.h>
#include <midiOutput.h>

/*****************************************************************************/
/* Local function prototypes                                                 */
/*****************************************************************************/
static void halSystemClockConfig(void);
static void halGPIOInitialize(void);

extern PCD_HandleTypeDef hpcd_USB_FS;

///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes HW subsystem
void sysInitialization(void)
{
	// Init HAL
	HAL_Init();	
	
	halSystemTimerInitialize();
	
	// Configures GPIO pins
	halGPIOInitialize();

	// Configure the system clock
	halSystemClockConfig();

	// Start USB device stack
	midiUSBDeviceInitialize();
	//MX_USB_DEVICE_Init();
	
	// initializes hw MIDI serial port
	halMIDIPortInitialize();
	midiInputInitialize();
	midiOutputInitialize();

	// initialize app
	appInitialization();
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Configures system clock
static void halSystemClockConfig(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInit;

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
	
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
	PeriphClkInit.UsbClockSelection = RCC_USBPLLCLK_DIV1_5;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);	

}

///////////////////////////////////////////////////////////////////////////////
/// @briegf Configures GPIO pins
static void halGPIOInitialize(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
  /* GPIO Ports Clock Enable */
	__GPIOA_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();
	__GPIOD_CLK_ENABLE();
	
  /*Configure GPIO pin : LED_Pin */
	GPIO_InitStruct.Pin = CONNECT_LED_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(CONNECT_LED_GPIO_Port, &GPIO_InitStruct);	

	GPIO_InitStruct.Pin = TX_LED_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(TX_LED_GPIO_Port, &GPIO_InitStruct);	
	
	GPIO_InitStruct.Pin = RX_LED_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(RX_LED_GPIO_Port, &GPIO_InitStruct);	
}


///////////////////////////////////////////////////////////////////////////////
/// @brief Initializes the Global MSP.
void HAL_MspInit(void)
{
	__HAL_RCC_AFIO_CLK_ENABLE();

	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  // NOJTAG: JTAG-DP Disabled and SW-DP Enabled 
	__HAL_AFIO_REMAP_SWJ_NOJTAG();
}


/**
* @brief This function handles USB low priority or CAN RX0 interrupts.
*/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  /* USER CODE BEGIN USB_LP_CAN1_RX0_IRQn 0 */

  /* USER CODE END USB_LP_CAN1_RX0_IRQn 0 */
	HAL_PCD_IRQHandler(&hpcd_USB_FS);
	/* USER CODE BEGIN USB_LP_CAN1_RX0_IRQn 1 */

	  /* USER CODE END USB_LP_CAN1_RX0_IRQn 1 */
}
