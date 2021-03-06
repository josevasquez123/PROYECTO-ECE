/*
 * pruebas.c
 *
 * Created: 5/21/2020 4:20:18 PM
 * Author : josem
 */ 

#define F_CPU 16000000UL

#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <setjmp.h>
#include "lib/config.h"
#include "lib/lcd.h"
#include "lib/i2c.h"

/**********************************TABLA DE IGUALDADES************************************/

/**********            COMUNIC=1 -----> UART                    ************************************/
/**********            COMUNIC=2 -----> I2C                     ************************************/
/**********            COMUNIC=3 -----> SPI                     ************************************/

/**********            PARIDAD=3 -----> SIN PARIDAD             ************************************/
/**********            PARIDAD=1 -----> PARIDAD IMPAR           ************************************/
/**********            PARIDAD=2 -----> PARIDAD PAR             ************************************/

/**********            NBIT=5 -----> 5 BITS DE DATO             ************************************/
/**********            NBIT=6 -----> 6 BITS DE DATO             ************************************/
/**********            NBIT=7 -----> 7 BITS DE DATO             ************************************/
/**********            NBIT=8 -----> 8 BITS DE DATO             ************************************/
/**********            NBIT=9 -----> 9 BITS DE DATO             ************************************/

/**********            BAUD=48 -----> 4800 BAUD RATE            ************************************/
/**********            BAUD=96 -----> 9600 BAUD RATE            ************************************/
/**********            BAUD=144 -----> 14.4K BAUD RATE          ************************************/
/**********            BAUD=192 -----> 19.2K BAUD RATE          ************************************/



/**********************************SUBRUTINAS************************************/
void config(void);
void config_ext_int(void);
void temp();
void mover_cursor_cambios_menu(void);
void mover_cursor(void);
void mostrar_cambios_comunic(void);
void config_int1_2(void);
void config_uart(void);
void mostrar_cambios_paridad(void);
void mostrar_cambios_nbit(void);
void mostrar_cambios_baudios(void);
void puertos_entrada(void);
void envio_uart(void);
void activacion_registros_uart(void);
void registro_bit_paridad(void);
void registro_cantidad_bits(void);
void registro_baud(void);
void clear_all(void);
void mensaje_final(void);
void error_paridad(void);
void retroceder(void);
void envio_i2c(void);
void intro_comunicaciones(void);
void intro_paridad(void);
void intro_nbit(void);
void intro_baud(void);
//void timer_init(void);
//void timer_on(void);
//void timer_off(void);

/**********************************VARIABLES************************************/
volatile bool flag_b2=0;
volatile bool flag_b3=0;
volatile bool flag_rep=1;
volatile bool flag_confirm=0;
volatile bool flag_loop=1;
volatile bool flag_now2=0;
volatile bool flag_stop_comunicaciones=0;;
volatile bool flag_paridad_lugar=0;
uint8_t bus_opcion=1;
uint8_t max_opcion;
uint8_t min_opcion=1;
uint8_t comunic=0;
uint8_t paridad=0;
uint8_t nbit=0;
uint8_t baud=0;
jmp_buf inicio;
jmp_buf comunicaciones;
jmp_buf lugar_paridad;
jmp_buf lugar_nbit;
jmp_buf lugar_baud;
volatile char mybuffer= ' ';


