/*
   Plik ds18b20.h

   (xyz.isgreat.org)  
*/

#ifndef DS18B20_H
#define DS18B20_H

/* DS18B20 przy³¹czony do portu  PD5 AVRa  */
#define SET_ONEWIRE_PORT     PORTB  |=  _BV(0)
#define CLR_ONEWIRE_PORT     PORTB  &= ~_BV(0)
#define IS_SET_ONEWIRE_PIN   PINB   &   _BV(0)
#define SET_OUT_ONEWIRE_DDR  DDRB   |=  _BV(0)
#define SET_IN_ONEWIRE_DDR   DDRB   &= ~_BV(0)

unsigned char ds18b20_ConvertT(unsigned char []);
int ds18b20_Read(unsigned char [], unsigned char []);
//int ds18b20_Write(unsigned char data1, unsigned char data2, unsigned char data3, unsigned char ROM_code[])
int ds18b20_Read_ROM(unsigned char []); 
int ds18b20_Match_ROM(unsigned char []);
void OneWireStrong(char);
unsigned char OneWireReset(void);
void OneWireWriteByte(unsigned char);
unsigned char OneWireReadByte(void);

#endif