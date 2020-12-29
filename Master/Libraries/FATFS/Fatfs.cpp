#include "Fatfs.h"

Fatfs::Fatfs(uint8_t id){
    _id[0] = id + '0';
	_id[1] = ':';
}
//Ϊ����ע�Ṥ����	 
//path:����·��������"0:"��"1:"
//mt:0��������ע�ᣨ�Ժ�ע�ᣩ��1������ע��
//����ֵ:ִ�н��
bool Fatfs::init()
{
    return f_mount(&fs, (const TCHAR*)_id, 1) == FR_OK ? true : false; 
}

bool Fatfs::close()
{
    return f_mount(NULL, (const TCHAR*)_id, 1) == FR_OK ? true : false; 
}

bool Fatfs::getSize(uint32_t* cardSize, uint32_t* volFree)
{
	FATFS *fs;
	DWORD fre_clust, fre_sect, tot_sect;
	
	//�õ�������Ϣ�����д�����
	res = f_getfree(_id, &fre_clust, &fs);
	if(res == FR_OK)
	{
		/* ����õ��ܵ����������Ϳ��������� */
		tot_sect = (fs->n_fatent - 2) * fs->csize;	//�õ���������
		fre_sect = fre_clust * fs->csize;			//�õ�����������
		
		#if _MAX_SS!=512
		tot_sect*=fs->ssize/512;
		fre_sect*=fs->ssize/512;
		#endif
		
		/*��ȡ��������С*/
		*cardSize = (uint32_t)(tot_sect);	//��λΪKB
		/*��ȡ������ʣ��ռ�Ĵ�С*/
		*volFree = (uint32_t)(fre_sect);	//��λΪKB

	}
	
	return res == FR_OK ? true : false; 
}

//��ʽ��
//path:����·��������"0:"��"1:"
//mode:ģʽ
//au:�ش�С
//����ֵ:ִ�н��
bool Fatfs::mkfs()
{
	return f_mkfs((const TCHAR*)_id, 0, 0) == FR_OK ? true : false; //��ʽ��,drv:�̷�;mode:ģʽ;au:�ش�С
}

//��ȡ�̷����������֣�
//path:����·��������"0:"��"1:"  
bool Fatfs::getlabel(char *path)
{
	char buf[20];
	uint32_t sn = 0;
	
	res = f_getlabel((const TCHAR *)path,(TCHAR *)buf,(DWORD*)&sn);
	if(res==FR_OK)
	{
		printf("\r\n����%s ���̷�Ϊ:%s\r\n",path,buf);
		printf("����%s �����к�:%X\r\n\r\n",path,sn); 
	}else printf("\r\n��ȡʧ�ܣ�������:%X\r\n",res);
	return res == FR_OK ? true : false; 
}
//�����̷����������֣����11���ַ�������֧�����ֺʹ�д��ĸ����Լ����ֵ�
//path:���̺�+���֣�����"0:ALIENTEK"��"1:OPENEDV"  
bool Fatfs::setlabel(char *path)
{
	res = f_setlabel((const TCHAR *)path);
	if(res==FR_OK)
	{
		printf("\r\n�����̷����óɹ�:%s\r\n",path);
	}else printf("\r\n�����̷�����ʧ�ܣ�������:%X\r\n",res);
	return res == FR_OK ? true : false; 
}

FRESULT Fatfs::get_error()
{
	return res;
}












/***�ļ�����***/

#define q_malloc	pvPortMalloc	//malloc
#define q_free		vPortFree	//free

File::File(){
    #if _USE_LFN
	static char lfn[_MAX_LFN * 2 + 1];
	cur_fno.lfname = lfn;
	cur_fno.lfsize = sizeof(lfn);
	#endif
}


//��·���µ��ļ�
//path:·��+�ļ���
//mode:��ģʽ
//����ֵ:ִ�н��
bool File::open(const char *path, uint8_t mode)
{
	file = q_malloc(sizeof(FIL));
	res = f_open((FIL*)file, (const  TCHAR*)path, mode);//���ļ���
	if(res != FR_OK)	q_free(file);
	else	state(path);
	
	return res == FR_OK ? true : false; 
}


bool File::close()
{
	res = f_close((FIL*)file);
	if(res == FR_OK && file != NULL)
	{
		q_free(file);
		file = NULL;
	}
	
	return res == FR_OK ? true : false; 
}

bool File::isOpen()
{
	return file == NULL ? false : true;
}

int File::write(uint8_t c)
{
    return f_putc((TCHAR)c, (FIL*)file);
}

int File::write(const TCHAR* c)
{
    return f_puts((const TCHAR*)c, (FIL*)file);
}

int File::write(const void* buff, uint32_t Size)
{
	FRESULT res;
	UINT i;
	
    res = f_write((FIL*)file, buff, Size, &i);

    if(res != FR_OK) return 0;

    return i;
}

extern "C" {
#include <stdio.h>
#include <stdarg.h>
}

// Work in progress to support printf.
// Need to implement stream FILE to write individual chars to chosen serial port

int File::printf (const char *__restrict __format, ...)
{
    char printf_buff[PRINTF_BUFFER_LENGTH];

    va_list args;
    va_start(args, __format);
    int ret_status = vsnprintf(printf_buff, sizeof(printf_buff), __format, args);
    //int ret_status = vsprintf(printf_buff,__format, args);
    va_end(args);
    write(printf_buff, PRINTF_BUFFER_LENGTH);

    return ret_status;
}

int File::read()
{
	uint8_t b;
	if(!read(&b, 1))	return 0;
    return b;
}

int File::read(void* buff, uint32_t Size)
{
	UINT i;
	
    res = f_read((FIL*)file, buff, Size, &i);

    if(res != FR_OK) return 0;

    return i;
}


