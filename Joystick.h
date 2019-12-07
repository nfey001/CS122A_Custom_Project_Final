/*
Name & Email: Nathaniel Fey, nfey001@ucr.edu
Lab Section: 021
Assignment: CS122A Custom Lab Project

I acknowledge all content contained herein, excluding template or example code, is my own original work.
*/
#ifndef JOYSTICK_H
#define JOYSTICK_H
#include <stdio.h>
#include <avr/interrupt.h>
#include <avr/io.h>
static unsigned char position = 0;
static unsigned char dispPos = 0;
unsigned short tmpA = 0x0000;
#define RIGHT 1
#define LEFT 2
/*------------------------------ JOYSTICK FUNCTIONALITY------------------------*/
void ADC_init()
{
	ADCSRA |= (1 <<ADEN) | (1 << ADSC) | (1 << ADATE);
	ADMUX |= 1<<REFS0 | 1 <<REFS1;

}


unsigned char getPosition()
{

	if (ADMUX  == 0xC0)
	{
		ADMUX = 0xC1; // switch between ADC1->ADC0 X & Y axis
	}
	else if (ADMUX == 0xC1)
	{
		ADMUX = 0xC0; // switch between ADC0->ADC1
	}
	
	
	tmpA = ADC; // set tmpA to the ADC value
	if (tmpA < 200 && ADMUX == 0xC0) // right
	{
		position = RIGHT;
		
	}
	else if (tmpA > 1000 && ADMUX == 0xC1) // left
	{
		position = LEFT;
		
	}

	return position;
}

#endif