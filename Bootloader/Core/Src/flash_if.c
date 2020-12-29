/*
*******************************************************************************************************
*
* �ļ����� : flash_if.c
* ��    �� : V1.0
* ��    �� : OpenRabbit
* ˵    �� : STM32�ڲ�FLASH�����ļ�
* 
*******************************************************************************************************
*/



/* ͷ�ļ� -----------------------------------------------------------*/
#include "flash_if.h"


/* �궨�� -----------------------------------------------------------*/


/* ���� -------------------------------------------------------------*/


/* �������� ---------------------------------------------------------*/




/**
  * @brief ��ȡĳ����ַ���ڵ�flash����
  * @param addr-flash��ַ
  * @retval	0~11-addr���ڵ�����
  * @note ������С��һ��
  */
uint8_t flash_if_get_sector(uint32_t addr)
{
	if(addr<ADDR_FLASH_SECTOR_1)return FLASH_SECTOR_0;
	else if(addr<ADDR_FLASH_SECTOR_2)return FLASH_SECTOR_1;
	else if(addr<ADDR_FLASH_SECTOR_3)return FLASH_SECTOR_2;
	else if(addr<ADDR_FLASH_SECTOR_4)return FLASH_SECTOR_3;
	else if(addr<ADDR_FLASH_SECTOR_5)return FLASH_SECTOR_4;
	else if(addr<ADDR_FLASH_SECTOR_6)return FLASH_SECTOR_5;
	else if(addr<ADDR_FLASH_SECTOR_7)return FLASH_SECTOR_6;
	else if(addr<ADDR_FLASH_SECTOR_8)return FLASH_SECTOR_7;
	else if(addr<ADDR_FLASH_SECTOR_9)return FLASH_SECTOR_8;
	else if(addr<ADDR_FLASH_SECTOR_10)return FLASH_SECTOR_9;
	else if(addr<ADDR_FLASH_SECTOR_11)return FLASH_SECTOR_10; 
	return FLASH_SECTOR_11;	
}



/**
  * @brief ���ֶ�FLASH����
  * @param addr-����ַ
  * @retval	��ȡ��������
  */
uint32_t flash_if_read_word(uint32_t faddr)
{
	return *(__IO uint32_t *)faddr; 
}






/**
  * @brief ��ָ����ַ��ʼд��ָ�����ȵ�����
  * @param pbuffer-���ݻ�����
  * @param write_addr-д��ַ(����Ϊ4�ı���)
  * @param num_byte_to_write-Ҫд�����ݸ���
  * @retval	None
  * @note ���밴��д,��ÿ��д32λ����
  * @note �ú�������������,�������ᱣ����������
  */
void flash_if_write(uint32_t *pbuffer,uint32_t write_addr,uint32_t num_byte_to_write)	
{ 
    FLASH_EraseInitTypeDef FlashEraseInit;
    HAL_StatusTypeDef FlashStatus=HAL_OK;
    uint32_t SectorError=0;
	uint32_t addrx=0;
	uint32_t endaddr=0;	
    if(write_addr<STM32_FLASH_BASE||write_addr%4)return;	//�Ƿ���ַ
    
 	HAL_FLASH_Unlock();             //����	
	addrx=write_addr;				//д�����ʼ��ַ
	endaddr=write_addr+num_byte_to_write*4;	//д��Ľ�����ַ
    
    if(addrx<0X1FFF0000)
    {
        while(addrx<endaddr)		//ɨ��һ���ϰ�.(�Է�FFFFFFFF�ĵط�,�Ȳ���)
		{
			if(flash_if_read_word(addrx)!=0XFFFFFFFF)//�з�0XFFFFFFFF�ĵط�,Ҫ�����������
			{   
                FlashEraseInit.TypeErase=FLASH_TYPEERASE_SECTORS;       //�������ͣ��������� 
                FlashEraseInit.Sector=flash_if_get_sector(addrx);   //Ҫ����������
                FlashEraseInit.NbSectors=1;                             //һ��ֻ����һ������
                FlashEraseInit.VoltageRange=FLASH_VOLTAGE_RANGE_3;      //��ѹ��Χ��VCC=2.7~3.6V֮��!!
                if(HAL_FLASHEx_Erase(&FlashEraseInit,&SectorError)!=HAL_OK) 
                    break;//����������	
			}
			else
				addrx+=4;
            FLASH_WaitForLastOperation(FLASH_WAITETIME);                //�ȴ��ϴβ������
        }
    }
    FlashStatus=FLASH_WaitForLastOperation(FLASH_WAITETIME);            //�ȴ��ϴβ������
	if(FlashStatus==HAL_OK)
	{
		while(write_addr<endaddr)//д����
		{
			if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,write_addr,*pbuffer)!=HAL_OK)//д������
			{ 
				break;	//д���쳣
			}
			write_addr+=4;
			pbuffer++;

		} 
	}
	HAL_FLASH_Lock();           //����
} 












/***************************** ���ÿƼ� www.whtiaotu.com (END OF FILE) *********************************/
