/*
 * i2c.c
 *
 * Created: 6/14/2020 4:12:46 PM
 *  Author: josem
 */ 

 #include "i2c.h"
 #include <stdint.h>
 
 void TWI_Init(){

	/////// Setting frequency ///////
	/*
		F = 	CPUclk/(16 + (2*TWBR*Prescaler))
		F =		16M/(16 + 2*8*4)
		F =		200kHz
	*/

	//Prescaler
	TWSR &=~ (1<<TWPS0);
	TWSR &=~ (1<<TWPS1);

	//factor divisor
	TWBR = 8;

 }

 bool TWI_startCond(){
	TWCR = ((1<<TWINT) | (1<<TWSTA) | (1<<TWEN));

	while(!(TWCR & (1<<TWINT)));

	if ((TWSR & 0xF8) == TWI_START)
		return false;

	return true;
 }

 bool TWI_restrtCond(){
   TWCR = ((1<<TWINT) | (1<<TWSTA) | (1<<TWEN));

   while(!(TWCR & (1<<TWINT)));

   if ((TWSR & 0xF8) == TWI_RESTART)
	  return false;

   return true;
 }

 void TWI_stopCond(){
	TWCR |= ((1<<TWINT) | (1<<TWSTO) | (1<<TWEN));
 }

 bool TWI_sendAdrr(uint8_t adrr, uint8_t action){
	
	uint8_t cmp = 0;
	adrr = (adrr << 1 );

	if (action == TWI_W){
		adrr &=~ 1;
		cmp = TWI_WT_SLA_ACK;
	}
	else{
		adrr |= 1;
		cmp = TWI_RD_SLA_ACK;
	}

	TWDR = adrr;
	TWCR = ((1<<TWINT) | (1<<TWEN));

	while(!(TWCR & (1<<TWINT)));

	if ((TWSR & 0xF8) == cmp)
		return false;
	 
	return true;
 }

 bool TWI_write(uint8_t data2write){
	
	bool ret = true;
	
	TWDR = data2write;
	TWCR = ((1<<TWINT) | (1<<TWEN));
	while(!(TWCR & (1<<TWINT)));
	
	if ((TWSR & 0xF8) == TWI_MT_DATA_ACK)
		ret = false;
	
	return ret;
 }

 uint8_t TWI_read(uint8_t ACK_NACK){
	
	TWCR = ((1 << TWINT) | (1 << TWEN) | (ACK_NACK << TWEA));

	while(!(TWCR & (1<<TWINT)));
	return TWDR;
 }
 
 //SOLO PARA EL EJEMPLO DE USO DEL I2C
 
 void init_temp(void) { //address=0x03(en el ejemplo)
	 
	 while(TWI_startCond());
	 while(TWI_sendAdrr(0x4b, TWI_W));
	 while(TWI_write(0xee));
	 TWI_stopCond();
 }
 
 float read_full_temp(void){
	 float tura;
	 uint8_t datah;
	 uint8_t datal;
	 int data;
	 
	 while(TWI_startCond());
	 while(TWI_sendAdrr(0x4b, TWI_W));
	 while(TWI_write(0xaa));
	 while(TWI_restrtCond());
	 while(TWI_sendAdrr(0x4b, TWI_R));
	 datah=TWI_read(TWI_ACK);       //Lectura parte alta
	 datal=TWI_read(TWI_NACK);     //Lectura parte alta y NACK
	 TWI_stopCond(); 
	 
	 data=datah;
	 if (datal==128){
		tura=data+0.5;
	 }
	 else{
		tura=data;
	 }
	 return(tura);
 }

