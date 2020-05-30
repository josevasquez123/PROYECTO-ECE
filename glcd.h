/*
 * glcd.h
 *
 * Created: 5/21/2020 6:15:50 PM
 *  Author: josem
 */ 


#ifndef GLCD_H_
#define GLCD_H_

#include "config.h"


void init_mmap(void);
void glcd_exec();
void glcd_reset(void);
void glcd_on(void);
void glcd_gotoy(uint8 y);
void glcd_gotox(uint8 x);
void glcd_putbyte(uint8 x);
void glcd_clearscreen(void);
uint8 glcd_putchar(uint8 ch, uint8 x, uint8 y, uint8 clrflag);
uint8 write(uint8 x, uint8 y, uint8 *txt, uint8 clrflag);

uint8 write_small(uint8 x, uint8 y, const char *txt, uint8 clrflag);
uint8 glcd_putchar_small(uint8 ch, uint8 x, uint8 y, uint8 clrflag);

extern volatile uint8 ismodechanged;
extern volatile uint8 mmap[1024];



#endif /* GLCD_H_ */