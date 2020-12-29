/*
 * MIT License
 * Copyright (c) 2020 robojkj
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "rtc.h"



/**
  * @brief  ����ʱ�������
  * @param  ��
  * @retval ��
  */
void RTC_TimeAndDate_Set(void)
{
    RTC_TimeTypeDef RTC_TimeStructure;
    RTC_DateTypeDef RTC_DateStructure;

    // ��ʼ��ʱ��
    RTC_TimeStructure.RTC_H12 = RTC_H12_AMorPM;
    RTC_TimeStructure.RTC_Hours = HOURS;
    RTC_TimeStructure.RTC_Minutes = MINUTES;
    RTC_TimeStructure.RTC_Seconds = SECONDS;
    RTC_SetTime(RTC_Format_BINorBCD, &RTC_TimeStructure);
    RTC_WriteBackupRegister(RTC_BKP_DRX, RTC_BKP_DATA);

    // ��ʼ������
    RTC_DateStructure.RTC_WeekDay = WEEKDAY;
    RTC_DateStructure.RTC_Date = DATE;
    RTC_DateStructure.RTC_Month = MONTH;
    RTC_DateStructure.RTC_Year = YEAR;
    RTC_SetDate(RTC_Format_BINorBCD, &RTC_DateStructure);
    RTC_WriteBackupRegister(RTC_BKP_DRX, RTC_BKP_DATA);
}

void RTC_Time_Set(uint8_t h, uint8_t m, uint8_t s)
{
    RTC_TimeTypeDef RTC_TimeStructure;

    // ��ʼ��ʱ��
    RTC_TimeStructure.RTC_H12 = RTC_H12_AMorPM;
    RTC_TimeStructure.RTC_Hours = h;
    RTC_TimeStructure.RTC_Minutes = m;
    RTC_TimeStructure.RTC_Seconds = s;
    RTC_SetTime(RTC_Format_BINorBCD, &RTC_TimeStructure);
    RTC_WriteBackupRegister(RTC_BKP_DRX, RTC_BKP_DATA);
}

void RTC_Date_Set(uint8_t w, uint8_t d, uint8_t m, uint8_t y)
{
    RTC_DateTypeDef RTC_DateStructure;

    // ��ʼ������
    RTC_DateStructure.RTC_WeekDay = w;
    RTC_DateStructure.RTC_Date = d;
    RTC_DateStructure.RTC_Month = m;
    RTC_DateStructure.RTC_Year = y;
    RTC_SetDate(RTC_Format_BINorBCD, &RTC_DateStructure);
    RTC_WriteBackupRegister(RTC_BKP_DRX, RTC_BKP_DATA);
}

/**
  * @brief  RTC���ã�ѡ��RTCʱ��Դ������RTC_CLK�ķ�Ƶϵ��
  * @param  ��
  * @retval ��
  */
uint8_t RTC_CLK_Config(void)
{
    RTC_InitTypeDef RTC_InitStructure;

    /*ʹ�� PWR ʱ��*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    /* PWR_CR:DBF��1��ʹ��RTC��RTC���ݼĴ����ͱ���SRAM�ķ��� */
    PWR_BackupAccessCmd(ENABLE);

#if defined (RTC_CLOCK_SOURCE_LSI)
    /* ʹ��LSI��ΪRTCʱ��Դ�������
     * Ĭ��ѡ��LSE��ΪRTC��ʱ��Դ
     */
    /* ʹ��LSI */
    RCC_LSICmd(ENABLE);
    /* �ȴ�LSI�ȶ� */
    while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
    {
    }
    /* ѡ��LSI��ΪRTC��ʱ��Դ */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);

#elif defined (RTC_CLOCK_SOURCE_LSE)

    /* ʹ��LSE */
    RCC_LSEConfig(RCC_LSE_ON);
    /* �ȴ�LSE�ȶ� */
    while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
    {
    }
    /* ѡ��LSE��ΪRTC��ʱ��Դ */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);