int main(void)
{
	
	config();								//CONFIGURACIONES PRINCIPALES DEL MICRO
	
	//PRIMERA INTRODUCCION EN LA PANTALLA GLCD
	Lcd_Init();
	
	setjmp(inicio);
	cli();
	config_ext_int();						//CONFIGURACION DE INT0,INT1 E INT2
	sei();
	
	Lcd_Clear();
	Lcd_Set_Cursor(1, 5);
	Lcd_Write_String("PROYECTO");
	Lcd_Set_Cursor(2, 7);
	Lcd_Write_String("ECE");
	
	_delay_ms(800);
	
	for (int i=0; i<16;i++)
	{
		_delay_ms(100);
		Lcd_Shift_Right();
	}
	_delay_ms(500);
	
	
	
    while (1) 
    {
			//EJEMPLO TEMPORAL DE COMO MOSTRAREMOS LA TEMP 
			while(flag_loop)
			{
				temp();
			}
			
			setjmp(comunicaciones);										//PUNTO DE SALTO
			
			config_int1_2();											//HABILITO LAS INT EXTERNAS 1 Y 2
			
			intro_comunicaciones();
			
			while(flag_loop)
			{
				mover_cursor_cambios_menu();							//ELECCION DEL TIPO DE COMUNICACION A USAR
				retroceder();
			}
			
			if (comunic==1)												//CONFIGURACION DEL UART
			{
				config_uart();
				
				puertos_entrada();										//DEFINO TODO LOS PUERTOS DE COMUNICACION COMO ENTRADA (POR PRECAUCIÓN)

				envio_uart();
			}
			else if (comunic==2)										//CONFIGURACION DEL I2C
			{
				puertos_entrada();										//DEFINO TODO LOS PUERTOS DE COMUNICACION COMO ENTRADA (POR PRECAUCIÓN)
				
				envio_i2c();
			}
			else if (comunic==3)										//CONFIGURACION DEL SPI
			{
				puertos_entrada();										//DEFINO TODO LOS PUERTOS DE COMUNICACION COMO ENTRADA (POR PRECAUCIÓN)
			}
			
			mensaje_final();
	}
}
	


/*****************************************************SUBRUTINAS********************************************************/


/********************************* CONFIGURACIONES PUERTO GLCD ***********************************/

void config(void){
	DDRA=0xff;																	//DEFINIR PUERTO DE DATOS DE GLCD COMO SALIDA
	DDRD &=~ (1<<DDD2)|(1<<DDD3)|(1<<DDD7);										//INT0,INT1,INT2 Y B4 (BOTON) COMO ENTRADA
	DDRB &=~ (1<<DDB2);
}


/********************************* CONFIGURACIONES INT EXTERNAS ***********************************/

void config_ext_int(void){
	MCUCR|=(1<<ISC11)|(1<<ISC01);				//CONFIGURAR FLANCO DE BAJADA INT0 E INT1
	MCUCR &=~ (1<<ISC00)|(1<<ISC10);			//CONFIGURAR FLANCO DE BAJADA INT0 E INT1
	MCUCSR &=~ (1<<ISC2);						//CONFIGURAR FLANCO DE BAJADA INT2
	GICR|=(1<<INT0);							//HABILITANDO INT0
	GICR&=~(1<<INT1)|(1<<INT2);					//DESHABILITANDO INT1 E INT2
}

/********************************* HABILITA BOTON 2 Y 3 ***********************************/

void config_int1_2(void){
	flag_loop=1;
	
	flag_rep=1;
	max_opcion=3;								//CANTIDAD MAXIMA DE OPCIONES EN EL GLCD
	bus_opcion=1;								//POSICION DEL CURSOR EN EL GLCD (VALOR INICIAL 1)
	flag_confirm=0;								//LIMPIA LA BANDERA PORQUE YA PASO EL PRIMER LOOP MENU
	flag_stop_comunicaciones=1;					//BANDERA PARA EL USO DEL BOTON DE RETROCESO
	cli();
	GICR|=(1<<INT1)|(1<<INT2);					//HABILITANDO INT1 E INT2
	sei();
}

/********************************* UART E I2C COMO ENTRADA ***********************************/

void puertos_entrada(void){
	//PUERTOS I2C
	DDRC &=~ (1<<DDC0)|(1<<DDC1);
	//PUERTOS UART
	DDRD &=~(1<<DDD0)|(1<<DDD1);
	//DESACTIVACION DE BOTON 1 Y 3
	cli();
	GICR &=~ (1<<INT0)|(1<<INT2);
	sei();
}

/********************************* ELECCION DE COMUNICACION ***********************************/

