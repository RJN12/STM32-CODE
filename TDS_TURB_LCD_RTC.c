/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define VREF 3.3f
#define VREF_T 3.3f
#define ADC_RES 4096.0f
#define TEMP 25.0f
#define ADDRESS_LCD 0x27
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c2;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C2_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */
/* USER CODE BEGIN PFP */
uint32_t adc_value = 0;
float volt = 0;
float tds_val, turbidity_val = 0;
uint8_t uart_buf[50];
RTC_TimeTypeDef current_time;
RTC_TimeTypeDef last_time;
uint8_t first_run = 1;
/* USER CODE END PFP */

#define SAMPLES 30
uint32_t adc_samples[SAMPLES];
/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void lcd_send_cmd (char cmd)
{
  char data_u, data_l,ret1;
	uint8_t data_t[4];
	data_u = (cmd&0xf0);
	data_l = ((cmd<<4)&0xf0);
	data_t[0] = data_u|0x0C;  //en=1, rs=0
	data_t[1] = data_u|0x08;  //en=0, rs=0
	data_t[2] = data_l|0x0C;  //en=1, rs=0
	data_t[3] = data_l|0x08;  //en=0, rs=0
	ret1 = HAL_I2C_Master_Transmit (&hi2c2, (ADDRESS_LCD << 1) ,(uint8_t *) data_t, 4, 100);
}

void lcd_send_data (char data)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data&0xf0);
	data_l = ((data<<4)&0xf0);
	data_t[0] = data_u|0x0D;  //en=1, rs=1
	data_t[1] = data_u|0x09;  //en=0, rs=1
	data_t[2] = data_l|0x0D;  //en=1, rs=1
	data_t[3] = data_l|0x09;  //en=0, rs=1
	HAL_I2C_Master_Transmit (&hi2c2, (ADDRESS_LCD << 1),(uint8_t *) data_t, 4, 100);
}

void lcd_init (void)
{
	// 4 bit initialisation


	lcd_send_cmd (0x28);
	HAL_Delay(1);
	lcd_send_cmd (0x08);
	HAL_Delay(1);
	lcd_send_cmd (0x01);
	HAL_Delay(1);
	HAL_Delay(1);
	lcd_send_cmd (0x06);
	HAL_Delay(1);
	lcd_send_cmd (0x0C);
}

