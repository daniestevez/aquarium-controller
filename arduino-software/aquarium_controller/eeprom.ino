#include <EEPROM.h>

#define EEPROM_SIZE 1024

void erase_eeprom() {
  int i;
  
  for (i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0);
  }
  
  return;
}
