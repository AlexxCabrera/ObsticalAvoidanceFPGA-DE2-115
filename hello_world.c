#include <stdio.h>
#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "altera_avalon_timer_regs.h"
#include "sys/alt_irq.h"
#include <altera_up_sd_card_avalon_interface.h>
#include "altera_up_avalon_character_lcd.h"

volatile int edge_capture;

alt_up_sd_card_dev *device_reference = NULL;//SDCARD Pointer

void delay(){ // delays for relative to the value of "speed"
	IOWR_ALTERA_AVALON_TIMER_CONTROL(0x1800, 8); //Stop value high
	IOWR_ALTERA_AVALON_TIMER_STATUS(0x1800, 2);  //Clear the TO bit
	IOWR_ALTERA_AVALON_TIMER_PERIODL(0x1800, 0x0D90);	//timer value Lower set 250ms
	IOWR_ALTERA_AVALON_TIMER_PERIODH(0x1800, 0x0000);	//timer value Higher set
	IOWR_ALTERA_AVALON_TIMER_CONTROL(0x1800, 4); //Start value high

	int timeOut = IORD_ALTERA_AVALON_TIMER_STATUS(0x1800) & 0x1;
			while(timeOut != 1){
				timeOut = IORD_ALTERA_AVALON_TIMER_STATUS(0x1800) & 0x1;
			}
}

void Delay1s(){ // delays for relative to the value of "speed"
	IOWR_ALTERA_AVALON_TIMER_CONTROL(0x1800, 8); //Stop value high
	IOWR_ALTERA_AVALON_TIMER_STATUS(0x1800, 2);  //Clear the TO bit
	IOWR_ALTERA_AVALON_TIMER_PERIODL(0x1800, 0xF080);	//timer value Lower set 1s
	IOWR_ALTERA_AVALON_TIMER_PERIODH(0x1800, 0x02FA);	//timer value Higher set
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
		IOWR_ALTERA_AVALON_PIO_DATA(HexAdress[i],127); //Clears all HEX displays
	}

	IOWR_ALTERA_AVALON_PIO_DATA(0x18e0, 0);	//clear the LEDred
	IOWR_ALTERA_AVALON_PIO_DATA(0x1850, 0);	//clear the LEDgreen
}

void numToHex(int digit,int address){ //Stores digit parameter into the HEX0 display

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

	IOWR_ALTERA_AVALON_PIO_DATA(address,hex);

}

void intToHexMultiNumber(int digit){

	int HexAddress[8] = {0x18d0,0x18c0,0x18b0,0x18a0,0x1890,0x1880,0x1870,0x1860};

	for(int i = 0; i<8; i++){
		if(i<4){
			IOWR_ALTERA_AVALON_PIO_DATA(HexAddress[i],64); //set hex 3-0 displays to 0
		}
		else{
			IOWR_ALTERA_AVALON_PIO_DATA(HexAddress[i],127); //hex 7-4 displays off
		}
	}

	for(int i=0; i<8; i++){
		if(digit>=10000000){
				numToHex(digit/10000000,HexAddress[7]);
				digit=digit%10000000;
			}
			else if(digit>=1000000){
				numToHex(digit/1000000,HexAddress[6]);
				digit=digit%1000000;
			}
			else if(digit>=100000){
				numToHex(digit/100000,HexAddress[5]);
				digit=digit%100000;
			}
			else if(digit>=10000){
				numToHex(digit/10000,HexAddress[4]);
				digit=digit%10000;
			}
			else if(digit>=1000){
				numToHex(digit/1000,HexAddress[3]);
				digit=digit%1000;
			}
			else if(digit>=100){
				numToHex(digit/100,HexAddress[2]);
				digit=digit%100;
			}
			else if(digit>=10){
				numToHex(digit/10,HexAddress[1]);
				digit=digit%10;
			}
			else if(digit>=1){
				numToHex(digit/1,HexAddress[0]);
			}
	}

}

void LCDSetup(){
	alt_up_character_lcd_dev * lcd;
	lcd = alt_up_character_lcd_open_dev("/dev/character_lcd_0");
	alt_up_character_lcd_init(lcd);
}

void LCDReset(alt_up_character_lcd_dev *lcd){
	alt_up_character_lcd_init(lcd);
}

