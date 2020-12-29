#include "Basic/FileGroup.h"
#include "DisplayPrivate.h"
#include "APP/TextEditor/TextEditorPrivate.h"
#include "LuaInterface/LuaScript.h"
#include "BSP/BSP.h"
#include "Basic/TasksManage.h"
#include "GUI/Page/APP_Type.h"

#include "Fatfs.h"

static lv_obj_t * appWindow;

#define FILE_NAME_LEN_MAX LV_FS_MAX_FN_LENGTH
#define FILE_CNT_MAX 64 

/*文件根路径*/
//static SdFile root;

static Fatfs fatfs(0);
static File file;


/*控件*/
static lv_obj_t * tabviewFm;
static lv_obj_t * tabDrive;
static lv_obj_t * tabFileList;
static lv_obj_t * labelPath;
static lv_obj_t * listFiles;

/**
  * @brief  创建Tabview
  * @param  *tabview:指向该控件的指针
  * @retval None
  */
static void Creat_Tabview(lv_obj_t** tabview)
{
    *tabview = lv_tabview_create(appWindow, NULL);
    lv_obj_set_size(*tabview, APP_WIN_WIDTH, APP_WIN_HEIGHT);
    lv_obj_align(*tabview, appWindow, LV_ALIGN_IN_TOP_MID, 0, 0);
    tabFileList = lv_tabview_add_tab(*tabview, LV_SYMBOL_LIST);
    tabDrive = lv_tabview_add_tab(*tabview, LV_SYMBOL_DRIVE);
}

/**
  * @brief  创建Tab
  * @param  *tab:指向驱动器信息tab的指针
  * @retval None
  */
