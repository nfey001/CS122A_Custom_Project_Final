/*
Name & Email: Nathaniel Fey, nfey001@ucr.edu
Lab Section: 021
Assignment: CS122A Custom Lab Project

I acknowledge all content contained herein, excluding template or example code, is my own original work.
*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include "Bit.h"
#include "Keypad.h"
#include "Timer.h"
#include "LCD.h"
#include "Joystick.h"
#include "usart_ATMega1284.h"

#define BUTTON (~PINA & 0x04) // defines button on PORTA Pin 3
const unsigned char MAX_SIZE = 3; // max size for password
uint8_t userPW[3]; // variable to hold user password entered
uint8_t readPW[3]; // holds eeprom values at an address
// Shared variables
static unsigned char KeypadInput; // global char to hold user input from keypad

uint8_t defPW[3]; // default password, can be changed by user
uint8_t checkPW[3]; // PW array to check user input
unsigned char passwordFlag = 0; // turns to 1 when password is guessed correctly
unsigned char resetFlag = 0; // turns to 1 when password reset is prompted
unsigned char LEDFlag = 0; // turns to 1 when LED is on
unsigned char currPWFlag = 0; // turns to 1 when current PW flag is entered correctly

// flags that are set to 1 when user enters a key
unsigned char pw1_flag = 0;
unsigned char pw2_flag = 0;
unsigned char pw3_flag = 0;

// flags are set to 1 when user enters a key during password change
unsigned char change_pw1_flag = 0;
unsigned char change_pw2_flag = 0;
unsigned char change_pw3_flag = 0;
unsigned char temp = 0x00; // temp is used to send data to atmega1284 servant

// Function prototyping
void selectionTick();
void setPW();
void getPW();
void combineTick();
void buttonTick();
void updatePW();
void servo_90_degrees();
void toggleLedFlag();

//-------------------- Enumeration of states.---------------------------------
enum KeypadInputState { Keypad_Start, Keypad_wait, Keypad_key } key_state = Keypad_Start;
enum passwordState {Password_Start, Password_insert_1, Password_insert_2, Password_insert_3,Password_enter, Password_check, Password_loop,
reset_insert_1, reset_insert_2, reset_insert_3} password_state = Password_Start;
enum displayPW {Display_Start, DisplayPW} display_state = Display_Start;
enum passwordChange{Change_start, change_enter_1, change_enter_2, change_enter_3, change_entered, change_check, change_loop} change_state = Change_start;
enum selectState {SELECT_START, SELECT_LED, SELECT_SERVO} select_state = SELECT_START;
enum combineState {COMBINE_START, COMBINE_TICKS} combine_state = COMBINE_START;
enum ButtonState {BUTTON_WAIT, BUTTON_PRESSED, BUTTON_HELD, BUTTON_RELEASED} button_state;

// --------------- End of Enumeration of States ----------------------------------

// Password SM that provides the password insert algorithm
// This SM acts as the start of the system, it prompts user input for password
void passwordTick()
{
	switch(password_state) // transitions
	{
		case Password_Start: // Prompt user for password
		// set character flags to 0
		pw1_flag = 0;
		pw2_flag = 0;
		pw3_flag = 0;
		
		password_state = Password_insert_1;
		break;
		case Password_insert_1:
		if (pw1_flag == 1 ) // first character entered, prompting second character
		{
			password_state = Password_insert_2;
		}
		if (KeypadInput == '#') // # is the reset key
		{
			password_state = reset_insert_1; // prompt to user to reset password
		}
		break;
		case Password_insert_2:
		if (pw2_flag == 1) // second character entered, prompting third character
		{
			password_state = Password_insert_3;
		}
		break;
		case Password_insert_3:
		if (pw3_flag == 1) // last character is entered
		{
			password_state = Password_enter; // go to enter
		}
		break;
		case Password_enter:
		KeypadInput = GetKeypadKey();
		if (KeypadInput == '*') // asterisk is enter key
		{
			password_state = Password_check; // go to check state
		}
		if (KeypadInput == '#') // # is the reset key
		{
			//resetFlag = 1; // set the reset flag
		}
		break;
		case Password_check:
		if (passwordFlag == 0) // if password is incorrect, start over
		{
			password_state = Password_Start;
		}
		else
		password_state = Password_loop; // loop until password reset is prompted (during LED/Servo motor selection)
		break;
		case reset_insert_1: // imagine this is PASSWORD INSERT 1
		if(pw1_flag == 1) // first character entered
		{
			password_state = reset_insert_2;
		}
		break;
		case reset_insert_2:
		if(pw2_flag == 1) // second character entered
		{
			password_state = reset_insert_3;
		}
		break;
		case reset_insert_3:
		if (pw3_flag == 1) // third character entered
		{
			
			if (currPWFlag == 1) // if the reset button was pressed after correct input, go to the password loop
			{
				currPWFlag = 0;
				password_state = Password_loop;
			}
			else
			password_state = Password_Start; // password reset, return to enter password state
		}
		break;
		case Password_loop:
		password_state = Password_loop;
		break;
	}
	
	switch(password_state) // actions
	{
		case Password_Start:
		
		break;
		case Password_insert_1:
		// delay of 500 ms for key press
		delay_ms(500);
		KeypadInput = GetKeypadKey();
		if (KeypadInput == '\0')
		{
			if (passwordFlag == 1)
			{
				LCD_DisplayString(1,"Incorrect PW");
			}
			else
			for(int i = 1; i <= MAX_SIZE; i++)
			{
				readPW[i-1] = eeprom_read_byte(i-1); // variable to hold the value of eeprom's specific address
				
			}
			LCD_DisplayString(1,"Enter PW");
		}
		if(KeypadInput == '#')
		{
			//resetFlag = 1;
		}
		else if (KeypadInput != '\0')
		{
			userPW[0] = KeypadInput; // insert guess into userPW
			pw1_flag = 1; // set flag that first character was entered
			
		}
		break;
		case Password_insert_2:
		delay_ms(500);
		
		KeypadInput = GetKeypadKey();
		if (KeypadInput == '\0')
		{
			LCD_DisplayString(1,"*");
		}
		if (KeypadInput != '\0')
		{
			userPW[1] = KeypadInput; //insert guess into userPw
			pw2_flag = 1; // set flag that second character was entered
			
		}
		
		break;
		case Password_insert_3:
		delay_ms(500);
		KeypadInput = GetKeypadKey();
		if (KeypadInput == '\0')
		{
			
			LCD_DisplayString(1,"**");
			
		}
		if (KeypadInput != '\0')
		{
			userPW[2] = KeypadInput; // insert guess into userPW
			pw3_flag = 1; // set flag that the third character was entered
			
		}
		break;
		case Password_enter:
		delay_ms(500);
		// user presses "*" to enter
		LCD_DisplayString(1,"***");
		break;
		case Password_check:
		delay_ms(500);
		// for loop compares the password user enters with the default/current password from eeprom (readPW array)
		for (int i = 0; i < MAX_SIZE; i++)
		{
			if (userPW[i] == readPW[i])
			{
				checkPW[i] = 1;// if both userPW and readPW character matches, enter a 1 to the checkPW
			}
			else
			checkPW[i] = 0; // enter a 0 meaning that the passwords do not match
			
		}
		// for loop iterates through the checkPW array to see if there are any 0's, if so do not set passwordFlag as correct
		for (int i = 0; i < MAX_SIZE; i++)
		{
			if(checkPW[i] == 0)
			{
				passwordFlag = 0; // incorrect password
				break;
				
			}
			else
			passwordFlag = 1; // correct password
		}
		break;
		
		case reset_insert_1: // reset password prompt state
		LCD_DisplayString(1,"Enter new PW");
		delay_ms(500);
		KeypadInput = GetKeypadKey();
		if (KeypadInput != '\0')// if not idle
		{
			defPW[0] = KeypadInput;// insert input into defPW
			eeprom_update_byte(0,defPW[0]); // update address 0 with new character
			pw1_flag = 1; // first character has been entered and updated
			
		}
		break;
		case reset_insert_2:
		LCD_DisplayString(1,"*");
		delay_ms(500);
		KeypadInput = GetKeypadKey();
		if (KeypadInput != '\0')
		{
			defPW[1] = KeypadInput;
			eeprom_update_byte(1,defPW[1]);// update address 1 with new character
			pw2_flag = 1; // second character has been entered and updated
		}
		break;
		case reset_insert_3:
		LCD_DisplayString(1,"**");
		delay_ms(500);
		KeypadInput = GetKeypadKey();
		if (KeypadInput != '\0')
		{
			defPW[2] = KeypadInput;
			eeprom_update_byte(2,defPW[2]);// update address 2 with new character
			pw3_flag = 1; // third character has been entered and updated
			//password_state = reset_insert_1;
		}
		break;
		case Password_loop:
		delay_ms(500);
		KeypadInput = GetKeypadKey();
		if (passwordFlag == 1) // if password entered is correct, do not allow key press except for '#' reset key
		{
			if (KeypadInput != '\0')
			{
				password_state = Password_loop; // Password loop ensures only the '#' key is allowed to be entered
			}
			if (resetFlag == 1) // reset is set when user hits '#' in the servo/LED selection and is current pw is entered correctly
			{
				password_state = reset_insert_1;
			}
		}
		break;
	}
}


int main(void)
{
	DDRA = 0x00; PORTA = 0xFF; // Button Input
	DDRB = 0xF0; PORTB = 0x0F; // half input, half output for keypad input
	DDRC = 0xFF; PORTC = 0x00; // PC7..4 outputs init 0s, PC3..0 inputs init 1s LCD
	DDRD = 0xFF; PORTD = 0x00; // Servo motor on PORTD
	LCD_init(); // initialize LCD
	ADC_init(); // Initialize ADC for joystick input
	
	button_state = BUTTON_WAIT;
	change_state = Change_start;
	password_state = Password_Start;
	select_state = SELECT_START;
	combine_state = COMBINE_START;
	
	TimerSet(125);
	TimerOn();
	initUSART(0); // using usart0
	setPW(); // set up the default password
	while (1)
	{
		
		combineTick();
	}

}


//-------- Finite State Machines---------------------------------------------------
// This State Machine will display a prompt on the LCD Screen for LED light and Servo activation
void selectionTick()
{
	switch(select_state)
	{
		case SELECT_START:
		if (passwordFlag == 1) // only allow a button press/joystick input if the password was correct
		{
			select_state = SELECT_LED;
		}
		else
		select_state = SELECT_START;
		break;
		case SELECT_LED:
		// if joystick is moved to the right, stay on LED
		if(getPosition() == RIGHT)
		{
			select_state = SELECT_LED;
			position = 0;
			dispPos = 1;
		}
		// if joystick is moved to the left, go to servo motor selection
		if(getPosition() == LEFT )
		{
			select_state = SELECT_SERVO;
			position = 0;
			dispPos = 2;
		}
		break;
		case SELECT_SERVO:
		if (getPosition() == RIGHT) // if joystick is moved to the right, go to LED
		{
			select_state = SELECT_LED;
			position = 0;
			dispPos = 1; // current position is updated (1 for LED)
		}
		if (getPosition() == LEFT) // if joystick is moved to the left, stay on servo
		{
			dispPos = 2; // current position is updated (2 for servo)
			select_state = SELECT_SERVO;
			position = 0;

		}
		break;
	}
	
	switch(select_state)
	{
		case SELECT_START:
		// do nothing
		break;
		case SELECT_LED:
		if (passwordFlag == 1) // Button will only respond if password was entered correctly
		{
			LCD_ClearScreen();
			LCD_DisplayString(1,"Press for LED");
			if (BUTTON)
			{
				toggleLedFlag();
				if(USART_IsSendReady(0)) // USART is ready, set a value to temp based on LEDFlag
				{
					USART_Flush(0);
					if (LEDFlag == 1)
					{
						temp = 0xFF;
					}
					if(LEDFlag == 0)
					{
						temp = 0x00;
					}
					
					USART_Send(temp,0);
					if (USART_HasTransmitted(0))
					{
						
					}
				}
			}
		}
		break;
		case SELECT_SERVO:
		if (passwordFlag == 1) // displays servo motor prompt, buttonTick has servo functionality
		{
			LCD_ClearScreen();
			LCD_DisplayString(1,"Press for Servo");
		}
		break;
	}
}

void buttonTick()
{
	
	switch(button_state) // transitions
	{
		case BUTTON_WAIT:
		if (BUTTON)
		{
			button_state = BUTTON_PRESSED;
		}
		
		break;
		case BUTTON_PRESSED:
		button_state = BUTTON_HELD;
		break;
		case BUTTON_HELD:
		if (BUTTON)
		{
			button_state = BUTTON_HELD;
		}
		else
		button_state = BUTTON_RELEASED;
		break;
		case BUTTON_RELEASED:
		button_state = BUTTON_WAIT;
		break;
	}
	
	switch(button_state)
	{
		case BUTTON_WAIT:
		break;
		case BUTTON_PRESSED:
		
		
		break;
		case BUTTON_HELD:
		
		if (dispPos == 2) // if display is on servo motor, button press will activate the servo motor
		{
			servo_90_degrees();
			// this is the servo motor, it will rotate 90 degrees and return to its starting position
			
		}
		else
		
		break;
		case BUTTON_RELEASED:
		break;
	}
}
void changePWTick()
{
	switch(change_state)
	{
		case Change_start:
		change_pw1_flag = 0;
		change_pw2_flag = 0;
		change_pw3_flag = 0;
		if(passwordFlag == 1)
		{
			KeypadInput = GetKeypadKey();
			if (KeypadInput == '#')
			{
				
				change_state = change_enter_1;
			}
			else
			change_state = Change_start;
		}
		break;
		case change_enter_1:
		if (change_pw1_flag == 1 ) // first character entered, prompting second character
		{
			change_state = change_enter_2;
		}
		break;
		case change_enter_2:
		if (change_pw2_flag == 1 ) // second character entered, prompting third character
		{
			change_state = change_enter_3;
		}
		break;
		case change_enter_3:
		if (change_pw3_flag == 1 ) // third character entered, check for "*" entered
		{
			change_state = change_entered;
		}
		break;
		case change_entered:
		KeypadInput = GetKeypadKey();
		if (KeypadInput == '*') // asterisk is enter key
		{
			change_state = change_check; // go to check state
		}
		break;
		case change_check:
		if (resetFlag == 0) // if password is incorrect, start over
		{
			change_state = Change_start;
		}
		else
		//resetFlag = 0;
		resetFlag = 1;
		change_state = change_loop;
		// loop until password reset is prompted (during LED/Servo motor selection)
		break;
		case change_loop:
		currPWFlag = 0;
		if (currPWFlag == 1)
		{
			change_state = Change_start;
		}
		else
		change_state = change_loop;
		break;
	}
	
	switch(change_state) // actions
	{
		case Change_start:
		
		break;
		case change_enter_1:
		LCD_ClearScreen();
		LCD_DisplayString(1,"Enter curr PW"); // enter the current password
		delay_ms(500);
		KeypadInput = GetKeypadKey();
		if (KeypadInput == '\0')
		{
			
		}
		if (KeypadInput != '\0')
		{
			userPW[0] = KeypadInput; // insert guess into userPW
			change_pw1_flag = 1; // set flag that first character was entered
			
		}
		break;
		case change_enter_2:
		LCD_ClearScreen();
		KeypadInput = GetKeypadKey();
		if (KeypadInput == '\0')
		{
			LCD_DisplayString(1,"*");
		}
		if (KeypadInput != '\0')
		{
			userPW[1] = KeypadInput; //insert guess into userPw
			change_pw2_flag = 1; // set flag that second character was entered
			
		}
		break;
		case change_enter_3:
		
		KeypadInput = GetKeypadKey();
		if (KeypadInput == '\0')
		{
			LCD_DisplayString(1,"**");
		}
		if (KeypadInput != '\0')
		{
			userPW[2] = KeypadInput; //insert guess into userPw
			change_pw3_flag = 1; // set flag that second character was entered
		}
		break;
		case change_entered:
		
		KeypadInput = GetKeypadKey();
		LCD_ClearScreen();
		if (KeypadInput == '\0')
		{
			
			LCD_DisplayString(1,"***");
		}
		if (KeypadInput == '*') // asterisk is enter key
		{
			change_state = change_check; // go to check state
		}
		break;
		case change_check:
		
		// for loop compares the password user enters with the default/current password from eeprom (readPW array)
		for (int i = 0; i < MAX_SIZE; i++)
		{
			if (userPW[i] == readPW[i])
			{
				checkPW[i] = 1;// if both userPW and readPW character matches, enter a 1 to the checkPW
			}
			else
			checkPW[i] = 0; // enter a 0 meaning that the passwords do not match
		}
		// for loop iterates through the checkPW array to see if there are any 0's, if so do not set passwordFlag as correct
		for (int i = 0; i < MAX_SIZE; i++)
		{
			if(checkPW[i] == 0)
			{
				resetFlag = 0; // incorrect password
				break;
			}
			else
			//passwordFlag = 1; // correct password
			resetFlag = 1;// prompt reset
			
		}
		break;
		case change_loop:
		
		KeypadInput = GetKeypadKey();
		if (passwordFlag == 1) // if password entered is correct, do not allow key press except for '#' reset key
		{
			if (KeypadInput != '\0')
			{
				change_state = change_loop; // Password loop ensures only the '#' key is allowed to be entered
			}
			if (KeypadInput == '#') // reset password key is the only key accepted
			{
				currPWFlag = 1;
			}
		}
		break;
	}
}
void combineTick()
{
	switch (combine_state)
	{
		case COMBINE_START:
		combine_state = COMBINE_TICKS;
		break;
		case COMBINE_TICKS:
		combine_state = COMBINE_TICKS;
		break;
	}
	
	switch(combine_state)
	{
		case COMBINE_START:
		break;
		case COMBINE_TICKS:
		passwordTick();
		selectionTick();
		buttonTick();
		changePWTick();
		break;
	}
}

// -------------- Custom Functions -------------------------------------------
void setPW()
{
	defPW[0] = 1;
	defPW[1] = 2;
	defPW[2] = 3;
	
}

void servo_PWM_off() // turn off servo motor
{
	TCCR1A = 0x00; // this tells the servo to provide no output
	
}
// function to update password in eeprom
void updatePW()
{
	for (int i = 0; i < MAX_SIZE; i++)
	{
		eeprom_update_byte(i,defPW[i]); // update character in a different memory location in each iteration
	}
	
}
// toggleLedFlag function simply switches between 1 and 0 based on its previous value
void toggleLedFlag()
{
	if (LEDFlag == 1) // if else statement toggles LED light
	{
		LEDFlag = 0;
	}
	else
	LEDFlag = 1;
}
// This function allows the servo motor to rotate 90 degrees and return back to its original position
void servo_90_degrees()
{
	TCCR1A |= (1<<COM1A1) | (1<<COM1B1) | (1<<WGM11); // output mode is configured
	TCCR1B |= (1<<WGM13) | (1<<WGM12) | (1<<CS11) | (1<<CS10);
	ICR1 = 2499;
	OCR1A = 97;
	
	_delay_ms(1500);
	OCR1A = 175;
	_delay_ms(1500);
	OCR1A = 300;
	_delay_ms(1500);
	OCR1A = 535;
	
	
	servo_PWM_off(); // turn off servo
}