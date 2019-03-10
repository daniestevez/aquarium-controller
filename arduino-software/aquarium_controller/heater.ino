#include <avr/eeprom.h>

#define HEATER_PIN 0

#define HEATER_OFFSET (byte *) 0x300

// 1/8 degree
#define LOW_THRESHOLD 2
// 1/16 degree
#define HIGH_THRESHOLD 1

int heater_temp;
boolean heater_on = false;

void boot_heater() {
  read_heater(&heater_temp);
  heater_on = heater();
  pinMode(HEATER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, heater_on);
  
  return;
}

void update_heater() {
  if (heater_on != heater()) {
    heater_on = !heater_on;
    digitalWrite(HEATER_PIN, heater_on);
  }
  
  return;
}

void write_heater(int temp) {
  eeprom_write_block((void *) &temp, HEATER_OFFSET, sizeof(int));
  
  return;
}

void read_heater(int *temp) {
  eeprom_read_block((void *) temp, HEATER_OFFSET, sizeof(int));

  return;
}

static boolean heater() {
  if (!temperatures_valid[WATER_SENSOR]) {
    /* temperature sensor is failing. turn the heater off */
    return false;
  }
  if (heater_on && temperatures[WATER_SENSOR] >= heater_temp + HIGH_THRESHOLD)
    return false;
  else if (!heater_on && temperatures[WATER_SENSOR] <= heater_temp - LOW_THRESHOLD)
    return true;

  return heater_on;    
}
