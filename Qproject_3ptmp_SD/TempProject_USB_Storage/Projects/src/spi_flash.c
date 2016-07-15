/* Includes ------------------------------------------------------------------*/
#include "spi_flash.h"
#include "string.h"

/**
  * @brief  Configures the SPI Peripheral.
  * @param  None
  * @retval None
  */
void SPI_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;
	
  /* Enable the SPI periph */
  RCC_APB1PeriphClockCmd(SPIx_CLK, ENABLE);
  
  /* Enable SCK, MOSI, MISO and NSS GPIO clocks */
  RCC_AHBPeriphClockCmd(SPIx_SCK_GPIO_CLK | SPIx_MISO_GPIO_CLK | SPIx_MOSI_GPIO_CLK, ENABLE);
  
  GPIO_PinAFConfig(SPIx_SCK_GPIO_PORT, SPIx_SCK_SOURCE, SPIx_SCK_AF);
  GPIO_PinAFConfig(SPIx_MOSI_GPIO_PORT, SPIx_MOSI_SOURCE, SPIx_MOSI_AF);
  GPIO_PinAFConfig(SPIx_MISO_GPIO_PORT, SPIx_MISO_SOURCE, SPIx_MISO_AF);
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_3;

  /* SPI SCK pin configuration */
  GPIO_InitStructure.GPIO_Pin = SPIx_SCK_PIN;
  GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStructure);

  /* SPI  MOSI pin configuration */
  GPIO_InitStructure.GPIO_Pin =  SPIx_MOSI_PIN;
  GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /* SPI MISO pin configuration */
  GPIO_InitStructure.GPIO_Pin = SPIx_MISO_PIN;
  GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Pin = SPIx_CS_PIN;
	GPIO_Init(SPIx_CS_GPIO_PORT, &GPIO_InitStructure);
  
	sFLASH_CS_HIGH();
	
  /* SPI configuration -------------------------------------------------------*/
  SPI_I2S_DeInit(SPIx);
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI2, &SPI_InitStructure); 
  SPI_RxFIFOThresholdConfig(SPI2, SPI_RxFIFOThreshold_QF);
	SPI_Cmd(SPI2, ENABLE);
}

void sFLASH_sector_write(uint8_t * buffer, uint32_t sector, uint8_t sector_number)
{
	uint32_t Address;

	while(sector_number != 0x00)
  {
		Address = sector * sFLASH_SPI_PAGESIZE;
    sFLASH_WritePage(buffer,Address,512);
		buffer+=FLASH_SECTOR_SIZE;
    sector++;
    sector_number--;
  }
}

void sFLASH_sector_read(uint8_t * buffer, uint32_t sector, uint8_t sector_number)
{
	uint32_t Address;
	
	while(sector_number != 0x00)
  {
		Address = sector * sFLASH_SPI_PAGESIZE;
		sFLASH_ReadBuffer(buffer,Address,512);
		buffer+=FLASH_SECTOR_SIZE;
    sector++;
    sector_number--;
  }
}

/**
  * @brief  Writes more than one byte to the FLASH with a single WRITE cycle 
  *         (Page WRITE sequence).
  * @note   The number of byte can't exceed the FLASH page size.
  * @param  pBuffer: pointer to the buffer  containing the data to be written
  *         to the FLASH.
  * @param  WriteAddr: FLASH's internal address to write to.
  * @param  NumByteToWrite: number of bytes to write to the FLASH, must be equal
  *         or less than "sFLASH_PAGESIZE" value.
  * @retval None
  */
void sFLASH_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();
  /*!< Send "Write to Memory " instruction */
  sFLASH_SendByte(sFLASH_CMD_WRITE);
  /*!< Send ReadAddr high nibble address byte to read from */
  sFLASH_SendByte((uint8_t)(WriteAddr >> 6));
  /*!< Send ReadAddr medium nibble address byte to read from */
  sFLASH_SendByte((uint8_t)(WriteAddr << 2));
  /*!< Send ReadAddr low nibble address byte to read from */
  sFLASH_SendByte(0x00);

  /*!< while there is data to be written on the FLASH */
  while (NumByteToWrite--)
  {
    /*!< Send the current byte */
    sFLASH_SendByte(*pBuffer);
    /*!< Point on the next byte to be written */
    pBuffer++;
  }

  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();

  /*!< Wait the end of Flash writing */
  sFLASH_WaitForWriteEnd();
}

/**
  * @brief  Reads a block of data from the FLASH.
  * @param  pBuffer: pointer to the buffer that receives the data read from the FLASH.
  * @param  ReadAddr: FLASH's internal address to read from.
  * @param  NumByteToRead: number of bytes to read from the FLASH.
  * @retval None
  */
