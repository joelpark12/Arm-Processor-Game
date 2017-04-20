/* f3d_systick.c --- 
 * 
 * Filename: f3d_systick.c
 * Description: 
 * Author: Bryce Himebaugh
 * Maintainer: 
 * Created: Thu Nov 14 07:57:37 2013
 * Last-Updated: 
 *           By: 
 *     Update #: 0
 * Keywords: 
 * Compatibility: 
 * 
 */

/* Commentary: 
 * 
 * 
 * 
 */

/* Change log:
 * 
 * 
 */

/* Copyright (c) 2004-2007 The Trustees of Indiana University and 
 * Indiana University Research and Technology Corporation.  
 * 
 * All rights reserved. 
 * 
 * Additional copyrights may follow 
 */

/* Code: */

#include <f3d_systick.h>
#include <f3d_uart.h>
#include <queue.h>

volatile int systick_flag = 0; // why is this volatile????
int SYSTICK_INT_SEC = 100;

void f3d_systick_init(void) {
  SysTick_Config(SystemCoreClock/SYSTICK_INT_SEC);
}

void SysTick_Handler(void) {
 if (!queue_empty(&txbuf)) { 
   flush_uart(); // gotta create this in your uart!
 }
 // do led stuff after this....
 static int i = 0;
 if(user_btn_read() == 0) {
   if(SYSTICK_INT_SEC == 10) {
     SYSTICK_INT_SEC = 100;
     f3d_systick_init();
   }
 }
 else if (user_btn_read() == 1) {
   if(SYSTICK_INT_SEC == 100) {
     SYSTICK_INT_SEC = 10;
     f3d_systick_init();
   }
 }
 if(i == 7) {
   f3d_led_all_off();
   f3d_led_on(i);
   i = 0;
 }
 else {
   f3d_led_all_off();
   f3d_led_on(i);
   i++;
 }
}
/* f3d_systick.c ends here */
