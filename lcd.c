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
	ret1 = HAL_I2C_Master_Transmit (&hi2c1, (ADDRESS_LCD << 1) ,(uint8_t *) data_t, 4, 100);
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
	HAL_I2C_Master_Transmit (&hi2c1, (ADDRESS_LCD << 1),(uint8_t *) data_t, 4, 100);
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

main()
{
    
 
lcd_init()
lcd_send_cmd(0x80)  
lcd_send_string("hello")
  
  }
  
  
