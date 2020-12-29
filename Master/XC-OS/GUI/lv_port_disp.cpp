#include "DisplayPrivate.h"

/* DMA settings
   - 0..2: 0 = no DMA, 1 = DMA1, 2 = DMA2 (DMA2-t is good!)
   - 0..7: DMA channel
   - 0..7: Stream					*/
#define LCD_DMA           2, 1, 4

#define DMAX_STREAMX_(a, b, c)			DMA ## a ## _Stream ## c
#define DMAX_STREAMX(a)					DMAX_STREAMX_(a)

#define DMAX_IT_TCIF_(a, b, c)			DMA_IT_TCIF ## c
#define DMAX_IT_TCIF(a)					DMAX_IT_TCIF_(a)

#define DMAX_STREAMX_IRQHANDLER_(a, b, c)		DMA ## a ## _Stream ## c ## _IRQHandler
#define DMAX_STREAMX_IRQHANDLER(a)				DMAX_STREAMX_IRQHANDLER_(a)

#define DMAX_STREAMX_IRQ_(a, b, c)				DMA ## a ## _Stream ## c ## _IRQn
#define DMAX_STREAMX_IRQ(a)						DMAX_STREAMX_IRQ_(a)

#define DMAX_CHANNELX_(a, b, c)				DMA_Channel_ ## b
#define DMAX_CHANNELX(a)					DMAX_CHANNELX_(a)

/*��Ļ�����ṹ���ַ*/
static lv_disp_drv_t * disp_drv_p;

/*��Ļ��������Ϣ*/
static lv_disp_buf_t disp_buf;

/*ƹ�һ�����������10������*/
static lv_color_t lv_disp_buf1[LV_HOR_RES_MAX * 10];
static lv_color_t lv_disp_buf2[LV_HOR_RES_MAX * 10];


#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(lv_log_level_t level, const char * file, uint32_t line, const char * dsc)
{
    Serial.printf("%s@%d->%s\r\n", file, line, dsc);
}
#endif

/**
  * @brief  ʹ��DMA���ͻ���������
  * @param  buf:��������ַ
  * @param  size:����������
  * @retval ��
  */
static void disp_fsmc_dma_send(void* buf, uint32_t size)
{
    DMA_Cmd(DMAX_STREAMX(LCD_DMA), DISABLE);                      //�ر�DMA����
    DMAX_STREAMX(LCD_DMA)->PAR = (uint32_t)buf;
    DMAX_STREAMX(LCD_DMA)->NDTR = size;
    DMA_Cmd(DMAX_STREAMX(LCD_DMA), ENABLE);                      //����DMA����
    
//    while(DMA_GetFlagStatus(DMA2_Stream3,DMA_FLAG_TCIF3) == RESET){} //�ȴ�DMA�������
//    DMA_ClearFlag(DMA2_Stream3, DMA_FLAG_TCIF3); // �����־
}


/* Display flushing */
static void disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    disp_drv_p = disp;
    
    //screen.drawFastRGBBitmap(area->x1, area->y1, (uint16_t*)color_p, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1));
    //lv_disp_flush_ready(disp_drv_p);/* tell lvgl that flushing is done */
	
    int16_t w = (area->x2 - area->x1 + 1);
    int16_t h = (area->y2 - area->y1 + 1);
    uint32_t size = w * h;		//	�����8�ֽڴ�����Ҫx2
    
    /*����ˢ������*/
    screen.setAddrWindow(area->x1, area->y1, area->x2, area->y2);
    
    /*DMA��������*/
    disp_fsmc_dma_send(color_p, size);
}