void mover_cursor_cambios_menu(void){
	mover_cursor();								//SUBRUTINA PARA MOVER EL CURSOR
	mostrar_cambios_comunic();					//SUBRUTINA PARA MOSTRAR EL CAMBIO EN LA PANTALLA DEPENDIENDO LA POS DEL CURSOR
}

void mover_cursor(void){
	if (flag_b2==1){							//ENTRA SOLO SI SE PRESIONO EL BOTON DE ADELANTAR
		if(bus_opcion==max_opcion){				//SI LA POSICION ACTUAL DEL CURSOR ES LA POS MAXIMA, SE IRA A LA POS 1
			bus_opcion=min_opcion;				//DEFINE LA POSICION ACTUAL COMO LA PRIMERA
		}
		else{
			bus_opcion ++;						//SI NO ESTA EL CURSOR EN LA POS MAXIMA, AUMENTA EN 1 SU POS
		}
		flag_b2=0;								//LIMPIO LA BANDERA DEL BOTON ADELANTAR PARA NO INGRESAR DE NUEVO A ESTE "IF" SI ES Q NO SE HA PRESIONADO DE NUEVO EL BOTON
		flag_rep=1;								//SETEO LA BANDERA REP PARA QUE MUESTRE LOS CAMBIOS EN EL GLCD
	}
	else if(flag_b3==1){
		if(bus_opcion==min_opcion){
			bus_opcion=max_opcion;
		}
		else{
			bus_opcion --;
		}
		flag_b3=0;
		flag_rep=1;
	}
	
}

/********************************* RETROCEDER UNA CONFIGURACION ***********************************/

void retroceder (void){
	
	bool flag_b4=0;
	
	if (!(PIND&(1<<PIND7))){
		
		flag_b4=1;
		
		while(!(PIND&(1<<PIND7)));
		
		if(flag_b4){
			
			if(flag_stop_comunicaciones){
				
				longjmp( inicio, 1 );
			}
			else if(flag_paridad_lugar){
				flag_paridad_lugar=0;
				longjmp( comunicaciones, 1 );
			}
			else if(max_opcion==4){
				nbit=0;
				longjmp( lugar_nbit, 1 );
			}
			else if(max_opcion==5){
				paridad=0;
				longjmp( lugar_paridad, 1 );
			}
		}
	}
}


/********************************* CONFIGURACION UART ********************************************/

void config_uart(void){
	
	setjmp(lugar_paridad);								//LUGAR DE SALTO
	
	flag_stop_comunicaciones=0;
	flag_paridad_lugar=1;
	max_opcion=3;
	bus_opcion=1;
	flag_rep=1;
	flag_loop=1;
	
	intro_paridad();
	
	while(flag_loop)
	{
		mover_cursor();
		mostrar_cambios_paridad();
		retroceder();
	}
	
	setjmp(lugar_nbit);									//LUGAR DE SALTO
	
	flag_paridad_lugar=0;
	max_opcion=5;
	bus_opcion=1;
	flag_rep=1;
	flag_loop=1;
	
	intro_nbit();

	while (flag_loop){
		mover_cursor();
		mostrar_cambios_nbit();
		retroceder();
	}
	
	setjmp(lugar_baud);									//LUGAR DE SALTO
	
	max_opcion=4;
	bus_opcion=1;
	flag_rep=1;
	flag_loop=1;
	
	intro_baud();
	
	while(flag_loop){
		mover_cursor();
		mostrar_cambios_baudios();
		retroceder();
	}
}

/********************************* CAMBIOS EN LA PANTALLA ***********************************/

