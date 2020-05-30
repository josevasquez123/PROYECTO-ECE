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
#include <stdint.h>
#include "librerias/config.h"
#include "librerias/abecedario.h"
#include "librerias/glcd.h"

/**********************************TABLA DE IGUALDADES************************************/

/**********            COMUNIC=1 -----> UART          ************************************/
/**********            COMUNIC=2 -----> I2C           ************************************/
/**********            COMUNIC=3 -----> SPI           ************************************/

/**********            PARIDAD=3 -----> SIN PARIDAD   ************************************/
/**********            PARIDAD=1 -----> PARIDAD IMPAR ************************************/
/**********            PARIDAD=2 -----> PARIDAD PAR   ************************************/



/**********************************SUBRUTINAS************************************/
void config(void);
void config_ext_int(void);
void mover_cursor_cambios_menu(void);
void mover_cursor(void);
void mostrar_cambios_comunic(void);
void config_int1_2(void);
void config_uart(void);
void mostrar_cambios_paridad(void);

/**********************************VARIABLES************************************/
volatile bool flag_b2=0;
volatile bool flag_b3=0;
volatile bool flag_rep=1;
volatile bool flag_confirm=0;
volatile bool flag_now_comunic=0;
volatile bool flag_loop1=1;
volatile bool flag_now_paridad=0;
volatile bool flag_loop2=1;
uint8_t bus_opcion=1;
uint8_t max_opcion;
uint8_t min_opcion=1;
uint8_t comunic=0;
uint8_t paridad=0;


int main(void)
{

	config();								//CONFIGURACIONES PRINCIPALES DEL MICRO
	cli();	
	config_ext_int();						//CONFIGURACION DE INT0,INT1 E INT2
	sei();
	//PRIMERA INTRODUCCION EN LA PANTALLA GLCD
	glcd_on();
	glcd_clearscreen();
	write_small(25,40,"PROYECTO",0);
	write_small(36,53,"ECE",0);
	
	
    while (1) 
    {
		if(flag_confirm)
		{
			config_int1_2();											//HABILITO LAS INT EXTERNAS 1 Y 2
			
			while(flag_loop1)
			{
				mover_cursor_cambios_menu();							//ELECCION DEL TIPO DE COMUNICACION A USAR
			}
			
			if (comunic==1)												//CONFIGURACION DEL UART
			{
				config_uart();	
			}
			else if (comunic==2)										//CONFIGURACION DEL I2C
			{
				
			}
			else if (comunic==3)										//CONFIGURACION DEL SPI
			{
				
			}
			else
			{
				//MOSTRAR ERROR POR EL GLCD
			}
		}
    }
}




/**********************************SUBRUTINAS************************************/

void config(void){
	DDRA=0xff;									//DEFINIR PUERTO DE DATOS DE GLCD COMO SALIDA
	DDRC=0xfc;									//DEFINIR PUERTO DE ESTADOS DE GLCD COMO  SALIDA
	DDRD &=~ (1<<DDD2);							//INT0 (BOTON) COMO ENTRADA
	GLCDPORT|=(1<<RST);						    //PRIMER ESTADO RESET EN HIGH DEL GLCD
}


void config_ext_int(void){
	MCUCR|=(1<<ISC11)|(1<<ISC01);				//CONFIGURAR FLANCO DE BAJADA INT0 E INT1
	MCUCR &=~ (1<<ISC00)|(1<<ISC10);			//CONFIGURAR FLANCO DE BAJADA INT0 E INT1
	MCUCSR &=~ (1<<ISC2);						//CONFIGURAR FLANCO DE BAJADA INT2
	GICR|=(1<<INT0);							//HABILITANDO INT0
}

void config_int1_2(void){
	flag_now_comunic=1;							//BANDERA QUE AFIRMA QUE PASO EL PRIMER LOOP MENU
	max_opcion=3;								//CANTIDAD MAXIMA DE OPCIONES EN EL GLCD
	bus_opcion=1;								//POSICION DEL CURSOR EN EL GLCD (VALOR INICIAL 1)
	flag_confirm=0;								//LIMPIA LA BANDERA PORQUE YA PASO EL PRIMER LOOP MENU
	cli();
	GICR|=(1<<INT1)|(1<<INT2);					//HABILITANDO INT1 E INT2
	sei();
}

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