void LCDLeft(alt_up_character_lcd_dev *lcd){
		LCDReset(lcd);
		alt_up_character_lcd_write(lcd, "Turning Left" , 12);
}
void LCDRight(alt_up_character_lcd_dev *lcd){
		LCDReset(lcd);
		alt_up_character_lcd_write(lcd, "Turning Right" , 13);
}
void LCDObstacle(alt_up_character_lcd_dev *lcd){
	alt_up_character_lcd_set_cursor_pos(lcd, 0, 1);
	alt_up_character_lcd_write(lcd, "Obstacle Ahead!" , 15);
}
void LCDForward(alt_up_character_lcd_dev *lcd){
	LCDReset(lcd);
	alt_up_character_lcd_write(lcd, "Moving Forward" , 14);
}
void LCDBackward(alt_up_character_lcd_dev *lcd){
	LCDReset(lcd);
	alt_up_character_lcd_write(lcd, "Moving Backwards" , 16);
}

int wheels;

void moveForward(){
	wheels = 1+2+32+64;
	IOWR_ALTERA_AVALON_PIO_DATA(0x810,wheels);

	alt_up_character_lcd_dev * lcd;
	lcd = alt_up_character_lcd_open_dev("/dev/character_lcd_0");
	LCDForward(lcd);
}

void moveBack(){
	wheels = 4+8+16+128;
	IOWR_ALTERA_AVALON_PIO_DATA(0x810,wheels);

	alt_up_character_lcd_dev * lcd;
	lcd = alt_up_character_lcd_open_dev("/dev/character_lcd_0");
	LCDBackward(lcd);
}

void moveTurnRight(){
	wheels = 2+64+8+16;
	IOWR_ALTERA_AVALON_PIO_DATA(0x810,wheels);

	alt_up_character_lcd_dev * lcd;
	lcd = alt_up_character_lcd_open_dev("/dev/character_lcd_0");
	LCDRight(lcd);
}

void moveTurnLeft(){
	wheels = 4+128+1+32;
	IOWR_ALTERA_AVALON_PIO_DATA(0x810,wheels);

	alt_up_character_lcd_dev * lcd;
	lcd = alt_up_character_lcd_open_dev("/dev/character_lcd_0");
	LCDLeft(lcd);
}

void stopCar(){
	wheels = 0;
	IOWR_ALTERA_AVALON_PIO_DATA(0x810,wheels);

	alt_up_character_lcd_dev * lcd;
	lcd = alt_up_character_lcd_open_dev("/dev/character_lcd_0");
	LCDReset(lcd);
}

