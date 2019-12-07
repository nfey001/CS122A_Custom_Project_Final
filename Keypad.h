#ifndef KEYPAD_H
#define KEYPAD_H

#define SET_BIT(p,i) ((p) |= (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GetBit(p,i) ((p) & (1 << (i)))

// Returns '\0' if no key pressed, else returns char '1', '2', ... '9', 'A', ...
// If multiple keys pressed, returns leftmost-topmost one
// Keypad must be connected to port C
/* Keypad arrangement
        PC4 PC5 PC6 PC7
   col  1   2   3   4
row
PC0 1   1 | 2 | 3 | A
PC1 2   4 | 5 | 6 | B
PC2 3   7 | 8 | 9 | C
PC3 4   * | 0 | # | D
*/
unsigned char GetKeypadKey() {

	PORTB = 0xEF; // Enable col 4 with 0, disable others with 1’s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINB,0)==0) { return(1); }// Returns a number, not a char
	if (GetBit(PINB,1)==0) { return(4); }// Returns a number, not a char
	if (GetBit(PINB,2)==0) { return(7); }// Returns a number, not a char
	if (GetBit(PINB,3)==0) { return('*'); }

	// Check keys in col 2
	PORTB = 0xDF; // Enable col 5 with 0, disable others with 1’s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINB,0)==0) { return(2); }// Returns a number, not a char
	if (GetBit(PINB,1)==0) { return(5); }// Returns a number, not a char
	if (GetBit(PINB,2)==0) { return(8); }// Returns a number, not a char
	if (GetBit(PINB,3)==0) { return(0); }// Returns a number, not a char
	// ... *****FINISH*****

	// Check keys in col 3
	PORTB = 0xBF; // Enable col 6 with 0, disable others with 1’s
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINB,0)==0) { return(3); }// Returns a number, not a char
	if (GetBit(PINB,1)==0) { return(6); }// Returns a number, not a char
	if (GetBit(PINB,2)==0) { return(9); }// Returns a number, not a char
	if (GetBit(PINB,3)==0) { return('#'); }
	// ... *****FINISH*****

	// Check keys in col 4	
	PORTB = 0x7F;
	asm("nop"); // add a delay to allow PORTC to stabilize before checking
	if (GetBit(PINB,0)==0) { return('A'); }
	if (GetBit(PINB,1)==0) { return('B'); }
	if (GetBit(PINB,2)==0) { return('C'); }
	if (GetBit(PINB,4)==0) { return('D'); }
	// ... *****FINISH*****

	return('\0'); // default value

}

#endif