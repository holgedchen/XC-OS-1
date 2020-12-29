
#include "mp3_decoder.h"
#include "mp3dec.h"
#include "BSP/BSP_MemoryPool.h"
#include "Basic/SysConfig.h"


//#define MP3_DEBUG_PRINTF(format, ...) Serial.printf(format, ##__VA_ARGS__)
#define MP3_DEBUG_PRINTF(format, ...)


extern "C" {
extern void Convert_Mono(short *buffer);	   //��������
extern void Convert_Stereo(short *buffer);
}

static uint8_t *_file_buffer_p;
static File file_p;		//�����ļ�ָ��
__mp3ctrl  *mp3ctrl;	//mp3���ƽṹ�� 


//����ID3V1 
//buf:�������ݻ�����(��С�̶���128�ֽ�)
//pctrl:MP3������
//����ֵ:0,��ȡ����
//    ����,��ȡʧ��
u8 mp3_id3v1_decode(u8* buf,__mp3ctrl *pctrl)
{
	ID3V1_Tag *id3v1tag;
	id3v1tag = (ID3V1_Tag*)buf;
	if (strncmp("TAG", (char*)id3v1tag->id, 3) == 0)//��MP3 ID3V1 TAG
	{
		if(id3v1tag->title[0])	strncpy((char*)pctrl->title, (char*)id3v1tag->title, 30);
		if(id3v1tag->artist[0])	strncpy((char*)pctrl->artist, (char*)id3v1tag->artist, 30); 
	    //�˴����������������Ϣ
	}else return 1;
	return 0;
}
//����ID3V2 
//buf:�������ݻ�����
//size:���ݴ�С
//pctrl:MP3������
//����ֵ:0,��ȡ����
//    ����,��ȡʧ��
u8 mp3_id3v2_decode(u8* buf,u32 size,__mp3ctrl *pctrl)
{
	ID3V2_TagHead *taghead;
	ID3V23_FrameHead *framehead; 
	u32 t;
	u32 tagsize;	//tag��С
	u32 frame_size;	//֡��С 
	taghead = (ID3V2_TagHead*)buf; 
	if(strncmp("ID3", (const char*)taghead->id, 3) == 0)//����ID3?
	{
		tagsize = ((u32)taghead->size[0] << 21) | ((u32)taghead->size[1] << 14)
				| ((u16)taghead->size[2] << 7) | taghead->size[3];//�õ�tag ��С
		pctrl->datastart = tagsize;		//�õ�mp3���ݿ�ʼ��ƫ����
		if(tagsize>size) tagsize = size;	//tagsize��������bufsize��ʱ��,ֻ��������size��С������
		if(taghead->mversion < 3)
		{
			MP3_DEBUG_PRINTF("not supported mversion!\r\n");
			return 1;
		}
		t = 10;
		while(t < tagsize)
		{
			framehead = (ID3V23_FrameHead*)(buf+t);
			frame_size = ((u32)framehead->size[0] << 24) | ((u32)framehead->size[1] << 16)
						|((u32)framehead->size[2] << 8) | framehead->size[3];//�õ�֡��С
 			if (strncmp("TT2", (char*)framehead->id, 3) == 0 || strncmp("TIT2", (char*)framehead->id, 4) == 0)//�ҵ���������֡,��֧��unicode��ʽ!!
			{
				strncpy((char*)pctrl->title, (char*)(buf + t + sizeof(ID3V23_FrameHead) + 1), min(frame_size - 1, MP3_TITSIZE_MAX - 1));
			}
 			if (strncmp("TP1", (char*)framehead->id, 3) == 0 || strncmp("TPE1", (char*)framehead->id, 4) == 0)//�ҵ�����������֡
			{
				strncpy((char*)pctrl->artist, (char*)(buf + t + sizeof(ID3V23_FrameHead) + 1), min(frame_size - 1, MP3_ARTSIZE_MAX - 1));
			}
			t += frame_size + sizeof(ID3V23_FrameHead);
		} 
	}else 
	{
		pctrl->datastart = 0;//������ID3,mp3�����Ǵ�0��ʼ
	}
	return 0;
}






