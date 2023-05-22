/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include <chrono>
#define pause PC_5
#define refresh PC_4
#include "DFRobot_RGBLCD.h"
#include "Timer.h"
#define USE_STM_HAL_API 1
#define BLINKING_RATE 1000ms

using namespace std::chrono;

Timer t;

DFRobot_RGBLCD lcd(16, 2, D14, D15);

IWDG_HandleTypeDef wdog;
Watchdog &watchdog = Watchdog::get_instance();
bool flip = false;

// Keyword volatile needed to make sure that
// variable is read from memory before evaluated
volatile int state = 0;
volatile int pressed2 = 1;
// External interrupt callback
void button_interrup1t_cb(void) {
  // Toggle state; 0=>1 or 1=>0
  state = 1;
  flip = !flip;

  if (flip == true)
    t.stop();
  else
    t.start();

}
void button_interrup2t_cb(void) {
  // Toggle state; 0=>1 or 1=>0
  pressed2 = 1;
  watchdog.kick();
}
#define WAIT_TIME_MS 500
// DigitalOut led1(LED1);
// InterruptIn button(A0, PullDown);
// InterruptIn button1(A1, PullDown);


void watchdog_setup() {
  wdog.Instance = IWDG;
  // Low Speed Internal (LSI) oscillator ~32 000 / prescaler 256 =
  // 125 downcounts per second
  wdog.Init.Prescaler = IWDG_PRESCALER_256;
  wdog.Init.Reload = 1250; // 125 count per second, thus 250 = 2 seconds
  // To prevent a watchdog reset, the refresh must occur when
  // the downcounter value is higher than zero, and lower than
  // the window value (window value set lower than the reload value)
  wdog.Init.Window = 0xFFF; // 0xFFF (4095) to disable window mode
  HAL_IWDG_Init(&wdog);
}

constexpr uint32_t WATCHDOG_TIMEOUT_MS = 10000;


int main() {
  lcd.init();
  DigitalOut led1(LED1);
  InterruptIn button1(A0, PullUp);
  InterruptIn button2(A1, PullUp); // Blue button has pullup
  
  t.start();
  //watchdog_setup();

  // lcd.printf("The time taken was %llu milliseconds\n",
  // duration_cast<milliseconds>(t.elapsed_time()).count());

  // Configure callback function for falling edge interrupt from button
  // and interrupts are also enabled for the pin
  /*
  button.fall(&button_interrupt_cb);
  Watchdog &watchdog = Watchdog::get_instance();
  watchdog.start(10000);
   */

  button1.fall(button_interrup1t_cb);
  button2.fall(button_interrup2t_cb);


  watchdog.start(WATCHDOG_TIMEOUT_MS);

  while (true) {
    lcd.clear();

    lcd.printf("Time: %.1f sek", (float)(duration_cast<milliseconds>(t.elapsed_time()).count())/1000.0);
    led1.write(state);

    // Toggle LED just to indicate that application is running
    printf("Toggle LED\n");
    led1 = !led1;

    if (state == 1) {
      printf("button1 is pressed\n");
      state = 0;
    }
    if (pressed2 == 1) {
      printf("button2 is pressed\n");
      pressed2 = 0;
    }

    if (flip == true)
        watchdog.kick();

    // "Pet", "kick" or "feed" the dog to reset the watchdog timer
    // watchdog.kick();

    //if (value == false) {
      //wdog.kick();
    //}

    ThisThread::sleep_for(100ms);
  }
}