void mostrar_cambios_comunic(void){
	if(flag_rep==1){
		switch(bus_opcion){
			
			case 1:
			comunic=3;
			Lcd_Clear();
			Lcd_Set_Cursor(1, 1);
			Lcd_Write_String("SPI(S)  I2C");
			Lcd_Set_Cursor(2, 1);
			Lcd_Write_String("UART");
			//write_small(0,0,"TIPOS DE COMUNICACION",0);
			//write_small(15,0,"1) UART (SELECCIONADO)",0);
			//write_small(30,0,"2) I2C",0);
			//write_small(45,0,"3) SPI",0);
			break;
			
			case 2:
			comunic=2;
			Lcd_Clear();
			Lcd_Set_Cursor(1, 1);
			Lcd_Write_String("SPI     I2C(S)");
			Lcd_Set_Cursor(2, 1);
			Lcd_Write_String("UART");
			//write_small(0,0,"TIPOS DE COMUNICACION",0);
			//write_small(15,0,"1) UART",0);
			//write_small(30,0,"2) I2C (SELECCIONADO)",0);
			//write_small(45,0,"3) SPI",0);
			break;
			
			case 3:
			comunic=1;
			Lcd_Clear();
			Lcd_Set_Cursor(1, 1);
			Lcd_Write_String("SPI     I2C");
			Lcd_Set_Cursor(2, 1);
			Lcd_Write_String("UART(S)");
			//write_small(0,0,"TIPOS DE COMUNICACION",0);
			//write_small(15,0,"1) UART",0);
			//write_small(30,0,"2) I2C",0);
			//write_small(45,0,"3) SPI (SELECCIONADO)",0);
			break;
		}
		flag_rep=0;
	}
}


void mostrar_cambios_paridad(void){
	if (flag_rep==1){
		switch (bus_opcion){
			case 1:
			paridad=2;
			Lcd_Clear();
			Lcd_Set_Cursor(1, 1);
			Lcd_Write_String("PAR(S)  IMPAR");
			Lcd_Set_Cursor(2, 1);
			Lcd_Write_String("NO PARIDAD");
			//write_small(0,0,"TIPOS DE BIT DE PARIDAD",0);
			//write_small(15,0,"1) PAR (SELECCIONADO)",0);
			//write_small(30,0,"2) IMPAR",0);
			//write_small(45,0,"3) NO PARIDAD",0);
			break;
			
			case 2:
			paridad=1;
			Lcd_Clear();
			Lcd_Set_Cursor(1, 1);
			Lcd_Write_String("PAR     IMPAR(S)");
			Lcd_Set_Cursor(2, 1);
			Lcd_Write_String("NO PARIDAD");
			//write_small(0,0,"TIPOS DE BIT DE PARIDAD",0);
			//write_small(15,0,"1) PAR",0);
			//write_small(30,0,"2) IMPAR (SELECCIONADO)",0);
			//write_small(45,0,"3) NO PARIDAD",0);
			break;
			
			case 3:
			paridad=3;
			Lcd_Clear();
			Lcd_Set_Cursor(1, 1);
			Lcd_Write_String("PAR     IMPAR");
			Lcd_Set_Cursor(2, 1);
			Lcd_Write_String("NO PARIDAD(S)");
			//write_small(0,0,"TIPOS DE BIT DE PARIDAD",0);
			//write_small(15,0,"1) PAR",0);
			//write_small(30,0,"2) IMPAR",0);
			//write_small(45,0,"3) NO PARIDAD (SELEC)",0);
			break;
		}
		flag_rep=0;
	}
}


