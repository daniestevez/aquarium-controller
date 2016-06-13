#include <Keypad.h>

#define ROWS 4
#define COLS 4

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte row_pins[ROWS] = {15, 8, 7, 6};
byte col_pins[COLS] = {5, 14, 16, 17};

static Keypad keypad = Keypad(makeKeymap(keys), row_pins, col_pins, ROWS, COLS);

char get_input() {
  char key = keypad.getKey();
  
  if (key) {
    input_backlight();
  }
  
  return key;
}