static HMP3Decoder	Mp3Decoder;		/* mp3����ָ��	*/
static MP3FrameInfo	Mp3FrameInfo;	/* mp3֡��Ϣ  	*/

static uint8_t *read_ptr;
static int	read_offset = 0;				/* ��ƫ��ָ�� */
static int	bytes_left = 0;					/* ʣ���ֽ��� */	
static uint32_t bw;            					/* File R/W count */
static uint32_t ucFreq;			/* ����Ƶ�� */





uint32_t __totsec;
uint32_t __totframes;
uint32_t __frames_count;
//��ȡMP3������Ϣ
//pname:MP3�ļ�·��
//pctrl:MP3������Ϣ�ṹ�� 
//����ֵ:0,�ɹ�
//    ����,ʧ��
bool mp3_get_info(const char *pname, __mp3ctrl* pctrl)
{
	MP3_FrameXing* fxing;
	MP3_FrameVBRI* fvbri;
	uint8_t *buf;
	int offset = 0;
	u32 p;
	short samples_per_frame;	//һ֡�Ĳ�������
	u32 totframes;				//��֡��
	
	buf = (uint8_t*)MemPool_Malloc(5 * 1024);
	if(buf == NULL)
	{
		MP3_DEBUG_PRINTF("memory fail\r\n");
		MemPool_Free(buf);
		return false;
	}
	
	bool res = file_p.read(buf, 5 * 1024);
	if(!res)
	{
		MP3_DEBUG_PRINTF("file read fail\r\n");
		MemPool_Free(buf);
		return false;
	}
	
	//��ȡ�ļ��ɹ�,��ʼ����ID3V2/ID3V1�Լ���ȡMP3��Ϣ
	mp3_id3v2_decode(buf, 5 * 1024, pctrl);	//����ID3V2����	
	
	file_p.seekSet(file_p.getSize() - 128);
	
	file_p.read(buf, 128);//��ȡ����128���ֽ�
	
	mp3_id3v1_decode(buf, pctrl);	//����ID3V1����
	
	//ƫ�Ƶ����ݿ�ʼ�ĵط�
	//��ȡ5K�ֽ�mp3����
	file_p.seekSet(pctrl->datastart);
	
	file_p.read(buf, 5 * 1024);
	offset = MP3FindSyncWord(buf, file_p.tell());	//����֡ͬ����Ϣ
	if(offset >= 0 && MP3GetNextFrameInfo(Mp3Decoder, &Mp3FrameInfo, &buf[offset]) == 0)//�ҵ�֡ͬ����Ϣ��,����һ����Ϣ��ȡ����	
	{
		p = offset + 4 + 32;
		fvbri = (MP3_FrameVBRI*)(buf + p);
		if(strncmp("VBRI", (char*)fvbri->id, 4) == 0)//����VBRI֡(VBR��ʽ)
		{
			MP3_DEBUG_PRINTF("is VBRI\r\n");
			if (Mp3FrameInfo.version == MPEG1)
				samples_per_frame = 1152;//MPEG1,layer3ÿ֡����������1152
			else
				samples_per_frame = 576;//MPEG2/MPEG2.5,layer3ÿ֡����������576
			totframes = ((u32)fvbri->frames[0] << 24) | ((u32)fvbri->frames[1] << 16) 
						| ((u16)fvbri->frames[2] << 8) | fvbri->frames[3];//�õ���֡��
			pctrl->totsec = totframes * samples_per_frame / Mp3FrameInfo.samprate;//�õ��ļ��ܳ���
		}else	//����VBRI֡,�����ǲ���Xing֡(VBR��ʽ)
		{
			MP3_DEBUG_PRINTF("no is VBRI\r\n");
			if (Mp3FrameInfo.version == MPEG1)	//MPEG1
			{
				p = Mp3FrameInfo.nChans == 2?32:17;
				samples_per_frame = 1152;	//MPEG1,layer3ÿ֡����������1152
			}else
			{
				p = Mp3FrameInfo.nChans == 2?17:9;
				samples_per_frame = 576;		//MPEG2/MPEG2.5,layer3ÿ֡����������576
			}
			p += offset + 4;
			fxing = (MP3_FrameXing*)(buf + p);
			if(strncmp("Xing", (char*)fxing->id,4) == 0 || strncmp("Info", (char*)fxing->id, 4) == 0)//��Xng֡
			{
				if(fxing->flags[3] & 0X01)//������frame�ֶ�
				{	
					totframes = ((u32)fxing->frames[0] << 24) | ((u32)fxing->frames[1] << 16)
								| ((u16)fxing->frames[2] << 8) | fxing->frames[3];//�õ���֡��
					pctrl->totsec = totframes * samples_per_frame / Mp3FrameInfo.samprate;//�õ��ļ��ܳ���
					MP3_DEBUG_PRINTF("XING frame\r\n");
				}else	//��������frames�ֶ�
				{
					pctrl->totsec = file_p.getSize() / (Mp3FrameInfo.bitrate / 8);
					MP3_DEBUG_PRINTF("XING no frame\r\n");
				}
			}else 		//CBR��ʽ,ֱ�Ӽ����ܲ���ʱ��
			{
				pctrl->totsec = file_p.getSize() / (Mp3FrameInfo.bitrate / 8);
				MP3_DEBUG_PRINTF("XING CBR\r\n");
			}
		}
		pctrl->bitrate = Mp3FrameInfo.bitrate;			//�õ���ǰ֡������
		pctrl->samplerate = Mp3FrameInfo.samprate; 	//�õ�������. 
		if(Mp3FrameInfo.nChans == 2)
			pctrl->outsamples = Mp3FrameInfo.outputSamps; //���PCM��������С 
		else
			pctrl->outsamples = Mp3FrameInfo.outputSamps * 2; //���PCM��������С,���ڵ�����MP3,ֱ��*2,����Ϊ˫�������
	}else//δ�ҵ�ͬ��֡
	{
		MP3_DEBUG_PRINTF("Synchronization frame not found\r\n");
		MemPool_Free(buf);
		return false;
	}
	__totframes = totframes;
	__totsec = pctrl->totsec;
	MemPool_Free(buf);
	return true;
}