void mostrar_cambios_nbit(void){
	if (flag_rep==1){
		switch (bus_opcion){
			case 1:
			nbit=5;
			Lcd_Clear();
			Lcd_Set_Cursor(1, 1);
			Lcd_Write_String("5(S)  6     7");
			Lcd_Set_Cursor(2, 1);
			Lcd_Write_String("8     9");
			//write_small(0,0,"CANTIDAD DE BITS",0);
			//write_small(15,0,"1) 5 (S)    2) 6",0);
			//write_small(30,0,"3) 7        4) 8",0);
			//write_small(45,0,"5) 9",0);
			break;
			
			case 2:
			nbit=6;
			Lcd_Clear();
			Lcd_Set_Cursor(1, 1);
			Lcd_Write_String("5     6(S)  7");
			Lcd_Set_Cursor(2, 1);
			Lcd_Write_String("8     9");
			//write_small(0,0,"CANTIDAD DE BITS",0);
			//write_small(15,0,"1) 5        2) 6 (S)",0);
			//write_small(30,0,"3) 7        4) 8",0);
			//write_small(45,0,"5) 9",0);
			break;
			
			case 3:
			nbit=7;
			Lcd_Clear();
			Lcd_Set_Cursor(1, 1);
			Lcd_Write_String("5     6     7(S)");
			Lcd_Set_Cursor(2, 1);
			Lcd_Write_String("8     9");
			//write_small(0,0,"CANTIDAD DE BITS",0);
			//write_small(15,0,"1) 5        2) 6",0);
			//write_small(30,0,"3) 7 (S)    4) 8",0);
			//write_small(45,0,"5) 9",0);
			break;
			
			case 4:
			nbit=8;
			Lcd_Clear();
			Lcd_Set_Cursor(1, 1);
			Lcd_Write_String("5     6     7");
			Lcd_Set_Cursor(2, 1);
			Lcd_Write_String("8(S)  9");
			//write_small(0,0,"CANTIDAD DE BITS",0);
			//write_small(15,0,"1) 5        2) 6",0);
			//write_small(30,0,"3) 7        4) 8 (S)",0);
			//write_small(45,0,"5) 9",0);
			break;

			case 5:
			nbit=9;
			Lcd_Clear();
			Lcd_Set_Cursor(1, 1);
			Lcd_Write_String("5     6     7");
			Lcd_Set_Cursor(2, 1);
			Lcd_Write_String("8     9(S)");
			//write_small(0,0,"CANTIDAD DE BITS",0);
			//write_small(15,0,"1) 5        2) 6",0);
			//write_small(30,0,"3) 7        4) 8",0);
			//write_small(45,0,"5) 9 (S)",0);
			break;
		}
		flag_rep=0;
	}
}

void mostrar_cambios_baudios(void){
	if (flag_rep==1){
		switch (bus_opcion){
			case 1:
			baud=48;
			Lcd_Clear();
			Lcd_Set_Cursor(1, 1);
			Lcd_Write_String("4800(S)  9600");
			Lcd_Set_Cursor(2, 1);
			Lcd_Write_String("14.4K    19.2K");
			//write_small(0,0,"VELOCIDAD BAUDIO",0);
			//write_small(13,0,"1) 4800 (SELECCIONADO)",0);
			//write_small(25,0,"2) 9600",0);
			//write_small(39,0,"3) 14.4K",0);
			//write_small(52,0,"4) 19.2K",0);
			break;
			
			case 2:
			baud=96;
			Lcd_Clear();
			Lcd_Set_Cursor(1, 1);
			Lcd_Write_String("4800     9600(S)");
			Lcd_Set_Cursor(2, 1);
			Lcd_Write_String("14.4K    19.2K");
			//write_small(0,0,"VELOCIDAD BAUDIO",0);
			//write_small(13,0,"1) 4800",0);
			//write_small(25,0,"2) 9600 (SELECCIONADO)",0);
			//write_small(39,0,"3) 14.4K",0);
			//write_small(52,0,"4) 19.2K",0);
			break;
			
			case 3:
			baud=144;
			Lcd_Clear();
			Lcd_Set_Cursor(1, 1);
			Lcd_Write_String("4800     9600");
			Lcd_Set_Cursor(2, 1);
			Lcd_Write_String("14.4K(S)   19.2K");
			//write_small(0,0,"VELOCIDAD BAUDIO",0);
			//write_small(13,0,"1) 4800",0);
			//write_small(25,0,"2) 9600",0);
			//write_small(39,0,"3) 14.4K (SELECCIONADO)",0);
			//write_small(52,0,"4) 19.2K",0);
			break;
			
			case 4:
			baud=192;
			Lcd_Clear();
			Lcd_Set_Cursor(1, 1);
			Lcd_Write_String("4800     9600");
			Lcd_Set_Cursor(2, 1);
			Lcd_Write_String("14.4K   19.2K(S)");
			//write_small(0,0,"VELOCIDAD BAUDIO",0);
			//write_small(13,0,"1) 4800",0);
			//write_small(25,0,"2) 9600",0);
			//write_small(39,0,"3) 14.4K",0);
			//write_small(52,0,"4) 19.2K (SELECCIONADO)",0);
			break;
		}
		flag_rep=0;
	}
}


