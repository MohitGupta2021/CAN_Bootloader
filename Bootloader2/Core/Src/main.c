/* USER CODE BEGIN Header */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
CAN_FilterTypeDef canfil; //CAN Bus Filter
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_BLOCK_SIZE          ( 1024 )                  //1KB
#define ETX_APP_START_ADDRESS   0x08004400
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_CAN_Init(void);
/* USER CODE BEGIN PFP */
#define MY_NAME 0x88U //
#define LOADER_NAME 0x88U // SENDER FROM

#define FLASH_START_ADDRESS ETX_APP_START_ADDRESS
#define PAGE_SIZE 1024U
#define BOOTLOADER_PAGES_NUMBER 8U
#define START_PAGE_ADRESS (FLASH_START_ADDRESS + BOOTLOADER_PAGES_NUMBER * PAGE_SIZE)
#define FLASH_PAGES_NUMBER 64U

enum commands {
	nothing, jump, erase, flash, test, loader
};

#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
 set to 'Yes') calls __io_putchar() */
int __io_putchar(int ch)
#else
int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the UART3 and Loop until the end of transmission */
	HAL_UART_Transmit(&huart1, (uint8_t*) &ch, 1, HAL_MAX_DELAY);

	return ch;
}
static void goto_application(void) {
	printf("Gonna Jump to Application...\n");
	void (*app_reset_handler)(
			void) = (void*)(*((volatile uint32_t*)(ETX_APP_START_ADDRESS + 4U)));

	if (app_reset_handler == (void*) 0xFFFFFFFF) {
		printf("Invalid Application... HALT!!!\r\n");
		while (1)
			;
	}

	__set_MSP(*(volatile uint32_t*) ETX_APP_START_ADDRESS);
	app_reset_handler();    //call the app reset handler
}

void CAN_Filter_Config(CAN_FilterTypeDef *canfil) {
	/* USER CODE BEGIN 2 */
	canfil->FilterBank = 0;
	canfil->FilterMode = CAN_FILTERMODE_IDMASK;
	canfil->FilterFIFOAssignment = CAN_RX_FIFO0;
	canfil->FilterIdHigh = 0;
	canfil->FilterIdLow = 0;
	canfil->FilterMaskIdHigh = 0;
	canfil->FilterMaskIdLow = 0;
	canfil->FilterScale = CAN_FILTERSCALE_32BIT;
	canfil->FilterActivation = ENABLE;
	canfil->SlaveStartFilterBank = 14;
}

uint32_t CAN_FLASH_PROGRAM(uint32_t byteAddress, uint8_t data[8]) {
	//
	//We have to unlock flash module to get control of registers
	HAL_FLASH_Unlock();
	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, byteAddress,
			*((uint64_t*) data)) == HAL_OK) {
		byteAddress += 8U; // 8 bo ładujemy od razu po dwa słowa - tyle mieści się w ramce cana
		// tutaj powinno się jeszcze znaleźć sprawdzanie czy nie piszemy po pamięci
	} else {
		Error_Handler();
	}
	// LOCK THE MEMORY FOR NOT BEEN CORRUPT
	HAL_FLASH_Lock();
	return byteAddress;
}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */
	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */
	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART1_UART_Init();
	MX_CAN_Init();
	/* USER CODE BEGIN 2 */

	CAN_Filter_Config(&canfil);
	// blue pill led pin (PC13) is used to indicate that loader is working, not necessary

	HAL_FLASH_Unlock();

	HAL_Delay(2000U);
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */

	CAN_RxHeaderTypeDef rxHeader;
	CAN_TxHeaderTypeDef txHeader;
	uint8_t data[8];
	uint32_t _; // na śmieci
	uint32_t byteAddress = FLASH_START_ADDRESS;


	txHeader.StdId = 11;
	txHeader.RTR = 1;
	txHeader.IDE = 0;

