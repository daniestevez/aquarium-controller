#include <avr/eeprom.h>
#include <Time.h>

#include "illumination.h"

#define FEEDING_OFFSET (byte *) 0x400

struct feed_data {
  int interval;
  int dose;
  time_t last;
};

static struct feed_data *feed_eeprom = (struct feed_data *) FEEDING_OFFSET;

int feeding_interval; /* in hours */
int feeding_dose;
time_t last_feeding;
boolean need_feeding;

void boot_feeding() {
  read_feeding();
  need_feeding = feeding();
  
  return;
}

void update_feeding() {
  need_feeding = feeding();
  
  return;
}

void write_feeding_interval(int interval) {
  eeprom_write_block((void *) &interval, &feed_eeprom->interval, sizeof(int));
  return;
}

void write_feeding_dose(int dose) {
  eeprom_write_block((void *) &dose, &feed_eeprom->dose, sizeof(int));
  return;
}

void write_last_feeding(time_t last) {
  eeprom_write_block((void *) &last, &feed_eeprom->last, sizeof(time_t));
  return;
}

void read_feeding() {
  eeprom_read_block((void *) &feeding_interval, &feed_eeprom->interval, sizeof(int));
  eeprom_read_block((void *) &feeding_dose, &feed_eeprom->dose, sizeof(int));
  eeprom_read_block((void *) &last_feeding, &feed_eeprom->last, sizeof(time_t));
  return;
}

static boolean feeding() {
  return light_on && (now() - last_feeding)/3600 >= feeding_interval;
}

void feed_now() {
  last_feeding = now();
  write_last_feeding(last_feeding);
  need_feeding = false;
}
  
