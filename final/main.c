//Troy Smith, smithtro
//Joel Park, jgpark

/* main.c */

#include <stm32f30x.h>  // Pull in include files for F30x standard drivers
#include <f3d_led.h>    
#include <f3d_uart.h>
#include <f3d_gyro.h>
#include <f3d_lcd_sd.h>
#include <f3d_i2c.h>
#include <f3d_accel.h>
#include <f3d_mag.h>
#include <f3d_nunchuk.h>
#include <f3d_rtc.h>
#include <f3d_systick.h>
#include <ff.h>
#include <diskio.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <f3d_user_btn.h>
//#include <time.h>

#define TIMER 20000
#define AUDIOBUFSIZE 128

//SD Card
FATFS Fatfs;        /* File system object */
FIL fid;        /* File object */
BYTE Buff[512];     /* File read buffer */
int ret;

//?
const char *b[1];

//AUDIO
extern uint8_t Audiobuf[AUDIOBUFSIZE];
extern int audioplayerHalf;
extern int audioplayerWhole;

//NUNCHUK
FATFS Fatfs;        /* File system object */
FIL fid;            /* File object */
BYTE Buff[512];     /* File read buffer */
int ret;
nunchuk_t nunchuk;

//FILES
FRESULT rc;			/* Result code */
DIR dir;			/* Directory object */
FILINFO fno;			/* File information object */
UINT bw, br;
unsigned int retval;
int bytesread;

//LCD
struct bmppixel pixel_data[128];
uint16_t pixel_color[128];
BYTE HeaderBuff[54];
BYTE RGBBuff[3];
FIL Fil;

//Hands LCD
uint16_t hand_color[128];
uint16_t user_color[128];

struct ckhd {
    uint32_t ckID;
    uint32_t cksize;
};

struct fmtck {
    uint16_t wFormatTag;
    uint16_t nChannels;
    uint32_t nSamplesPerSec;
    uint32_t nAvgBytesPerSec;
    uint16_t nBlockAlign;
    uint16_t wBitsPerSample;
};

void readckhd(FIL *fid, struct ckhd *hd, uint32_t ckID) {
    f_read(fid, hd, sizeof(struct ckhd), &ret);
    if (ret != sizeof(struct ckhd))
    exit(-1);
    if (ckID && (ckID != hd->ckID))
    exit(-1);
}

void die (FRESULT rc) {
    printf("Failed with rc=%u.n", rc);
    while (1);
}


//drawPicture - draws a picture on the LCD screen
//accepts an orientation and the file name of the .BMP to be drawn
//orient = 1 is portrait, orient = 0 is landscape
void drawPicture(int orient, char *name) {

  int row, i;

  // opens file designated by name
  rc = f_open(&Fil, name, FA_READ);
  if (rc) die(rc);
  
  // reads from file
  rc = f_read(&Fil, HeaderBuff, sizeof HeaderBuff, &br);
  if (orient == 1) {
    for (row = 160; row >= 0; row--) {
      rc = f_read(&Fil, &pixel_data, sizeof pixel_data, &br);
      if (rc || !br) break;

      for(i = 128; i >= 0; i--) {
	RGBBuff[0] = ((uint16_t) (pixel_data[i].b)) >> 3;
	RGBBuff[1] = ((uint16_t) (pixel_data[i].g)) >> 2;
	RGBBuff[2] = ((uint16_t) (pixel_data[i].r)) >> 3;
	pixel_color[i] = (RGBBuff[0] << 11) | (RGBBuff[1] << 5) | RGBBuff[2];
      }

      f3d_lcd_setAddrWindow(0, row, 128, row, MADCTLGRAPHICS);
      f3d_lcd_pushColor(pixel_color, 128);
    }
  } else { 
    for (row = 128; row >= 0; row--) {
      rc = f_read(&Fil, &pixel_data, sizeof pixel_data, &br);
      if (rc || !br) break;

      for(i = 160; i >= 0; i--) {
	RGBBuff[0] = ((uint16_t) (pixel_data[i].b)) >> 3;
	RGBBuff[1] = ((uint16_t) (pixel_data[i].g)) >> 2;
	RGBBuff[2] = ((uint16_t) (pixel_data[i].r)) >> 3;
	pixel_color[i] = (RGBBuff[0] << 11) | (RGBBuff[1] << 5) | RGBBuff[2];
      }

      f3d_lcd_setAddrWindow(row, 0, row, 160, MADCTLGRAPHICS);
      f3d_lcd_pushColor(pixel_color, 160);
    }   
  }
}


