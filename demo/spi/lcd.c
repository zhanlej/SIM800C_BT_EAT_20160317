
#include "lcd.h"

static void lcd_wr_cmd(unsigned char cmd)
{
    eat_spi_write(&cmd, 1, EAT_TRUE);
}
static void lcd_wr_data(unsigned char data)
{
    eat_spi_write(&data, 1, EAT_FALSE);
}

#define ELEC_VOLUME_MODE         0x81
#define PAGE_ADDRESS_SET         0xB0
#define PAGE_ADDRESS_ZERO        0x00
/* column 0-131 address set */
#define COL_ADDR_SET_HIGH        0x10
#define COL_ADDR_SET_LOW         0x00/*0x00*/

static void di_setContrast(unsigned char contrast)
{
  lcd_wr_cmd(ELEC_VOLUME_MODE);
  /* only support 7bit contrast, but MSB is treated null */
  lcd_wr_cmd(contrast);
}

void LcdWriteCommand(u8 cmd)
{ /* 调用LCD写命令接口函数 */
  lcd_wr_cmd(cmd);
}
void LcdWriteData(u8 data)
{ /* 调用LCD写数据接口函数 */
    lcd_wr_data(data);
}
void LcdSetDisplayAddress (u8 x, u8 y)
{ /* 调用LCD设置显示位置的接口函数 */
    u8 yPage;
    yPage = y/Y_PAGE_SIZE;
    if (((y/Y_PAGE_SIZE) >= Y_PAGES) || (x >= X_PIXELS))
    {
        return;
    }

    lcd_wr_cmd(PAGE_ADDRESS_SET|yPage);
    lcd_wr_cmd(COL_ADDR_SET_HIGH|x>>4);
    lcd_wr_cmd(COL_ADDR_SET_LOW|(x & 0x0F));
}

#ifdef __SIMCOM_PROJ_SIM800H__ 
#define LCD_RST EAT_PIN29_PCM_CLK
#elif defined __SIMCOM_PROJ_SIM800__
#define LCD_RST EAT_PIN68_PCM_CLK
#elif defined __SIMCOM_PROJ_SIM800C__
#define LCD_RST EAT_PIN41_NETLIGHT
#elif defined __SIMCOM_PROJ_SIM800F__
#define LCD_RST EAT_PIN67_GPIO11
#elif defined __SIMCOM_PROJ_SIM808__
#define LCD_RST EAT_PIN41_PWM2
#endif
/**
 * init the LCD 
 */
void lcd_init(void)
{   
    eat_lcd_light_sw(EAT_TRUE, EAT_BL_STEP_24_MA);
    eat_gpio_setup(LCD_RST, EAT_GPIO_DIR_OUTPUT, EAT_GPIO_LEVEL_LOW);
    eat_gpio_write(LCD_RST, EAT_GPIO_LEVEL_LOW);
    eat_sleep(100);
    eat_gpio_write(LCD_RST, EAT_GPIO_LEVEL_HIGH);
    eat_sleep(300);
    eat_spi_init(EAT_SPI_CLK_13M, EAT_SPI_4WIRE, EAT_SPI_BIT8, EAT_FALSE, EAT_TRUE);
    eat_sleep(100);
    lcd_wr_cmd(0xe2);  
    eat_sleep(10); 
    lcd_wr_cmd(0xae);
    lcd_wr_cmd(0xae);
    #if defined(TP_SIM800W_OLD)
    lcd_wr_cmd(0xA0);//ADC_SELECT|NORMAL_DIR);
    lcd_wr_cmd(0xC8); /*SHL select*/
    #else
    lcd_wr_cmd(0xA1);//ADC_SELECT|NORMAL_DIR);
    lcd_wr_cmd(0xC0); /*SHL select*/
    #endif
    lcd_wr_cmd(0x40);  
    lcd_wr_cmd(0xA3);/*0xA3*/
    lcd_wr_cmd(0x2C);
    eat_sleep(1);
    lcd_wr_cmd(0x2E);
    eat_sleep(1);
    lcd_wr_cmd(0x2F);
    lcd_wr_cmd(0xAD);/*set static indicator off*/
    lcd_wr_cmd(0x24);
    eat_sleep(10);
    lcd_wr_cmd(0x81);
    lcd_wr_cmd(30);
    lcd_wr_cmd(0xAF);
    eat_trace("lcd_init ok");
}