/********************************* ENVIO DE DATOS UART ***********************************/

void envio_uart(void){
	
	//char dato[12]="hello world\r";				//DATO DE EJEMPLO PARA LA SIMULACION
	
	activacion_registros_uart();				// ACTIVO LA CONFIGURACION HECHA POR EL USUARIO
	flag_now2=1;								// ACTIVO FLAG PARA UTILIZAR LA OTRA FUNCIONALIDAD DEL BOTON 2
	
	Lcd_Clear();
	Lcd_Set_Cursor(1, 2);
	Lcd_Write_String("ENVIO DE DATOS");
	Lcd_Set_Cursor(2, 3);
	Lcd_Write_String("TEMP =");
	//write_small(15,19,"ENVIO DE DATOS",0);
	//write_small(30,35,"ACTIVADO",0);
	//write_small(45,30,"TEMP = ",0);
	//SOLO PARA DEMOSTRACION
	int temp=0;
	char s[10];
	
	while(flag_stop_comunicaciones==0){
		
		while(!(UCSRA&(1<<UDRE)));
		temp++;
		UDR=temp+'0';
		Lcd_Set_Cursor(2, 10);
		Lcd_Write_String("     ");
		sprintf(s,"%u C",temp);
		Lcd_Set_Cursor(2, 10);
		Lcd_Write_String(s);
		//write_small(45,70,s,0);

		_delay_ms(1000);
	}
	
	clear_all();
}


void activacion_registros_uart(void){
	DDRD|=(1<<DDD1);							//TXD COMO SALIDA
	cli();
	registro_baud();
	UCSRC|=(1<<URSEL);
	UCSRC&=~(1<<UMSEL);							//MODO ASINCRONO
	UCSRA|=(1<<U2X);							//DOBLE VELOCIDAD DE TRANSMISION
	UCSRB|=(1<<RXEN)|(1<<TXEN)|(1<<RXCIE);		//HABILITA LA TRANSMISION, RECEPCION E INT POR RECEPCION
	
	registro_bit_paridad();
	registro_cantidad_bits();
	sei();
}

void registro_bit_paridad(void){
	switch (paridad)
	{
		case 1:
		UCSRC|=(1<<UPM1)|(1<<UPM0);
		break;
		
		case 2:
		UCSRC|=(1<<UPM1);
		UCSRC&=~(1<<UPM0);
		break;
		
		case 3:
		UCSRC&=~(1<<UPM0)|(1<<UPM1);
		break;
	}
}

void registro_cantidad_bits(void){
	switch (nbit)
	{
		case 5:
		UCSRC&=~(1<<UCSZ0)|(1<<UCSZ1);
		UCSRB &=~(1<<UCSZ2);
		break;
		
		case 6:
		UCSRC&=~(1<<UCSZ1);
		UCSRC|=(1<<UCSZ0);
		UCSRB &=~(1<<UCSZ2);
		break;
		
		case 7:
		UCSRC&=~(1<<UCSZ0);
		UCSRC|=(1<<UCSZ1);
		UCSRB &=~(1<<UCSZ2);
		break;
		
		case 8:
		UCSRB&=~(1<<UCSZ2);
		UCSRC|=(1<<UCSZ1)|(1<<UCSZ0);
		break;
		
		case 9:
		UCSRC|=(1<<UCSZ1)|(1<<UCSZ0)|(1<<UCSZ2);
		UCSRB|=(1<<UCSZ2);
		break;
	}
}