// x1 - bottom left x
// y1 - bottom left y
// x2 - bottom right x
// y2 - bottom right y
// c  - color
void drawHandDown(int x1, int y1, int x2, int y2, uint16_t *c)
{
  f3d_lcd_setAddrWindow(x1, y1, x2, y2, MADCTLGRAPHICS);
  f3d_lcd_pushColor(c, 128);
}

void eraseHand(int x1, int y1, int x2, int y2)
{
  uint16_t pixel_color[128];
  int i;
  for (i = 0; i < 128; i ++) {
    pixel_color[i] = 0x0000;
  }
  f3d_lcd_setAddrWindow(x1, y1, x2, y2, MADCTLGRAPHICS);
  f3d_lcd_pushColor(pixel_color, 128);
}


//used to play open and play the sounds, this was all given to us in the lab
int play(char *name) {
	rc = f_open(&fid, name, FA_READ);

	if (!rc) {
	struct ckhd hd;
	uint32_t  waveid;
	struct fmtck fck;
		
	readckhd(&fid, &hd, 'FFIR');
		
	f_read(&fid, &waveid, sizeof(waveid), &ret);
	if ((ret != sizeof(waveid)) || (waveid != 'EVAW'))
		 return -1;
		
	readckhd(&fid, &hd, ' tmf');
		
	f_read(&fid, &fck, sizeof(fck), &ret);
		
	if (hd.cksize != 16) {
		 printf("extra header info %d\n", hd.cksize - 16);
		 f_lseek(&fid, hd.cksize - 16);
	}
		       
		
	while(1) {
		readckhd(&fid, &hd, 0);
		if (hd.ckID == 'atad')
		break;
		 f_lseek(&fid, hd.cksize);
	}
		
	f_read(&fid, Audiobuf, AUDIOBUFSIZE, &ret);
	hd.cksize -= ret;
	audioplayerStart();
	while (hd.cksize) {
	  int next = hd.cksize > AUDIOBUFSIZE/2 ? AUDIOBUFSIZE/2 : hd.cksize;
	  if (audioplayerHalf) {
	if (next < AUDIOBUFSIZE/2)
	  bzero(Audiobuf, AUDIOBUFSIZE/2);
	f_read(&fid, Audiobuf, next, &ret);
	hd.cksize -= ret;
	audioplayerHalf = 0;
	  }
	  if (audioplayerWhole) {
	if (next < AUDIOBUFSIZE/2)
	  bzero(&Audiobuf[AUDIOBUFSIZE/2], AUDIOBUFSIZE/2);
	f_read(&fid, &Audiobuf[AUDIOBUFSIZE/2], next, &ret);
	hd.cksize -= ret;
	audioplayerWhole = 0;
	  }
	}
	audioplayerStop();
  }
  
  rc = f_close(&fid);
	  
  if (rc) die(rc);
  return 0;
}

int nunInput(void) {
  while(1) {
    f3d_nunchuk_read(&nunchuk);
    
    if(nunchuk.z == 1){
      while(nunchuk.z == 1){
	f3d_nunchuk_read(&nunchuk);
      }
      return 1;  
    }
    
    else if(nunchuk.c == 1){
      while(nunchuk.c == 1){
	f3d_nunchuk_read(&nunchuk);
      }
      return 0;
    }
    
    else if(nunchuk.jy == 0xFF){
      while(nunchuk.jy == 0xFF){
	f3d_nunchuk_read(&nunchuk);
      }
      return 2;
    }
    
    else if(nunchuk.jy == 0x00){
      while(nunchuk.jy == 0x00){
	f3d_nunchuk_read(&nunchuk);
      }
      return 3;
    }
  }
}

int rando()
{
  f3d_nunchuk_read(&nunchuk);
  //printf("%d\n", nunchuk.ax);
  int randnum = nunchuk.ax + nunchuk.ay + nunchuk.az;
  return randnum;
}