#endif /* RTC_CLOCK_SOURCE_LSI */

    /* ʹ��RTCʱ�� */
    RCC_RTCCLKCmd(ENABLE);

    /* �ȴ� RTC APB �Ĵ���ͬ�� */
    RTC_WaitForSynchro();

    /*=====================��ʼ��ͬ��/�첽Ԥ��Ƶ����ֵ======================*/
    /* ����������ʱ��ck_spare = LSE/[(255+1)*(127+1)] = 1HZ */

    /* �����첽Ԥ��Ƶ����ֵ */
    RTC_InitStructure.RTC_AsynchPrediv = ASYNCHPREDIV;
    /* ����ͬ��Ԥ��Ƶ����ֵ */
    RTC_InitStructure.RTC_SynchPrediv = SYNCHPREDIV;
    RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
    /* ��RTC_InitStructure�����ݳ�ʼ��RTC�Ĵ��� */
    if (RTC_Init(&RTC_InitStructure) == ERROR)
    {
        return 0;
        //Serial.printf("\n\r RTC ʱ�ӳ�ʼ��ʧ�� \r\n");
    }
    return 1;
}

/*
 *    Ҫʹ�� RTC �����жϣ��谴������˳�������
 * 1. �� EXTI �� 17 ����Ϊ�ж�ģʽ������ʹ�ܣ�Ȼ��ѡ����������Ч��
 * 2. ���� NVIC �е� RTC_Alarm IRQ ͨ��������ʹ�ܡ�
 * 3. ���� RTC ������ RTC ���ӣ����� A ������ B����
 *
 *
*/
void RTC_AlarmSet(void)
{
    NVIC_InitTypeDef  NVIC_InitStructure;
    EXTI_InitTypeDef  EXTI_InitStructure;
    RTC_AlarmTypeDef  RTC_AlarmStructure;

    /*=============================�ڢٲ�=============================*/
    /* RTC �����ж����� */
    /* EXTI ���� */
    EXTI_ClearITPendingBit(EXTI_Line17);
    EXTI_InitStructure.EXTI_Line = EXTI_Line17;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /*=============================�ڢڲ�=============================*/
    /* ʹ��RTC�����ж� */
    NVIC_InitStructure.NVIC_IRQChannel = RTC_Alarm_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    /*=============================�ڢ۲�=============================*/
    /* ʧ������ ������������ʱ���ʱ�������ʧ������*/
    RTC_AlarmCmd(RTC_Alarm_X, DISABLE);
    /* ��������ʱ�� */
    RTC_AlarmStructure.RTC_AlarmTime.RTC_H12     = RTC_H12_AMorPM;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Hours   = ALARM_HOURS;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Minutes = ALARM_MINUTES;
    RTC_AlarmStructure.RTC_AlarmTime.RTC_Seconds = ALARM_SECONDS;
    RTC_AlarmStructure.RTC_AlarmMask = ALARM_MASK;
    RTC_AlarmStructure.RTC_AlarmDateWeekDaySel = ALARM_DATE_WEEKDAY_SEL;
    RTC_AlarmStructure.RTC_AlarmDateWeekDay = ALARM_DATE_WEEKDAY;


    /* ����RTC Alarm X��X=A��B�� �Ĵ��� */
    RTC_SetAlarm(RTC_Format_BINorBCD, RTC_Alarm_X, &RTC_AlarmStructure);

    /* ʹ�� RTC Alarm X �ж� */
    RTC_ITConfig(RTC_IT_ALRA, ENABLE);

    /* ʹ������ */
    RTC_AlarmCmd(RTC_Alarm_X, ENABLE);

    /* ��������жϱ�־λ */
    RTC_ClearFlag(RTC_FLAG_ALRAF);
    /* ��� EXTI Line 17 ����λ (�ڲ����ӵ�RTC Alarm) */
    EXTI_ClearITPendingBit(EXTI_Line17);
}

/**
  * @brief  RTC���ã�ѡ��RTCʱ��Դ������RTC_CLK�ķ�Ƶϵ��
  * @param  ��
  * @retval ��
  */