//	CAN1->MCR ^= CAN_MCR_INRQ;
	HAL_CAN_ConfigFilter(&hcan, &canfil);
	HAL_CAN_Start(&hcan);
	HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);

	txHeader.StdId = LOADER_NAME;
	txHeader.DLC = 0U;
	txHeader.IDE = 0U;
	HAL_CAN_AddTxMessage(&hcan, &txHeader, data, &_);
	printf("normal test start \n");
	// waiting for answer
	HAL_Delay(1000U);

	if (((HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &rxHeader, data) == HAL_OK)
			&& rxHeader.StdId == ((MY_NAME << (11U - 8U)) | loader))
			|| (*(uint32_t*) (START_PAGE_ADRESS) != 0x20005000U)) {
		while (1) {
			if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) > 0) {

				if (HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &rxHeader, data)
						== HAL_OK) {
//			 FOR  JUMP TO THE APPLICATION CODE
					if (rxHeader.StdId == ((MY_NAME << (11U - 8U)) | jump)) {
//						jumpToApp();
						printf("jump to app \n");
						goto_application();

					}

//	 ERASE THE CODE
					if (rxHeader.StdId == ((MY_NAME << (11U - 8U)) | erase)) {

						printf("erase is doing \n");
						printf("Erasing the Flash memory...\r\n");
						HAL_StatusTypeDef ret;

						FLASH_EraseInitTypeDef EraseInitStruct;
						uint32_t SectorError;

						EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
						EraseInitStruct.PageAddress = ETX_APP_START_ADDRESS;
						EraseInitStruct.NbPages = 47;                 //47 Pages
						HAL_FLASH_Unlock();
						ret = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
						if (ret != HAL_OK) {
							break;
						}
//
						HAL_FLASH_Lock();
					}

//				RECEIVING THE CAN ID FROM ALONG WITH THE DATA FROM THE PCAN USB
					if (rxHeader.StdId == ((MY_NAME << (11U - 8U)) | flash)) {
						printf("FLASING is doing \n");
						byteAddress = CAN_FLASH_PROGRAM(byteAddress, data);
					}

//					 TESTING THE CAN ID WHETHER IT IS WORKING OR NOT

					if (rxHeader.StdId == ((MY_NAME << (11U - 8U)) | test)) {
						printf("testing is doing \n");
						HAL_CAN_AddTxMessage(&hcan, &txHeader, data, &_);
					}

				} else
					printf("normal test end \n");

				HAL_CAN_AddTxMessage(&hcan, &txHeader, data, &_);
			}
			/* USER CODE END WHILE */

			/* USER CODE BEGIN 3 */
		}
	}
	else {
		printf("jump to app last");

		goto_application();
	}
//
	/* USER CODE END 2 */

	/* USER CODE BEGIN 3 */
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief CAN Initialization Function
 * @param None
 * @retval None
 */
static void MX_CAN_Init(void) {

	/* USER CODE BEGIN CAN_Init 0 */
	/* USER CODE END CAN_Init 0 */

	/* USER CODE BEGIN CAN_Init 1 */
	/* USER CODE END CAN_Init 1 */
	hcan.Instance = CAN1;
	hcan.Init.Prescaler = 4;
	hcan.Init.Mode = CAN_MODE_NORMAL;
	hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
	hcan.Init.TimeSeg1 = CAN_BS1_2TQ;
	hcan.Init.TimeSeg2 = CAN_BS2_1TQ;
	hcan.Init.TimeTriggeredMode = DISABLE;
	hcan.Init.AutoBusOff = DISABLE;
	hcan.Init.AutoWakeUp = DISABLE;
	hcan.Init.AutoRetransmission = DISABLE;
	hcan.Init.ReceiveFifoLocked = DISABLE;
	hcan.Init.TransmitFifoPriority = DISABLE;
	if (HAL_CAN_Init(&hcan) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN CAN_Init 2 */
	/* USER CODE END CAN_Init 2 */

}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void) {

	/* USER CODE BEGIN USART1_Init 0 */
	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */
	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */
	/* USER CODE END USART1_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

	/*Configure GPIO pin : PC13 */
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
