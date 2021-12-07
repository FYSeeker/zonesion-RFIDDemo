/*********************************************************************************************
* �ļ���uart.c
* ���ߣ�fuyou 2018.03.21
* ˵����etc������
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/

/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include "eh.h"
#include "sys_init.h"
#include "uart.h"
#include "ic.h"


/*********************************************************************************************
* ���ƣ�CRC16_CCITT_FALSE
* ���ܣ�CRCУ��
* ��������
* ���أ�У����
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
unsigned short CRC16_CCITT_FALSE(unsigned char *puchMsg, unsigned int s1,unsigned int s2)  
{  
	unsigned short wCRCin = 0xFFFF;  
	unsigned short wCPoly = 0x1021;  
	unsigned char  wChar = 0;  

	for(u8 x=s1;x<s2+1;x++)
	{  
		wChar = puchMsg[x];  
		wCRCin ^= (wChar << 8);  
		for(int i = 0;i < 8;i++)  
		{  
			if(wCRCin & 0x8000)  
				wCRCin = (wCRCin << 1) ^ wCPoly;  
			else  
				wCRCin = wCRCin << 1;  
		}  
	}  
  return (wCRCin) ;  
}


/*********************************************************************************************
* ���ƣ�mcuRead_etcUII
* ���ܣ���UII
* ������EPC����
* ���أ�1���ɹ���0��ʧ��
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
unsigned char mcuRead_etcUII(unsigned char* UIIarray)
{	
	unsigned char i,check_temp[3];
	u16 crc_16;
	/*��ETC UIIָ��*/
	u8 readEtcUII[9] = {0xbb,0x00,0x22,0x00,0x00,
						0x7e,0x00,0x00};

	crc_16 = CRC16_CCITT_FALSE(readEtcUII,1,5);				//����У��
	readEtcUII[6] = (unsigned char)(crc_16 >> 8);
	readEtcUII[7] = (unsigned char)(crc_16 & 0xff);
	
	Uart0_Send_LenString(readEtcUII,8);							//���Ͷ���������
	delay_ms(50);
		
	if((UART0_RX_STA&0x80)==0x80)
	{	
		if((U0RX_Buf[1]==0x01)&&(U0RX_Buf[2]==0x22))			//�����ɹ�
		{	
			crc_16 = CRC16_CCITT_FALSE(U0RX_Buf,1,(UART0_RX_STA&0x3F)-3);//����У��
			check_temp[0] = (unsigned char)(crc_16 >> 8);
			check_temp[1] = (unsigned char)(crc_16 & 0xff);
			
			if((check_temp[0]==U0RX_Buf[(UART0_RX_STA&0x3F)-2])&&(check_temp[1]==U0RX_Buf[(UART0_RX_STA&0x3F)-1]))//У����ȷ
			{	
				for(i=0;i<12;i++)
				{
					UIIarray[i] = U0RX_Buf[i+7];				//��ȡ����
				}
				UART0_RX_STA = 0;
				return 1;
			}
		}
		UART0_RX_STA = 0;
	}
	return 0;
}


/*********************************************************************************************
* ���ƣ�mcuRead_EtcMemory
* ���ܣ���ָ������
* ���������룬���ţ��洢����(mb)����ʼ��ַ����ȡ���ȣ���ȡ����
* ���أ�1���ɹ���0��ʧ��
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
unsigned char mcuRead_EtcMemory(	unsigned char* password,unsigned char* epc,unsigned char mb,
							 		unsigned short sAdd,unsigned short len,unsigned char* Data)
{
	u16 i;
	unsigned char check_temp[3];
	/*����������*/
	unsigned char readMem[32] = {	0xbb,0x00,0x29,0x00,0x17,
									password[0],password[1],password[2],password[3],
									0x00,0x0c,epc[0],epc[1],epc[2],epc[3],epc[4],epc[5],epc[6],epc[7],epc[8],epc[9],epc[10],epc[11],
									mb,(u8)(sAdd>>8),(u8)(sAdd&0xff),(u8)(len>>8),(u8)(len&0xff),0x7e,0x00,0x00};	
	
	u16 crc_16 = CRC16_CCITT_FALSE(readMem,1,28);				//����У��
	readMem[29] = (unsigned char)(crc_16 >> 8);				//����У��
	readMem[30] = (unsigned char)(crc_16 & 0xff);				//����У��
	Uart0_Send_LenString(readMem,31);							//���Ͷ���������
	delay_ms(50);
	
	if((UART0_RX_STA&0x80)==0x80)
	{
		if((U0RX_Buf[1]==0x01)&&(U0RX_Buf[2]==0x29))									//�����ɹ�
		{
			crc_16 = CRC16_CCITT_FALSE(U0RX_Buf,1,(UART0_RX_STA&0x3F)-3);//CRCУ��
			check_temp[0] = (unsigned char)(crc_16 >> 8);
			check_temp[1] = (unsigned char)(crc_16 & 0xff);
			
			if((check_temp[0]==U0RX_Buf[(UART0_RX_STA&0x3F)-2])&&(check_temp[1]==U0RX_Buf[(UART0_RX_STA&0x3F)-1]))//У����ȷ
			{	
				for(i=0;i<len*2;i++)
				{
					Data[i] = U0RX_Buf[i+5];					//��ȡ��������
				}
				UART0_RX_STA = 0;
				return 1;
			}
		}
		UART0_RX_STA = 0;
	}
	return 0;
}


