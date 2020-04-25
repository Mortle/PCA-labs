#include <dos.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>

int melody[] = { 784, 392, 440, 440, 784, 440, 493, 493 };
int ports[] = { 0x40, 0x41, 0x42 };
int channels[] = { 0xE2, 0xE4, 0xE6 };

const int MELODY_LENGTH = 8;

void play_melody() {
	for (int i = 0; i < MELODY_LENGTH; i++) {
		long base = 1193180, kd;
		outp(0x43, 0xB6);
		kd = base / melody[i];
		outp(0x42, kd % 256);
		kd /= 256;
		outp(0x42, kd);

		outp(0x61, inp(0x61) | 3);
		delay(200);
		outp(0x61, inp(0x61) & 0xfc);
	}
}

void print_random() {
	long max;

	printf("Enter random cap (0..65536): ");
	while (!scanf("%ld", &max) || max < 1 || max > 65535) {
		printf("Try again: ");
		rewind(stdin);
	}

	outp(0x43, 0xB4);
	outp(0x42, max % 256);
	max /= 256;
	outp(0x42, max);
	outp(0x61, inp(0x61) | 1);

	rewind(stdin);

	unsigned int num, num_low, num_high;
	outp(0x43, 128);
	num_low = inp(0x42);
	num_high = inp(0x42);
	num = num_high * 256 + num_low;
	printf("\nRandom number: %u\n", num);
	outp(0x61, inp(0x61) & 0xFD);
}

void print_division_ratio() {
  // 0 - 00000000, 64 - 01000000, 128 - 10000000
	int blocks[] = { 0, 64, 128 };
	for (int channel = 0; channel < 3; channel++) {
    unsigned dr_low, dr_high, dr = 0, max = 0;

		if (channel == 2) {
			outp(0x61, inp(0x61) | 3);
    }
		for (unsigned long j = 0; j < 65536; j++) {
			outp(0x43, blocks[channel]);
			dr_low = inp(ports[channel]);
			dr_high = inp(ports[channel]);
			dr = dr_high * 256 + dr_low;
		}
		if (dr > max) {
			max = dr;
    }
		if (channel == 2) {
			outp(0x61, inp(0x61) & 0xFC);
    }
		printf("%X, channel %d\n", max, channel);
	}
}

void print_condition_words() {
  for (int i = 0; i < 3; i++) {
    unsigned char state;

    outp(0x43, channels[i]);
    state = inp(ports[i]);
    for (int i = 0; i < 8; i++) {
      printf("%c", state % 2 + '0');
      state /= 2;
    }
    printf("\n");
  }
}

int main() {
	char choice;
	while (1) {
		printf("1: Play melody \n");
		printf("2: Print condition words\n");
		printf("3: Print division ratio\n");
		printf("4: Generate random number\n");
		printf("5: Exit\n\n");
		choice = getch();
		system("cls");
		switch (choice) {
		case '1':
			play_melody();
			getch();
			break;
		case '2':
      print_condition_words();
			getch();
			break;
		case '3':
			print_division_ratio();
			getch();
			break;
		case '4':
			print_random();
			break;
		case '5':
			return 0;
		default:
			printf("Error, Please try again!\n");
			break;
		}
	}
}
