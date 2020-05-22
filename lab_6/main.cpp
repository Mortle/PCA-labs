#include <dos.h>
#include <conio.h>
#include <stdio.h>

#define ATTEMPTS 3

struct VIDEO {
	unsigned char symbol;
	unsigned char attribute;
};

unsigned char mask[] = {
	0x02,  0x06, 0x05, 0x02, 0x02, 0x06, 0x02, 0x05, 0
};

const unsigned char defaultAttribute = 0x40;

void printScanCode(int scanCode);

int escNotUsed = 1;

void interrupt(*oldKeyboardHandler)(...);
void interrupt newKeyboardHandler(...) {
	int numberOfAttempt = 0;
	unsigned char scanCode = 0;
	scanCode = inp(0x60);
	if (scanCode == 0x01) escNotUsed = 0;

	if (scanCode == 0xFE && numberOfAttempt != ATTEMPTS) {
		numberOfAttempt++;
	}
	if (numberOfAttempt == ATTEMPTS) {
		puts( "Processing keyboard error!" );
	}
	else {
		printScanCode(scanCode);
	}	
	outp(0x20, 0x20);							// reset interrupts controller
}

void setNewKeyboardHandler() {
	_disable();
	oldKeyboardHandler = getvect(0x09); 
	setvect(0x09, newKeyboardHandler);
	_enable();
}

void resetNewKeyboardHandler() {
	_disable();
	setvect(0x09, oldKeyboardHandler);
	_enable();
}

void printScanCode(int scanCode) {
	VIDEO far* screen = (VIDEO far*)MK_FP(0xB800, 0);
	if (scanCode != 0xFE && scanCode != 0xFA) {
		for (int i = 1; i >= 0; i--) {
			int fourBitsOfScanCode = ((scanCode >> (i * 4)) & 0x0F);
			if (fourBitsOfScanCode < 10) {
				screen->symbol = fourBitsOfScanCode + '0';
			}
			else {
				screen->symbol = fourBitsOfScanCode - 10 + 'A';
			}

			screen->attribute = defaultAttribute;
			screen++;
		}
	}
}

void writeToPort(unsigned char mask);
void blink() {
	for (int i = 0; mask[i] != 0; i++) {
		writeToPort(0xED);										// set blink bits
		delay(200);
		writeToPort(mask[i]);
		delay(200);

		writeToPort(0xED);										// reset bits disabling blink
		delay(200);
		writeToPort(0x00);
		delay(200);
	}
}

void writeToPort(unsigned char mask) {
	for (int i = 0; i <= ATTEMPTS; i++) {
		while (inp(0x64) & 0x02);				// wait untill buffer is empty
		outp(0x60, mask);
		delay(50);
		
	}
}

int main() {

	setNewKeyboardHandler();
	blink();
	while (escNotUsed);
	resetNewKeyboardHandler();

	return 0;
}