void registro_baud(void){
	switch (baud)
	{
		case 48:
		UBRRL=0b10100000;
		UBRRH=0b0001;
		break;
		
		case 96:
		UBRRL=0b11001111;
		UBRRH=0x0;
		break;
		
		case 144:
		UBRRL=0b10001010;
		UBRRH=0x0;
		break;
		
		case 192:
		UBRRL=0b01100111;
		UBRRH=0x0;
		break;
	}
}


/********************************* LIMPIEZA DE TODO LOS REGISTROS UTILIZADOS ***********************************/
void clear_all(void){
	
	flag_now2=0;
	flag_stop_comunicaciones=0;
	flag_loop=1;
	mybuffer=' ';
	paridad=0;
	nbit=0;
	baud=0;
	cli();
	UCSRA&=~(1<<U2X);
	UCSRB=0x00;
	UCSRC=0x80;
	UCSRC&=~(1<<URSEL);
	UBRRL=0x00;
	UBRRH=0x0;
	DDRD &=~(1<<DDD0)|(1<<DDD1);
	sei();
}


/************************************ TIMER 3 SEGUNDOS **************************************/

//void timer_init(void){
//TCCR1A &=~(1<<WGM11)|(1<<WGM10);
//TCCR1B |=(1<<WGM12);
//TCCR1B &=~(1<<WGM13);
//
//OCR1A = (F_CPU/1024/fclk)-1;
//
//TIMSK |= (1<<OCIE1A);
//}
//
//void timer_on(void){
//TCNT1L=0X00;
//TCNT1H=0x00;
//TCCR1B |=(1<<CS12)|(1<<CS10);
//TCCR1B &=~(1<<CS11);
//}
//
//void timer_off(void){
//TCCR1B &=~(1<<CS11)|(1<<CS12)|(1<<CS10);
//TIMSK &=~ (1<<OCIE1A);
//TCNT1L=0X00;
//TCNT1H=0x00;
//}

/********************************* ENVIO I2C ***********************************/
void envio_i2c(void){
	cli();
	TWI_Init();
	sei();
	init_temp();
	flag_now2=1;								// ACTIVO FLAG PARA UTILIZAR LA OTRA FUNCIONALIDAD DEL BOTON 2
	flag_stop_comunicaciones=0;
	float dato_temp;
	char s[10];
	
	Lcd_Clear();
	Lcd_Set_Cursor(1, 2);
	Lcd_Write_String("ENVIO DE DATOS");
	Lcd_Set_Cursor(2, 3);
	Lcd_Write_String("TEMP =");
	//write_small(15,19,"ENVIO DE DATOS",0);
	//write_small(30,35,"ACTIVADO",0);
	//write_small(45,30,"TEMP = ",0);
	
	while(flag_stop_comunicaciones==0){
		
		init_temp();
		
		dato_temp=read_full_temp();

		uint8_t datoh= trunc(dato_temp);
		uint8_t datol=(dato_temp-trunc(dato_temp))*100;
		datol=trunc(datol);
		
		Lcd_Set_Cursor(2, 10);
		Lcd_Write_String("     ");
		sprintf(s,"%u.%u C",datoh,datol);
		Lcd_Set_Cursor(2, 10);
		Lcd_Write_String(s);
		//write_small(45,70,s,0);
		
		_delay_ms(1000);
	}
	
	
	flag_now2=0;
	flag_stop_comunicaciones=0;
	flag_loop=1;
	
	TWI_stopCond();
	
}

/*********************************** TEMP **************************************/

void temp(void){
	cli();
	TWI_Init();
	sei();
	init_temp();

	float dato_temp;
	char s[10];
	
	Lcd_Clear();
	Lcd_Set_Cursor(1, 3);
	Lcd_Write_String("TEMPERATURA");
	//write_small(15,19,"ENVIO DE DATOS",0);
	//write_small(30,35,"ACTIVADO",0);
	//write_small(45,30,"TEMP = ",0);
	
	while(flag_loop){
		
		init_temp();
		
		dato_temp=read_full_temp();

		uint8_t datoh= trunc(dato_temp);
		uint8_t datol=(dato_temp-trunc(dato_temp))*100;
		datol=trunc(datol);
		
		Lcd_Set_Cursor(2, 6);
		Lcd_Write_String("     ");
		sprintf(s,"%u.%u C",datoh,datol);
		Lcd_Set_Cursor(2, 6);
		Lcd_Write_String(s);
		//write_small(45,70,s,0);
		
		_delay_ms(1000);
	}
	
	TWI_stopCond();
}




