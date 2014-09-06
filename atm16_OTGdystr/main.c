// WYswietlacz LED dla Old Timers Garage
#include <stdio.h>
#include <avr/io.h>
#include "delay.h"
//#include "ds18b20.h"
#include <avr/interrupt.h>

unsigned int counter = 0;
float litry = 0;
float kwota = 0;
float cena = 6.0;
//	unsigned char ROM1[] = {,,,,,,,};
//	DOT. TERMOMETR I ODCZYT
//unsigned char ds18b20_pad[9] = {0}; //zmienna przejmuje dane z DSa
//double temp = 0;
//long int tempint = 0;
//unsigned char ROM1[] = { 0x28, 0x60, 0xf2, 0x33, 0x04, 0x00, 0x00, 0xa6 }; // marker na gorze
//	WYSWIETLANIE
char led_bufor[3][6] = {0};
unsigned char LEDmultiplex = 1;

//unsigned char KtoryROM = 1; //wartosc startowa
																																						#define ILOSC_ELEM_ZNAK 36
unsigned char znak[] = {63, 6, 91, 79, 102, 109, 125, 7, 127, 111,  0, 64, 119, 124, 88, 94, 121, 113, 61, 116, 4, 30, 118, 56, 55, 40, 92, 115, 80, 109, 120, 62, 42, 118, 91, 255};
//                       0  1   2   3    4    5    6  7    8    9  10  11  12   13   14  15  16   17   18   19  20 21  22   23  24  25  26  27   28  29   30   31  32  33   34  35
//                      zero     liczby jak akt. st. WYSOKIM       nic -   A    B    C   D   E    F    G    H   I  J   K    L   M   N   O   P    R   S    t    U   W   Y    Z   all
//                               liczenie 255-kod w funkcji ISR T0 OVF

void init()
{
	//	wejscie/wyjscie - jedynki na bitach gdzie WYjscie
	DDRA = 0xFF; // katody
	DDRB = 0xFF; // katody
	DDRC = 0xFF; // belki a-dp
	DDRD = 0x70; // 01110000, 1wire, 3puste, INT1, INT0, TX, RX
	PORTD = 0x80;	// rozne (1wire)



	// TIMERY
	TCCR0 |= (1<<CS01) | (0<<CS00);	 //timer0 ON, prescaler /8, przemiatanie wyswietlacza, czestotliwosc rzeczywista = 3904Hz
	TCCR0 &= ~(1<<CS02);
	TIMSK |= (1<<TOIE0);			 //przerwanie od przepelnienia T0
	TCNT0 = 0;						 //start od zera

	//config timera dla naliczania i innych wzdetow
	TCCR2 |= 0x05;	//prescaler /1024 
	TCCR2 &= ~(1<<CS21);
	TIMSK |= (1<<TOIE2);	// przerwanie przepelnienia
	TCNT2 = 0;

	// PRZERWANIA
	//MCUCR |= (1<<ISC01);	//konfiguracja przerwania zewn. INT0, aktywne zboczem opadajacym
	//MCUCR |= (1<<ISC11);	//konfiguracja przerwania zewn. INT1, aktywne zboczem opadajacym
	//GICR |= 0xC0;			//wlaczenie przerwania INT0 i INT1

	sei();					// globalne wlaczenie przerwan

	
	// zaswiec wszystko - self-test
	// unikac ponizszego = zapalone wszystkie diody na raz - zbyt duzy prad!!! tu nie ma multipleksowania
	//PORTA = 0xFF;	// katody aktywne stanem WYSOKIM
	//PORTB = 0xFF;	// katody
	//PORTC = 0x00;	// belki aktywne stanem NISKIM
	WriteLED(0, 8, 8, 8, 8, 8, 8, 0);
	WriteLED(1, 8, 8, 8, 8, 8, 8, 0);
	WriteLED(2, 8, 8, 8, 8, 8, 8, 0);
	_delay_ms(1000);
	while(~PIND & 0x80)
	{
		WriteLED(1, 10, 10, 10, 10, 10, 10, 0);
		WriteLED(2, 10, 10, 10, 10, 10, 10, 0);
		WriteLED(0, 1, 2, 3, 4, 5, 6, 0);
		_delay_ms(1000);
		WriteLED(0, 10, 10, 10, 10, 10, 10, 0);
		WriteLED(1, 1, 2, 3, 4, 5, 6, 0);
		_delay_ms(1000);
		WriteLED(1, 10, 10, 10, 10, 10, 10, 0);
		WriteLED(2, 1, 2, 3, 4, 5, 6, 0);
		_delay_ms(1000);
	}
	return;
}

