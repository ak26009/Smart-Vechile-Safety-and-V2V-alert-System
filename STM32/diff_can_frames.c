/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "mcp2515.h"
#include <string.h>
#include <stdio.h>
#include "sensors.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
	// Flame Digital Pin
#define FLAME_DO_PORT GPIOB
#define FLAME_DO_PIN  GPIO_PIN_0

#define MMA8452_ADDR   (0x1C << 1)   // SA0 = GND
#define WHO_AM_I_REG   0x0D
#define CTRL_REG1      0x2A
#define OUT_X_MSB      0x01

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart2;
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */
can_frame txFrame;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
//static void MX_GPIO_Init(void);
//static void MX_SPI1_Init(void);
//static void MX_USART2_UART_Init(void);
//static void MX_ADC1_Init(void);
//void MX_I2C1_Init(void);

void MX_ADC1_Init(void);
void MX_SPI1_Init(void);
void MX_USART2_UART_Init(void);
void MX_GPIO_Init(void);
void MX_I2C1_Init(void);



void MMA8452_Init(void);
void MMA8452_ReadAccel(float *ax, float *ay, float *az);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

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
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  mcp2515_reset();
  MMA8452_Init();

  HAL_Delay(100);

  mcp2515_setBitrate(CAN_500KBPS, MCP_16MHZ);
  mcp2515_setNormalMode();

  // Prepare CAN message
  txFrame.can_id  = 0x123;
  txFrame.can_dlc = 5;

  txFrame.data[0] = 'H';
  txFrame.data[1] = 'E';
  txFrame.data[2] = 'L';
  txFrame.data[3] = 'L';
  txFrame.data[4] = 'O';
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_UART_Transmit(&huart2, (uint8_t*)"Loop\r\n", 6, 100);
  while (1)
  {
      /* USER CODE END WHILE */

      /* USER CODE BEGIN 3 */
      // 1️⃣ Read analog sensors
      float temp = Read_LM35();
      uint16_t noise = Read_Noise();
      uint16_t flame = Read_Flame();

      // Convert temperature to uint16 (e.g., 28.9°C -> 289)
      uint16_t temp_val = (uint16_t)(temp * 10);

      // -------- CAN FRAME 1: TEMP, NOISE, FLAME --------
      txFrame.can_id  = 0x300;
      txFrame.can_dlc = 6;  // 2 bytes each

      txFrame.data[0] = (temp_val >> 8) & 0xFF; // Temp MSB
      txFrame.data[1] = temp_val & 0xFF;        // Temp LSB

      txFrame.data[2] = (noise >> 8) & 0xFF;    // Noise MSB
      txFrame.data[3] = noise & 0xFF;           // Noise LSB

      txFrame.data[4] = (flame >> 8) & 0xFF;    // Flame MSB
      txFrame.data[5] = flame & 0xFF;           // Flame LSB

      mcp2515_sendMessage(&txFrame);
      HAL_Delay(1);  // Short delay between frames

      // 2️⃣ Read accelerometer
      float ax, ay, az;
      MMA8452_ReadAccel(&ax, &ay, &az);

      // Convert float accel to int16 (e.g., 1.97 -> 197)
      int16_t ax_i = (int16_t)(ax * 100);
      int16_t ay_i = (int16_t)(ay * 100);
      int16_t az_i = (int16_t)(az * 100);

      // -------- CAN FRAME 2: ACCELEROMETER --------
      txFrame.can_id  = 0x301;
      txFrame.can_dlc = 6; // 2 bytes each

      txFrame.data[0] = (ax_i >> 8) & 0xFF; // AX MSB
      txFrame.data[1] = ax_i & 0xFF;        // AX LSB

      txFrame.data[2] = (ay_i >> 8) & 0xFF; // AY MSB
      txFrame.data[3] = ay_i & 0xFF;        // AY LSB

      txFrame.data[4] = (az_i >> 8) & 0xFF; // AZ MSB
      txFrame.data[5] = az_i & 0xFF;        // AZ LSB

      mcp2515_sendMessage(&txFrame);

      // 3️⃣ Debug output via UART
      char buf[128];
      snprintf(buf, sizeof(buf),
               "Temp: %.2fC Noise:%u Flame:%u AX:%.2f AY:%.2f AZ:%.2f\r\n",
               temp, noise, flame, ax, ay, az);
      HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);

      HAL_Delay(500); // Delay before next reading
  }


/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16; // need to check
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
float Read_LM35(void)
{
    uint16_t adc = Read_ADC_Channel(ADC_CHANNEL_0);

    float voltage = (adc * 3.3f) / 4095.0f;
    float temperature = voltage * 100.0f;

    return temperature;
}

uint16_t Read_ADC_Channel(uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel = channel;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint16_t val = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);

    return val;
}


uint16_t Read_Noise(void)
{
    return Read_ADC_Channel(ADC_CHANNEL_1);
}

uint16_t Read_Flame(void)
{
    return Read_ADC_Channel(ADC_CHANNEL_9);  // PB1
}

uint8_t Read_Flame_Digital(void)
{
    return HAL_GPIO_ReadPin(FLAME_DO_PORT, FLAME_DO_PIN);
}

void MMA8452_Write(uint8_t reg, uint8_t data)
{
    HAL_I2C_Mem_Write(&hi2c1, MMA8452_ADDR, reg, 1, &data, 1, 100);
}

void MMA8452_Read(uint8_t reg, uint8_t *buf, uint8_t len)
{
    HAL_I2C_Mem_Read(&hi2c1, MMA8452_ADDR, reg, 1, buf, len, 100);
}

void MMA8452_Init(void)
{
    uint8_t who;
    MMA8452_Read(WHO_AM_I_REG, &who, 1);

    char msg[40];
    sprintf(msg, "WHO_AM_I: 0x%02X\r\n", who);
    HAL_UART_Transmit(&huart2,(uint8_t*)msg,strlen(msg),100);

    MMA8452_Write(CTRL_REG1, 0x01); // active mode
}

void MMA8452_ReadAccel(float *ax, float *ay, float *az)
{
    uint8_t buf[6];
    MMA8452_Read(OUT_X_MSB, buf, 6);

    int16_t x = (buf[0]<<8 | buf[1]) >> 4;
    int16_t y = (buf[2]<<8 | buf[3]) >> 4;
    int16_t z = (buf[4]<<8 | buf[5]) >> 4;

    *ax = x / 2048.0f;
    *ay = y / 2048.0f;
    *az = z / 2048.0f;
}


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