/*********************************************************************************************
* ���ƣ�mcuWrite_Etc2Byte
* ���ܣ�дָ������,2���ֽ�
* ���������룬���ţ��洢���򣬿�ʼ��ַ������
* ���أ�1���ɹ���0��ʧ��
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
unsigned char mcuWrite_Etc2Byte(	unsigned char* password,unsigned char* epc,unsigned char mb,
							 		unsigned short sAdd,unsigned char* Data)
{
	unsigned char check_temp[3];
	/*����������*/
	unsigned char writeMem[34] = {	0xbb,0x00,0x46,0x00,0x19,
									password[0],password[1],password[2],password[3],
									0x00,0x0c,epc[0],epc[1],epc[2],epc[3],epc[4],epc[5],epc[6],epc[7],epc[8],epc[9],epc[10],epc[11],
									mb,(u8)(sAdd>>8),(u8)(sAdd&0xff),0x00,0x01,Data[0],Data[1],
									0x7e,0x00,0x00};	
	
	u16 crc_16 = CRC16_CCITT_FALSE(writeMem,1,30);				//����У��
	writeMem[31] = (unsigned char)(crc_16 >> 8);				//����У��
	writeMem[32] = (unsigned char)(crc_16 & 0xff);				//����У��
	
	Uart0_Send_LenString(writeMem,33);							//����д����
	delay_ms(50);
	
	if((UART0_RX_STA&0x80)==0x80)
	{
		if((U0RX_Buf[1]==0x01)&&(U0RX_Buf[2]==0x46)&&(U0RX_Buf[5]==0x00))//�����ɹ�
		{
			crc_16 = CRC16_CCITT_FALSE(U0RX_Buf,1,(UART0_RX_STA&0x3F)-3);//CRCУ��
			check_temp[0] = (unsigned char)(crc_16 >> 8);
			check_temp[1] = (unsigned char)(crc_16 & 0xff);
			
			if((check_temp[0]==U0RX_Buf[(UART0_RX_STA&0x3F)-2])&&(check_temp[1]==U0RX_Buf[(UART0_RX_STA&0x3F)-1]))//У����ȷ
			{	
				UART0_RX_STA = 0;
				return 1;
			}
		}
		UART0_RX_STA = 0;
	}
	return 0;
}



/*********************************************************************************************
* ���ƣ�etc_userinfo
* ���ܣ���ȡ�û���Ϣ
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void etc_userinfo(void)
{	
	unsigned char i;
	
	/*�����ɹ���������*/
	unsigned char sendCorrect[19] = {	0xbb,0x0E,0xE1,
										0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
										0x00,0x00,
										0x00};
	/*��������*/
	unsigned char etcPassword[5] = {0x00,0x00,0x00,0x00};
	unsigned char etcEPC[13] = {0};
	unsigned char etcData[3] = {0};
	
	/*��EPC*/
	if(mcuRead_etcUII(etcEPC))
	{		
		for(i=0;i<12;i++)
		{
			sendCorrect[i+3] = etcEPC[i];						//��ȡEPC
		}
		
		/*�����*/
		if(mcuRead_EtcMemory(etcPassword,etcEPC,0x03,0x0000,0x0001,etcData))
		{
			for(i=0;i<2;i++)
			{
				sendCorrect[i+15] = etcData[i];						//��ȡ���
			}
			sendCorrect[17] = xor_calculate(sendCorrect,1,16);			//����У��
			Uart1_Send_LenString(sendCorrect,18);					//�����ɹ�
		}
	}	
	UART1_RX_STA = 0;	
}