void lcd_send_string (char *str)
{
	while (*str) lcd_send_data (*str++);
}

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
float READ_TDS_Sensor(void)
{
    float tds = 0;
    float voltage = 0;
    uint32_t adc_sum = 0;

    // ADD THIS: Configure ADC for Channel 0 (PA0) - TDS Sensor
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_0;  // PA0 for TDS
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    // Take 30 samples
    for(int i = 0; i < SAMPLES; i++)
    {
        HAL_ADC_Start(&hadc1);
        if(HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
        {
            adc_samples[i] = HAL_ADC_GetValue(&hadc1);
            adc_sum += adc_samples[i];
        }
        HAL_ADC_Stop(&hadc1);
        HAL_Delay(33);  // 33ms × 30 = ~1 second total
    }

    // Calculate average
    uint32_t adc_avg = adc_sum / SAMPLES;
    voltage = (adc_avg * VREF) / ADC_RES;

    // Calculate TDS
    tds = (133.42f * voltage * voltage * voltage
           - 255.86f * voltage * voltage
           + 857.39f * voltage) * 0.5f;

    if(tds < 0) tds = 0;

    return tds;
}


float READ_Turbidity_Sensor(void)
{
    float turbidity = 0;
    uint32_t adc_sum = 0;

    // Configure ADC for Channel 4 (PA4)
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = ADC_CHANNEL_4;
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    // Take 30 samples for noise reduction
    for(int i = 0; i < SAMPLES; i++)
    {
        HAL_ADC_Start(&hadc1);
        if(HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
        {
            adc_samples[i] = HAL_ADC_GetValue(&hadc1);
            adc_sum += adc_samples[i];
        }
        HAL_ADC_Stop(&hadc1);
        HAL_Delay(33);
    }

    // Calculate average ADC value
    float adc_avg = (float)(adc_sum / SAMPLES);

    // ========================================
    // CALIBRATION CONSTANTS (Measure these!)
    // ========================================
    // Calibration Point 1: Clean water
    const float ADC_CLEAN = 2800.0f;   // ADC value for clean water (0-5 NTU)
    const float NTU_CLEAN = 0.0f;      // Reference NTU for clean water

    // Calibration Point 2: Turbid standard
    const float ADC_TURBID = 300.0f;   // ADC value for turbid water (1000 NTU)
    const float NTU_TURBID = 1000.0f;  // Reference NTU for turbid water

    // ========================================
    // LINEAR CALIBRATION FORMULA
    // ========================================
    // Calculate slope (m)
    float m = (NTU_TURBID - NTU_CLEAN) / (ADC_TURBID - ADC_CLEAN);

    // Calculate y-intercept (b)
    float b = NTU_CLEAN - (m * ADC_CLEAN);

    // Apply linear equation: NTU = m * ADC + b
    turbidity = (m * adc_avg) + b;

    // Limit to valid range
    if(turbidity < 0) turbidity = 0;
    if(turbidity > 3000) turbidity = 3000;

    return turbidity;
}



uint8_t Check_Time(uint8_t seconds)

{
	RTC_TimeTypeDef current_date;
	HAL_RTC_GetTime(&hrtc, &current_time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &current_date, RTC_FORMAT_BIN);

	if (first_run)
	{
		last_time = current_time;

		first_run = 0;
	}
int32_t time_diff = current_time.Seconds - last_time.Seconds;
if (time_diff < 0)
{
    time_diff += 60;
}

if (time_diff >= seconds)

{
last_time = current_time;
return 1;
}

return 0;
}

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
  MX_USART3_UART_Init();
  MX_ADC1_Init();
  MX_RTC_Init();
  MX_I2C2_Init();
  lcd_init();
  /* USER CODE BEGIN 2 */
  RTC_TimeTypeDef current_date;
    HAL_RTC_GetTime(&hrtc, &current_time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &current_date, RTC_FORMAT_BIN);



    lcd_send_cmd(0x80);
    lcd_send_string("Water Quality");
    lcd_send_cmd(0xC0);
    lcd_send_string("Monitor v2.0");
    HAL_Delay(3000);

    lcd_send_cmd(0x01);
    HAL_Delay(5);

    char msg[] = "Dual Sensor Water Quality Monitor Started\r\n";
    HAL_UART_Transmit(&huart3, (uint8_t*)msg, strlen(msg), 100);
    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
      /* USER CODE END WHILE */
  	  if(Check_Time(2))
  	  {
  	      tds_val = READ_TDS_Sensor();
          turbidity_val = READ_Turbidity_Sensor();


  	      HAL_ADC_Start(&hadc1);
  	      if(HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
  	      {
  	          adc_value = HAL_ADC_GetValue(&hadc1);
  	          volt = (adc_value * VREF) / ADC_RES;
  	      }
  	      HAL_ADC_Stop(&hadc1);

  	      int volt_mv = (int)((volt * 1000) + 0.5f);
  	      int tds_int = (int)(tds_val + 0.5f);
          int turb_int = (int)(turbidity_val + 0.5f);

          int len = snprintf((char*)uart_buf, sizeof(uart_buf),
                            "TDS: %d ppm | Turbidity: %d NTU\r\n",
                            tds_int, turb_int);
          HAL_UART_Transmit(&huart3, uart_buf, len, 200);
  	    // Display on LCD - Line 1: TDS
  	            lcd_send_cmd(0x80);
  	            char line1[20];
  	            snprintf(line1, 20, "TDS:%d", tds_int);
  	            lcd_send_string(line1);

  	            // Display on LCD - Line 2: Turbidity
  	            lcd_send_cmd(0xC9);  // Position at middle of line 2
  	            char line2[20];
  	            snprintf(line2, 20, "Tur:%d", turb_int);
  	            lcd_send_string(line2);

  	          lcd_send_cmd(0xC0);

  	          // Check worst case first
  	          if(tds_int >= 900 || turb_int >= 300)
  	          {
  	              lcd_send_string("BAD");  // Poor
  	          }
  	          else if(tds_int >= 600 || turb_int >= 150)
  	          {
  	              lcd_send_string("FAR");  // Fair
  	          }
  	          else if(tds_int >= 300 || turb_int >= 50)
  	          {
  	              lcd_send_string("GUD");  // Good
  	          }
  	          else
  	          {
  	              lcd_send_string("EXC");  // Excellent
  	          }
  	        }
  	    }
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
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
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
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
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x20303E5D;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
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
