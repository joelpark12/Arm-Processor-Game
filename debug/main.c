/* main.c 
   debug this function using gdb*/


/*
  The expected performace of the program is as follows:
  We want to print out the actual bits of numbers! What a great
  way to understand how number representation works

 */

//The libraries to include
#include <f3d_uart.h>
#include <stdio.h> 
#include <stdint.h>
//The arrays that store the bits

void delay(void) {
  int i = 2000000;
  while (i-- > 0) {
    asm("nop"); /* This stops it optimising code out */
  }
}

int gitBitsChar(unsigned char c, char answer[]){
  int i=7;
  while(i>-1) {
    if(c&1) answer[i] = '1';
    else answer[i] = '0';
    c=c>>1;
    i--;
  }
  answer[8] = 0; // need the extra bit
  return 0;
}

int gitBitsShort(unsigned short s, char answer[]){
  int i = 15;
  while(i>0) { // s will never be less than 1
    if(s&1) answer[i] = '1';
    else answer[i] = '0';
    s=s>>1;
    i--;
  }
  answer[16]=0; // last bit be a 0
  return 0;
}

int gitBitsInt(int yikes, char answer[]){
  int i = 31; // cannot be 16, needs to be 32 - 1
  while(i>0){ // need to compare I
    if(yikes&1) answer[i] = '1';
    else answer[i] = '0';
    yikes=yikes>>1; //switch the bit shift
    i--; // subtract since we start from 31
  }
  answer[32]=0; // 0 bit at end
  return 0;
}

//runs some test on our program
int main(){
  f3d_uart_init();
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);
  char ans1[32];
  char ans2[32];
  char ans3[33];
  char ans4[33];
  while (1){
    gitBitsShort((unsigned short) 3200, ans1);
    gitBitsChar((unsigned char) 'a', ans2);
    gitBitsInt(-94, ans3);
    gitBitsInt((unsigned int) 234, ans4);
    printf("3200 = %s\n", ans1);
    printf("unsigned a = %s\n", ans2);
    printf("-94 = %s\n", ans3);
    printf("unsigned int 234 = %s\n", ans4);
  }
 
}

void assert_failed(uint8_t* file, uint32_t line) {
/* Infinite loop */
/* Use GDB to find out why we're here */
  while (1);
}
