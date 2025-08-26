/*-------------------------------------------------*/
/*                 ���ҵ�������                    */
/*-------------------------------------------------*/
/*           ʵ�ּ�LCD������ʾ���ܵ�Դ�ļ�         */
/*-------------------------------------------------*/

#include "stm32f10x.h"   //������Ҫ��ͷ�ļ�
#include "LCD12864S.h"   //������Ҫ��ͷ�ļ�
#include "delay.h"       //������Ҫ��ͷ�ļ�
/*-------------------------------------------------*/
/*����������ʼ��LCD12864����                       */
/*��  ������                                       */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void LCD12864_Init(void)
{			
	GPIO_InitTypeDef  GPIO_InitStructure;                      //����һ������IO�ı���

	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );    //ʹ��GPIOBʱ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;   
                                                               //׼������RST-PB12,SCLK-PB13,SID-PB14,CS-PB15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;           //���������ʽ  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;          //IO����50M
	GPIO_Init(GPIOB, &GPIO_InitStructure);                     //����PB6 PB7
   
    LCD_RST_L; Delay_Ms(1);                                   
    LCD_RST_H; Delay_Ms(100);                                  //��λLCD���ȴ�LCD�Լ죬����40ms
    LCD_Write_COM(0x30); Delay_Us(150);                        //�������ã���ʱ����100uS
    LCD_Write_COM(0x30); Delay_Us(40);                         //�������ã���ʱ����37uS
    LCD_Write_COM(0x0c); Delay_Us(150);                        //����ʾ����ʱ����100uS
    LCD_Write_COM(0x01); Delay_Ms(15);                         //��������ʱ����10ms
    LCD_Write_COM(0x06);                                       //�����趨�㣬��ɳ�ʼ��
}

/*-------------------------------------------------*/
/*��������LCD12864д�ֽں���                       */
/*��  ����Byte����Ҫд����ֽ�                     */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void LCD12864_SendByte(unsigned char Byte)
{
    unsigned char i;
    for(i=0; i<8; i++){
        if((Byte<<i)&0x80){
            LCD_SID_H;
        }
        else{
            LCD_SID_L;
        }
        LCD_SCLK_L;	
        LCD_SCLK_H;	
    }
}

/*-------------------------------------------------*/
/*��������LCD12864дָ���                       */
/*��  ����COMdode����Ҫд���ָ��                  */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void LCD_Write_COM(unsigned char COMdode)
{
    LCD_CS_H;			                 //CS�øߵ�ƽ��׼����������
    Delay_Us(100);
    LCD12864_SendByte(0xF8);             //0xF8Ϊдָ��
    LCD12864_SendByte(COMdode&0xF0);     //д��4λ
    LCD12864_SendByte((COMdode<<4)&0xF0);//д��4λ
    Delay_Us(100);
}
/*-------------------------------------------------*/
/*��������LCD12864д���ݺ���                       */
/*��  ����DATdode����Ҫд�������                  */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void LCD_Write_DAT(unsigned char DATdode)
{
    LCD_CS_H;			                 //CS�øߵ�ƽ��׼����������
    Delay_Us(100);
    LCD12864_SendByte(0xFA);             //0xFAΪд����
    LCD12864_SendByte(DATdode&0xF0);     //д��4λ
    LCD12864_SendByte((DATdode<<4)&0xF0);//д��4λ
    Delay_Us(100);
}

/*-------------------------------------------------*/
/*������������m^n                                  */
/*��  ����m:Ϊ����,n:Ϊ��                          */
/*����ֵ��������                                 */
/*-------------------------------------------------*/
unsigned int Compute_Power(unsigned char m,unsigned char n)
{
	unsigned int result=1;
	while(n--)
	{
	  result*=m;
	}
	return result;
}

/*-------------------------------------------------*/
/*������������LCD��ʾ����                          */
/*��  ����x:��,1-4��y:�У�0-7                      */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void LCD_Set_Position(unsigned char x, unsigned char y)
{
    switch(x){
        case 1:   LCD_Write_COM(0x80 + y); break;	
		case 2:   LCD_Write_COM(0x90 + y); break;	
        case 3:   LCD_Write_COM(0x88 + y); break;	
        case 4:   LCD_Write_COM(0x98 + y); break;	        
        default:  LCD_Write_COM(0x80 + y); break;	 
    } 
}

/*-------------------------------------------------*/
/*��������LCD��ʾһ���ַ���                        */
/*��  ����x:��,1-4��y:�У�0-7                      */
/*        DisplayString:Ҫ��ʾ���ַ���             */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void LCD_Display_String(unsigned char x,unsigned char y,unsigned char *DisplayString)
{ 
    LCD_Set_Position(x,y);
 	while(*DisplayString>0)
   { 
      LCD_Write_DAT(*DisplayString);
      DisplayString++;
   }
}

/*-------------------------------------------------*/
/*��������LCD��ʾһ������                          */
/*��  ����x:��,1-4��y:��,0-7, lenth:��ʾ���ֵ�λ�� */
/*        DisplayNum:Ҫ��ʾ������,���65535        */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void LCD_Display_Num(unsigned char x,unsigned char y,unsigned char lenth,unsigned int DisplayNum)
{ 
    unsigned char t,temp;
    LCD_Set_Position(x,y);
 	for(t=0;t<lenth;t++){
        temp=(DisplayNum/Compute_Power(10,lenth-t-1))%10;
        LCD_Write_DAT(temp+'0');
    }
}

/*-------------------------------------------------*/
/*��������LCD��ʾһ����С���������                */
/*��  ����x:��,1-4��y:��,0-7, point:С�����λ��   */
/*        DisplayNum:Ҫ��ʾ������,���9999         */
/*����ֵ����                                       */
/*-------------------------------------------------*/
void LCD_Display_Num_point(unsigned char x,unsigned char y,unsigned char point,unsigned int DisplayNum)
{ 
    unsigned char t,temp;
    LCD_Set_Position(x,y);
		if(point==0){
			for(t=0;t<4;t++){
				temp=(DisplayNum/Compute_Power(10,3-t))%10;
				LCD_Write_DAT(temp+'0');
			}
			if(DisplayNum/1000==0)
				LCD_Write_DAT(' ');
		}
    if(point==1){
        for(t=0;t<2;t++){
					temp=(DisplayNum/Compute_Power(10,3-t))%10;
					LCD_Write_DAT(temp+'0');
				}
			LCD_Write_DAT('.');
			LCD_Write_DAT(DisplayNum%10+'0');
    }
    if(point==2){
			LCD_Write_DAT(DisplayNum/100+'0');
			LCD_Write_DAT('.');
			LCD_Write_DAT(DisplayNum/10%10+'0');
			LCD_Write_DAT(DisplayNum%10+'0');
    }
}
