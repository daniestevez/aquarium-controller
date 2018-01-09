#include <avr/eeprom.h>

#define LIGHT_PIN 1
#define LIGHT_OFFSET (byte *) 0x200

int light_rise;
int light_set;

boolean light_on;

void boot_illumination() {
  read_light_time(&light_rise, &light_set);
  light_on = light_on_time();
  pinMode(LIGHT_PIN, OUTPUT);
  digitalWrite(LIGHT_PIN, light_on);
  
  return;
}

void update_illumination() {
  if (light_on != light_on_time()) {
    light_on = !light_on;
    digitalWrite(LIGHT_PIN, light_on);
  }
  
  return;
}

static boolean light_on_time() {
  time_t utc, t;
  TimeElements tm;
  TimeChangeRule *tcr;
  int time_now;
  
  utc = now();
  t = tz.toLocal(utc, &tcr);
  breakTime(t, tm);

  time_now = tm.Hour*60 + tm.Minute;
  
  if (light_rise <= time_now && time_now < light_set) {
    return true;
  }
  else {
    return false;
  }
}

void write_light_time(int on, int off) {
  byte b;
  int i;
  
  eeprom_write_block((void *) &on, LIGHT_OFFSET, sizeof(int));
  eeprom_write_block((void *) &off, LIGHT_OFFSET + sizeof(int), sizeof(int));
  
  return;
}

void read_light_time(int *on, int *off) {
  int i;
  
  eeprom_read_block((void *) on, LIGHT_OFFSET, sizeof(int));
  eeprom_read_block((void *) off, LIGHT_OFFSET + sizeof(int), sizeof(int));

  return;
}
