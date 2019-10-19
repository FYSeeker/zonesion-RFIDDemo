/*********************************************************************************************
* �ļ���el.c
* ���ߣ�fuyou 2018.5.23
* ˵����el����ˢ��ϵͳ������el����ϵͳ����  
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/

/*********************************************************************************************
* ͷ�ļ�
*********************************************************************************************/
#include "el.h"
#include "led.h"
#include "uart.h"
#include "oled.h"
#include "relay.h"
#include "time.h"


/*ϵͳģʽ,0-->����ˢ��ϵͳ��1-->����ϵͳ*/
u8 sysMode = 0;

/*ic��ģʽ,0-->����ģʽ��1-->��ֵģʽ*/
u8 icMode = 0;
/*ic��A������*/
u8 icPasswordA[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
/*ic��B������*/
u8 icPasswordB[6] = {0xff,0xff,0xff,0xff,0xff,0xff};

/*********************************************************************************************
* ���ƣ�buzzer_ioInit
* ���ܣ�io��ʼ����P04
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void buzzer_ioInit()
{
	P0SEL &= ~(1<<4);											//ͨ��io
	P0DIR |= (1<<4);											//����Ϊ���
}


/*********************************************************************************************
* ���ƣ�xor_count
* ���ܣ����У�����
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
unsigned char xor_count(unsigned char* array,unsigned char s1,unsigned char s2)
{
	unsigned char i,check_temp;
	
	check_temp = array[s1];
	for(i = s1+1;i<(s2+1);i++)
	{	
		check_temp ^= array[i];									//���У��
	}
	
	return check_temp;
}



/*********************************************************************************************
* ���ƣ�mcuRead_UID
* ���ܣ���UID
* ��������ȡ��ַ������BUF
* ���أ�1���ɹ���0��ʧ��
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
unsigned char mcuRead_UID(unsigned char icAdd,unsigned char* UIDarray)
{	
	unsigned char i=0,check_temp=0;
	/*��IC������*/
	unsigned char readUID[7] = {	0xAB,0xBA,
									icAdd,0x10,0x00,
									0x00};	
	readUID[5] = xor_count(readUID,2,4);						//����У��
	Uart0_Send_LenString(readUID,6);							//���Ͷ���������
    
    while((UART0_RX_STA&0x80)!=0x80)
    {
        delay_ms(1);
        i++;
        if(i>49) break;
    }
		
	if((UART0_RX_STA&0x80)==0x80)
	{	
		if(U0RX_Buf[3]==0x81)									//�����ɹ�
		{	
			check_temp = xor_count(U0RX_Buf,2,(UART0_RX_STA&0x7f)-1);//���У��
			
			if(check_temp==U0RX_Buf[UART0_RX_STA&0x7f])		//У����ȷ
			{	
				for(i=0;i<4;i++)
				{
					UIDarray[i] = U0RX_Buf[i+5];				//��ȡ����
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
* ���ƣ�mcuRead_memory
* ���ܣ���ָ������
* ������IC����ַ���������飬A/B�飬���룬����
* ���أ�1���ɹ���0��ʧ��
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
unsigned char mcuRead_memory(	unsigned char icAdd,unsigned char M1,unsigned char M2,
							 	unsigned char group,unsigned char* password,unsigned char* Data)
{
	unsigned char i=0,check_temp=0;
	/*����������*/
	unsigned char readMem[16] = {	0xAB,0xBA,
									icAdd,0x12,0x09,
									M1,M2,group,
									password[0],password[1],password[2],password[3],password[4],password[5],
									0x00};	
	
	readMem[14] = xor_count(readMem,2,13);						//����У��
	Uart0_Send_LenString(readMem,15);							//���Ͷ���������
    
    while((UART0_RX_STA&0x80)!=0x80)
    {
        delay_ms(1);
        i++;
        if(i>49) break;
    }
	
	if((UART0_RX_STA&0x80)==0x80)
	{
		if(U0RX_Buf[3]==0x81)									//�����ɹ�
		{
			check_temp = xor_count(U0RX_Buf,2,(UART0_RX_STA&0x7f)-1);//���У��
			
			if(check_temp==U0RX_Buf[UART0_RX_STA&0x7f])		//У����ȷ
			{		
				for(i=0;i<16;i++)
				{
					Data[i] = U0RX_Buf[i+5+2];					//��ȡ��������
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
* ���ƣ�mcuWrite_memory
* ���ܣ�д������
* ������IC����ַ���������飬A/B�飬���룬����
* ���أ�1���ɹ���0��ʧ��
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
unsigned char mcuWrite_memory(	unsigned char icAdd,unsigned char M1,unsigned char M2,
							 	unsigned char group,unsigned char* password,unsigned char* Data)
{
	unsigned char i=0,check_temp=0;
	/*����������*/
	unsigned char readMem[32] = {	0xAB,0xBA,
									icAdd,0x13,0x19,
									M1,M2,group,
									password[0],password[1],password[2],password[3],password[4],password[5],
									Data[0],Data[1],Data[2],Data[3],Data[4],Data[5],Data[6],Data[7],
									Data[8],Data[9],Data[10],Data[11],Data[12],Data[13],Data[14],Data[15],
									0x00};	
	
	readMem[30] = xor_count(readMem,2,29);						//����У��
	Uart0_Send_LenString(readMem,31);							//���Ͷ���������
    
    while((UART0_RX_STA&0x80)!=0x80)
    {
        delay_ms(1);
        i++;
        if(i>49) break;
    }
	
	if((UART0_RX_STA&0x80)==0x80)
	{
		if(U0RX_Buf[3]==0x81)									//�����ɹ�
		{
			check_temp = xor_count(U0RX_Buf,2,(UART0_RX_STA&0x7f)-1);//���У��
			
			if(check_temp==U0RX_Buf[UART0_RX_STA&0x7f])		//У����ȷ
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
* ���ƣ�reported_icCardNumber
* ���ܣ��ϱ�IC����
* ��������
* ���أ�0-->��ȡʧ�ܣ�1-->��ȡ�ɹ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
u8 reported_icCardNumber(void)
{
	unsigned char i;
	
	/*�����ɹ���������*/
	unsigned char sendCorrect[9] = {	0xBF,0x04,0x0A,
										0x00,0x00,0x00,0x00,
										0x00};
	
	unsigned char cardUID[5] = {0};
	
	if(mcuRead_UID(0x00,cardUID))								//��ȡ�ɹ�
	{	
		for(i=0;i<4;i++)
		{
			sendCorrect[i+3] = cardUID[i];						//��ȡ����
		}
		sendCorrect[7] = xor_count(sendCorrect,1,6);			//����У��
		Uart1_Send_LenString(sendCorrect,8);					//�����ɹ�
        return 1;
	}
    return 0;
}


/*********************************************************************************************
* ���ƣ�reported_icRemaining
* ���ܣ��ϱ�ic�����
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void reported_icRemaining(void)
{
	unsigned char i;
	
	/*�����ɹ���������*/
	unsigned char sendCorrect[21] = {	0xBF,0x10,0x0B,
										0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
										0x00};
	unsigned char ICData[17] = {0};
	
	if(mcuRead_memory(0x00,0x01,0x01,0x0A,icPasswordA,ICData))
	{
		for(i=0;i<16;i++)
		{
			sendCorrect[i+3] = ICData[i];						//��ȡ��������
		}
		sendCorrect[19] = xor_count(sendCorrect,1,18);			//����У��
		Uart1_Send_LenString(sendCorrect,20);					//�����ɹ�
	}
}



/*********************************************************************************************
* ���ƣ�reported_icInfo
* ���ܣ��ϱ��û���Ϣ�����ţ����
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void reported_icInfo()
{
	unsigned char i;
	
	/*�����ɹ���������*/
	unsigned char sendCorrect[13] = {	0xBF,0x08,0x0B,
										0x00,0x00,0x00,0x00,	//����
										0x00,0x00,0x00,0x00,	//���
										0x00};
	/*��ſ���*/
	unsigned char cardUID[5] = {0};
    /*�������*/
	unsigned char ICData[17] = {0};
	
	if(mcuRead_UID(0x00,cardUID))								//��ȡ����
	{	
		for(i=0;i<4;i++)
		{
			sendCorrect[i+3] = cardUID[i];						//��ȡ����
		}
		
		if(mcuRead_memory(0x00,0x01,0x01,0x0a,icPasswordA,ICData))//�����
		{
			for(i=0;i<4;i++)
			{
				sendCorrect[10-i] = ICData[15-i];				//��ȡ���
			}
			sendCorrect[11] = xor_count(sendCorrect,1,10);		//����У��
			Uart1_Send_LenString(sendCorrect,12);				//������Ϣ
            
            /*����������*/
            P0_4 = 0;
            delay_ms(20);
            P0_4 = 1;          
		}
	}
}



/*********************************************************************************************
* ���ƣ�update_icData
* ���ܣ��޸�ic������
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void update_icData()
{
	u8 i,group;
    u8* password;
	/*����ʧ�ܷ�������*/
	unsigned char readError[6] = {	0xBF,0x01,0x0b,
									0xff,
									0x00};
	/*�����ɹ���������*/
	unsigned char sendCorrect[6] = {	0xBF,0x01,0x0B,
										0x00,
										0x00};
	/*Ҫд������*/
	unsigned char WriteData[17] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
									0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		
    /*����ģʽѡ��A/B������*/
    if(icMode==1)
    {
        group = 0x0b;
        password = icPasswordB;
    }
    else
    {     
        group = 0x0a;
        password = icPasswordA;
    }
    
    //��������
	for(i=0;i<U1RX_Buf[1];i++)
	{
		WriteData[15-i] = U1RX_Buf[((UART1_RX_STA&0x7f)-1)-i];		
	}
    
	//д����
	if(mcuWrite_memory(0x00,0x01,0x01,group,password,WriteData))
	{
		sendCorrect[4] = xor_count(sendCorrect,1,3);			//����У��
		Uart1_Send_LenString(sendCorrect,5);					//�����ɹ�
		UART1_RX_STA = 0;
	}
	else
	{
		readError[4] = xor_count(readError,1,3);				//����У��
		Uart1_Send_LenString(readError,5);						//����ʧ��
		UART1_RX_STA = 0;
	}
}



/*********************************************************************************************
* ���ƣ�update_icPassword
* ���ܣ�����ic������
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void update_icPassword()
{
    u8 i=0;
    
    if(icMode==1)
    {
        for(i=0;i<6;i++)
        {
           icPasswordB[i] = U1RX_Buf[3+i];//����B������
        }
    }
    else
    {                           
        for(i=0;i<6;i++)
        {
           icPasswordA[i] = U1RX_Buf[3+i];//����A������
        }
    }
}



/*********************************************************************************************
* ���ƣ�update_price
* ���ܣ�����Ʊ��
* ������price��Ʊ��
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void update_price(unsigned short* price)
{	
	/*�����ɹ���������*/
	unsigned char sendCorrect[6] = {	0xBF,0x00,0x0C,
										0x00};
	
	*price = U1RX_Buf[3]*256 + U1RX_Buf[4];
	
	sendCorrect[3] = xor_count(sendCorrect,1,2);//����У��
	Uart1_Send_LenString(sendCorrect,4);		//�����ɹ�	
}



/*********************************************************************************************
* ���ƣ�mcuRead_idCard
* ���ܣ���ȡID������
* ��������ȡ��ַ������BUF
* ���أ�1���ɹ���0��ʧ��
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
unsigned char mcuRead_idCard(unsigned char icAdd,unsigned char* idBuf)
{	
	unsigned char i=0,check_temp=0;
	/*��IC������*/
	unsigned char readIdCommand[7] = {	0xAB,0xBA,
                                        icAdd,0x15,0x00,
                                        0x00};	
	readIdCommand[5] = xor_count(readIdCommand,2,4);						//����У��
	Uart0_Send_LenString(readIdCommand,6);							//���Ͷ���������
    
    while((UART0_RX_STA&0x80)!=0x80)
    {
        delay_ms(1);
        i++;
        if(i>49) break;
    }
    
	if((UART0_RX_STA&0x80)==0x80)
	{	
		if(U0RX_Buf[3]==0x81)									//�����ɹ�
		{	
			check_temp = xor_count(U0RX_Buf,2,(UART0_RX_STA&0x7f)-1);//���У��
			if(check_temp==U0RX_Buf[UART0_RX_STA&0x7f])		    //У����ȷ
			{	
				for(i=0;i<4;i++)
				{
					idBuf[i] = U0RX_Buf[i+5];				//��ȡ����
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
* ���ƣ�oled_display
* ���ܣ�oled��ʾ
* ����������ϵͳƱ��
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void oled_display(u16 price,u8* idCardNumber)
{
    if(sysMode==1)
    {
        u32 CardNumber=0;
        u8 CardNumber_temp[11]={0};
        
        //16����ת10����
        for(u8 i=0;i<4;i++)
        {
            CardNumber += idCardNumber[i]*pow(256,i);
        }
        //����ת�ַ�
        CardNumber_temp[10] = '\0';
        for(u8 i=0;i<10;i++)
        {
            CardNumber_temp[9-i] = (CardNumber/(u32)pow(10,i))%10 + '0';
        }
        //��ʾ��Ϣ
        OLED_ShowString(1,1,"CardNumber:",11);							
        OLED_ShowString(1,32,CardNumber_temp,10);						//��ʾ���ţ�10����
    }
    else
    {
        u8 priceTemp[5];
        
        priceTemp[4] = '\0';
        for(u8 i=0;i<4;i++)
        {
            priceTemp[4-i] = (price/(u32)pow(10,i))%10 + '0';
        }
        //��ʾ��Ϣ    
        OLED_ShowString(20,1,"Fare:",6);							
        OLED_ShowString(50,1,priceTemp,6);							//��ʾƱ�ۣ�10����
    }
}


/*********************************************************************************************
* ���ƣ�reported_idInfo
* ���ܣ��ϱ�ID����Ϣ
* ������
* ���أ�
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
u8 reported_idInfo()
{
    u8 idReportedBuf[] = {  0xBF,0x04,0x0F,
                            0x00,0x00,0x00,0x00,
                            0x00};
    u8 idCardNumber[4] = {0};
    
    if(mcuRead_idCard(0x00,idCardNumber))                   //��ȡ�ɹ�
    {
        for(u8 i=0;i<4;i++)
        {
            idReportedBuf[3+i] = idCardNumber[i];
        }
		idReportedBuf[7] = xor_count(idReportedBuf,1,6);        //����У��
		Uart1_Send_LenString(idReportedBuf,8);					//�����ɹ�
        
        //��ʾ����
        oled_display(0,idCardNumber);
        
        return 1;
	}
    return 0;
}




/*********************************************************************************************
* ���ƣ�pc_el
* ���ܣ�pc����el��
* ��������
* ���أ���
* �޸ģ�
* ע�ͣ�
*********************************************************************************************/
void pc_el()
{
	static u16 price=2;									    //ϵͳƱ��
	
    //����ϵͳģʽ
    if(sysMode==1)
    {
        if((UART1_RX_STA&0x80)==0x80)								//���ݽ������
        {
            u8 check_temp = xor_count(U1RX_Buf,1,(UART1_RX_STA&0x7f)-1);//���У��
            if(check_temp==U1RX_Buf[UART1_RX_STA&0x7f])		        //У����ȷ
            {
                switch(U1RX_Buf[2])
                {
                    //����ϵͳ�����Ų���
                    case 0x0f:
                        relay1_control(1);
                        relay_tiem = 20*3;
                        break;
                }
            }
            UART1_RX_STA = 0;
        } 
    }
    //����ˢ��ϵͳģʽ
    else
    {
        if((UART1_RX_STA&0x80)==0x80)							//���ݽ������
        {
            u8 check_temp = xor_count(U1RX_Buf,1,(UART1_RX_STA&0x7f)-1);//���У��
            if(check_temp==U1RX_Buf[UART1_RX_STA&0x7f])		    //У����ȷ
            {
                switch(U1RX_Buf[2])
                {
                    //ic��ֵ��ic�ۿ�
                    case 0x0b:
                        update_icData();				        //����ic������
                        break;
                        
                    //����ϵͳƱ��
                    case 0x0c:						
                        update_price(&price);				
                        break;
                    
                    //�޸�ic��ʹ��ģʽ
                    case 0x0d:						
                        if(U1RX_Buf[3]==0x0b)
                            icMode = 1;                         //��ֵģʽ
                        else
                            icMode = 0;                         //����ģʽ
                        break;    
                    
                    //�޸�ic������
                    case 0x0e:
                        update_icPassword();
                        break;
                }
                //��ʾ��Ϣ
                oled_display(price,NULL);
            }
            UART1_RX_STA = 0;
        } 
    }
}