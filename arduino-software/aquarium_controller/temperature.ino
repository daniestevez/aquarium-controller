#include <OneWire.h>
#include <avr/eeprom.h>

#include "temperature.h"

#define SCRATCHPAD_SIZE 9

#define CONVERSION_DELAY 750

#define CONVERT_T 0x44
#define READ_SCRATCHPAD 0xBE

#define TEMP_LSB 0
#define TEMP_MSB 1
#define TH 2
#define TL 3
#define COUNT_REMAIN 6
#define COUNT_PERC 7
#define SCRATCH_CRC 8

#define SENSOR_ADDR_OFFSET (byte *) 0x100

#define ONEWIRE_PIN 4

OneWire ds(ONEWIRE_PIN);

int temperatures[SENSORS];
boolean temperatures_valid[SENSORS];

static byte addresses[SENSORS][ADDR_SIZE];

#define N_DISCOVERED 4

byte discovered_therms[N_DISCOVERED][ADDR_SIZE];
int n_discovered;
int discovered_temps[N_DISCOVERED];

int boot_temperatures() {
  int i;
  byte scratch[SCRATCHPAD_SIZE];
  
  read_all_addresses();
  
  convert_t_all();
  delay(CONVERSION_DELAY);
    
  for (i = 0; i < SENSORS; i++) {
    if ((temperatures_valid[i] = read_scratchpad(addresses[i], scratch))) {
      temperatures[i] = read_temperature(scratch);
    }
  }
  
  return true;
} 

void discover_therms() {
  byte scratch[SCRATCHPAD_SIZE];
  int i;
  byte addr[ADDR_SIZE];
  
  // DISCOVER SENSORS ON BUS
  for (i = 0; i < N_DISCOVERED && ds.search(addr); ) {
    if (addr[0] == 0x10) { // device is DS18S20
      memcpy(discovered_therms + i++, addr, ADDR_SIZE);
    }
  }
  ds.reset_search();
  n_discovered = i;
  
  convert_t_all();
  delay(CONVERSION_DELAY);
  for (i = 0; i < n_discovered; i++) {
    read_scratchpad(discovered_therms[i], scratch);
    discovered_temps[i] = read_temperature(scratch);
  }
}

void update_temperatures() {
  int i;
  byte scratch[SCRATCHPAD_SIZE];
  
  // TODO: error checking
  for (i = 0; i < SENSORS; i++) {
    read_scratchpad(addresses[i], scratch);
    temperatures[i] = read_temperature(scratch);
  }
  
  convert_t_all();
 
  return;
}

void temperature_to_str(int temp, char *s) {
  if (temp < 0) {
    *s++ = '-';
    temp = -temp;
  }
  itoa(temp >> 4, s, 10);
  s += strlen(s);
  *s++ = '.';
  itoa((temp & 0x000F)*100 >> 4, s, 10);
  // stupid hack to handle the case when the decimal part is .0?
  if (!s[1]) {
    s[1] = s[0];
    s[0] = '0';
    s[2] = '\0';
  }
  
  return;
}

int read_temperature(byte scratch[]) {
  // extended resolution temperature using integer arithmetic
  // the least significant bit is 2^-4 degrees
  // assume COUNT_PERC = 16
  int temp;
  
  temp = (int(scratch[TEMP_MSB]) << 8) | scratch[TEMP_LSB];
  temp >>= 1;
  temp <<= 4;
  temp -= 0x0004; // 0.25=2^-2
  temp += 0x10 - scratch[COUNT_REMAIN];
  
  return temp;
}

void convert_t_all() {
  // issue CONVERT T to all devices
  ds.reset();
  ds.write(0xCC);
  ds.write(CONVERT_T,1);
}

boolean read_scratchpad(byte addr[], byte scratch[]) {
  // issue READ SCRATCHPAD
  // returns true if OK
  int i;
  
  ds.reset();
  ds.select(addr);
  ds.write(READ_SCRATCHPAD);

  for ( i = 0; i < SCRATCHPAD_SIZE; i++) {
    scratch[i] = ds.read();
  }
  
  // check CRC
  return OneWire::crc8(scratch, SCRATCHPAD_SIZE - 1) == scratch[SCRATCH_CRC];
}

void write_sensor_address(int sensor, byte addr[]) {
  eeprom_write_block((void *) addr, SENSOR_ADDR_OFFSET + sensor*ADDR_SIZE, ADDR_SIZE);
  
  return;
}

void read_sensor_address(int sensor, byte addr[]) {
  eeprom_read_block((void *) addr, SENSOR_ADDR_OFFSET + sensor*ADDR_SIZE, ADDR_SIZE);
  
  return;
}

void read_all_addresses() {
  int i;
  
  for (i = 0; i < SENSORS; i++) {
    read_sensor_address(i, addresses[i]);
  }
}

