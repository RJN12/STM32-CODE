HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

   
   uart_buf_len = sprintf(uart_buf, "SPI Test Write\r\n");
   HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, uart_buf_len, 100);

 
   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
   HAL_SPI_Transmit(&hspi1, (uint8_t *)&EEPROM_WREN, 1, 100);
   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);



       spi_buf[0] = 0x01;
     spi_buf[1] = 0x02;
     spi_buf[2] = 0x03;
    spi_buf[3] = 0x04;
         spi_buf[4] = 0x05;
      spi_buf[5] = 0x06;
           spi_buf[6] = 0x07;
        spi_buf[7] = 0x08;
             spi_buf[8] = 0x09;


 

    addr = 0x00;
   addr1 = 0x00;

   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
   HAL_SPI_Transmit(&hspi1, (uint8_t *)&EEPROM_WRITE, 1, 100);


   HAL_SPI_Transmit(&hspi1, (uint8_t *)&addr, 1, 100);
  HAL_SPI_Transmit(&hspi1, (uint8_t *)&addr1, 1, 100);
   HAL_SPI_Transmit(&hspi1, (uint8_t *)spi_buf, 10, 100);
   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
