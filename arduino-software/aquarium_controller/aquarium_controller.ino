#include <Event.h>
#include <Timer.h>
#include <Wire.h>

#include <Time.h>

#include <avr/wdt.h>

#include "temperature.h"
#include "feeding.h"

#define TEMPERATURE_INTERVAL 2100
#define PRINCIPAL_INTERVAL 100
#define BACKLIGHT_INTERVAL 40
#define ILLUMINATION_INTERVAL 900
#define HEATER_INTERVAL 9800
#define FEEDING_INTERVAL 1500

Timer timer;

char key_pressed = '\0';

TimeElements time;

boolean notification() {
  return need_feeding;
}

void setup(void) {
  wdt_enable(WDTO_4S);
  
  pinMode(2, INPUT_PULLUP); // unused pin

  boot_backlight();
  boot_interface();
  boot_clock(); // TODO: handle error
  boot_illumination();
  boot_temperatures(); // TODO handle error
  boot_heater();
  boot_feeding();
  
  timer.every(TEMPERATURE_INTERVAL, update_temperatures);
  timer.every(PRINCIPAL_INTERVAL, update_principal);
  timer.every(BACKLIGHT_INTERVAL, update_backlight);
  timer.every(ILLUMINATION_INTERVAL, update_illumination);
  timer.every(HEATER_INTERVAL, update_heater);
  timer.every(FEEDING_INTERVAL, update_feeding);
  
  finish_boot_interface();  
  return;
}

void loop(void) {
  char input;

  wdt_reset();
  
  timer.update();
  
  input = get_input();
  if (input) {
    process_input(input);
  }
}
