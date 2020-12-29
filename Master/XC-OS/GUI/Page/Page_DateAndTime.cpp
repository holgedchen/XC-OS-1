#include "Basic/FileGroup.h"
#include "GUI/DisplayPrivate.h"
#include "rtc.h"

static lv_obj_t * appWindow;
static lv_obj_t* rl_hour;
static lv_obj_t* rl_minute;
static lv_obj_t* rl_seconds;
static lv_obj_t * btn_setting;


#define time24_str "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24"
#define time60_str "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n\
                  30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59\n60"


static void btn_setting_event_cb(lv_obj_t * btn, lv_event_t event)
{
    if (event == LV_EVENT_PRESSED)
    {
        //lv_obj_set_width(btn, lv_obj_get_width(btn) + (10));
    }
    else if (event == LV_EVENT_RELEASED)
    {
        //lv_obj_set_width(btn, lv_obj_get_width(btn) - (10));
        
        //char buf[5];
        //lv_roller_get_selected_str(rl_hour, buf, sizeof(buf));
        //printf("Selected month: %s, %d\n", buf, lv_roller_get_selected(rl_hour));
        
        uint8_t hour = lv_roller_get_selected(rl_hour);
        uint8_t minute = lv_roller_get_selected(rl_minute);
        uint8_t seconds = lv_roller_get_selected(rl_seconds);
        RTC_Time_Set(hour, minute, seconds);
        
        lv_obj_t *mbox = lv_mbox_create(appWindow, NULL);
        lv_mbox_set_text(mbox,"OK");
        lv_mbox_start_auto_close(mbox, 1000);
    }
    else if (event == LV_EVENT_LONG_PRESSED)
    {
        //lv_obj_del(btn);
    }
}



/**
  * @brief  ҳ���ʼ���¼�
  * @param  ��
  * @retval ��
  */
static void Setup()
{
    /*����ҳ���Ƶ�ǰ̨*/
    lv_obj_move_foreground(appWindow);
    
    rl_hour = lv_roller_create(appWindow, NULL);
    lv_roller_set_options(rl_hour, time24_str, true);
    //lv_roller_set_selected(rl_hour, 1, false);
    lv_roller_set_visible_row_count(rl_hour, 4);
    lv_obj_align(rl_hour, appWindow, LV_ALIGN_IN_TOP_LEFT, 70, 100);
    
    rl_minute = lv_roller_create(appWindow, NULL);
    lv_roller_set_options(rl_minute, time60_str, true);
    //lv_roller_set_selected(rl_minute, 1, false);
    lv_roller_set_visible_row_count(rl_minute, 4);
    lv_obj_align(rl_minute, rl_hour, LV_ALIGN_IN_TOP_MID, 70, 0);
    
    rl_seconds = lv_roller_create(appWindow, NULL);
    lv_roller_set_options(rl_seconds, time60_str, true);
    //lv_roller_set_selected(rl_seconds, 1, false);
    lv_roller_set_visible_row_count(rl_seconds, 4);
    lv_obj_align(rl_seconds, rl_minute, LV_ALIGN_IN_TOP_RIGHT, 70, 0);
    
    lv_obj_t * btn_l;
    btn_setting = lv_btn_create(appWindow, NULL);
    lv_obj_align(btn_setting, rl_minute, LV_ALIGN_IN_BOTTOM_MID, 0, 70);
    lv_obj_set_event_cb(btn_setting, btn_setting_event_cb);
    lv_obj_set_width(btn_setting, 80);
    
    btn_l = lv_label_create(btn_setting, NULL);
    lv_label_set_text(btn_l, "Setting");
    
}

/**
  * @brief  ҳ���˳��¼�
  * @param  ��
  * @retval ��
  */
static void Exit()
{
    lv_obj_clean(appWindow);
}

/**
  * @brief  ҳ���¼�
  * @param  event:�¼����
  * @param  param:�¼�����
  * @retval ��
  */
static void Event(int event, void* param)
{
    lv_obj_t * btn = (lv_obj_t*)param;
    if(event == LV_EVENT_CLICKED)
    {
        if(btn == btnBack)
        {
            page.PagePop();
        }
    }
}

/**
  * @brief  ҳ��ע��
  * @param  pageID:Ϊ��ҳ������ID��
  * @retval ��
  */
void PageRegister_DaTInfo(uint8_t pageID)
{
    appWindow = AppWindow_GetCont(pageID);
    lv_style_t * style = (lv_style_t *)lv_cont_get_style(appWindow, LV_CONT_STYLE_MAIN);
    *style = lv_style_pretty;
    page.PageRegister(pageID, Setup, NULL, Exit, Event);
}