void tryb_naliczanie_paliwa()
{
	WriteLED(2, 10, (int)cena%10, (int)(10*cena)%10, (int)(100*cena)%10, 10, 10, 2);	// napisz 6.00 na wyswietlaczu krotkim
	TIMSK |= (1<<TOIE2);	// wlacz timer
	TCNT2 = 0;				// wyzeruj timer
	litry = 0;
	kwota = 0;

	while( kwota < 9999.00 )	/////////////////// DO ILU TANKUJEMY
	{
#define _litry(d) (unsigned int)(litry*d)%10
#define _kwota(d) (unsigned int)(kwota*d)%10
		//WriteLED(0, a=(int)(litry/1000)%10, b=(int)(litry/100)%10, c=(int)(litry/10)%10, d=(int)(litry)%10, e=(int)(litry*10)%10, ((int)(litry*100))%10, 4);
		//WriteLED(1, a=(int)(kwota/1000)%10, b=(int)(kwota/100)%10, c=(int)(kwota/10)%10, d=(int)(kwota)%10, e=(int)(kwota*10)%10, ((int)(kwota*100))%10, 4);
		WriteLED(0, _litry(0.001), _litry(0.01), _litry(0.1), _litry(1), _litry(10), _litry(100), 4);
		WriteLED(1, _kwota(0.001), _kwota(0.01), _kwota(0.1), _kwota(1), _kwota(10), _kwota(100), 4);
	}

	
	return;
}

void pokaz_napis_OTG(int czas_ms)
{
	sei();
	WriteLED(0, 10, 10, 10, 10, 10, 10, 0);
	WriteLED(2, 10, 10, 10, 10, 10, 10, 0);
	WriteLED(1, 10, 0, 23, 15, 10, 10, 0);
	_delay_ms(czas_ms/3);
	WriteLED(1, 30, 20, 24, 16, 28, 29, 0);
	_delay_ms(czas_ms/3);
	WriteLED(1, 18, 12, 28, 12, 18, 16, 0);
	_delay_ms(czas_ms/3);

	return;

}



int main()
{
	// konfiguracja startowa
	init();

	pokaz_napis_OTG(3000);

	while(1)
	{
		tryb_naliczanie_paliwa();
	}

	main();
		
	return 0;
}

#define UC unsigned char

void WriteLED( char ktora_linijka, UC a, UC b, UC c, UC d, UC e, UC f, UC dot_pos)
{
	char test = 0;
	led_bufor[ktora_linijka][0] = znak[a];
	led_bufor[ktora_linijka][1] = znak[b];
	led_bufor[ktora_linijka][2] = znak[c];
	led_bufor[ktora_linijka][3] = znak[d];
	led_bufor[ktora_linijka][4] = znak[e];
	led_bufor[ktora_linijka][5] = znak[f];

	if(dot_pos)
		led_bufor[ktora_linijka][dot_pos-1] += 128;

	return;
}

ISR(TIMER2_OVF_vect)   	// przerwanie od przepelnienia Timera2
{
#define DIFF 0.023
//#define DIFF 0.251
	if(!(counter % 16) && counter != 0)
	{
		litry = litry + DIFF;
		kwota = kwota + (cena * DIFF);
	//	led_bufor[2][0] += 1;
	}
	if(!(counter % 30000) && counter != 0)
	{
		TCCR2 &= 0xF8;	// timer stop
		pokaz_napis_OTG(5000);
		TCCR2 |= 0x05;	// continue
		WriteLED(2, 10, (int)cena%10, (int)(10*cena)%10, (int)(100*cena)%10, 10, 10, 2);
	}
	counter++;
}

ISR(TIMER0_OVF_vect)   	// przerwanie od przepelnienia Timera0
{
	//TCCR0 &= ~(0x03/*3*/);
	switch(LEDmultiplex)
	{
	case 1:					 PORTB = 0x00; 
			LEDmultiplex++;  PORTA = 0x01; PORTC = 255 - led_bufor[0][0]; break; 
	case 2: LEDmultiplex++;  PORTA <<= 1;  PORTC = 255 - led_bufor[0][1]; break;
	case 3: LEDmultiplex++;  PORTA <<= 1;  PORTC = 255 - led_bufor[0][2]; break;
	case 4: LEDmultiplex++;  PORTA <<= 1;  PORTC = 255 - led_bufor[0][3]; break;
	case 5: LEDmultiplex++;  PORTA <<= 1;  PORTC = 255 - led_bufor[0][4]; break;
	case 6: LEDmultiplex++;  PORTA <<= 1;  PORTC = 255 - led_bufor[0][5]; break;
	case 7: LEDmultiplex++;  PORTA <<= 1;  PORTC = 255 - led_bufor[1][5]; break;
	case 8: LEDmultiplex++;  PORTA <<= 1;  PORTC = 255 - led_bufor[1][4]; break;

	case 9: 				 PORTA = 0x00; 				
			LEDmultiplex++;  PORTB = 0x01; PORTC = 255 - led_bufor[2][0]; break;
	case 10:LEDmultiplex++;  PORTB <<= 1;  PORTC = 255 - led_bufor[2][1]; break;
	case 11:LEDmultiplex++;  PORTB <<= 1;  PORTC = 255 - led_bufor[2][2]; break;
	case 12:LEDmultiplex++;  PORTB <<= 1;  PORTC = 255 - led_bufor[2][3]; break;
	case 13:LEDmultiplex++;  PORTB <<= 1;  PORTC = 255 - led_bufor[1][0]; break;
	case 14:LEDmultiplex++;  PORTB <<= 1;  PORTC = 255 - led_bufor[1][1]; break;
	case 15:LEDmultiplex++;  PORTB <<= 1;  PORTC = 255 - led_bufor[1][2]; break;
	case 16:LEDmultiplex=1;  PORTB <<= 1;  PORTC = 255 - led_bufor[1][3]; break;
	}
	//TCCR0 |= 0x02;

}