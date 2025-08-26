/*-------------------------------------------------*/
/*                 ���ҵ�������                    */
/*-------------------------------------------------*/
/*           ʵ��LCD12864������ʾ���ܵ�ͷ�ļ�      */
/*-------------------------------------------------*/

#ifndef __LCD12864S_H
#define __LCD12864S_H

#define LCD_RST_L      GPIO_ResetBits(GPIOB, GPIO_Pin_12)                //����PB12��ƽ��RST�õ͵�ƽ
#define LCD_RST_H      GPIO_SetBits(GPIOB, GPIO_Pin_12)                  //����PB12��ƽ��RST�øߵ�ƽ

#define LCD_SCLK_L     GPIO_ResetBits(GPIOB, GPIO_Pin_13)                //����PB13��ƽ��SCLK�õ͵�ƽ
#define LCD_SCLK_H     GPIO_SetBits(GPIOB, GPIO_Pin_13)                  //����PB13��ƽ��SCLK�øߵ�ƽ

#define LCD_SID_L      GPIO_ResetBits(GPIOB, GPIO_Pin_14)                //����PB14��ƽ��SID�õ͵�ƽ
#define LCD_SID_H      GPIO_SetBits(GPIOB, GPIO_Pin_14)                  //����PB14��ƽ��SID�øߵ�ƽ

#define LCD_CS_L       GPIO_ResetBits(GPIOB, GPIO_Pin_15)                //����PB15��ƽ��CS�õ͵�ƽ
#define LCD_CS_H       GPIO_SetBits(GPIOB, GPIO_Pin_15)                  //����PB15��ƽ��CS�øߵ�ƽ

void LCD12864_Init(void);                                                //��ʼ��LCD12864����	
void LCD_Write_COM(unsigned char COMdode);                               //LCDдָ���	
void LCD_Write_DAT(unsigned char DATdode);                               //LCDд���ݺ���
void LCD_Set_Position(unsigned char x, unsigned char y);                 //����LCD��ʾ����
void LCD_Display_String(unsigned char x,unsigned char y,unsigned char *DisplayString);
                                                                         //��ʾ�ַ���
void LCD_Display_Num(unsigned char x,unsigned char y,unsigned char lenth,unsigned int DisplayNum);
                                                                         //��ʾһ������
void LCD_Display_Num_point(unsigned char x,unsigned char y,unsigned char point,unsigned int DisplayNum);
                                                                         //��ʾһ����С���������

#endif