/*
//�õ���ǰ����ʱ��
//fx:�ļ�ָ��
//mp3x:mp3���ſ�����
void mp3_get_curtime(FIL*fx,__mp3ctrl *mp3x)
{
	u32 fpos=0;  	 
	if(fx->File_CurOffset>mp3x->datastart)fpos=fx->File_CurOffset-mp3x->datastart;	//�õ���ǰ�ļ����ŵ��ĵط� 
	mp3x->cursec=fpos*mp3x->totsec/(fx->File_Size-mp3x->datastart);	//��ǰ���ŵ��ڶ�������?	
	Wav_Handle.DataPosition;
}Wav_Handle.DataSize
*/


static short *mp3_tempbuf;
int outputSamps;
int *_mp3_dma_size;






bool MP3FileRead()
{
	short 	*Outpcmbuf;			 //�������ָ��
	
	//MP3_DEBUG_PRINTF("%d, %d\r\n", file.tell(), bytes_left);	
	//Ѱ��֡ͬ�������ص�һ��ͬ���ֵ�λ��
	read_offset = MP3FindSyncWord(read_ptr, bytes_left);
	//û���ҵ�ͬ����
	if(read_offset < 0)
	{
		bw = file_p.read(_file_buffer_p, MP3INPUTBUF_SIZE);
		if(file_p.get_error() != FR_OK)
		{
			MP3_DEBUG_PRINTF("��ȡʧ�� -> %d\r\n", file_p.get_error());
			return false;	//Ӧ�Ǳ����˳�
		}
		read_ptr = _file_buffer_p;
		bytes_left = bw;
		//MP3_DEBUG_PRINTF("read_offset < 0");
		return false;	//	���½������֡ͬ��
	}
	
	read_ptr += read_offset;					//ƫ����ͬ���ֵ�λ��
	bytes_left -= read_offset;				//ͬ����֮������ݴ�С	
	if(bytes_left < 1024)							//��������
	{
		/* ע������ط���Ϊ���õ���DMA��ȡ������һ��Ҫ4�ֽڶ���  */
		int i = (uint32_t)(bytes_left) & 3;					//�ж϶�����ֽ�
		if(i) i = 4 - i;									//��Ҫ������ֽ�
		memcpy(_file_buffer_p + i, read_ptr, bytes_left);		//�Ӷ���λ�ÿ�ʼ����
		read_ptr = _file_buffer_p + i;						//ָ�����ݶ���λ��
		//��������
		bw = file_p.read(_file_buffer_p + bytes_left + i, MP3INPUTBUF_SIZE - bytes_left - i);
		bytes_left += bw;									//��Ч��������С
		//MP3_DEBUG_PRINTF("bytes_left < 1024");
	}
	
	//��ʼ���� ������mp3����ṹ�塢������ָ�롢��������С�������ָ�롢���ݸ�ʽ
	
	Outpcmbuf = mp3_tempbuf;
	int err = MP3Decode(Mp3Decoder, &read_ptr, &bytes_left, Outpcmbuf, 0);
	
	//������
	if (err != ERR_MP3_NONE)									
	{
		switch (err)
		{
			case ERR_MP3_INDATA_UNDERFLOW:
				MP3_DEBUG_PRINTF("ERR_MP3_INDATA_UNDERFLOW\r\n");
				bw = file_p.read(_file_buffer_p, MP3INPUTBUF_SIZE);
				read_ptr = _file_buffer_p;
				bytes_left = bw;
				break;
			case ERR_MP3_MAINDATA_UNDERFLOW:
				/* do nothing - next call to decode will provide more mainData */
				MP3_DEBUG_PRINTF("ERR_MP3_MAINDATA_UNDERFLOW\r\n");
				break;		
			default:
				MP3_DEBUG_PRINTF("UNKNOWN ERROR:%d\r\n", err);	
			
				// ������֡
				if (bytes_left > 0)
				{
					bytes_left --;
					read_ptr ++;
				}
			
				break;
		}
	}
	else		//�����޴���׼�������������PCM
	{
		MP3GetLastFrameInfo(Mp3Decoder, &Mp3FrameInfo);	//��ȡ������Ϣ				
		/* �����DAC */
		outputSamps = Mp3FrameInfo.outputSamps;	//PCM���ݸ���
		if (outputSamps > 0)
		{
			if (Mp3FrameInfo.nChans == 1)	//������
			{
				//������������Ҫ����һ�ݵ���һ������
				/*for (int i = outputSamps - 1; i >= 0; i--)
				{
					Outpcmbuf[i * 2] = Outpcmbuf[i];
					Outpcmbuf[i * 2 + 1] = Outpcmbuf[i];
				}
				outputSamps *= 2;*/
				Convert_Mono((short*)Outpcmbuf);
				outputSamps *= 2;
			}else	//������
			{
				//Convert_Stereo((short*)Outpcmbuf);
			}
		}
		
		/* ���ݽ�����Ϣ���ò����� */
		if (Mp3FrameInfo.samprate == 0)
		{
			MP3_DEBUG_PRINTF("samprate error\r\n");
			return false;
		}else if (Mp3FrameInfo.samprate != ucFreq)	//������ 
		{
			ucFreq = Mp3FrameInfo.samprate;
			
			MP3_DEBUG_PRINTF(" \r\n Bitrate       %dKbps", Mp3FrameInfo.bitrate/1000);
			MP3_DEBUG_PRINTF(" \r\n Samprate      %dHz", ucFreq);
			MP3_DEBUG_PRINTF(" \r\n BitsPerSample %db", Mp3FrameInfo.bitsPerSample);
			MP3_DEBUG_PRINTF(" \r\n nChans        %d", Mp3FrameInfo.nChans);
			MP3_DEBUG_PRINTF(" \r\n Layer         %d", Mp3FrameInfo.layer);
			MP3_DEBUG_PRINTF(" \r\n Version       %d", Mp3FrameInfo.version);
			MP3_DEBUG_PRINTF(" \r\n OutputSamps   %d", Mp3FrameInfo.outputSamps);
			MP3_DEBUG_PRINTF("\r\n");
			//I2S_AudioFreq_Default = 2��������֡��ÿ�ζ�Ҫ������
			if(ucFreq >= I2S_AudioFreq_Default)
			{
				//���ݲ������޸�I2S����
				Timer_SetInterruptFreqUpdate(XC_TIM_WAVPLAYER, ucFreq);
			}
		}
		
		__frames_count++;
		
		//(��ʱ��*�Ѳ��Ŵ�����)/�ܴ�����=�Ѳ�������
		/*MP3_DEBUG_PRINTF("%d / %d, %d / %d\r\n",
					__frames_count,
					__totframes,
					(__totsec * __frames_count) / __totframes,
					__totsec);*/
		
		*_mp3_dma_size = outputSamps;
		//MP3_DEBUG_PRINTF("OK, %d\r\n", *_mp3_dma_size);
		return true;
	}//else ��������
	
	return false;
}



