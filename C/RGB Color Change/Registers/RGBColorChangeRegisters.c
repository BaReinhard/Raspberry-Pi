//  Access Registers, Basics taken from elinux.org
//  Authors: Brett Reinhard 
#define BCM2708_PERI_BASE        0x3F000000 // Base Address For BCM on Raspberry Pi 2 & 3, if using other RPI set to 0x20000000 
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) // GPIO controller, can be found at 0x200000 away from BASE
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)
int  mem_fd;
void *gpio_map;
// I/O access
volatile unsigned *gpio;
// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
// Using INP before OUT, will allow for the proper bits to be set to 0, before setting as an output
// If using the pin strictly for INP, you do not need to call OUT_GPIO
// Usage for these macros are as follows: INP_GPIO(12); Sets Pin 12 as On for Input
// INP_GPIO(10);OUT_GPIO(10); Sets pin 10 as Output, making sure to use INP before OUT to ensure proper bits are set
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
// Allows for setting a pin for an alternate function such as for communicating with SPI or I2C
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
#define NANO_SECOND_MULTIPLIER  1000000  // 1 millisecond = 1,000,000 Nanoseconds This is used for nanosleep

// Sets THE GPIO SET Register with the Proper information, Proper usage is as follows:
// GPIO_SET = 1 << 18; Turns Pin 18 to HIGH,ON.
// GPIO_CLR = 1 << 18; Turns Pin 18 to LOW,OFF.
// Note: Setting GPIO_SET as GPIO_SET = 0 << 18; May cause problems when using multiple pins.
// Use the SET macro to turn on, and CLR macro to turn off.
#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

#define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH aka 2 ^ pinNum, if pin 3 HIGH = 2 ^ 3 = 8
 
#define GPIO_PULL *(gpio+37) // Pull up/pull down
#define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock
 
void setup_io();
void SoftPWM(int dutyCycle, int Pin);
 
 
int main(int argc, char **argv)
{
  	int RED,GREEN,BLUE,i;
  // Choose which Pins to use, these are BCM Pin Numbers, see pinout.xyz for more pin numbers.
  	RED = 13; 
  	GREEN = 26;
  	BLUE = 19;
  	int g,rep;

  // Set up gpio pointer for direct register access
  	setup_io();

    // Sets RED Pin as Output
	INP_GPIO(RED);
    OUT_GPIO(RED);
    // Sets GREEN Pin as Output
    INP_GPIO(GREEN);
    OUT_GPIO(GREEN);
    // Sets BLUE Pin as Output
    INP_GPIO(BLUE);
    OUT_GPIO(BLUE);

    // I Have left in Previously Used Arduino C Code as a reference to see the difference between the two:
	while(1){
		//-----------------------------------------------------------------------
		// r = 255;g = 0;b = 0; for (int i = 0; i < 256; ++i){RGB(r,i,b);}
		GPIO_SET = 1 << RED;
		GPIO_CLR = 1 << GREEN;
		GPIO_CLR = 1 << BLUE;
		// RED TO YELLOW
		for (i = 1; i < 256; i++){SoftPWM(i,GREEN);}
		//-----------------------------------------------------------------------
		// r = 255; g = 255;b = 0; for (int i = 255; i >= 0; --i){RGB(i,g,b); } 
	    GPIO_SET = 1 << RED;
	    GPIO_SET = 1 << GREEN;
	    GPIO_CLR = 1 << BLUE;
	    // YELLOW TO GREEN
	    for(i = 255; i>=0;i--){SoftPWM(i,RED);}
		//-----------------------------------------------------------------------
		// r = 0;g = 255;b = 0; for (int i = 0; i < 256; ++i){RGB(r,g,i);}
	    GPIO_CLR = 1 << RED;
	    GPIO_SET = 1 << GREEN;
	    GPIO_CLR = 1 << BLUE;
	    // GREEN TO TEAL
	    for (i = 0; i <= 255; i++){SoftPWM(i,BLUE);}
		//-----------------------------------------------------------------------
		// r = 0;g = 255;b = 255; for (int i = 255; i >= 0; --i){RGB(r,i,b);}
	    GPIO_CLR = 1 << RED;
	    GPIO_SET = 1 << GREEN;
	    GPIO_SET = 1 << BLUE;
	    // TEAL TO BLUE
	    for (i = 255; i>=0;i--){SoftPWM(i,GREEN);}
		//-----------------------------------------------------------------------
		// r = 0;g = 0;b = 255; for (int i = 0; i < 256; ++i){RGB(i,g,b);}
	    GPIO_CLR = 1 << RED;
	    GPIO_CLR = 1 << GREEN;
	    GPIO_SET = 1 << BLUE;
	    // BLUE TO MAGENTA
	    for (i = 0; i <= 255 ; i++){SoftPWM(i,RED);}
		//-----------------------------------------------------------------------
		// r = 255;g = 0;b = 255; for (int i = 255; i >= 0; --i){RGB(r,g,i);}
	    GPIO_SET = 1 << RED;
	    GPIO_CLR = 1 << GREEN;
	    GPIO_SET = 1 << BLUE; 
	    // MAGENTA TO RED
	    for(i = 255; i >=0; i--){SoftPWM(i,BLUE);}
		//-----------------------------------------------------------------------
	  

  	}
 	GPIO_CLR = 1 << RED;
	GPIO_CLR = 1 << GREEN;
	GPIO_CLR = 1 << BLUE;
  	return 0;
 
} // main
 
