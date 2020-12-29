#ifndef __MP3DECODER_H__
#define __MP3DECODER_H__

#include "stdint.h"
#include "Arduino.h"
#include "Fatfs.h"


#define MP3_TITSIZE_MAX		40		//����������󳤶�
#define MP3_ARTSIZE_MAX		40		//����������󳤶�
#define MP3_FILE_BUF_SZ     5*1024	//MP3����ʱ,�ļ�buf��С

//V1.0 ˵��
//1,֧��16λ������/������MP3�Ľ���
//2,֧��CBR/VBR��ʽMP3����
//3,֧��ID3V1��ID3V2��ǩ����
//4,֧�����б�����(MP3�����320Kbps)����
#pragma pack(1) //��1�ֽڶ���
//ID3V1 ��ǩ 
typedef struct 
{
    u8 id[3];		   	//ID,TAG������ĸ
    u8 title[30];		//��������
    u8 artist[30];		//����������
	u8 year[4];			//���
	u8 comment[30];		//��ע
	u8 genre;			//���� 
}ID3V1_Tag;

//ID3V2 ��ǩͷ 
typedef struct 
{
    u8 id[3];		   	//ID
    u8 mversion;		//���汾��
    u8 sversion;		//�Ӱ汾��
    u8 flags;			//��ǩͷ��־
    u8 size[4];			//��ǩ��Ϣ��С(��������ǩͷ10�ֽ�).����,��ǩ��С=size+10.
}ID3V2_TagHead;

//ID3V2.3 �汾֡ͷ
typedef struct 
{
    u8 id[4];		   	//֡ID
    u8 size[4];			//֡��С
    u16 flags;			//֡��־
}ID3V23_FrameHead;

//MP3 Xing֡��Ϣ(û��ȫ���г���,���г����õĲ���)
typedef struct 
{
    u8 id[4];		   	//֡ID,ΪXing/Info
    u8 flags[4];		//��ű�־
    u8 frames[4];		//��֡��
	u8 fsize[4];		//�ļ��ܴ�С(������ID3)
}MP3_FrameXing;
 
//MP3 VBRI֡��Ϣ(û��ȫ���г���,���г����õĲ���)
typedef struct 
{
    u8 id[4];		   	//֡ID,ΪXing/Info
	u8 version[2];		//�汾��
	u8 delay[2];		//�ӳ�
	u8 quality[2];		//��Ƶ����,0~100,Խ������Խ��
	u8 fsize[4];		//�ļ��ܴ�С
	u8 frames[4];		//�ļ���֡�� 
}MP3_FrameVBRI;


//MP3���ƽṹ��
typedef struct 
{
    u8 title[MP3_TITSIZE_MAX];	//��������
    u8 artist[MP3_ARTSIZE_MAX];	//����������
    u32 totsec ;				//���׸�ʱ��,��λ:��
    u32 cursec ;				//��ǰ����ʱ��
	
    u32 bitrate;	   			//������
	u32 samplerate;				//������
	u16 outsamples;				//PCM�����������С(��16λΪ��λ),������MP3,�����ʵ�����*2(����DAC���)
	
	u32 datastart;				//����֡��ʼ��λ��(���ļ������ƫ��)
}__mp3ctrl;

#pragma pack()    //ȡ���Զ����ֽڶ��뷽ʽ


bool mp3_get_info(const char *pname, __mp3ctrl* pctrl);
bool MP3FileRead();


bool mp3_init(String path, File fp, uint8_t **fbp, short **buf, int **size);
void mp3_deinit();
uint32_t mp3_get_totsec();
void mp3_set_file_pos(uint32_t pos);

#define MP3INPUTBUF_SIZE	3000
#define MP3BUFFER_SIZE		2304









#endif