int shaker(void) {
  float f[3];
  int countdown = 3;
  while(countdown) {
    delay(500);
    f3d_accel_read(f);
    //printf("%f %f % f\n", fabsf(f[0]), fabsf(f[1]), fabsf(f[2]));
    if (fabsf(f[0]) > .1 || fabsf(f[1]) > .1 || fabsf(f[2]) > 1.2) {
      //printf("TRIPPED\n");
      return 1;
    }
    countdown--;
    
  }
  return 0;
}

void youWin() {
  drawPicture(1, "win.bmp");
  delay(10);
  play("batman.wav");
  while(1) {
    f3d_lcd_fillScreen2(BLACK);
    drawPicture(1, "win.bmp");
  }
}

int gameLogic(void) {
  int lives = 3;
  int wins = 0;
  int round = 1;
  int pattern[10]; //holds the sequence of numbers
  int cont = 0;
  //srand(time(0));
  f3d_lcd_drawString(5, 150, "LIVES = 3", WHITE, BLACK);
  f3d_lcd_drawString(60, 150, " WINS = 0 ", WHITE, BLACK); //DISPLAY WINS COUNT
  while(lives && wins != 10){
    int current =  rando() % 4; //random number between 0 and 3
    //printf("%d", current);
    // 0 = red 1 = blue 2 = green 3 = yellow
    int input[10]; //holds the user inputted sequence of numbers
    pattern[round-1] = current;
    int i;
    int j;
    int k;
    for(i=0; i<round; i++){ //shows pattern to user
      if(pattern[i] == 0){
	//c
	drawHandDown(110,10,120,20,hand_color);
	play("clang_x.wav");
	//delay(500);
	eraseHand(110,10,120,20);
	delay(75);
	printf("red\n");
      }
      else if(pattern[i] == 1){
	//z
	drawHandDown(110,130,120,140,hand_color);
	play("camera1.wav");
	//delay(500);
	eraseHand(110,130,120,140);
	delay(75);
	printf("blue\n");
      }
      else if(pattern[i] == 2){
	//up
	drawHandDown(10,10,20,20,hand_color);
	play("coin2.wav");
	//delay(500);
	eraseHand(10,10,20,20);
	delay(75);
	printf("green\n");
      }
      else if(pattern[i] == 3){
	//down
	drawHandDown(5,130,15,140,hand_color);
	play("boing_x.wav");
	//delay(500);
	eraseHand(5,130,15,140);
	delay(75);
	printf("yellow\n");
      }

    }
    printf("-------\n");
    int success = 1;
    for(j=0; j<round; j++){ //user inputs their pattern
      int n;
      //scanf("%d", &n);
      n = nunInput();
      input[j] = n;
      if(pattern[j] != n){
	success = 0;
	lives --;
	wins = 0;
	switch(lives){
	case(1):  f3d_lcd_drawString(5, 150, "LIVES = 1", WHITE, BLACK);
	  break;
	case(2):  f3d_lcd_drawString(5, 150, "LIVES = 2", WHITE, BLACK);
	  break;
	case(3):  f3d_lcd_drawString(5, 150, "LIVES = 3", WHITE, BLACK);
	  break;
	case(0):  f3d_lcd_drawString(5, 150, "LIVES = 0", WHITE, BLACK);
	  break;
	}
	//f3d_lcd_drawString(53, 150, pchar, WHITE, BLACK); //DISPLAY LIVES COUNT
	delay(10);
	f3d_lcd_drawString(60, 150, " WINS = 0 ", WHITE, BLACK);

	memset(pattern, 0, 10);
	round = 1;
	f3d_lcd_drawString(28, 2, "That's wrong!", WHITE, BLACK);
	play("buzzer_x.wav");
	delay(10);
	f3d_lcd_drawString(28, 2, "That's wrong!", BLACK, BLACK);
	break;
      }
      else{
	if (n == 0){
	  drawHandDown(110,10,120,20,user_color);
	  play("clang_x.wav");
	  //delay(500);
	  eraseHand(110,10,120,20);
	  //delay(75);
	  }
	else if (n == 1){
	  drawHandDown(110,130,120,140,user_color);
	  play("camera1.wav");
	  //delay(500);
	  eraseHand(110,130,120,140);
	  //delay(75);
	  //printf("blue\n");
	}
	else if (n == 2){
	  drawHandDown(10,10,20,20,user_color);
	  play("coin2.wav");
	  //delay(500);
	  eraseHand(10,10,20,20);
	  //delay(75);
	}
	else if (n == 3){
	  drawHandDown(5,130,15,140,user_color);
	  play("boing_x.wav");
	  //delay(500);
	  eraseHand(5,130,15,140);
	  delay(75);
	}
      }
    }
    if(success){
      round++;
      wins++;
      //char l = wins + '0';
      //char *pchar2 = &l;
      delay(10);
      eraseHand(108,150,113,160);
      switch(wins){
      case(1):  f3d_lcd_drawString(60, 150, " WINS = 1 ", WHITE, BLACK);
	break;
      case(2):  f3d_lcd_drawString(60, 150, " WINS = 2 ", WHITE, BLACK);
	break;
      case(3):  f3d_lcd_drawString(60, 150, " WINS = 3 ", WHITE, BLACK);
	break;
      case(4):  f3d_lcd_drawString(60, 150, " WINS = 4 ", WHITE, BLACK);
	break;
      case(5):  f3d_lcd_drawString(60, 150, " WINS = 5 ", WHITE, BLACK);
	break;
      case(6):  f3d_lcd_drawString(60, 150, " WINS = 6 ", WHITE, BLACK);
	break;
      case(7):  f3d_lcd_drawString(60, 150, " WINS = 7 ", WHITE, BLACK);
	break;
      case(8):  f3d_lcd_drawString(60, 150, " WINS = 8 ", WHITE, BLACK);
	break;
      case(9):  f3d_lcd_drawString(60, 150, " WINS = 9 ", WHITE, BLACK);
	break;
      case(10):  f3d_lcd_drawString(60, 150, " WINS = 10 ", WHITE, BLACK);
	break;
      }
    }

    if(wins == 7){
      //printf("You win!\n");
      
      youWin();
      //drawPicture(1, "win.bmp");
      return 0;
    }
    if (lives == 0){
      int upLives;
      if (cont == 0) {
	f3d_lcd_drawString(29, 2, "SHAKE BOARD!", WHITE, BLACK);
	upLives = shaker();
	f3d_lcd_drawString(29, 2, "SHAKE BOARD!", BLACK, BLACK);
      }

      if (upLives && cont == 0) {
	f3d_lcd_drawString(27, 2, "LIVES WENT UP!", WHITE, BLACK);
	delay(1000);
	f3d_lcd_drawString(27, 2, "LIVES WENT UP!", BLACK, BLACK);
	lives++;
	f3d_lcd_drawString(5, 150, "LIVES = 1", WHITE, BLACK);
	cont = 1;
      } else {
	drawPicture(1, "gameover.bmp");
	play("gong.wav");
	//f3d_lcd_drawString(37, 2, "GAME OVER!", WHITE, BLACK);
	return 0;
      }
    }
     memset(input, 0, 10);
  }
  return 0;
}

int main(void) { 

	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
  
	//inits
	f3d_delay_init();
	f3d_uart_init();
	delay(10);
	f3d_lcd_init();
	delay(10);
	f3d_lcd_sd_interface_init();
	delay(10);
	f3d_i2c1_init();
	delay(10);
	f3d_rtc_init();
	delay(10);
	f3d_nunchuk_init();
	delay(10);
	f3d_accel_init();
	delay(10);
	f3d_timer2_init();
	delay(10);
	f3d_dac_init();
	delay(10);
	f3d_systick_init();
  
	f_mount(0, &Fatfs);

	drawPicture(1, "DOG.BMP");

	int i;
	for(i = 0; i < 128; i++){
	  hand_color[i] = 0x001F;
	}
	int j;
	for(j = 0; j < 128; j++){
	  user_color[j] = 0x07FF;
	}


	while(1) {	  
	  delay(300);
	  // play("thermo.wav");
	  gameLogic();
	  //int yup = rando();
	  //yup = yup % 4;
	  //printf("rando: &d \n", yup);
	  return 0;
	}
  
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line) {
  /* Infinite loop */
  /* Use GDB to find out why we're here */
  while (1);
}
#endif


/* main.c ends here */