void sFLASH_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();

  /*!< Send "Read from Memory " instruction */
  sFLASH_SendByte(sFLASH_CMD_READ);

  /*!< Send ReadAddr high nibble address byte to read from */
  sFLASH_SendByte((uint8_t)(ReadAddr >> 6));
  /*!< Send ReadAddr medium nibble address byte to read from */
  sFLASH_SendByte((uint8_t)(ReadAddr << 2));
  /*!< Send ReadAddr low nibble address byte to read from */
  sFLASH_SendByte(0x00);

	/* Read a byte from the FLASH */
  sFLASH_SendByte(sFLASH_DUMMY_BYTE);
  /* Read a byte from the FLASH */
  sFLASH_SendByte(sFLASH_DUMMY_BYTE);
  /* Read a byte from the FLASH */
  sFLASH_SendByte(sFLASH_DUMMY_BYTE);
  /* Read a byte from the FLASH */
  sFLASH_SendByte(sFLASH_DUMMY_BYTE);
  while (NumByteToRead--) /*!< while there is data to be read */
  {
    /*!< Read a byte from the FLASH */
    *pBuffer = sFLASH_ReadByte();
    /*!< Point to the next location where the byte read will be saved */
    pBuffer++;
  }

  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();
}

/**
  * @brief  Erases the specified FLASH page.
  * @param SectorAddr: address of the sector to erase.
  * @retval : None
  */
void SPI_FLASH_PageErase(uint32_t PageAddr)
{
  /* Sector Erase */
  /* Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();
  /* Send Sector Erase instruction */
  sFLASH_SendByte(sFLASH_CMD_PE);
  /*!< Send ReadAddr high nibble address byte to read from */
  sFLASH_SendByte((uint8_t)(PageAddr >> 6));
  /*!< Send ReadAddr medium nibble address byte to read from */
  sFLASH_SendByte((uint8_t)(PageAddr << 2));
  /*!< Send ReadAddr low nibble address byte to read from */
  sFLASH_SendByte(0x00);
  /* Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();

  /* Wait the end of Flash writing */
  sFLASH_WaitForWriteEnd();
}

/**
  * @brief  Reads FLASH identification.
  * @param  None
  * @retval FLASH identification
  */

uint32_t sFLASH_ReadID(void)
{
	uint32_t Temp = 0, Temp0 = 0, Temp1 = 0, Temp2 = 0;
  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();
	
  /*!< Send "RDID " instruction */
  sFLASH_SendByte(0x9F);

  /*!< Read a byte from the FLASH */
  Temp0 = sFLASH_ReadByte();

  /*!< Read a byte from the FLASH */
  Temp1 = sFLASH_ReadByte();

  /*!< Read a byte from the FLASH */
  Temp2 = sFLASH_ReadByte();

  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();

  Temp = ((Temp0 << 16) | (Temp1 << 8) | Temp2);
	
  return Temp;
}

/**
  * @brief  Reads a byte from the SPI Flash.
  * @note   This function must be used only if the Start_Read_Sequence function
  *         has been previously called.
  * @param  None
  * @retval Byte Read from the SPI Flash.
  */
uint8_t sFLASH_ReadByte(void)
{
  return (sFLASH_SendByte(sFLASH_DUMMY_BYTE));
}

/**
  * @brief  Sends a byte through the SPI interface and return the byte received
  *         from the SPI bus.
  * @param  byte: byte to send.
  * @retval The value of the received byte.
  */
uint8_t sFLASH_SendByte(uint8_t byte)
{
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE)==RESET);
	SPI_SendData8(SPI2,byte);
	
	while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE)==RESET);
	return SPI_ReceiveData8(SPI2);
}

/**
  * @brief  Polls the status of the Write In Progress (WIP) flag in the FLASH's
  *         status register and loop until write opertaion has completed.
  * @param  None
  * @retval None
  */
void sFLASH_WaitForWriteEnd(void)
{
  uint8_t flashstatus1 = 0;

  /*!< Select the FLASH: Chip Select low */
  sFLASH_CS_LOW();

  /*!< Send "Read Status Register" instruction */
  sFLASH_SendByte(sFLASH_CMD_RDSR);

  /*!< Loop as long as the memory is busy with a write cycle */
  do
  {
    /*!< Send a dummy byte to generate the clock needed by the FLASH
    and put the value of the status register in FLASH_Status variable */
    flashstatus1 = sFLASH_ReadByte();
  }
  while ((flashstatus1 & sFLASH_RDY_FLAG) == RESET); /* Write in progress */

  /*!< Deselect the FLASH: Chip Select high */
  sFLASH_CS_HIGH();
}