/*********************************************************************************************
* ���ƣ�etc_topUp
* ���ܣ�etc��ֵ
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void etc_topUp(u8* etcData)
{
	u8 i,CorrectFlag=0;
	/*����ʧ�ܷ�������*/
	unsigned char readError[5] = {	0xbb,0x00,0xff,
									0x00};
	/*�����ɹ���������*/
	unsigned char sendCorrect[5] = {	0xbb,0x00,0x00,
										0x00};
	/*����*/
	unsigned char etcPassword[5] = {0x00,0x00,0x00,0x00};
	/*EPC*/
	unsigned char etcEPC[13] = {0};
		
	for(i=0;i<U1RX_Buf[1];i++)
	{
		etcData[1-i] = U1RX_Buf[((UART1_RX_STA&0x3F)-2)-i];		//���³�ֵ����	
	}
	
    //��5�Σ���߳ɹ���
    for(i=0;i<5;i++)
    {
        /*��EPC��д���*/
        if((mcuRead_etcUII(etcEPC)) && (mcuWrite_Etc2Byte(etcPassword,etcEPC,0x03,0x0000,etcData)))
        {				
            sendCorrect[3] = xor_calculate(sendCorrect,1,2);			//����У��
            Uart1_Send_LenString(sendCorrect,4);					//���سɹ���Ϣ
            CorrectFlag = 1;                                        //��ǳɹ�
            break;
        }
    }
    
	if(!CorrectFlag)
	{
		readError[3] = xor_calculate(readError,1,2);				//����У��
		Uart1_Send_LenString(readError,4);						//����ʧ����Ϣ
	}
}



/*********************************************************************************************
* ���ƣ�sticks_ioInit
* ���ܣ�����io��ʼ����P00��P01
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void sticks_ioInit()
{
	P0SEL &= ~(1<<0);											//ͬ��io
	P0SEL &= ~(1<<1);											//ͬ��io
	P0DIR |= (1<<0);											//����Ϊ���
	P0DIR |= (1<<1);
}



/*********************************************************************************************
* ���ƣ�sticks_up
* ���ܣ�̧��
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void sticks_up()
{
	P0_0 = 0;
	P0_1 = 1;
	delay_ms(400);
	P0_0 = 0;
	P0_1 = 0;	
}

/*********************************************************************************************
* ���ƣ�sticks_ioInit
* ���ܣ��ո�
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void sticks_down()
{
	P0_0 = 1;
	P0_1 = 0;
	delay_ms(400);
	P0_0 = 0;
	P0_1 = 0;		
}


/*********************************************************************************************
* ���ƣ�pc_etc
* ���ܣ�pc����etc��
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void pc_etc()
{	
	u8 check_temp;
	static u16 x2=0;
	u8 etcData[3] = {0};
	
	if((UART1_RX_STA&0x80)==0x80)								//���ݽ������
	{
		if(U1RX_Buf[0]==0xfa)									//ȷ������ͷ
		{		
			check_temp = xor_calculate(U1RX_Buf,1,(UART1_RX_STA&0x3F)-2);//���У��
			
			if(check_temp==U1RX_Buf[(UART1_RX_STA&0x3F)-1])		//У����ȷ
			{
				switch(U1RX_Buf[2])
				{
					case 0xe2:	
						/*��ȡ��ֵ����*/
						etcData[0] = U1RX_Buf[(UART1_RX_STA&0x3F)-3];
						etcData[1] = U1RX_Buf[(UART1_RX_STA&0x3F)-2];
						/*��ֵ*/
						etc_topUp(etcData);						
						break;
					case 0xe3:
						sticks_up();
						break;
					case 0xe4:	
						sticks_down();
						break;
				}
			}
		}
		UART1_RX_STA = 0;
	}
	else
	{
		if(x2>999)
		{
			etc_userinfo();										//��ȡ�û���Ϣ
			
			LED2 = 0;
			delay_ms(1);											//����
			LED2 = 1;
			x2 = 0;
		}	
		
		delay_ms(1);
		x2++;
	}
}