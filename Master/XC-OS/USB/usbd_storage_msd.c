#include "usbd_msc_mem.h"
#include "usb_conf.h"
#include "SdioDriver/sdio_sdcard.h"

//���֧�ֵ��豸��
#define STORAGE_LUN_NBR     1

const int8_t  STORAGE_Inquirydata[] =  //36
{

    /* LUN 0 */
    0x00,
    0x80,
    0x02,
    0x02,
    (USBD_STD_INQUIRY_LENGTH - 5),
    0x00,
    0x00,
    0x00,
    'X', 'C', ' ', ' ', ' ', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
    'S', 'D', 'C', 'A', 'R', 'D', ' ', ' ', /* Product      : 16 Bytes */
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    '1', '.', '0', '0',                     /* Version      : 4 Bytes */
};


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

__IO uint32_t count = 0;

int8_t STORAGE_Init (uint8_t lun)
{
    return SD_Init();
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
    *block_size = 512;
    *block_num = SDCardInfo.CardCapacity / SDCardInfo.CardBlockSize;

    return (0);
}

/**
  * @brief  check whether the medium is ready
  * @param  lun : logical unit number
  * @retval Status
  */
int8_t  STORAGE_IsReady (uint8_t lun)
{
    return 0;
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
  * @param  blk_len : nmber of blocks to be read
  * @retval Status
  */
int8_t STORAGE_Read (uint8_t lun,
                     uint8_t *buf,
                     uint32_t blk_addr,
                     uint16_t blk_len)
{
    int8_t res = 0;

    res = SD_ReadDisk(buf, blk_addr, blk_len);

    return res;
}
/**
  * @brief  Write data to the medium
  * @param  lun : logical unit number
  * @param  buf : Pointer to the buffer to write from
  * @param  blk_addr :  address of 1st block to be written
  * @param  blk_len : nmber of blocks to be read
  * @retval Status
  */
int8_t STORAGE_Write (uint8_t lun,
                      uint8_t *buf,
                      uint32_t blk_addr,
                      uint16_t blk_len)
{
    int8_t res = 0;

    res =   SD_WriteDisk(buf, blk_addr, blk_len);

    return res;
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
/**
  * @}
  */


/**
  * @}
  */


/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