static void Creat_TabDrive(lv_obj_t * tab)
{
    /*单位转换*/
#define CONV_MB(size) (size*0.000512f)
#define CONV_GB(size) (CONV_MB(size)/1024.0f)
#if 0
    /*获取储存器大小*/
    uint32_t cardSize = SD.card()->cardSize();
    /*获取储存器剩余空间的大小*/
    uint32_t volFree = SD.vol()->freeClusterCount() * SD.vol()->blocksPerCluster();
#endif
	
	/*获取储存器大小*/
	uint32_t cardSize = 0;
	/*获取储存器剩余空间的大小*/
	uint32_t volFree = 0;
	
	bool res = fatfs.getSize(&cardSize, &volFree);

    /*总空间*/
    float totalSize = CONV_GB(cardSize);
    /*未使用空间*/
    float freeSize = CONV_GB(volFree);
    /*已使用空间*/
    float usageSize = totalSize - freeSize;

    /*LineMeter显示已使用空间*/
    lv_obj_t * lmeter = lv_lmeter_create(tab, NULL);
    lv_obj_t * lmeterLabel = lv_label_create(lmeter, NULL);
    lv_obj_set_size(lmeter, 150, 150);
    lv_obj_align(lmeter, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_lmeter_set_range(lmeter, 0, 100);
    lv_lmeter_set_scale(lmeter, 240, 31);

    /*已使用空间百分比*/
    int16_t dispUseage = usageSize / totalSize * 100.0f;
    lv_lmeter_set_value(lmeter, dispUseage);
    lv_label_set_text_fmt(lmeterLabel, "%d%%", dispUseage);

    /*标签样式为大字体*/
    static lv_style_t labelStyle = *lv_obj_get_style(lmeterLabel);
    labelStyle.text.font = &lv_font_roboto_28;
    lv_label_set_style(lmeterLabel, LV_LABEL_STYLE_MAIN, &labelStyle);

    /*标签居中对齐*/
    lv_obj_align(lmeterLabel, lmeter, LV_ALIGN_CENTER, 0, 0);

    /*SD Size*/
    lv_obj_t * labelSD = lv_label_create(lmeter, NULL);
    lv_label_set_text_fmt(labelSD, "%0.2fGB / %0.2fGB", usageSize, totalSize);
    lv_obj_align(labelSD, lmeter, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
}

/*错误对话框mbox控件*/
static lv_obj_t * mboxError;
/**
  * @brief  mbox控件事件
  * @param  *obj:对象地址
  * @param  event:事件类型
  * @retval None
  */
static void mbox_event_handler(lv_obj_t * obj, lv_event_t event)
{
    /*单击事件*/
    if(event == LV_EVENT_CLICKED)
    {
        /*关闭对话框*/
        lv_mbox_start_auto_close(obj, 20);
    }
    /*取消事件*/
    if(event == LV_EVENT_CANCEL)
    {
        /*关闭对话框*/
        lv_mbox_start_auto_close(obj, 20);
    }
    /*删除事件*/
    if(event == LV_EVENT_DELETE)
    {
        /*删除控件地址*/
        mboxError = NULL;
    }
}

/**
  * @brief  抛出对话框
  * @param  *text:对话框文本
  * @retval true对话框抛出成功，false对话框正在占用
  */
static bool MboxThorw(const char *text)
{
    /*对话框是否占用*/
    if(mboxError)
        return false;
    
    /*对话框激活*/
    MessageBox_Activate(
        true,
        tabFileList,
        &mboxError,
        200, 200,
        text,
        NULL,
        mbox_event_handler
    );
    
    return true;
}

/**
  * @brief  打开文本文件
  * @param  filename:文件路径
  * @retval None
  */
static void OpenTextFile(String filename)
{
    /*判断文本编辑器是否被锁定*/
    if(TextEditorGetLocked())
    {
        MboxThorw("text editor is running");
        return;
    }
#if 0
    /*文本文件对象*/
    SdFile file;
    /*打开文件*/
    if(file.open(filename.c_str(), O_RDWR))
    {
        /*文件长度是否大于缓冲区长度*/
        if(file.available() < TextBuffSize)
        {
            char* pText = (char*)MemPool_Malloc(TextBuffSize);
            /*清缓冲区*/
            memset(pText, 0, TextBuffSize);
            /*载入缓冲区*/
            file.read(pText, TextBuffSize);
            
            /*Lua解释器指向文本缓冲区*/
            LuaCodeSet(pText);
            //page.PagePush(PAGE_LuaScript);
            /*文本编辑器指向文本缓冲区*/
            TextEditorSet(pText, file);
            /*切换文本编辑器页面*/
            page.PagePush(PAGE_TextEditor);
        }
        else
        {
            /*文件过大*/
            char str[50];
            sprintf(
                str, 
                "file size too large!\n(%0.2fKB > buffer size(%0.2fKB))", 
                (float)file.available() / 1024.0f, TextBuffSize / 1024.0f
            );
            MboxThorw(str);
        }
        file.close();
    }
    else
    {
        MboxThorw("file open faild");
    }
#endif
	
	if (file.open(filename.c_str(), FA_READ | FA_WRITE | FA_OPEN_ALWAYS))
	{
		/*文件长度是否大于缓冲区长度*/
		if (file.getSize() < TextBuffSize)
		{
			char* pText = (char*)ta_alloc(TextBuffSize);
			/*清缓冲区*/
			memset(pText, 0, TextBuffSize);
			/*载入缓冲区*/
			file.read(pText, TextBuffSize);
			
			/*Lua解释器指向文本缓冲区*/
			LuaCodeSet(pText);
			//page.PagePush(PAGE_LuaScript);
			/*文本编辑器指向文本缓冲区*/
			TextEditorSet(pText, file);
			/*切换文本编辑器页面*/
			page.PagePush(PAGE_TextEditor);
		}
		else
		{
			/*文件过大*/
			char str[50];
			sprintf(
			    str, 
				"file size too large!\n(%0.2fKB > buffer size(%0.2fKB))", 
				(float)file.getSize() / 1024.0f,
				TextBuffSize / 1024.0f);
			MboxThorw(str);
		}
		file.close();
	}
	else
	{
		MboxThorw("file open faild");
	}
}

/************************File Analyzer*************************/
/*文件类型枚举*/
typedef enum{
    FT_UNKONWN,
    FT_IMG,
    FT_VIDEO,
    FT_AUDIO,
    FT_TEXT,
    FT_DIR,
    FT_MAX
}File_Type;

typedef void(*OpenFileFunc_t)(const char*);
extern void Wav_OpenFile(const char* path);
extern void Bv_OpenFile(const char* path);

/*文件类型表*/
typedef struct{
    const char* extName;
    const char* sym;
    File_Type type;
    OpenFileFunc_t openFileFunc;
}FileTypeList_TypeDef;
/*扩展名图标映射表*/
static const FileTypeList_TypeDef FileTypeList[] = {
    {"",     LV_SYMBOL_FILE,   FT_UNKONWN},
    {".bv",  LV_SYMBOL_VIDEO,  FT_VIDEO, Bv_OpenFile},
    {".wav", LV_SYMBOL_AUDIO,  FT_AUDIO, Wav_OpenFile},
	{".mp3", LV_SYMBOL_AUDIO,  FT_AUDIO, Wav_OpenFile},
    {".lua", LV_SYMBOL_EDIT,   FT_TEXT},
    {".txt", LV_SYMBOL_EDIT,   FT_TEXT},
    {".html",LV_SYMBOL_EDIT,   FT_TEXT},
    {".log", LV_SYMBOL_EDIT,   FT_TEXT},
    {".xtrc",LV_SYMBOL_EDIT,   FT_TEXT},
    {".xlrc",LV_SYMBOL_EDIT,   FT_TEXT},
    {".c",   LV_SYMBOL_EDIT,   FT_TEXT},
    {".h",   LV_SYMBOL_EDIT,   FT_TEXT},
    {".cpp", LV_SYMBOL_EDIT,   FT_TEXT},
    {".png", LV_SYMBOL_IMAGE,  FT_IMG},
    {".jpg", LV_SYMBOL_IMAGE,  FT_IMG},
    {".gif", LV_SYMBOL_IMAGE,  FT_IMG},
};

/*切换工作路径*/
static void PathChange(lv_obj_t * tab, const char *newPath);
/*当前工作路径*/
String NowFilePath = "/";
/**
  * @brief  打开文件夹
  * @param  enter:是否进入，切换到上一层文件夹
  * @param  folder:子文件夹名称
  * @retval None
  */
static void FolderAccess(bool enter = false, const char* folder = NULL)
{
    /*打开文件夹*/
    if(enter)
    {
        NowFilePath = NowFilePath + "/" + String(folder);
    }
    /*退出文件夹*/
    else
    {
        /*计算当前工作路径最后一个字符开始到'/'号的字符个数*/
        int index = NowFilePath.lastIndexOf('/');
        if(index > 0)
        {
            /*将当前文件夹名从当前工作路径去除*/
            NowFilePath = NowFilePath.substring(0, index);
        }
    }
    /*切换新的工作路径*/
    PathChange(tabFileList, NowFilePath.c_str());
}

/**
  * @brief  获取文件信息索引
  * @param  *filename:文件名
  * @retval 索引号
  */
static int FileInfoGetIndex(const char *filename)
{
    String Name = String(filename);
    /*全部切换为小写*/
    Name.toLowerCase();
    
    /*查表*/
    for(uint8_t i = 1; i < __Sizeof(FileTypeList); i++)
    {
        /*找到对应扩展名*/
        if(Name.endsWith(FileTypeList[i].extName))
        {
            /*返回索引号*/
            return i;
        }
    }
    /*未知文件索引*/
    return 0;
}

/**
  * @brief  文件被选中事件
  * @param  *obj:对象地址
  * @param  event:事件类型
  * @retval None
  */
static void FileEvent_Handler(lv_obj_t * obj, lv_event_t event)
{
    /*点击事件*/
    if(event == LV_EVENT_CLICKED)
    {
        /*如果对话框开启，先关闭对话框*/
        if(mboxError)
        {
            lv_event_send(mboxError, LV_EVENT_CANCEL, 0);
            return;
        }
		
        /*获取被点击的文件信息*/
        const char* name = lv_list_get_btn_text(obj);
        const char* sym = (const char*)lv_img_get_src(lv_list_get_btn_img(obj));
        int index = FileInfoGetIndex(name);
        
        /*将当前工作路径后边加上文件名*/
        String filePath =  NowFilePath + "/" + String(name);
        /*是否为文件夹*/
        if(strcmp(sym, LV_SYMBOL_DIRECTORY) == 0)
        {
            /*进入文件夹*/
            FolderAccess(true, name);
        }
        /*如果为纯文本类型文件*/
        else if(FileTypeList[index].type == FT_TEXT)
        {
            /*打开文本文件*/
            OpenTextFile(filePath);
        }
        /*如果有打开方式*/
        else if(index > 0)
        {
            if(FileTypeList[index].openFileFunc)
                FileTypeList[index].openFileFunc(filePath.c_str());
        }
    }
}

/**
  * @brief  创建路径文本显示
  * @param  path:路径字符串
  * @retval None
  */
static void Creat_LabelPath(const char* path)
{
    /*过滤根目录*/
    String text = String(path + 1);
    
    /*替换'/'*/
    text.replace("/"," "LV_SYMBOL_RIGHT" ");
    
    /*添加根目录*/
    text = "/" + text;
    
    labelPath = lv_label_create(tabFileList, NULL);
    lv_label_set_text(labelPath, text.c_str());
    if(lv_obj_get_width(labelPath) >= APP_WIN_WIDTH - 10)
    {
        lv_label_set_align(labelPath, LV_LABEL_ALIGN_RIGHT);
    }
    lv_label_set_long_mode(labelPath, LV_LABEL_LONG_CROP);
    lv_label_set_anim_speed(labelPath, 20);
//    lv_label_set_body_draw(labelPath, true);
    lv_obj_set_width(labelPath, APP_WIN_WIDTH - 10);
    lv_obj_align(labelPath, NULL, LV_ALIGN_IN_TOP_MID, 0, 5);
}

/**
  * @brief  创建文件列表
  * @param  *tab: tab对象
  * @param  **list:list控件二级地址
  * @param  *path:当前文件路径
  * @retval None
  */
static void Creat_TabFileList(lv_obj_t * tab, lv_obj_t** list, const char *path)
{
#if 0
    /*路径是否合法*/
    if (!root.open(path))
        return;
    
    Creat_LabelPath(path);
    
    /*Create a list*/
    *list = lv_list_create(tab, NULL);
    lv_obj_set_width(*list, lv_obj_get_width_fit(tab) - 10);
    lv_obj_set_height(*list, lv_obj_get_height_fit(tab) - 15 - lv_obj_get_height(labelPath));
    lv_obj_align(*list, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -5);
    lv_obj_set_auto_realign(*list, true);
    lv_list_set_edge_flash(*list, true);
    
    /*Loading files*/
    SdFile file;
    char fileName[FILE_NAME_LEN_MAX];
    uint16_t FileCnt = 0;
    /*遍历目录所有项*/
    while (file.openNext(&root, O_RDONLY))
    {
        /*文件是否隐藏*/
        if (!file.isHidden())
        {
            file.getName(fileName,sizeof(fileName));
            
            int i = FileInfoGetIndex(fileName);
            /*添加list新的按钮*/
            lv_obj_t * list_btn = lv_list_add_btn(
                *list,
                file.isDir() ? LV_SYMBOL_DIRECTORY : FileTypeList[i].sym,
                fileName
            );
            
            if(list_btn != NULL)
            {
                /*添加按钮墨水效果*/
                lv_btn_set_ink_in_time(list_btn, 200);
                lv_btn_set_ink_out_time(list_btn, 200);
                lv_obj_set_event_cb(list_btn, FileEvent_Handler);
            }
        }
        file.close();
        
        FileCnt++;
        /*防止文件过多*/
        if(FileCnt >= FILE_CNT_MAX)
            break;
    }
#endif
	
    /*路径是否合法*/
    if (!file.exists(path))
        return;
    
    Creat_LabelPath(path);
    
    /*Create a list*/
    *list = lv_list_create(tab, NULL);
    lv_obj_set_width(*list, lv_obj_get_width_fit(tab) - 10);
    lv_obj_set_height(*list, lv_obj_get_height_fit(tab) - 15 - lv_obj_get_height(labelPath));
    lv_obj_align(*list, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -5);
    lv_obj_set_auto_realign(*list, true);
    lv_list_set_edge_flash(*list, true);
	
    uint16_t FileCnt = 0;
	
    /*遍历目录所有项*/
	file.opendir(path);
    while (file.readdir())
    {
        /*文件是否隐藏*/
        if (!file.isHidden())
        {
			char* fileName = file.getName();
			
            int i = FileInfoGetIndex(fileName);
			
            /*添加list新的按钮*/
            lv_obj_t * list_btn = lv_list_add_btn(
                *list,
                file.isDir() ? LV_SYMBOL_DIRECTORY : FileTypeList[i].sym,
                fileName
            );
			
            if(list_btn != NULL)
            {
                /*添加按钮墨水效果*/
                lv_btn_set_ink_in_time(list_btn, 200);
                lv_btn_set_ink_out_time(list_btn, 200);
                lv_obj_set_event_cb(list_btn, FileEvent_Handler);
            }
			
        }
        FileCnt++;
        /*防止文件过多*/
        if(FileCnt >= FILE_CNT_MAX)
            break;
    }
	file.closedir();
}

/**
  * @brief  改变当前工作路径
  * @param  *tab: tab对象
  * @param  *newPath:新的工作路径
  * @retval 无
  */
static void PathChange(lv_obj_t * tab, const char *newPath)
{
    lv_obj_del_safe(&listFiles);
    lv_obj_del_safe(&labelPath);
	
#if 0
	root.close();
#endif
	
	
	
    Creat_TabFileList(tab, &listFiles, newPath);
}

/**
  * @brief  页面初始化事件
  * @param  无
  * @retval 无
  */
static void Setup()
{
    /*获取文件系统使用权*/
    xSemaphoreTake(SemHandle_FileSystem, 1000);
    
    lv_obj_move_foreground(appWindow);
    __ExecuteOnce(Creat_Tabview(&tabviewFm));
    
    lv_obj_set_hidden(tabviewFm, false);
    PathChange(tabFileList, NowFilePath.c_str()); 
}

/**
  * @brief  页面循环事件
  * @param  无
  * @retval 无
  */
static void Loop()
{
    /*检查是否在第二个tab(ID:1)*/
    if(lv_tabview_get_tab_act(tabviewFm) == 1)
    {
        /*加载储存设备使用情况*/
        __ExecuteOnce((
            lv_tabview_set_sliding(tabviewFm, false),
            Preloader_Activate(true, tabDrive),
            Creat_TabDrive(tabDrive),
            Preloader_Activate(false, NULL),
            lv_tabview_set_sliding(tabviewFm, true)
        ));
    }
}

/**
  * @brief  页面退出事件
  * @param  无
  * @retval 无
  */
static void Exit()
{
    lv_obj_del_safe(&listFiles);
    lv_obj_set_hidden(tabviewFm, true);
    
    /*归还文件系统使用权*/
    xSemaphoreGive(SemHandle_FileSystem);
}

/**
  * @brief  页面事件
  * @param  event:事件编号
  * @param  param:事件参数
  * @retval 无
  */
static void Event(int event, void* param)
{
    lv_obj_t * btn = (lv_obj_t*)param;
    if(event == LV_EVENT_CLICKED)
    {
        /*返回按键*/
        if(btn == btnBack)
        {
            /*如果对话框开启，先关闭对话框*/
            if(mboxError)
            {
                lv_event_send(mboxError, LV_EVENT_CANCEL, 0);
                return ;
            }
            
            /*如果当前为根目录*/
            if(NowFilePath == "/")
            {
                /*退出该页面*/
                page.PagePop();
            }

            /*文件根路径开启*/
#if 0
            if(root.isOpen())
            {
                /*退出当前文件夹*/
                FolderAccess();
            }
#endif
		        /*退出当前文件夹*/
		        FolderAccess();
        }
    }
}

/**
  * @brief  页面注册
  * @param  pageID:为此页面分配的ID号
  * @retval 无
  */
void PageRegister_FileExplorer(uint8_t pageID)
{
    appWindow = AppWindow_GetCont(pageID);
    page.PageRegister(pageID, Setup, Loop, Exit, Event);
}


