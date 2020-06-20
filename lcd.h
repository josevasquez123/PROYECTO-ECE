/*
 * lcd.h
 *
 * Created: 6/20/2020 2:40:20 PM
 *  Author: josem
 */ 


#ifndef LCD_H_
#define LCD_H_

#include "config.h"

void Lcd_Port(char a);
void Lcd_Cmd(char a);
void Lcd_Clear(void);
void Lcd_Set_Cursor(char a, char b);
void Lcd_Init(void);
void Lcd_Write_Char(char a);
void Lcd_Write_String(char *a);
void Lcd_Shift_Right(void);
void Lcd_Shift_Left(void);


#endif /* LCD_H_ */