String File::readStringUntil(char terminator)
{
    String ret;
	_startMillis = millis();
    int c = read();
    while (c >= 0 && c != terminator)
    {
        ret += (char)c;
		c = read();
		if (millis() - _startMillis > 10)  return "";
    }
    return ret;
}

FRESULT File::get_error()
{
	return res;
}

//�ļ���дָ��ƫ��
//offset:����׵�ַ��ƫ����
//����ֵ:ִ�н��.
bool File::seekSet(uint64_t offset)
{
	res = f_lseek((FIL*)file, offset);
    return res == FR_OK ? true : false; 
}

//��ȡ�ļ���ǰ��дָ���λ��.
//����ֵ:λ��
uint32_t File::tell()
{
    return f_tell((FIL*)file);
}

//��ȡ�ļ���С
//����ֵ:�ļ���С
uint32_t File::getSize()
{
	return f_size((FIL*)file);
}

//��ȡ�ļ�����
char* File::getName()
{
	char *fn;
	#if _USE_LFN
	fn=*cur_fno.lfname ? cur_fno.lfname : cur_fno.fname;
	#else
	fn=fileinfo.fname;;
	#endif
	return fn;
}

//��ȡ�ļ�����
uint32_t File::getDate()
{
	return cur_fno.fdate;
}

//��ȡ�ļ�ʱ��
uint32_t File::getTime()
{
	return cur_fno.ftime;
}

//��ȡ�ļ�����
uint32_t File::getAttrib()
{
	return cur_fno.fattrib;
}


//�ļ�״̬
bool File::state(const TCHAR* path)
{
	res = f_stat((const TCHAR*)path, &cur_fno);
	return res == FR_OK ? true : false; 
}

//ɾ���ļ�/Ŀ¼
//pname:�ļ�/Ŀ¼·��+����
//����ֵ:ִ�н��
bool File::unlink(const char *path)
{
	res = f_unlink((const TCHAR *)path);
	return res == FR_OK ? true : false; 
}

//�޸��ļ�/Ŀ¼����(���Ŀ¼��ͬ,�������ƶ��ļ�Ŷ!)
//oldname:֮ǰ������
//newname:������
//����ֵ:ִ�н��
bool File::rename(const char *oldname, const char* newname)
{
	res = f_rename((const TCHAR *)oldname,(const TCHAR *)newname);
	return res == FR_OK ? true : false; 
}

//�ļ���Ŀ¼�Ƿ����
bool File::exists(const char *path)
{
/*
while(*path != '\0')
{
	Serial.print(*path++);
	i++;
}
*/
	if(!state(path))
	{
		//�����ǲ������ŵ�·��
		if(get_error() == 6)
		{
			bool ret = opendir((const char *)path);
			if(ret) closedir();
			return ret;
		}
		return false;
	}
	
	if(isFile())
	{
		bool ret = open((const char *)path, FA_READ);
		if(ret) close();
		return ret;
	}
	else
	{
		bool ret = opendir((const char *)path);
		if(ret) closedir();
		return ret;
	}
}

//����Ŀ¼
//pname:Ŀ¼·��+����
//����ֵ:ִ�н��
bool File::mkdir(const char* path)
{
	res = f_mkdir((const TCHAR *)path);
	return res == FR_OK ? true : false;
}


//��Ŀ¼
 //path:·��
//����ֵ:ִ�н��
bool File::opendir(const char *path)
{
	dir_obj = q_malloc(sizeof(DIR));
	res = f_opendir((DIR*)dir_obj,(const TCHAR*)path);//���ļ���
	if(res != FR_OK)	q_free(dir_obj);
	else
	{
		//#if _USE_LFN
		//cur_fno.lfsize = _MAX_LFN * 2 + 1;
		//cur_fno.lfname = (char*)q_malloc(sizeof(cur_fno.lfsize));
		//#endif
	}
	
	return res == FR_OK ? true : false;
}

//�ر�Ŀ¼ 
//����ֵ:ִ�н��
bool File::closedir()
{
	res = f_closedir((DIR*)dir_obj);
	if(res == FR_OK && dir_obj != NULL)
	{
		q_free(dir_obj);
		dir_obj = NULL;
		//q_free(cur_fno.lfname);
	}
	
	return res == FR_OK ? true : false;
}

//��ȡһ��Ŀ¼��
bool File::readdir()
{
	res = f_readdir((DIR*)dir_obj,&cur_fno);//��ȡһ���ļ�����Ϣ
	if(res != FR_OK || cur_fno.fname[0] == 0)
	{
		return false;
	}
	return true;
}

bool File::scan_files(char *path)
{
	int i;
	char *fn;
	
	bool ret = opendir(path);
	if (ret)
	{
		i = strlen(path);
		for (;;)
		{
			//��ȡĿ¼�µ����ݣ��ٶ����Զ�����һ���ļ�
			//Ϊ��ʱ��ʾ������Ŀ��ȡ��ϣ�����
			ret = readdir();
			if (!ret) break;
			fn = getName();
			//���ʾ��ǰĿ¼������
			if (*fn == '.') continue;
			//Ŀ¼���ݹ��ȡ
			if (isHidden())
			{
				//�ϳ�����Ŀ¼��
				sprintf(&path[i], "/%s", fn);
				//�ݹ����
				ret = scan_files(path);
				path[i] = 0;
				//��ʧ�ܣ�����ѭ��
				if (!ret)	break;
			}
			else
			{
				Serial.printf("%s/%s\r\n", path, fn);	//����ļ���
				/* ������������ȡ�ض���ʽ���ļ�·�� */
			}
		}
	}
	closedir();
	return ret;
}

















