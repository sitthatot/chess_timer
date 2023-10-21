/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "rng.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "ILI9341_Touchscreen.h"

#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"

#include "timer_screen.h"
#include "pause_screen.h"
#include "setup_screen.h"
#include "end_screen.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define print(x) HAL_UART_Transmit(&huart3, (uint8_t*)x, strlen(x),1000)
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
volatile uint32_t adc_val = 0;
uint32_t dutyCycle = 25;
uint8_t pwm;

int minuteA = 10;
int secondA = 0;

int minuteB = 10;
int secondB = 0;
char timeString[10];
int state = 0;
int sendZero = 0;
int sendOne = 0;
uint8_t playerSignal = 0; //0==white 1==black
uint8_t isBackgroundFill = 0; //0 == pause
char ch1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void displayTime(int minute, int second) {
	char toMinute[10];
	char toSecond[10];

	sprintf(toMinute, "%02d", minute);
	sprintf(toSecond, "%02d", second);
	sprintf(timeString, "%02d:%02d", minute, second);
//	print("\r");
//	print(timeString);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	uint16_t color;
	uint16_t bg;
  /* USER CODE END 1 */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

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
  MX_USART3_UART_Init();
  MX_SPI5_Init();
  MX_RNG_Init();
  MX_TIM2_Init();
  MX_I2C4_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */
	ILI9341_Init(); //initial driver setup to drive ili9341
	HAL_ADC_Start(&hadc1);

	char stateString[10];
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
//		print("A");
//		displayTime(minuteB, secondB);
//		sprintf(stateString, "%2d", state);
//		print(stateString);
		//----------------------------------------------------------PERFORMANCE TEST
//		if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_SET) {
//			playerSignal = !playerSignal;
//		}
		HAL_UART_Receive(&huart6, (uint8_t*) &ch1, 1, 1000);
		if (ch1 == '0') {
			playerSignal = 0;
		} else if (ch1 == '1') {
			playerSignal = 1;
		}
		if(state != 1){
			isBackgroundFill = 0;
		}

		if (state == 0) {
			ILI9341_Draw_Image((const char*) image_data_setup,
			SCREEN_VERTICAL_2);
			minuteA = 0;
			secondA = 10;
			minuteB = 10;
			secondB = 0;

			if (sendZero == 0) {
				//send 0
				print("0");
				HAL_UART_Transmit(&huart6, (uint8_t*) "0", 1, 1000);
				sendZero = 1;
			}
		} else if (state == 1) {
			if (sendOne == 0) {
				print("1");
				HAL_UART_Transmit(&huart6, (uint8_t*) "1", 1, 1000);
				sendOne = 1;
			}
			//send 1
			HAL_TIM_Base_Start_IT(&htim1);
			if(isBackgroundFill == 0){

			ILI9341_Draw_Image((const char*) image_data_tot, SCREEN_VERTICAL_2);
			isBackgroundFill=1;
			}
			displayTime(minuteA, secondA);
			ILI9341_Set_Rotation(SCREEN_HORIZONTAL_2);
			ILI9341_Draw_Text(timeString, 40, 140, WHITE, 3, BLACK);
			displayTime(minuteB, secondB);
			ILI9341_Draw_Text(timeString, 180, 140, WHITE, 3, BLACK);
		} else if (state == 2) { //PAUSE
			HAL_TIM_Base_Stop_IT(&htim1);
			ILI9341_Draw_Image((const char*) image_data_pause,
			SCREEN_VERTICAL_2);
//			ILI9341_Draw_Text("Hello", 140, 100, WHITE, 4, YELLOW);
		} else if (state == 3) {
			//Picture
			ILI9341_Draw_Image((const char*) image_data_end, SCREEN_VERTICAL_2);

		}

	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 144;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 6;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	while (1) {
	}
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
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