void LidarReadTest(){
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

int LidarReadDistanceL(){ // returns distance lower byte
	int LdrData = 0; //Value being read

	//Hold until data = 89. [A9 A9]base16 is start of new data set
	while(LdrData!=89){
		LdrData = IORD_ALTERA_AVALON_PIO_DATA(0x820);
	}
	delay(); //0x0D90 delays for 8bits (a byte) of data
	delay(); //0x0D90 delays for 8bits (a byte) of data
	delay(); //0x0D90 delays for 8bits (a byte) of data
	LdrData = IORD_ALTERA_AVALON_PIO_DATA(0x820);
	return LdrData;
}

int LidarReadDistanceH(){ // returns distance upper byte
	int LdrData = 0; //Value being read

	//Hold until data = 89. [A9 A9]base16 is start of new data set
	while(LdrData!=89){
		LdrData = IORD_ALTERA_AVALON_PIO_DATA(UART_0_BASE);
	}
	delay(); //0x0D90 delays for 8bits (a byte) of data
	delay(); //0x0D90 delays for 8bits (a byte) of data
	delay(); //0x0D90 delays for 8bits (a byte) of data
	delay(); //0x0D90 delays for 8bits (a byte) of data
	LdrData = IORD_ALTERA_AVALON_PIO_DATA(UART_0_BASE);
	return LdrData;
}

//data valid up to roughly 655.35 meters
//Front Sensor
int LidarReadDistance(){ // returns both lower and upper byte distance as one value (2 bytes)
	int LdrDataL = 0; //Lower byte of distance
	int LdrDataH = 0; //Higher byte of distance

	//Hold until data = 89. [A9 A9]base16 is start of new data set
	while(LdrDataL!=89){
		LdrDataL = IORD_ALTERA_AVALON_PIO_DATA(UART_0_BASE);
	}
	delay(); //0x0D90 delays for 8bits (a byte) of data
	delay(); //0x0D90 delays for 8bits (a byte) of data
	delay(); //0x0D90 delays for 8bits (a byte) of data
	LdrDataL = IORD_ALTERA_AVALON_PIO_DATA(UART_0_BASE);
	delay(); //0x0D90 delays for 8bits (a byte) of data
	LdrDataH = IORD_ALTERA_AVALON_PIO_DATA(UART_0_BASE);
	LdrDataH = LdrDataH * 256; //Shifts it a byte to the left
	return LdrDataL + LdrDataH; // return the lower and upper bytes as one value
}

//data valid up to roughly 655.35 meters
//Back Sensor
int LidarReadDistance2(){ // returns both lower and upper byte distance as one value (2 bytes)
	int LdrDataL = 0; //Lower byte of distance
	int LdrDataH = 0; //Higher byte of distance

	//Hold until data = 89. [A9 A9]base16 is start of new data set
	while(LdrDataL!=89){
		LdrDataL = IORD_ALTERA_AVALON_PIO_DATA(UART_1_BASE);
	}
	delay(); //0x0D90 delays for 8bits (a byte) of data
	delay(); //0x0D90 delays for 8bits (a byte) of data
	delay(); //0x0D90 delays for 8bits (a byte) of data
	LdrDataL = IORD_ALTERA_AVALON_PIO_DATA(UART_1_BASE);
	delay(); //0x0D90 delays for 8bits (a byte) of data
	LdrDataH = IORD_ALTERA_AVALON_PIO_DATA(UART_1_BASE);
	LdrDataH = LdrDataH * 256; //Shifts it a byte to the left
	return LdrDataL + LdrDataH; // return the lower and upper bytes as one value
}

void key0_ISR(){
	printf("key0\n");
	moveForward();
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

void carMovement(){

	int distanceFront;
	int IR;

	distanceFront = LidarReadDistance();

	if(distanceFront > 40 ){ // move forward if there is more than 40 cm in front of car
		moveForward();
		printf("Moving Forward...\n");
	}
	else if(distanceFront < 20){ // back up if too close to wall
		moveBack();
		printf("Moving Backwards...\n");
		while(distanceFront < 35){ // stop backing up when 35 cm away from wall
			distanceFront = LidarReadDistance();
			Delay1s();
		}
	}
	else{
		IR = IORD_ALTERA_AVALON_PIO_DATA(PIO_0_BASE); //used to check left and right
		if(IR == 3){
			moveTurnLeft();
			printf("Turning Left...\n");
		}
		else if(IR == 2){
			moveTurnLeft();
			printf("Turning Left...\n");
		}
		else if(IR == 1){
			moveTurnRight();
			printf("Turning Right...\n");
		}else if(IR == 0){
			stopCar();
			Delay1s();
			moveBack();
		}else{
			printf("IR SENSOR ERROR...\n");
		}
	}
}

int main()
{
	printf("Program Started\n");
	clear_LEDr_LEDg_Hex();

	pio_init(); //set up key interrupts
	IOWR_ALTERA_AVALON_TIMER_CONTROL(0x1800, 8); //Stop value high
	IOWR_ALTERA_AVALON_TIMER_STATUS(0x1800, 2);  //Clear the TO bit
	IOWR_ALTERA_AVALON_TIMER_PERIODL(0x1800, 0x0D90);	//timer value Lower set 250ms
	IOWR_ALTERA_AVALON_TIMER_PERIODH(0x1800, 0x0000);	//timer value Higher set

	alt_up_character_lcd_dev * lcd;
	lcd = alt_up_character_lcd_open_dev("/dev/character_lcd_0");
	alt_up_character_lcd_init(lcd);

	int DstFront; // distance value from lidar
	//int DstBack; // distance value from lidar
	int sw; // switches value
	//int IR; //IR readings
	while(1){
		sw = IORD_ALTERA_AVALON_PIO_DATA(SWITCHES_BASE); //switch0 is the start switch for the car

		if(sw == 1){
			DstFront = LidarReadDistance();  //Grab 2 bytes from lidar
			intToHexMultiNumber(DstFront); // display Lidars data on HEX

			Delay1s();
			carMovement();

		}else{
			stopCar();
		}
	}

	printf("done");

  return 0;
}