/********************************* MENSAJE FINAL ***********************************/
void mensaje_final(void){
	Lcd_Clear();
	Lcd_Set_Cursor(1, 3);
	Lcd_Write_String("COMUNICACION");
	Lcd_Set_Cursor(2, 3);
	Lcd_Write_String("TERMINADA :)");
	//write_small(25,27,"COMUNICACION",0);
	//write_small(36,30,"TERMINADA :)",0);
	//cli();
	//timer_init();
	//timer_on();
	//sei();
	//while (!flag_stop);
	//cli();
	//timer_off();
	//sei();
	//flag_stop=0;
	_delay_ms(3000);
	longjmp( inicio, 1 );
}


/********************************* INTRODUCCIONES LCD ***********************************/
void intro_comunicaciones(void)
{
	Lcd_Clear();
	Lcd_Set_Cursor(1, 5);
	Lcd_Write_String("TIPOS DE");
	Lcd_Set_Cursor(2, 2);
	Lcd_Write_String("COMUNICACIONES");

	_delay_ms(800);

	for (int i=0; i<16;i++)
	{
		_delay_ms(100);
		Lcd_Shift_Right();
	}
	_delay_ms(500);
}

void intro_paridad(void)
{
	Lcd_Clear();
	Lcd_Set_Cursor(1, 5);
	Lcd_Write_String("TIPOS DE");
	Lcd_Set_Cursor(2, 2);
	Lcd_Write_String("BIT DE PARIDAD");

	_delay_ms(800);

	for (int i=0; i<16;i++)
	{
		_delay_ms(100);
		Lcd_Shift_Right();
	}
	_delay_ms(500);
}

void intro_nbit(void)
{
	Lcd_Clear();
	Lcd_Set_Cursor(1, 3);
	Lcd_Write_String("CANTIDAD DE");
	Lcd_Set_Cursor(2, 7);
	Lcd_Write_String("BITS");

	_delay_ms(800);

	for (int i=0; i<16;i++)
	{
		_delay_ms(100);
		Lcd_Shift_Right();
	}
	_delay_ms(500);
}

void intro_baud(void)
{
	Lcd_Clear();
	Lcd_Set_Cursor(1, 2);
	Lcd_Write_String("VELOCIDAD DE");
	Lcd_Set_Cursor(2, 5);
	Lcd_Write_String("BAUDIOS");

	_delay_ms(800);

	for (int i=0; i<16;i++)
	{
		_delay_ms(100);
		Lcd_Shift_Right();
	}
	_delay_ms(500);
}







/********************************************************* INTERRUPCIONES **********************************************************/

ISR(INT0_vect){
	
		flag_loop=0;								//BANDERA PARA ROMPER EL PRIMER LOOP DE LA CONFIG UART
		flag_rep=1;									//BANDERA PARA MOSTRAR CAMBIOS EN LA PANTALLA GLCD'
}

ISR(INT1_vect){
	if(flag_now2==1)
	{
		flag_stop_comunicaciones=1;
	}
	else
	{
		flag_b2=1;										//BANDERA PARA MOVER EL CURSOR EN 1
	}
}

ISR(INT2_vect){
	flag_b3=1;										//BANDERA PARA RETROCEDER EL CURSOR EN 1
}


//ISR(TIMER1_COMPA_vect){
//flag_stop_comunicaciones=1;
//}


//PARA TENER EL DATO EN 2 UINT8_T
//double v=25.562677678;
//
//uint8_t a= trunc(v);
//uint8_t b=(v-trunc(v))*100;
//b=trunc(b);