#define LSE_STARTUP_TIMEOUT     ((uint16_t)0x05000)
uint8_t RTC_CLK_Config_Backup(void)
{
    __IO uint16_t StartUpCounter = 0;
    FlagStatus LSEStatus = RESET;
    RTC_InitTypeDef RTC_InitStructure;

    /* ʹ�� PWR ʱ�� */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    /* PWR_CR:DBF��1��ʹ��RTC��RTC���ݼĴ����ͱ���SRAM�ķ��� */
    PWR_BackupAccessCmd(ENABLE);

    /*=========================ѡ��RTCʱ��Դ==============================*/
    /* Ĭ��ʹ��LSE�����LSE��������ʹ��LSI */
    /* ʹ��LSE */
    RCC_LSEConfig(RCC_LSE_ON);

    /* �ȴ�LSE�����ȶ��������ʱ���˳� */
    do
    {
        LSEStatus = RCC_GetFlagStatus(RCC_FLAG_LSERDY);
        StartUpCounter++;
    }
    while((LSEStatus == RESET) && (StartUpCounter != LSE_STARTUP_TIMEOUT));


    if(LSEStatus == SET )
    {
        //Serial.printf("\n\r LSE �����ɹ� \r\n");
        /* ѡ��LSE��ΪRTC��ʱ��Դ */
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    }
    else
    {
        //Serial.printf("\n\r LSE ���ϣ�תΪʹ��LSI \r\n");

        /* ʹ��LSI */
        RCC_LSICmd(ENABLE);
        /* �ȴ�LSI�ȶ� */
        while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
        {
        }

        //Serial.printf("\n\r LSI �����ɹ� \r\n");
        /* ѡ��LSI��ΪRTC��ʱ��Դ */
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
    }

    /* ʹ�� RTC ʱ�� */
    RCC_RTCCLKCmd(ENABLE);
    /* �ȴ� RTC APB �Ĵ���ͬ�� */
    RTC_WaitForSynchro();

    /*=====================��ʼ��ͬ��/�첽Ԥ��Ƶ����ֵ======================*/
    /* ����������ʱ��ck_spare = LSE/[(255+1)*(127+1)] = 1HZ */

    /* �����첽Ԥ��Ƶ����ֵΪ127 */
    RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
    /* ����ͬ��Ԥ��Ƶ����ֵΪ255 */
    RTC_InitStructure.RTC_SynchPrediv = 0xFF;
    RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
    /* ��RTC_InitStructure�����ݳ�ʼ��RTC�Ĵ��� */
    if (RTC_Init(&RTC_InitStructure) == ERROR)
    {
        //Serial.printf("\n\r RTC ʱ�ӳ�ʼ��ʧ�� \r\n");
        return 0;
    }
    return 1;
}


uint8_t init_rtc(void)
{
    uint8_t info = 0;
    /*
     * ���������ù�RTCʱ��֮��������ݼĴ���0д��һ�����������
     * ����ÿ�γ����������е�ʱ���ͨ����ⱸ�ݼĴ���0��ֵ���ж�
     * RTC �Ƿ��Ѿ����ù���������ù��Ǿͼ������У����û�����ù�
     * �ͳ�ʼ��RTC������RTC��ʱ�䡣
     */

    /* RTC���ã�ѡ��ʱ��Դ������RTC_CLK�ķ�Ƶϵ�� */
    RTC_CLK_Config();

    //�������Ӻ���
    /* �������� */
    //RTC_AlarmSet();

    if (RTC_ReadBackupRegister(RTC_BKP_DRX) != RTC_BKP_DATA)
    {
        /* ����ʱ������� */
        RTC_TimeAndDate_Set();
        info = 1;
    }
    else
    {
        /* ����Ƿ��Դ��λ */
        if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)
        {
            //Serial.printf("\r\n ������Դ��λ....\r\n");
            info = 2;
        }
        /* ����Ƿ��ⲿ��λ */
        else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)
        {
            //Serial.printf("\r\n �����ⲿ��λ....\r\n");
            info = 3;
        }

        //Serial.printf("\r\n ����Ҫ��������RTC....\r\n");

        /* ʹ�� PWR ʱ�� */
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
        /* PWR_CR:DBF��1��ʹ��RTC��RTC���ݼĴ����ͱ���SRAM�ķ��� */
        PWR_BackupAccessCmd(ENABLE);
        /* �ȴ� RTC APB �Ĵ���ͬ�� */
        RTC_WaitForSynchro();

        //�������Ӻ���
        /* ���RTC�жϱ�־λ */
        //RTC_ClearFlag(RTC_FLAG_ALRAF);
        /* ��� EXTI Line 17 ����λ (�ڲ����ӵ�RTC Alarm) */
        //EXTI_ClearITPendingBit(EXTI_Line17);
    }
    return info;
}


// �����жϷ�����
void RTC_Alarm_IRQHandler(void)
{
    if(RTC_GetITStatus(RTC_IT_ALRA) != RESET)
    {
        RTC_ClearITPendingBit(RTC_IT_ALRA);
        EXTI_ClearITPendingBit(EXTI_Line17);
    }
    /* ����ʱ�䵽���������� */
    //BEEP_ON;
}