bool mp3_init(String path, File fp, uint8_t **fbp, short **buf, int **size)
{
	file_p = fp;
	
	_file_buffer_p = *fbp;
	
	//��ʼ��MP3������
	Mp3Decoder = MP3InitDecoder();
	if(Mp3Decoder == NULL)
	{
		MP3_DEBUG_PRINTF("Initializes the Helix decoding library equipment\r\n");
		return false;	// ֹͣ����
	}
	
	//��ȡMP3��Ϣ
	mp3ctrl = (__mp3ctrl*)MemPool_Malloc(sizeof(__mp3ctrl));
	memset(mp3ctrl,0,sizeof(__mp3ctrl));//�������� 
	bool res = mp3_get_info(path.c_str(), mp3ctrl);
	if(!res)
	{
		return false;
	}
	/*
	MP3_DEBUG_PRINTF(mp3ctrl->totsec);
	MP3_DEBUG_PRINTF(mp3ctrl->bitrate);
	MP3_DEBUG_PRINTF(mp3ctrl->samplerate);
	MP3_DEBUG_PRINTF(mp3ctrl->outsamples);
	MP3_DEBUG_PRINTF(mp3ctrl->datastart);
	*/
	MemPool_Free(mp3ctrl);
	
	mp3_tempbuf = (short*)MemPool_Malloc(MP3BUFFER_SIZE*2);
	if(mp3_tempbuf == NULL)
	{
		MP3_DEBUG_PRINTF("mp3_tempbuf NULL\r\n");
		return false;	/* ֹͣ���� */
	}
	
	_mp3_dma_size = (int*)MemPool_Malloc(sizeof(int));
	if(_mp3_dma_size == NULL)
	{
		MP3_DEBUG_PRINTF("_mp3_dma_size NULL\r\n");
		return false;	/* ֹͣ���� */
	}
	
	//��������ۼ� ÿ����һ�����ھ�+1
	__frames_count = 0;
	
	file_p.seekSet(0);
	//��ʼ���ļ���ȥ��MP3ͷ�ļ�
	file_p.read( _file_buffer_p, MP3INPUTBUF_SIZE);
	if(file_p.get_error() != FR_OK)
	{
		MemPool_Free(_file_buffer_p);
		_file_buffer_p = NULL;
		MP3_DEBUG_PRINTF("FileRead%sfail -> %d\r\n", file_p.get_error());
		MP3FreeDecoder(Mp3Decoder);
		return false;
	}
	
	read_ptr = _file_buffer_p;
	bytes_left = bw;

	ucFreq = 44100;
	
	*buf = mp3_tempbuf;
	*size = _mp3_dma_size;
	
	//MP3_DEBUG_PRINTF("%X %X\r\n", buf, size);
	//MP3_DEBUG_PRINTF("%X %X\r\n", mp3_tempbuf, _mp3_dma_size);
	return true;
}




void mp3_deinit()
{
	MP3FreeDecoder(Mp3Decoder);
}


void mp3_set_file_pos(uint32_t pos)
{
	file_p.seekSet(pos);
	bw = file_p.read(_file_buffer_p, MP3INPUTBUF_SIZE);
	read_ptr = _file_buffer_p;
	bytes_left = bw;
}




uint32_t mp3_get_totsec()
{
	return __totsec;
}


















