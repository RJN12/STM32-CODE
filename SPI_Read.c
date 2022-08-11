addr = 0x00;
     addr1 = 0x00;

     uart_buf_len = sprintf(uart_buf, "SPI Test Reading\r\n");
       HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, uart_buf_len, 100);
       HAL_Delay(000);


         wip = 1;
         while (wip)
         {
         
           HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
           HAL_SPI_Transmit(&hspi1, (uint8_t *)&EEPROM_RDSR, 1, 100);
           HAL_SPI_Receive(&hspi1, (uint8_t *)spi_buf, 1, 100);
           HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

         
           wip = spi_buf[0] & 0b00000001;
         }


         int h;

        for(h=0;h<10;h++)
        {

       
         HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
         HAL_SPI_Transmit(&hspi1, (uint8_t *)&EEPROM_READ, 1, 100);


         HAL_SPI_Transmit(&hspi1, (uint8_t *)&addr, 1, 100);
        HAL_SPI_Transmit(&hspi1, (uint8_t *)&addr1, 1, 100);

         HAL_SPI_Receive(&hspi1, (uint8_t *)spi_buf, 1, 100);
         HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

       
         uart_buf_len = sprintf(uart_buf,
                                 "0x%02x on address 0x%02x \r\n",
                                 (unsigned int)spi_buf[0],addr);
         HAL_UART_Transmit(&huart1, (uint8_t *)uart_buf, uart_buf_len, 100);
        addr = addr + 1;
        HAL_Delay(100);
         }



char uart_buf[50];
  int uart_buf_len;
  char spi_buf[20];


  uint8_t addr,addr1;
  uint8_t wip;


const uint8_t EEPROM_READ = 0b00000011;
const uint8_t EEPROM_WRITE = 0b00000010;
const uint8_t EEPROM_WRDI = 0b00000100;
const uint8_t EEPROM_WREN = 0b00000110;
const uint8_t EEPROM_RDSR = 0b00000101;
const uint8_t EEPROM_WRSR = 0b00000001;