void SoftPWM(int dutyCycle, int Pin)
{
	// Modifies the 255 Value into a Percentage
 	double dutyCyc = dutyCycle/2.55;
  	double onTime,offTime;
  	// Used for frequency, 488 Hz gives roughly ~2msec
	int msecs = 2,
		// Used as how long SoftPWM should be running do-while loop
		TIME_PER_CALL = 5;
	// Calculates how many *secs the Pin will stay high	
	onTime = msecs*dutyCyc/100;
	// Calculates how many *secs the Pin will stay low
	offTime = (msecs*(100-dutyCyc)/100)/2;
	// Structures used to hold values for nanosleep
	struct timespec on,off;
	// Data Type used to calculate when to terminate the do-while loop
	clock_t start,end, diff;
	// Gets start time
	start = clock();
	// Sets On and Off Structures with the proper nanosecond amount.
	on.tv_sec = 0;
	// Converts msecs to nano seconds
	on.tv_nsec =(onTime) * NANO_SECOND_MULTIPLIER;
	off.tv_sec = 0;
	// Converts msecs to nano seconds
	off.tv_nsec = (offTime) * NANO_SECOND_MULTIPLIER;
	do
	{  
		// Sets Pin Off
		GPIO_CLR = 1 << Pin;
		// Sleeps for Specified Off Amount
		nanosleep(&off,NULL);
		// Sets Pin On
		GPIO_SET = 1 << Pin;
		// Sleeps for Specified On Amount
		nanosleep(&on,NULL);
		// Sets Pin Off
		GPIO_CLR = 1 << Pin;
		// Sleeps for Specified Off Amount
		nanosleep(&off,NULL);
		// One Cycle is Completed
		// Get Clock Snapshot
		end = clock();
		// Calculate the elapsed time from start to "end"
		diff = ((end - start)*(10000))/CLOCKS_PER_SEC;
	}while(diff <= TIME_PER_CALL); // Check to see if the difference is still less than TIME_PER_CALL to continue while loop
} // SoftPWM
 
//
// Set up a memory regions to access GPIO
//
void setup_io()
{
   /* open /dev/mem */
   if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
      printf("can't open /dev/mem \n");
      exit(-1);
   }
 
   /* mmap GPIO */
   gpio_map = mmap(
      NULL,             //Any adddress in our space will do
      BLOCK_SIZE,       //Map length
      PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
      MAP_SHARED,       //Shared with other processes
      mem_fd,           //File to map
      GPIO_BASE         //Offset to GPIO peripheral
   );
 
   close(mem_fd); //No need to keep mem_fd open after mmap
 
   if (gpio_map == MAP_FAILED) {
      printf("mmap error %d\n", (int)gpio_map);//errno also set!
      exit(-1);
   }
 
   // Always use volatile pointer!
   gpio = (volatile unsigned *)gpio_map;
 
 
} // setup_io
