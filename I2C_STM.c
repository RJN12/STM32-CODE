

/* *****************************************************
	int ret;
	char dataBuffer[] = "Hello world!";
/* *****************************************************


/* *******************************MAIN    **********************

int main(void)
{
  

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
				
ret = HAL_I2C_Master_Transmit(&hi2c1,  (4 << 1),(uint8_t *)dataBuffer, 11, 100);
    /* USER CODE END WHILE */
						if ( ret != HAL_OK ) {
						      HAL_GPIO_WritePin(GPIOB,  GPIO_PIN_7, 1);
								 }
		 
		
						 if ( ret == HAL_OK ) {
							 HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7); 

						HAL_Delay(200);
						 }
		 		HAL_Delay(1000);
		 
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}
