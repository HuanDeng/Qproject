/**
  ******************************************************************************
  * @file    usbd_storage_msd.c
  * @author  MCD application Team
  * @version V1.0.0
  * @date    31-January-2014
  * @brief   This file provides the disk operations functions.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usbd_msc_mem.h"
#include "MSC_SD.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define STORAGE_LUN_NBR                  1 

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* USB Mass storage Standard Inquiry Data */
const int8_t  STORAGE_Inquirydata[] = {
  
  /* LUN 0 */
  0x00,		
  0x80,		
  0x02,		
  0x02,
  (USBD_STD_INQUIRY_LENGTH - 5),
  0x00,
  0x00,	
  0x00,
  'S', 'T', 'M', ' ', ' ', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
  'm', 'i', 'c', 'r', 'o', 'S', 'D', ' ', /* Product      : 16 Bytes */
  'F', 'l', 'a', 's', 'h', ' ', ' ', ' ',
  '1', '.', '0' ,'0',                     /* Version      : 4 Bytes */
}; 

__IO uint32_t count = 0;
/* Private function prototypes -----------------------------------------------*/
int8_t STORAGE_Init (uint8_t lun);

int8_t STORAGE_GetCapacity (uint8_t lun, 
                           uint32_t *block_num, 
                           uint32_t *block_size);

int8_t  STORAGE_IsReady (uint8_t lun);

int8_t  STORAGE_IsWriteProtected (uint8_t lun);

int8_t STORAGE_Read (uint8_t lun, 
                        uint8_t *buf, 
                        uint32_t blk_addr,
                        uint16_t blk_len);

int8_t STORAGE_Write (uint8_t lun, 
                        uint8_t *buf, 
                        uint32_t blk_addr,
                        uint16_t blk_len);

int8_t STORAGE_GetMaxLun (void);


USBD_STORAGE_cb_TypeDef USBD_MICRO_SDIO_fops =
{
  STORAGE_Init,
  STORAGE_GetCapacity,
  STORAGE_IsReady,
  STORAGE_IsWriteProtected,
  STORAGE_Read,
  STORAGE_Write,
  STORAGE_GetMaxLun,
  (int8_t *)STORAGE_Inquirydata,
};

USBD_STORAGE_cb_TypeDef  *USBD_STORAGE_fops = &USBD_MICRO_SDIO_fops;

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initialize the storage medium
  * @param  lun : logical unit number
  * @retval Status
  */
	
int8_t STORAGE_Init (uint8_t lun)
{
	int8_t res=0;	  
	res=SD_Initialize();
	if(res)//STM32 SPI的bug,在sd卡操作失败的时候如果不执行下面的语句,可能导致SPI读写异常
	{
		SD_SPI_ReadWriteByte(0xff);//提供额外的8个时钟
	}
  return (0);
}

/**
  * @brief  return medium capacity and block size
  * @param  lun : logical unit number
  * @param  block_num :  number of physical block
  * @param  block_size : size of a physical block
  * @retval Status
  */
int8_t STORAGE_GetCapacity (uint8_t lun, uint32_t *block_num, uint32_t *block_size)
{ 
  if( SD_Detect() == SD_NOT_PRESENT)
  {
    return (-1);
  }
  
  *block_size =  512;  
  *block_num = SD_GetSectorCount(); 
  
  return (0);
}

/**
  * @brief  check whether the medium is ready
  * @param  lun : logical unit number
  * @retval Status
  */
int8_t  STORAGE_IsReady (uint8_t lun)
{
  static uint8_t status = 0;
  
  if( SD_Detect() == SD_NOT_PRESENT)
  {
    status = 1;
    return (-1);
  }
  else if(status == 1)
  {
    status = 0;
    if( SD_Initialize() != 0)
    {
      return (-1);
    } 
    return 0;
  }
  return (0);
}

/**
  * @brief  check whether the medium is write-protected
  * @param  lun : logical unit number
  * @retval Status
  */
int8_t  STORAGE_IsWriteProtected (uint8_t lun)
{
  return  0;
}

/**
  * @brief  Read data from the medium
  * @param  lun : logical unit number
  * @param  buf : Pointer to the buffer to save data
  * @param  blk_addr :  address of 1st block to be read
  * @param  blk_len : number of blocks to be read
  * @retval Status
  */
int8_t STORAGE_Read (uint8_t lun, 
                 uint8_t *buf, 
                 uint32_t blk_addr,                       
                 uint16_t blk_len)
{
  int8_t res=0;
  if( SD_Detect() == SD_NOT_PRESENT)
  {
    return (-1);
  }  
  
	res=SD_ReadDisk(buf,blk_addr,blk_len);
	
	if(res)//STM32 SPI的bug,在sd卡操作失败的时候如果不执行下面的语句,可能导致SPI读写异常
	{
		SD_SPI_ReadWriteByte(0xff);//提供额外的8个时钟
	}
   
  if(res==0x00)return 0;	 
  else return 5;	
}
/**
  * @brief  Write data to the medium
  * @param  lun : logical unit number
  * @param  buf : Pointer to the buffer to write from
  * @param  blk_addr :  address of 1st block to be written
  * @param  blk_len : number of blocks to be read
  * @retval Status
  */
int8_t STORAGE_Write (uint8_t lun, 
                  uint8_t *buf, 
                  uint32_t blk_addr,
                  uint16_t blk_len)
{
	int8_t res=0;
	
  if( SD_Detect() == SD_NOT_PRESENT)
  {
    return (-1);
  }  
  res=SD_WriteDisk(buf,blk_addr,blk_len);
	
  if(res==0x00)return 0;	 
  else return 5;	
}

/**
  * @brief  Return number of supported logical unit
  * @param  None
  * @retval number of logical unit
  */

int8_t STORAGE_GetMaxLun (void)
{
  return (STORAGE_LUN_NBR - 1);
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

