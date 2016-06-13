#define BACKLIGHT_PIN 3

#define BL_INPUT_EXPIRE 30000

#define FADE_STEP 1
#define FAST_FADE_STEP 5

#define BL_MAX 255
#define BL_MIN 25

#define BL_OSCIL_LOW 20
#define BL_OSCIL_HIGH 200

static byte fade_direction = 1;
static byte bl_level = BL_MAX;

static unsigned long last_input = 0;

void boot_backlight() {
   pinMode(BACKLIGHT_PIN, OUTPUT);
   analogWrite(BACKLIGHT_PIN, BL_MAX);
   
   return;
}

void input_backlight() {
  last_input = millis();
  bl_level = BL_MAX;
  analogWrite(BACKLIGHT_PIN, bl_level);
}

void update_backlight() {
  boolean input_expired = millis() - last_input > BL_INPUT_EXPIRE;
  
  if (!input_expired) return;
  
  if (notification()) {
    if (bl_level > BL_OSCIL_HIGH) {
      bl_level -= FAST_FADE_STEP;
    }
    else {   
      if (bl_level - BL_OSCIL_LOW < FADE_STEP) {
        fade_direction = 1;
      }
      else if (BL_OSCIL_HIGH - bl_level < FADE_STEP) {
        fade_direction = -1;
      }
      bl_level += FADE_STEP*fade_direction;
    }
    analogWrite(BACKLIGHT_PIN, bl_level);
  }
  else if (bl_level > BL_MIN) {
    if (bl_level - BL_MIN <= FAST_FADE_STEP) {
      bl_level = BL_MIN;
    }
    else {
      bl_level -= FAST_FADE_STEP;
    }
    analogWrite(BACKLIGHT_PIN, bl_level);
  }
  
  return;
}

