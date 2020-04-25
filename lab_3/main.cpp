#include <dos.h>

struct VIDEO {
	unsigned char sym;
	unsigned char attr;
};

// IRQ 0-7
void interrupt(*int0) (...); // timer
void interrupt(*int1) (...); // keyboard
void interrupt(*int2) (...); // cascade switching of the second controller
void interrupt(*int3) (...); // port COM 2
void interrupt(*int4) (...); // port COM 1
void interrupt(*int5) (...); // port LPT 2
void interrupt(*int6) (...); // floppy drive
void interrupt(*int7) (...); // port LPT 1

// IRQ 8-15
void interrupt(*int8) (...); // real time clock
void interrupt(*int9) (...); // free
void interrupt(*intA) (...); // video adapter
void interrupt(*intB) (...); // free
void interrupt(*intC) (...); // mouse PS/2
void interrupt(*intD) (...); // math coprocessor
void interrupt(*intE) (...); // first hard drive controller
void interrupt(*intF) (...); // second hard drive controller

void get_registers_data();
void print_video_data(int offset, int);
int color = 0x00;

// New interrupt handlers
void interrupt new0(...) { get_registers_data(); int0(); }
void interrupt new1(...) { get_registers_data(); int1(); }
void interrupt new2(...) { get_registers_data(); int2(); }
void interrupt new3(...) { get_registers_data(); int3(); }
void interrupt new4(...) { get_registers_data(); int4(); }
void interrupt new5(...) { get_registers_data(); int5(); }
void interrupt new6(...) { get_registers_data(); int6(); }
void interrupt new7(...) { get_registers_data(); int7(); }

void interrupt new8(...) { get_registers_data(); int8(); }
void interrupt new9(...) { color++; get_registers_data(); int9(); } // keyboard int9
void interrupt newA(...) { get_registers_data(); intA(); }
void interrupt newB(...) { get_registers_data(); intB(); }
void interrupt newC(...) { get_registers_data(); intC(); }
void interrupt newD(...) { get_registers_data(); intD(); }
void interrupt newE(...) { get_registers_data(); intE(); }
void interrupt newF(...) { get_registers_data(); intF(); }

void print_video_data(int offset, int val) {
	char temp;

	 // Getting video buffer
	VIDEO far* screen = (VIDEO far *)MK_FP(0xB800, 0);

	screen += offset;
	for (int i = 0; i < 8; i++) {
		temp = val % 2;           // getting last bit
		val /= 2;                 // filling screen
		screen->sym = temp + '0'; // saving bit as a symbol
		screen->attr = color;     // setting color
		screen++; 					      // next symbol
	}
}

void get_registers_data() {
	int rg;

	// Master interrupts controller
	rg = inp(0x21);  // mask register
	print_video_data(0, rg);

	outp(0x20, 0x0B);
	rg = inp(0x20);	 // service register
	print_video_data(9, rg);

	outp(0x20, 0x0A);
	rg = inp(0x20);	 // requests register
	print_video_data(18, rg);

	// Slave interrupts controller
	rg = inp(0xA1);	 // mask register
	print_video_data(80, rg);

	outp(0xA0, 0x0B);
	rg = inp(0xA0);	 // service register
	print_video_data(80 + 9, rg);

	outp(0xA0, 0x0A);
	rg = inp(0xA0);	 // requests register
	print_video_data(80 + 18, rg);
}

void init() {

	// IRQ 0-15
	int8 = getvect(0x08);
	int9 = getvect(0x09);
	intA = getvect(0x0A);
	intB = getvect(0x0B);
	intC = getvect(0x0C);
	intD = getvect(0x0D);
	intE = getvect(0x0E);
	intF = getvect(0x0F);
	int0 = getvect(0x70);
	int1 = getvect(0x71);
	int2 = getvect(0x72);
	int3 = getvect(0x73);
	int4 = getvect(0x74);
	int5 = getvect(0x75);
	int6 = getvect(0x76);
	int7 = getvect(0x77);

	// Redefining IRQ 0-7
	setvect(0xD0, new8);
	setvect(0xD1, new9);
	setvect(0xD2, newA);
	setvect(0xD3, newB);
	setvect(0xD4, newC);
	setvect(0xD5, newD);
	setvect(0xD6, newE);
	setvect(0xD7, newF);

	// Redefining IRQ 8-15
	setvect(0x08, new0);
	setvect(0x09, new1);
	setvect(0x0A, new2);
	setvect(0x0B, new3);
	setvect(0x0C, new4);
	setvect(0x0D, new5);
	setvect(0x0E, new6);
	setvect(0x0F, new7);

	_disable(); // CLI - disabling interrupts handling

	// Master Interrupts Controller
	outp(0x20, 0x11);	// ICW1 - initialization of the master controller
	outp(0x21, 0xD0);	// ICW2 - base vector for master
	outp(0x21, 0x04);	// ICW3 - slave bit port in binary format
	outp(0x21, 0x01);	// ICW4 - default

	// Slave Interrupts Controller
	outp(0xA0, 0x11); // ICW1 - initialization of the slave controller
	outp(0xA1, 0x08);	// ICW2 - base vector for slave
	outp(0xA1, 0x02);	// ICW3 - number of ports connected to the master
	outp(0xA1, 0x01);	// ICW4 - default

	_enable(); // STI - resuming interrupts handling
}

int main() {
	unsigned far *fp;
	init();

	FP_SEG(fp) = _psp; // getting segment
	FP_OFF(fp) = 0x2c; // geting offset
	_dos_freemem(*fp);

  // Terminate-And-Stay-Resident DOS program
	_dos_keep(0, (_DS - _CS) + (_SP / 16) + 1);
	return 0;
}
