SIM800C does not contain the PWM, and SD card functions.

The clock frequency of I2C is about 60K Hz, the clock frequency of SPI is about 500K Hz.

  I2C pin define:
    I2C_SCL EAT_PIN3_UART1_RTS
    I2C_SDA EAT_PIN4_UART1_CTS
  SPI pin define:
   SPI_CS    EAT_PIN5_UART1_DCD
   SPI_CLK   EAT_PIN6_UART1_DTR
   SPI_MOSI  EAT_PIN7_UART1_RI
   SPI_MISO  EAT_PIN14_SIM_DET
   SPI_DC    EAT_PIN42_STATUS