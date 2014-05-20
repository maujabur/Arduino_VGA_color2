// Tested on Arduino Mega only

// Code modified from http://www.gammon.com.au/forum/?id=11608

//#include <avr/pgmspace.h>
#include "image_BARS.h"

// a pixel every VDIV lines (2^n is valid)
#define VDIV 8

const int horizontalBytes = 60;  // 480 pixels wide
const int verticalPixels = 480;  // 480 pixels high

#include "TimerHelpers.h"
#include <avr/sleep.h>

const byte hSyncPin = 9;     // <------- HSYNC
const byte vSyncPin = 12;    // <------- VSYNC

// outputs hardcoded to PORTC (pins 37 to 30)
// color format depends on the resistor network on the outputs
// imag_XXX.h files encoded using rr0ggbb0 format

const int verticalLines = verticalPixels / VDIV;
const int vMask = VDIV-1;
const int horizontalPixels = horizontalBytes * 8;

const byte verticalBackPorchLines = 35;  // includes sync pulse?
const int verticalFrontPorchLines = 525 - verticalBackPorchLines;

volatile int vLine;
volatile int messageLine;
volatile byte backPorchLinesToGo;

#define nop asm volatile ("nop\n\t")

// ISR: Vsync pulse
ISR (TIMER1_OVF_vect)
{
  vLine = 0; 
  messageLine = 0;
  backPorchLinesToGo = verticalBackPorchLines;
} // end of TIMER1_OVF_vect

// ISR: Hsync pulse ... this interrupt merely wakes us up
ISR (TIMER2_OVF_vect)
{
} // end of TIMER2_OVF_vect


void setup()
{

  //  Serial.begin(9600);
  //  Serial.println(freeRam());
  //  delay(100);

  // disable Timer 0
  TIMSK0 = 0;  // no interrupts on Timer 0
  OCR0A = 0;   // and turn it off
  OCR0B = 0;
  // Timer 1 - vertical sync pulses
  pinMode (vSyncPin, OUTPUT); 
  Timer1::setMode (15, Timer1::PRESCALE_1024, Timer1::CLEAR_B_ON_COMPARE);
  OCR1A = 259;  // 16666 / 64 uS = 260 (less one)
  OCR1B = 0;    // 64 / 64 uS = 1 (less one)
  TIFR1 = _BV (TOV1);   // clear overflow flag
  TIMSK1 = _BV (TOIE1);  // interrupt on overflow on timer 1

  // Timer 2 - horizontal sync pulses
  pinMode (hSyncPin, OUTPUT); 
  Timer2::setMode (7, Timer2::PRESCALE_8, Timer2::CLEAR_B_ON_COMPARE);
  OCR2A = 63;   // 32 / 0.5 uS = 64 (less one)
  OCR2B = 7;    // 4 / 0.5 uS = 8 (less one)
  TIFR2 = _BV (TOV2);   // clear overflow flag
  TIMSK2 = _BV (TOIE2);  // interrupt on overflow on timer 2

    // prepare to sleep between horizontal sync pulses  
  set_sleep_mode (SLEEP_MODE_IDLE);  

  // direct manipulation
  DDRC = B11111111;


}  // end of setup

// draw
void doScan ()
{

  // after vsync we do the back porch
  if (backPorchLinesToGo)
  {
    backPorchLinesToGo--;
    return;   
  }  // end still doing back porch

  // if all lines done, do the front porch
  if (vLine >= verticalPixels)
    return;

  // pre-load pointer for speed
  register char * messagePtr =  &message [messageLine] [0];

  delayMicroseconds (1);

  // how many pixels to send
  register byte i = horizontalBytes;

  while (i--)
    PORTC = * messagePtr++; //  pgm_read_byte (messagePtr++);

  // stretch last pixel 
  nop; 
  nop; 
  nop;

  PORTC = 0;  // back to black

  // finished this line 
  vLine++;

  // every N pixels it is time to move to a new line in our text
  if ((vLine  & vMask) == 0)
    messageLine++;

}  // end of doOneScanLine

void loop() 
{
  // loop to avoid overhead of function all
  while (true)
  {
    // sleep to ensure we start up in a predictable way
    sleep_mode ();
    doScan ();
  }  // end of while
}  // end of loop

// Timer 1 - Vertical sync

//   Period: 16.64 mS (60 Hz)
//      1/60 * 1e6 = 16666.66 uS
//   Pulse for 64 uS  (2 x HSync width of 32 uS)
//    Sync pulse: 2 lines
//    Back porch: 33 lines
//    Active video: 480 lines
//    Front porch: 10 lines
//       Total: 525 lines

// Timer 2 - Horizontal sync

//   Period: 32 uS (31.25 kHz)
//      (1/60) / 525 * 1e6 = 31.74 uS
//   Pulse for 4 uS (96 times 39.68 nS)
//    Sync pulse: 96 pixels
//    Back porch: 48 pixels
//    Active video: 640 pixels
//    Front porch: 16 pixels
//       Total: 800 pixels

// Pixel time =  ((1/60) / 525 * 1e9) / 800 = 39.68  nS
//  frequency =  1 / (((1/60) / 525 * 1e6) / 800) = 25.2 MHz

// However in practice, it we can only pump out pixels at 375 nS each because it
//  takes 6 clock cycles to read one in from RAM and send it out the port.

// REFERENCE
/*
 VGA colour video generation
 
 Author:   Nick Gammon
 Date:     22nd April 2012
 Version:  1.0
 
 Version 1.0: initial release
 Version 1.1: Amended to output 64 colours
 
 Connections:
 
 (30) C7 : Red pixel output (470 ohms in series) ------------> Pin 1 on DB15 socket
 (31) C6 : Dull Red pixel output (1K resistor in series) ----> Pin 1 on DB15 socket (also)
 (33) C4 : Green pixel output (470 ohms in series) ----------> Pin 2 on DB15 socket
 (34) C3 : Dull Green pixel output (1K resistor in series) --> Pin 2 on DB15 socket (also)
 (35) C2 : Blue pixel output (470 ohms in series) -----------> Pin 35 on DB15 socket
 (36) C1 : Dull Blue pixel output (1K resistor in series) ---> Pin 36 on DB15 socket (also)
 (09) H6 : Horizontal Sync (68 ohms in series) --------------> Pin 13 on DB15 socket
 (12) B6 : Vertical Sync (68 ohms in series) ----------------> Pin 14 on DB15 socket
 Gnd : --> Pins 5, 6, 7, 8, 10 on DB15 socket
 
 
 Note: As written, this sketch has 34 bytes of free SRAM memory. (more, if using Arduino Mega)
 
 PERMISSION TO DISTRIBUTE
 
 Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
 and associated documentation files (the "Software"), to deal in the Software without restriction, 
 including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
 subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in 
 all copies or substantial portions of the Software.
 
 
 LIMITATION OF LIABILITY
 
 The software is provided "as is", without warranty of any kind, express or implied, 
 including but not limited to the warranties of merchantability, fitness for a particular 
 purpose and noninfringement. In no event shall the authors or copyright holders be liable 
 for any claim, damages or other liability, whether in an action of contract, 
 tort or otherwise, arising from, out of or in connection with the software 
 or the use or other dealings in the software. 
 
 */

