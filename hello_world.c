#include <stdio.h>
#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "altera_avalon_timer_regs.h"
#include "sys/alt_irq.h"

volatile int edge_capture;


void delay(){ // delays for relative to the value of "speed"
	IOWR_ALTERA_AVALON_TIMER_CONTROL(0x1800, 8); //Stop value high
	IOWR_ALTERA_AVALON_TIMER_STATUS(0x1800, 2);  //Clear the TO bit
	IOWR_ALTERA_AVALON_TIMER_PERIODL(0x1800, 0x0D90);	//timer value Lower bit set 250ms
	IOWR_ALTERA_AVALON_TIMER_PERIODH(0x1800, 0x0000);	//timer value Higher bit set
	IOWR_ALTERA_AVALON_TIMER_CONTROL(0x1800, 4); //Start value high

	int timeOut = IORD_ALTERA_AVALON_TIMER_STATUS(0x1800) & 0x1;
			while(timeOut != 1){
				timeOut = IORD_ALTERA_AVALON_TIMER_STATUS(0x1800) & 0x1;
			}
}

void carDelay(){ // delays for relative to the value of "speed"
	IOWR_ALTERA_AVALON_TIMER_CONTROL(0x1800, 8); //Stop value high
	IOWR_ALTERA_AVALON_TIMER_STATUS(0x1800, 2);  //Clear the TO bit
	IOWR_ALTERA_AVALON_TIMER_PERIODL(0x1800, 0xF080);	//timer value Lower bit set 250ms
	IOWR_ALTERA_AVALON_TIMER_PERIODH(0x1800, 0x02FA);	//timer value Higher bit set
	IOWR_ALTERA_AVALON_TIMER_CONTROL(0x1800, 4); //Start value high

	int timeOut = IORD_ALTERA_AVALON_TIMER_STATUS(0x1800) & 0x1;
			while(timeOut != 1){
				timeOut = IORD_ALTERA_AVALON_TIMER_STATUS(0x1800) & 0x1;
			}
}

void clearHex(){
	int HexAdress[8] = {0x18d0,0x18c0,0x18b0,0x18a0,0x1890,0x1880,0x1870,0x1860}; //Hex Addresses


	for(int i = 0; i<8; i++){
		IOWR_ALTERA_AVALON_PIO_DATA(HexAdress[i],127); //Clears all HEX displays
	}
}

void clear_LEDr_LEDg_Hex(){ //Resets the components
	int HexAdress[8] = {0x18d0,0x18c0,0x18b0,0x18a0,0x1890,0x1880,0x1870,0x1860}; //Hex Addresses

	for(int i = 0; i<8; i++){
		IOWR_ALTERA_AVALON_PIO_DATA(HexAdress[i],127); //Clears all HEX displays except 0 and 1
	}

	IOWR_ALTERA_AVALON_PIO_DATA(0x18e0, 0);	//clear the LEDred
	IOWR_ALTERA_AVALON_PIO_DATA(0x1850, 0);	//clear the LEDgreen
}

void numToHex(int digit){ //Stores digit parameter into the HEX0 display

	int hex;
	//int HexAdress[8] = {0x18d0,0x18c0,0x18b0,0x18a0,0x1890,0x1880,0x1870,0x1860};

	switch(digit){
		case 0:
			hex = 64;
			break;
		case 1:
			hex = 121;
			break;
		case 2:
			hex = 36;
			break;
		case 3:
			hex = 48;
			break;
		case 4:
			hex = 25;
			break;
		case 5:
			hex = 18;
			break;
		case 6:
			hex = 2;
			break;
		case 7:
			hex = 120;
			break;
		case 8:
			hex = 0;
			break;
		case 9:
			hex = 16;
			break;
		case 10: //A
			hex = 8;
			break;
		case 11: //B
			hex = 3;
			break;
		case 12: //C
			hex = 70;
			break;
		case 13: //D
			hex = 33;
			break;
		case 14: //E
			hex = 6;
			break;
		case 15: //F
			hex = 14;
			break;
		default:
			hex = 127;
	}

	IOWR_ALTERA_AVALON_PIO_DATA(0x18d0,hex);

}

int wheels;

void moveForward(){
	wheels = 1+2+32+64;
	IOWR_ALTERA_AVALON_PIO_DATA(0x810,wheels);
	carDelay();
	carDelay();
	IOWR_ALTERA_AVALON_PIO_DATA(0x810,0);
}

void moveBack(){
	wheels = 4+8+16+128;
	IOWR_ALTERA_AVALON_PIO_DATA(0x810,wheels);
	carDelay();
	carDelay();
	IOWR_ALTERA_AVALON_PIO_DATA(0x810,0);
}