static bool touchpad_read(lv_indev_drv_t * indev, lv_indev_data_t * data)
{
    static lv_coord_t last_x = 0;
    static lv_coord_t last_y = 0;

#define touchpad_is_pressed() tp_dev.touched()
	
    /*Save the state and save the pressed coordinate*/
    data->state = touchpad_is_pressed() ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL;
    if(data->state == LV_INDEV_STATE_PR)
    {
        //touchpad_get_xy(&last_x, &last_y);

		byte n = tp_dev.getNoOfTouches();
		touch.num = n;
		touch.event = true;
		for (uint8_t i = 1; i < n + 1; i++)
		{
			TouchPoint TP = tp_dev.getTouchLoc(i);
			touch.TP[i-1] = TP;
			//Serial.println("TPoint:"+String(i)+" M:" + String(TP.M)+" P:"+String(TP.P)+" Pos:("+String(TP.X)+','+String(TP.Y)+") ");
			/*
				TP.M=Touch_mode:
						0==> Press Down
						1==> Lift Up
						2=>> Contact
						3==>: No event
				TP.P==> Touch_pressure
				TP.X==> position x
				TP.Y==> position y
			*/
			last_x = TP.X;
			last_y = TP.Y;
		}
    }	else
	{
		touch.event = false;
	}
	
    /*Set the coordinates (if released use the last pressed coordinates)*/
    data->point.x = last_x;
    data->point.y = last_y;

    return false; /*Return `false` because we are not buffering and no more data to read*/
}


extern "C" {
void DMAX_STREAMX_IRQHANDLER(LCD_DMA)(void)
{
    /*DMA��������ж�*/
    if(DMA_GetITStatus(DMAX_STREAMX(LCD_DMA), DMAX_IT_TCIF(LCD_DMA)) != RESET)
    {
        lv_disp_flush_ready(disp_drv_p);/* tell lvgl that flushing is done */
        DMA_ClearITPendingBit(DMAX_STREAMX(LCD_DMA), DMAX_IT_TCIF(LCD_DMA));
    }
}
}


/**DMAX_CHANNELX(LCD_DMA)
  * @brief  DMA��ʼ��
  * @param  ��
  * @retval ��
  */
static void lv_disp_fsmc_dma_init()
{
    DMA_InitTypeDef  DMA_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE); //DMA2ʱ��ʹ��
    DMA_DeInit(DMAX_STREAMX(LCD_DMA));
    while(DMA_GetCmdStatus(DMAX_STREAMX(LCD_DMA)) != DISABLE) {} //�ȴ�DMA������

    /* ���� DMA Stream */
    DMA_InitStructure.DMA_Channel = DMAX_CHANNELX(LCD_DMA);  //ͨ��ѡ��
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t) & (lv_disp_buf1); //DMA�ڴ��ַ
    DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)LCD_ADDR_DATA;//DMA �洢��0��ַ
    DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToMemory;//�ڴ����鵽�ڴ�
    DMA_InitStructure.DMA_BufferSize = sizeof(lv_disp_buf1);//���ݴ�����
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable;//���������ģʽ
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;//�洢������ģʽ
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//�������ݳ���:16λ
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//�洢�����ݳ���:16λ
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;// ʹ����ͨģʽ
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;//�е����ȼ�
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_1QuarterFull;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;//�洢��ͻ�����δ���
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//����ͻ�����δ���
    DMA_Init(DMAX_STREAMX(LCD_DMA), &DMA_InitStructure);//��ʼ��DMA Stream
    
    NVIC_EnableIRQ(DMAX_STREAMX_IRQ(LCD_DMA));
    DMA_ITConfig(DMAX_STREAMX(LCD_DMA), DMA_IT_TC, ENABLE);
}




static void lv_indev_init()
{
    /*Initialize the touch pad*/
    lv_indev_drv_t indev_drv;

    lv_indev_drv_init(&indev_drv);             /*Descriptor of a input device driver*/
    indev_drv.type = LV_INDEV_TYPE_POINTER;    /*Touch pad is a pointer-like device*/
    indev_drv.read_cb = touchpad_read;      /*Set your driver function*/
    lv_indev_drv_register(&indev_drv);         /*Finally register the driver*/
}

void lv_disp_init()
{
	
	lv_disp_fsmc_dma_init();

#if LV_USE_LOG != 0
    lv_log_register_print_cb(my_print); /* register print function for debugging */
#endif

    lv_disp_buf_init(&disp_buf, lv_disp_buf1, lv_disp_buf2, sizeof(lv_disp_buf1)/sizeof(lv_color_t));

    /*Initialize the display*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LV_HOR_RES_MAX;
    disp_drv.ver_res = LV_VER_RES_MAX;
    disp_drv.flush_cb = disp_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);
    
    lv_indev_init();
}