void mostrar_cambios_comunic(void){
	if(flag_rep==1){
		switch(bus_opcion){
			
			case 1:
			comunic=1;
			glcd_clearscreen();
			write_small(0,0,"TIPOS DE COMUNICACION",0);
			write_small(15,0,"1) UART (SELECCIONADA)",0);
			write_small(30,0,"2) I2C",0);
			write_small(45,0,"3) SPI",0);
			break;
			
			case 2:
			comunic=2;
			glcd_clearscreen();
			write_small(0,0,"TIPOS DE COMUNICACION",0);
			write_small(15,0,"1) UART",0);
			write_small(30,0,"2) I2C (SELECCIONADA)",0);
			write_small(45,0,"3) SPI",0);
			break;
			
			case 3:
			comunic=3;
			glcd_clearscreen();
			write_small(0,0,"TIPOS DE COMUNICACION",0);
			write_small(15,0,"1) UART",0);
			write_small(30,0,"2) I2C",0);
			write_small(45,0,"3) SPI (SELECCIONADA)",0);
			break;
		}
		flag_rep=0;
	}
}

void config_uart(void){
	max_opcion=3;
	bus_opcion=1;
	flag_now_comunic=0;
	flag_now_paridad=1;
	flag_rep=1;
	
	glcd_clearscreen();
	write_small(0,0,"TIPOS DE BIT DE PARIDAD",0);
	write_small(15,0,"1) PAR",0);
	write_small(30,0,"2) IMPAR",0);
	write_small(45,0,"3) NO PARIDAD",0);
	
	while(flag_loop2)
	{
		mover_cursor();
		mostrar_cambios_paridad();
	}
}

void mostrar_cambios_paridad(void){
	if (flag_rep==1){
		switch (bus_opcion){
			case 1:
			paridad=1;
			glcd_clearscreen();
			write_small(0,0,"TIPOS DE BIT DE PARIDAD",0);
			write_small(15,0,"1) PAR (SELECCIONADA)",0);
			write_small(30,0,"2) IMPAR",0);
			write_small(45,0,"3) NO PARIDAD",0);
			break;
			
			case 2:
			paridad=2;
			glcd_clearscreen();
			write_small(0,0,"TIPOS DE BIT DE PARIDAD",0);
			write_small(15,0,"1) PAR",0);
			write_small(30,0,"2) IMPAR (SELECCIONADA)",0);
			write_small(45,0,"3) NO PARIDAD",0);
			break;
			
			case 3:
			paridad=3;
			glcd_clearscreen();
			write_small(0,0,"TIPOS DE BIT DE PARIDAD",0);
			write_small(15,0,"1) PAR",0);
			write_small(30,0,"2) IMPAR",0);
			write_small(45,0,"3) NO PARIDAD (SELEC)",0);
			break;
		}
		flag_rep=0;
	}
}


/**********************************INTERRUPCIONES************************************/

ISR(INT0_vect){
	
	if (flag_now_paridad){
		flag_loop2=0;								//BANDERA PARA ROMPER EL PRIMER LOOP DE LA CONFIG UART
		flag_rep=1;									//BANDERA PARA MOSTRAR CAMBIOS EN LA PANTALLA GLCD'
	}
	
    else if(flag_now_comunic){
		flag_loop1=0;								//BANDERA PARA ROMPER EL PRIMER LOOP
		flag_rep=1;									//BANDERA PARA MOSTRAR CAMBIOS EN LA PANTALLA GLCD'
	}
	else{
		flag_confirm=1;								//BANDERA DE CONFIRMACION
		flag_rep=1;									//BANDERA PARA MOSTRAR CAMBIOS EN LA PANTALLA GLCD'
	}
}

ISR(INT1_vect){
	flag_b2=1;										//BANDERA PARA MOVER EL CURSOR EN 1
}

ISR(INT2_vect){
	flag_b3=1;										//BANDERA PARA RETROCEDER EL CURSOR EN 1
}