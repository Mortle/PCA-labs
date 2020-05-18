#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <io.h>

int get_update_in_progress_flag() {
      outp(0x70, 0x0A);
      return (inp(0x71) & 0x80);
}

unsigned char get_RTC_register(int reg) {
      outp(0x70, reg);
      return inp(0x71);
}

unsigned int delay_time = 0;

char date[6];

unsigned int registers[] = { 0x00, 0x02, 0x04, 0x07, 0x08, 0x09 };

void interrupt(*old_timer)(...);
void interrupt(*old_alarm)(...);

void get_time();
void setTime();
void delay(unsigned int);
void set_alarm();
void reset_alarm();
void input_time();
unsigned char bcd_to_dec(int val);
unsigned char dec_to_bcd(int val);

void interrupt new_timer(...)
{
    delay_time++;

    outp(0x70, 0x0C);
    inp(0x71);

    outp(0x20, 0x20);
    outp(0xA0, 0x20);
}

void interrupt new_alarm(...)
{
    puts("alarm!");
    old_alarm();
    reset_alarm();
}

int main()
{
    int delay_ms;
    while (1) {
        printf("1 - Current time\n");
        printf("2 - Set time\n");
        printf("3 - Set alarm\n");
        printf("4 - Set delay\n");
        printf("5 - Exit\n");

        switch (getch()) {
        case '1':
            get_time();
            break;

        case '2':
            setTime();
            break;

        case '3':
            set_alarm();
            break;

        case '4':
            fflush(stdin);
            printf("Input delay in milliseconds: ");
            scanf("%d", &delay_ms);
            delay(delay_ms);
            break;

		case '5':
			system("cls");
			printf("Exiting...");
			delay(1200);
			return 0;

        default:
            system("cls");
            break;
        }
    }
}

void get_time()
{
    char *month_to_text[] =
    {
        "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"
    };

    system("cls");

    int i = 0;
    for (i = 0; i < 6; i++) {
        outp(0x70, registers[i]);

        date[i] = inp(0x71);
    }


    int dec_date[6];
    for (i = 0; i < 6; i++)
    {
        dec_date[i] = bcd_to_dec(date[i]);
    }

    printf(" %2d:%2d:%2d", dec_date[2], dec_date[1], dec_date[0]);
    printf(" %s, %2d, 20%2d\n", month_to_text[dec_date[4] - 1], dec_date[3], dec_date[5]);
}

void setTime()
{

    input_time();

    disable();

	while (get_update_in_progress_flag());

    outp(0x70, 0xB);
    outp(0x71, inp(0x71) | 0x80);		// restrict RTC updating

    for (int i = 0; i < 4; i++)
    {
        outp(0x70, registers[i]);
        outp(0x71, date[i]);
    }


    outp(0x70, 0xB);
    outp(0x71, inp(0x71) & 0x7F);		// allow RTC updating

    enable();
    system("cls");
}

void delay(unsigned int ms)
{
    disable();

    old_timer = getvect(0x70);
    setvect(0x70, new_timer);

    enable();

    outp(0xA1, inp(0xA1) & 0xFE); 		// OCW1 - allow IRQ8
    // 0xFE = 11111110
    outp(0x70, 0xB);
    outp(0x71, inp(0x71) | 0x40);		// allow IRQ8 interrupt
    // 0x40 = 01000000

    delay_time = 0;
    while (delay_time <= ms);

    puts("Delay's end");
    setvect(0x70, old_timer);
    return;
}

void set_alarm()
{
    input_time();

    disable();
	old_alarm = getvect(0x4A);
	setvect(0x4A, new_alarm);


	outp(0xA1, inp(0xA1) & 0xFE); // Allow IRQ8

	outp(0x70, 0x0B);
	int b = inp(0x71);
	outp(0x70, 0x0B);
	outp(0x71, b | 0x60);	// BCD, allow alarm interrupts

    outp(0x70, 0x05);
    outp(0x71, date[2]);


    outp(0x70, 0x03);
    outp(0x71, date[1]);

    outp(0x70, 0x01);
    outp(0x71, date[0]);

	printf("HOURS: %2d, MIN: %2d, SECS: %2d\n", bcd_to_dec(date[2]), bcd_to_dec(date[1]), bcd_to_dec(date[0]));

    enable();
    printf("Alarm enabled\n");
}

void reset_alarm()
{
    if (old_alarm == NULL)
    {
        return;
    }

    disable();


    setvect(0x4A, old_alarm);
    outp(0xA1, (inp(0xA0) | 0x01));

    unsigned int res;
    do
    {
        outp(0x70, 0xA);
        res = inp(0x71) & 0x80;
    } while (res);

    outp(0x70, 0x05);
    outp(0x71, 0x00);

    outp(0x70, 0x03);
    outp(0x71, 0x00);

    outp(0x70, 0x01);
    outp(0x71, 0x00);

    outp(0x70, 0xB);
    outp(0x71, (inp(0x71) & 0xDF));		// rectirct IRQ8 intterupt

    enable();
}

void input_time()
{
    int n;

    do {
        fflush(stdin);
        printf("Input hours: ");
        scanf("%i", &n);
    } while ((n > 23 || n < 0));
    date[2] = dec_to_bcd(n);

    do {
        fflush(stdin);
        printf("Input minutes: ");
        scanf("%i", &n);
    } while (n > 59 || n < 0);
    date[1] = dec_to_bcd(n);

    do {
        fflush(stdin);
        printf("Input seconds: ");
        scanf("%i", &n);
    } while (n > 59 || n < 0);
    date[0] = dec_to_bcd(n);
}

unsigned char bcd_to_dec(int val) {
   return (unsigned char) ((val / 16 * 10) + (val % 16));
}

unsigned char dec_to_bcd(int val) {
   return (unsigned char) ((val / 10 * 16) + (val % 10));
}