void moveTurnRight(){
	wheels = 2+64+8+16;
	IOWR_ALTERA_AVALON_PIO_DATA(0x810,wheels);
	carDelay();
	carDelay();
	IOWR_ALTERA_AVALON_PIO_DATA(0x810,0);
}

void moveTurnLeft(){
	wheels = 4+128+1+32;
	IOWR_ALTERA_AVALON_PIO_DATA(0x810,wheels);
	carDelay();
	carDelay();
	IOWR_ALTERA_AVALON_PIO_DATA(0x810,0);
}

void getDistance(){
	int LdrData = 0;
	int temp = 0;
	while(1){
		LdrData = IORD_ALTERA_AVALON_PIO_DATA(0x820);
		printf("LdrData = %x\n",LdrData);
		while(LdrData == temp){
			temp = IORD_ALTERA_AVALON_PIO_DATA(0x820);
		}
	}
}

void LidarRead(){
	int LdrData = 0; //Value being read
	int temp = 0;    //Value used to compare against read value

	//Hold until data = 89. [A9 A9]base16 is start of new data set
	while(LdrData!=89){
		LdrData = IORD_ALTERA_AVALON_PIO_DATA(0x820);
	}
	delay(); //0x0D90 delays for 8bits (a byte) of data
	for(int i=0; i<10; i++){ //Loop 10 times. Start [0x59 0x59] End [0x59]
		LdrData = IORD_ALTERA_AVALON_PIO_DATA(0x820);
		printf("%x ",LdrData);
		while(LdrData == temp){ 					//Hold till new value
			temp = IORD_ALTERA_AVALON_PIO_DATA(0x820);
		}
		while(LdrData == temp){							//For data that slips by last while loop.
			temp = IORD_ALTERA_AVALON_PIO_DATA(0x820); //This was an issue before the second while loop
		}
	}
	printf("\n");
}

void key0_ISR(){
	printf("key0\n");
	moveForward();
	//LidarRead();
}

void key1_ISR(){
	printf("key1\n");
	moveBack();
}

void key2_ISR(){
	printf("key2\n");
	moveTurnRight();
}

void key3_ISR(){
	printf("key3\n");
	moveTurnLeft();
}

void handle_key_interrupts(void* context)
{
	volatile int *edge_capture_ptr = (volatile int*) context;
		*edge_capture_ptr = IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEY_0_BASE);

		if(*edge_capture_ptr & 0b1000){
			key3_ISR();
		}
		else if(*edge_capture_ptr & 0b0100){
			key2_ISR();
		}
		else if(*edge_capture_ptr & 0b0010){
			key1_ISR();
		}
		else if(*edge_capture_ptr & 0b0001){
			key0_ISR();
		}
		else{
			printf("ERROR");
		}
		//reset and call everything again
		IOWR_ALTERA_AVALON_PIO_EDGE_CAP(0x1840, 0);
		IORD_ALTERA_AVALON_PIO_EDGE_CAP(0x1840);
		return;
}

static void pio_init () /* Initialize the button_pio. */
{
	volatile int edge_capture;
	/* Recast the edge_capture pointer to match the alt_irq_register() function prototype. */
	void* edge_capture_ptr = (void*) &edge_capture;

	/* Enable key3 as interrupt. */
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(0x1840, 0b1111);

	/* Reset the edge capture register. */
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(0x1840, 0x0);

	/* Register the ISR. */
	alt_irq_register( KEY_0_IRQ, edge_capture_ptr, handle_key_interrupts);
}

int main()
{
	printf("Program Started\n");
	clear_LEDr_LEDg_Hex();

	pio_init(); //set up key interrupts
	IOWR_ALTERA_AVALON_TIMER_CONTROL(0x1800, 8); //Stop value high
	IOWR_ALTERA_AVALON_TIMER_STATUS(0x1800, 2);  //Clear the TO bit
	//IOWR_ALTERA_AVALON_TIMER_PERIODL(0x1800, 0xF080);	//timer value Lower bit set 250ms
	//IOWR_ALTERA_AVALON_TIMER_PERIODH(0x1800, 0x02FA);	//timer value Higher bit set
	IOWR_ALTERA_AVALON_TIMER_PERIODL(0x1800, 0x0D90);	//timer value Lower bit set 250ms
	IOWR_ALTERA_AVALON_TIMER_PERIODH(0x1800, 0x0000);	//timer value Higher bit set


	while(1){
		//LidarRead();
		//delay();
		//delay();
	}

	printf("done");

  return 0;
}
