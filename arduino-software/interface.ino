#include <U8glib.h>
#include <Time.h>
#include <Timezone.h>

#include "temperature.h"
#include "principal.h"

// Language setting. Default is English
//#define LANG_ES

#define PRINCIPAL 0
#define MENU 1
#define TEST_INPUT 2
#define CLOCK_MENU 3
#define CLOCK_SET 4
#define THERM_MENU 5
#define THERM_CONSULT 6
#define SET_THERM 7
#define ERASE_MENU 8
#define ERASE_EEPROM 9
#define LIGHT_MENU 10
#define LIGHT_SEE 11
#define LIGHT_SAVE 12
#define TEMP_MENU 13
#define TEMP_SEE 14
#define TEMP_SET 15
#define FEED_MENU 16
#define FEED_SEE 17
#define FEED_INTERVAL 18
#define FEED_DOSE 19
#define FEED_NOW 20

#define WIDTH 128
#define HEIGHT 64

U8GLIB_DOGM128 u8g(10, 9);

/* Functions to handle screen */
struct screen_program {
  void (*draw)();
  void (*prepare)();
  void (*input_handle)(char);
};

struct screen_program boot_program = { boot_screen, NULL, NULL };

static struct screen_program *program = &boot_program;

/* Menu information */
struct menu {
  const u8g_pgm_uint8_t *title;
  const u8g_pgm_uint8_t **items;
  int *links;
  int *next_actives;
};

static struct menu *menu;

/* Input information */
#define INP_BUF_LEN 20
static char input_buffer[INP_BUF_LEN];
static int input_idx;

struct input {
  const u8g_pgm_uint8_t *title;
  const u8g_pgm_uint8_t **msgs;
  boolean (*process)();
};

static struct input *input;

static int return_link;
static int return_active;


/** MAIN MENU **/
#ifdef LANG_ES
static const u8g_pgm_uint8_t main_menu_title[] U8G_PROGMEM = "MEN\332 PRINCIPAL";
static const u8g_pgm_uint8_t main_menu_items_0[] U8G_PROGMEM = "Reloj";
static const u8g_pgm_uint8_t main_menu_items_1[] U8G_PROGMEM = "Term\363metros";
static const u8g_pgm_uint8_t main_menu_items_2[] U8G_PROGMEM = "Iluminaci\363n";
static const u8g_pgm_uint8_t main_menu_items_3[] U8G_PROGMEM = "Temperatura";
static const u8g_pgm_uint8_t main_menu_items_4[] U8G_PROGMEM = "Alimentaci\363n";
static const u8g_pgm_uint8_t main_menu_items_5[] U8G_PROGMEM = "Borrar EEPROM";
#else
static const u8g_pgm_uint8_t main_menu_title[] U8G_PROGMEM = "MAIN MENU";
static const u8g_pgm_uint8_t main_menu_items_0[] U8G_PROGMEM = "Clock";
static const u8g_pgm_uint8_t main_menu_items_1[] U8G_PROGMEM = "Thermometers";
static const u8g_pgm_uint8_t main_menu_items_2[] U8G_PROGMEM = "Lightning";
static const u8g_pgm_uint8_t main_menu_items_3[] U8G_PROGMEM = "Temperatura";
static const u8g_pgm_uint8_t main_menu_items_4[] U8G_PROGMEM = "Feeding";
static const u8g_pgm_uint8_t main_menu_items_5[] U8G_PROGMEM = "Erase EEPROM";
#endif
static const u8g_pgm_uint8_t *main_menu_items[] = {
  main_menu_items_0, main_menu_items_1, main_menu_items_2, main_menu_items_3, main_menu_items_4,
  main_menu_items_5,
  NULL
};
static int main_menu_links[] = { CLOCK_MENU, THERM_MENU, LIGHT_MENU, TEMP_MENU, FEED_MENU, ERASE_MENU };
static int main_menu_next_actives[] = { 0, 0, 0, 0, 0, 0 };
static struct menu main_menu = { main_menu_title, main_menu_items, main_menu_links, main_menu_next_actives };

/**  CLOCK MENU **/
#ifdef LANG_ES
static const u8g_pgm_uint8_t clock_menu_title[] U8G_PROGMEM = "RELOJ";
static const u8g_pgm_uint8_t clock_menu_items_0[] U8G_PROGMEM = "Cambiar fecha y hora";
#else
static const u8g_pgm_uint8_t clock_menu_title[] U8G_PROGMEM = "CLOCK";
static const u8g_pgm_uint8_t clock_menu_items_0[] U8G_PROGMEM = "Set date and time";
#endif
static const u8g_pgm_uint8_t *clock_menu_items[] = {
  clock_menu_items_0, NULL
};
static int clock_menu_links[] = { CLOCK_SET };
static int clock_menu_next_actives[] = { 0 };
static struct menu clock_menu = { clock_menu_title, clock_menu_items, clock_menu_links, clock_menu_next_actives };

/** CHANGE CLOCK **/
#ifdef LANG_ES
static const u8g_pgm_uint8_t clock_set_title[] U8G_PROGMEM = "CAMBIAR FECHA Y HORA";
static const u8g_pgm_uint8_t clock_set_0[] U8G_PROGMEM = "Introduzca nueva hora local";
static const u8g_pgm_uint8_t clock_set_1[] U8G_PROGMEM = "Formato: YYYYMMDDHHMMSS";
#else
static const u8g_pgm_uint8_t clock_set_title[] U8G_PROGMEM = "SET DATE AND TIME";
static const u8g_pgm_uint8_t clock_set_0[] U8G_PROGMEM = "Enter new local time";
static const u8g_pgm_uint8_t clock_set_1[] U8G_PROGMEM = "Format: YYYYMMDDHHMMSS";
#endif
static const u8g_pgm_uint8_t *clock_set_msgs[] = {clock_set_0, clock_set_1, NULL};
static struct input clock_set = { clock_set_title, clock_set_msgs, process_clock };

/**  THERMOMETER MENU **/
#ifdef LANG_ES
static const u8g_pgm_uint8_t therm_menu_title[] U8G_PROGMEM = "TERM\323METROS";
static const u8g_pgm_uint8_t therm_menu_items_0[] U8G_PROGMEM = "Consultar temperaturas";
static const u8g_pgm_uint8_t therm_menu_items_1[] U8G_PROGMEM = "Config. term\363metro agua";
static const u8g_pgm_uint8_t therm_menu_items_2[] U8G_PROGMEM = "Config. term\363metro aire";
#else
static const u8g_pgm_uint8_t therm_menu_title[] U8G_PROGMEM = "THERMOMETERS";
static const u8g_pgm_uint8_t therm_menu_items_0[] U8G_PROGMEM = "Read temperatures";
static const u8g_pgm_uint8_t therm_menu_items_1[] U8G_PROGMEM = "Set water thermometer";
static const u8g_pgm_uint8_t therm_menu_items_2[] U8G_PROGMEM = "Set air thermometer";
#endif
static const u8g_pgm_uint8_t *therm_menu_items[] = {
  therm_menu_items_0, therm_menu_items_1, therm_menu_items_2, NULL
};
static int therm_menu_links[] = { THERM_CONSULT, SET_THERM, SET_THERM };
static int therm_menu_next_actives[] = { 0, WATER_SENSOR, AIR_SENSOR };
static struct menu therm_menu = { therm_menu_title, therm_menu_items, therm_menu_links, therm_menu_next_actives };

/** SET THERM **/
#ifdef LANG_ES
static const u8g_pgm_uint8_t therm_set_title[] U8G_PROGMEM = "CONFIGURAR TERM\323METRO";
static const u8g_pgm_uint8_t therm_set_0[] U8G_PROGMEM = "Introduzca el n\372mero de term\363tro";
#else
static const u8g_pgm_uint8_t therm_set_title[] U8G_PROGMEM = "SET THERMOMETER";
static const u8g_pgm_uint8_t therm_set_0[] U8G_PROGMEM = "Enter thermometer number";
#endif
static const u8g_pgm_uint8_t *therm_set_msgs[] = {therm_set_0, NULL};
static struct input therm_set = { therm_set_title, therm_set_msgs, process_therm };

/**  ERASE EEPROM MENU **/
#ifdef LANG_ES
static const u8g_pgm_uint8_t erase_menu_title[] U8G_PROGMEM = "BORRAR EEPROM";
static const u8g_pgm_uint8_t erase_menu_items_0[] U8G_PROGMEM = "Cancelar";
static const u8g_pgm_uint8_t erase_menu_items_1[] U8G_PROGMEM = "Borrar";
#else
static const u8g_pgm_uint8_t erase_menu_title[] U8G_PROGMEM = "ERASE EEPROM";
static const u8g_pgm_uint8_t erase_menu_items_0[] U8G_PROGMEM = "Cancel";
static const u8g_pgm_uint8_t erase_menu_items_1[] U8G_PROGMEM = "Erase";
#endif
static const u8g_pgm_uint8_t *erase_menu_items[] = {
  erase_menu_items_0, erase_menu_items_1, NULL
};
static int erase_menu_links[] = { MENU, ERASE_EEPROM };
static int erase_menu_next_actives[] = { 5, 0 };
static struct menu erase_menu = { erase_menu_title, erase_menu_items, erase_menu_links, erase_menu_next_actives };

/**  LIGHT MENU **/
#ifdef LANG_ES
static const u8g_pgm_uint8_t light_menu_title[] U8G_PROGMEM = "ILUMINACI\323N";
static const u8g_pgm_uint8_t light_menu_items_0[] U8G_PROGMEM = "Consultar horario";
static const u8g_pgm_uint8_t light_menu_items_1[] U8G_PROGMEM = "Cambiar hora encendido";
static const u8g_pgm_uint8_t light_menu_items_2[] U8G_PROGMEM = "Cambiar hora apagado";
#else
static const u8g_pgm_uint8_t light_menu_title[] U8G_PROGMEM = "LIGHTNING";
static const u8g_pgm_uint8_t light_menu_items_0[] U8G_PROGMEM = "See programming";
static const u8g_pgm_uint8_t light_menu_items_1[] U8G_PROGMEM = "Change on time";
static const u8g_pgm_uint8_t light_menu_items_2[] U8G_PROGMEM = "Change off time";
#endif
static const u8g_pgm_uint8_t *light_menu_items[] = {
  light_menu_items_0, light_menu_items_1, light_menu_items_2, NULL
};
static int light_menu_links[] = { LIGHT_SEE, LIGHT_SAVE, LIGHT_SAVE };
static int light_menu_next_actives[] = { 0, 0, 1 };
static struct menu light_menu = { light_menu_title, light_menu_items, light_menu_links, light_menu_next_actives };

/** CHANGE LIGHT TIME **/
#ifdef LANG_ES
static const u8g_pgm_uint8_t light_set_title[] U8G_PROGMEM = "HORA ILUMINACI\323N";
static const u8g_pgm_uint8_t light_set_0[] U8G_PROGMEM = "Introduzca nueva hora local";
static const u8g_pgm_uint8_t light_set_1[] U8G_PROGMEM = "Formato: HHMM";
#else
static const u8g_pgm_uint8_t light_set_title[] U8G_PROGMEM = "LIGHTNING TIME";
static const u8g_pgm_uint8_t light_set_0[] U8G_PROGMEM = "Enter new local time";
static const u8g_pgm_uint8_t light_set_1[] U8G_PROGMEM = "Format: HHMM";
#endif
static const u8g_pgm_uint8_t *light_set_msgs[] = {light_set_0, light_set_1, NULL};
static struct input set_light = { light_set_title, light_set_msgs, process_light };

/**  TEMPERATURE MENU **/
#ifdef LANG_ES
static const u8g_pgm_uint8_t temp_menu_title[] U8G_PROGMEM = "TEMPERATURA";
static const u8g_pgm_uint8_t temp_menu_items_0[] U8G_PROGMEM = "Consultar programaci\363n";
static const u8g_pgm_uint8_t temp_menu_items_1[] U8G_PROGMEM = "Cambiar programaci\363n";
#else
static const u8g_pgm_uint8_t temp_menu_title[] U8G_PROGMEM = "TEMPERATURE";
static const u8g_pgm_uint8_t temp_menu_items_0[] U8G_PROGMEM = "See programming";
static const u8g_pgm_uint8_t temp_menu_items_1[] U8G_PROGMEM = "Change programming";
#endif
static const u8g_pgm_uint8_t *temp_menu_items[] = {
  temp_menu_items_0, temp_menu_items_1, NULL
};
static int temp_menu_links[] = { TEMP_SEE, TEMP_SET };
static int temp_menu_next_actives[] = { 0, 0 };
static struct menu temp_menu = { temp_menu_title, temp_menu_items, temp_menu_links, temp_menu_next_actives };

/** CHANGE TEMP **/
#ifdef LANG_ES
static const u8g_pgm_uint8_t temp_set_title[] U8G_PROGMEM = "PROG. TEMPERATURA";
static const u8g_pgm_uint8_t temp_set_0[] U8G_PROGMEM = "Introduzca nueva temperatura programada";
static const u8g_pgm_uint8_t temp_set_1[] U8G_PROGMEM = "Ej: 25*5 = 25.5\260C";
#else
static const u8g_pgm_uint8_t temp_set_title[] U8G_PROGMEM = "SET TEMPERATURE";
static const u8g_pgm_uint8_t temp_set_0[] U8G_PROGMEM = "Enter new temperature setting";
static const u8g_pgm_uint8_t temp_set_1[] U8G_PROGMEM = "Ex: 25*5 = 25.5\260C";
#endif
static const u8g_pgm_uint8_t *temp_set_msgs[] = {temp_set_0, temp_set_1, NULL};
static struct input set_temp = { temp_set_title, temp_set_msgs, process_temp };

/**  FEEDING MENU **/
#ifdef LANG_ES
static const u8g_pgm_uint8_t feed_menu_title[] U8G_PROGMEM = "ALIMENTACI\323N";
static const u8g_pgm_uint8_t feed_menu_items_0[] U8G_PROGMEM = "Consultar programaci\363n";
static const u8g_pgm_uint8_t feed_menu_items_1[] U8G_PROGMEM = "Cambiar intervalo";
static const u8g_pgm_uint8_t feed_menu_items_2[] U8G_PROGMEM = "Cambiar dosis";
static const u8g_pgm_uint8_t feed_menu_items_3[] U8G_PROGMEM = "Alimentar";
#else
static const u8g_pgm_uint8_t feed_menu_title[] U8G_PROGMEM = "FEEDING";
static const u8g_pgm_uint8_t feed_menu_items_0[] U8G_PROGMEM = "See programming";
static const u8g_pgm_uint8_t feed_menu_items_1[] U8G_PROGMEM = "Set interval";
static const u8g_pgm_uint8_t feed_menu_items_2[] U8G_PROGMEM = "Set dose";
static const u8g_pgm_uint8_t feed_menu_items_3[] U8G_PROGMEM = "Feed";
#endif
static const u8g_pgm_uint8_t *feed_menu_items[] = {
  feed_menu_items_0, feed_menu_items_1, feed_menu_items_2, feed_menu_items_3, NULL
};
static int feed_menu_links[] = { FEED_SEE, FEED_INTERVAL, FEED_DOSE, FEED_NOW };
static int feed_menu_next_actives[] = { 0, 0, 0, 0 };
static struct menu feed_menu = { feed_menu_title, feed_menu_items, feed_menu_links, feed_menu_next_actives };

/** FEED INTERVAL **/
#ifdef LANG_ES
static const u8g_pgm_uint8_t feed_interval_title[] U8G_PROGMEM = "PROG. ALIMENTACI\323";
static const u8g_pgm_uint8_t feed_interval_0[] U8G_PROGMEM = "Introduzca nuevo intervalo";
static const u8g_pgm_uint8_t feed_interval_1[] U8G_PROGMEM = "(en horas)";
#else
static const u8g_pgm_uint8_t feed_interval_title[] U8G_PROGMEM = "SET FEEDING";
static const u8g_pgm_uint8_t feed_interval_0[] U8G_PROGMEM = "Enter new interval";
static const u8g_pgm_uint8_t feed_interval_1[] U8G_PROGMEM = "(in hours)";
#endif
static const u8g_pgm_uint8_t *feed_interval_msgs[] = {feed_interval_0, feed_interval_1, NULL};
static struct input set_feed_interval = { feed_interval_title, feed_interval_msgs, process_feed_interval };

/** FEED DOSE **/
#ifdef LANG_ES
static const u8g_pgm_uint8_t feed_dose_0[] U8G_PROGMEM = "Introduzca nueva dosis";
static const u8g_pgm_uint8_t feed_dose_1[] U8G_PROGMEM = "(en bolas)";
#else
static const u8g_pgm_uint8_t feed_dose_0[] U8G_PROGMEM = "Enter new dose";
static const u8g_pgm_uint8_t feed_dose_1[] U8G_PROGMEM = "(in pellets)";
#endif
static const u8g_pgm_uint8_t *feed_dose_msgs[] = {feed_dose_0, feed_dose_1, NULL};
static struct input set_feed_dose = { feed_interval_title, feed_dose_msgs, process_feed_dose };

void process_input(char input) {
  if (program->input_handle) program->input_handle(input);
}

void boot_interface() {
  picture_loop();
}

void finish_boot_interface() {
  change_to(PRINCIPAL);
}

static int active_item;

static void input_init() {
  *input_buffer = '\0';
  input_idx = 0;
  return;
}

static void input_draw() {
  const u8g_pgm_uint8_t **msg;
  int line;
  int fls;
  
  u8g.setFont(u8g_font_6x10);
  u8g.setFontPosTop();
  
  u8g.drawStrP(64 - u8g.getStrWidthP(input->title)/2, 0, input->title);
  u8g.drawHLine(0, u8g.getFontLineSpacing(), WIDTH);
  
  u8g.setFont(u8g_font_baby);
  u8g.setFontPosTop();
  fls = u8g.getFontLineSpacing();
  for (msg = input->msgs, line = 2*fls; *msg != NULL; msg++, line += fls) {
    u8g.drawStrP(0, line, *msg);
  }
  
  u8g.drawHLine(5, HEIGHT - 10, WIDTH - 20);
  u8g.setFont(u8g_font_6x10);
  u8g.setFontPosBottom();
  u8g.drawStr(5, HEIGHT - 10, input_buffer);
  
  return;
}

static void inp_handler(char in) {
  switch(in) {
    case 'D':
      change_to(PRINCIPAL);
      break;
    case 'A':
      if (input->process()) { /* TODO: handle error */
        active_item = return_active;
        change_to(return_link);
      }
      break;
    case 'C':
      break;
    case 'B':
      if (input_idx == 0) break;
      input_buffer[--input_idx] = '\0';
      picture_loop();
      break;
    default:
      if (input_idx == INP_BUF_LEN - 1) break;
      input_buffer[input_idx++] = in;
      input_buffer[input_idx] = '\0';
      picture_loop();
      break;
  }
}

static screen_program input_program = { input_draw, NULL, inp_handler };

static void menu_draw() {
  u8g_uint_t list_start;
  u8g_uint_t fls;
  int i;
  int start;
  const u8g_pgm_uint8_t **item;
  int items_per_page;
  
  u8g.setFont(u8g_font_6x10);
  u8g.setFontPosTop();
  
  u8g.drawStrP(64 - u8g.getStrWidthP(menu->title)/2, 0, menu->title);
  list_start = u8g.getFontLineSpacing() + 1;
  u8g.drawHLine(0, list_start - 1, WIDTH);
  
  u8g.setFont(u8g_font_baby);
  u8g.setFontPosTop();
  fls = u8g.getFontLineSpacing();
  items_per_page = (HEIGHT - list_start)/fls;
  start =  (active_item/items_per_page)*items_per_page; // starting item
  for (item = menu->items + start, i = 0; *item != NULL && i < items_per_page; item++, i++) {
    if (start + i == active_item) {
      /* negative colour */
      u8g.drawBox(0, list_start + i*fls, WIDTH, fls - u8g.getFontDescent());
      u8g.setDefaultBackgroundColor();
    }
    u8g.drawStrP(5, list_start + i*fls, *item);
    if (start + i == active_item) {
      u8g.setDefaultForegroundColor();
    }
  };
  
  return;
}

static void menu_handler(char input) {
  int old_active;
  
  switch (input) {
    case 'D':
      change_to(PRINCIPAL);
      break;
    case '2':
      if (active_item != 0) {
        active_item--;
        picture_loop();
      }
      break;
    case '8':
      if (menu->items[active_item+1] != NULL) {
        active_item++;
        picture_loop();
      }
      break;
    case '5':
    case '6':
    case 'A':
      old_active = active_item;
      active_item = menu->next_actives[old_active];
      change_to(menu->links[old_active]);
      break;
    case '4':
    case 'B':
      active_item = return_active;
      change_to(return_link);
    default:
      break;
  }
  return;
}

static struct screen_program menu_program = { menu_draw, NULL, menu_handler };

static void picture_loop() {
  // picture loop
  if (program->prepare) program->prepare();
  u8g.firstPage();  
  do {
    program->draw();
  } while(u8g.nextPage());
}

static void boot_screen() {
  u8g.setFont(u8g_font_6x10);
  u8g.drawStr(0, 13, "Booting...");
  
  return;
}

static char time_str[20];
static char date_str[20];
static char temps[2][20];

static void prepare_principal() {
  time_t utc, t;
  TimeElements tm;
  TimeChangeRule *tcr;
  
  utc = now();
  t = tz.toLocal(utc, &tcr);
  breakTime(t, tm);
  sprintf(time_str, "%02d:%02d:%02d", tm.Hour, tm.Minute, tm.Second);
  sprintf(date_str, "%d/%d/%02d", tm.Day, tm.Month, (1970 + tm.Year) % 100);

  temperature_to_str(temperatures[0], temps[0]);
  temperature_to_str(temperatures[1], temps[1]);

  return;
}

static const u8g_pgm_uint8_t degrees_label[] U8G_PROGMEM = "\260C";

static void principal_draw() {
  char buf[50];
  
  u8g.drawXBMP(0, 0, principal_width, principal_height, principal_bits);
  
  u8g.setFont(u8g_font_6x10);
  u8g.setFontPosTop();
  u8g.drawStr(85, 5, temps[0]);
  u8g.drawStrP(85 + u8g.getStrWidth(temps[0]), 5, degrees_label);
  
  u8g.drawStr(3, 52, time_str);
  
  u8g.setFont(u8g_font_baby);
  u8g.setFontPosTop();
  u8g.drawStr(96, 14, temps[1]);
  u8g.drawStrP(96 + u8g.getStrWidth(temps[1]), 14, degrees_label);
  
  u8g.drawStr(126 - u8g.getStrWidth(date_str), 53, date_str);
  
  if (light_on) {
    u8g.drawBox(9, 7, 2, 2);
  }
  if (heater_on) {
    u8g.drawBox(17, 7, 2, 2);
  }
  
  if (need_feeding) {
#ifdef LANG_ES
    sprintf(buf, "Alimentar %d bolas y pulsar #", feeding_dose);
#else
    sprintf(buf, "Feed %d pellets and press #", feeding_dose);
#endif
    u8g.drawStr(5, 25, buf);
  }

  return;
}

static void principal_handler(char input) {
  switch (input) {
    case 'C':
      active_item = 0;
      change_to(MENU);
      break;
    case '#':
      feed_now();
      break;
    default:
      break;
  }
  return;
}

static struct screen_program principal_program = { principal_draw, prepare_principal, principal_handler };

void time_to_str(TimeElements tm, char *s) {
  itoa(tm.Hour, s, 10);
  s += strlen(s);
  *s++ = ':';
  itoa(tm.Minute, s, 10);
  s += strlen(s);
  *s++ = ':';
  itoa(tm.Second, s, 10);
  s += strlen(s);
  *s++ = ' ';
  itoa(tm.Day, s, 10);
  s += strlen(s);
  *s++ = '/';
  itoa(tm.Month, s, 10);
  s += strlen(s);
  *s++ = '/';
  itoa(1970 + tm.Year, s, 10);
  
  return;
}

struct date_hour {
  char year[4];
  char month[2];
  char day[2];
  char hour[2];
  char minute[2];
  char second[2];
};

boolean process_clock() {
  char buf[5];
  struct date_hour *dh = (struct date_hour*) input_buffer;
  tmElements_t tm;
  time_t local, utc;
  
  memcpy(buf, dh->year, sizeof(dh->year));
  buf[sizeof(dh->year)] = '\0';
  tm.Year = CalendarYrToTm(atoi(buf));
  if (tm.Year < 0) return false;
  
  memcpy(buf, dh->month, sizeof(dh->month));
  buf[sizeof(dh->month)] = '\0';
  tm.Month = atoi(buf);
  if (tm.Month < 1 || tm.Month > 12) return false;
  
  memcpy(buf, dh->day, sizeof(dh->day));
  buf[sizeof(dh->day)] = '\0';
  tm.Day = atoi(buf);
  if (tm.Day < 1 || tm.Day > 31) return false;
  
  memcpy(buf, dh->hour, sizeof(dh->hour));
  buf[sizeof(dh->hour)] = '\0';
  tm.Hour = atoi(buf);
  if (tm.Hour < 0 || tm.Hour > 24) return false;
  
  memcpy(buf, dh->minute, sizeof(dh->minute));
  buf[sizeof(dh->minute)] = '\0';
  tm.Minute = atoi(buf);
  if (tm.Minute < 0 || tm.Minute > 60) return false;
  
  memcpy(buf, dh->second, sizeof(dh->second));
  buf[sizeof(dh->second)] = '\0';
  tm.Second = atoi(buf);
  if (tm.Second < 0 || tm.Second > 60) return false;
  
  local = makeTime(tm);
  utc = tz.toUTC(local);
  RTC.set(utc);
  setTime(utc);
  
  return true;
}

static boolean process_therm() {
  int n = input_buffer[0] - '0';
  
  write_sensor_address(active_item, discovered_therms[n]);
  read_all_addresses();
  
  return true;
}

struct h_m {
  char hour[2];
  char minute[2];
};

boolean process_light() {
  byte h, m, s;
  int t;
  char buf[5];
  struct h_m *hour = (struct h_m*) input_buffer;

  memcpy(buf, hour->hour, sizeof(hour->hour));
  buf[sizeof(hour->hour)] = '\0';
  h = atoi(buf);
  if (h < 0 || h > 24) return false;
  
  memcpy(buf, hour->minute, sizeof(hour->minute));
  buf[sizeof(hour->minute)] = '\0';
  m = atoi(buf);
  if (m < 0 || m > 60) return false;
  
  t = h*60 + m;
  
  if (active_item == 0) {
    light_rise = t;
  }
  else {
    light_set = t;
  }
  write_light_time(light_rise, light_set);
  return_active = 0;

  return true;
}

boolean process_temp() {
  char buf[5];
  int integer, fract;
  int temp;
  
  memcpy(buf, input_buffer, 2);
  buf[2] = '\0';
  integer = atoi(buf);
  fract = atoi(input_buffer + 3);
  
  temp = integer;
  temp <<= 4;
  temp += (fract*16)/10;
  
  heater_temp = temp;
  write_heater(temp);
  
  return true;
}

static boolean process_feed_interval() {
  feeding_interval = atoi(input_buffer);
  write_feeding_interval(feeding_interval);
  
  return true;
}

static boolean process_feed_dose() {
  feeding_dose = atoi(input_buffer);
  write_feeding_dose(feeding_dose);
  
  return true;
}  

#ifdef LANG_ES
static const u8g_pgm_uint8_t title_temperatures[] U8G_PROGMEM = "TEMPERATURAS";
static const char thermometer_label[] PROGMEM = "Term\363metro ";
#else
static const u8g_pgm_uint8_t title_temperatures[] U8G_PROGMEM = "TEMPERATURES";
static const char thermometer_label[] PROGMEM = "Thermometer ";
#endif

static void therm_draw() {
  u8g_uint_t list_start;
  u8g_uint_t fls;
  int i;
  char buf[20];
  
  u8g.setFont(u8g_font_6x10);
  u8g.setFontPosTop();
  
  u8g.drawStrP(64 - u8g.getStrWidthP(title_temperatures)/2, 0, title_temperatures);
  list_start = u8g.getFontLineSpacing() + 1;
  u8g.drawHLine(0, list_start - 1, WIDTH);
  
  u8g.setFontPosTop();
  fls = u8g.getFontLineSpacing();
  for (i = 0; i < n_discovered; i++) {
    memcpy_P(buf, thermometer_label, 11);
    buf[11] = '0' + i;
    buf[12] = '\0';
    u8g.drawStr(0, list_start + i*fls, buf);
    temperature_to_str(discovered_temps[i], buf);
    u8g.drawStr(86, list_start + i*fls, buf);
    u8g.drawStrP(86 + u8g.getStrWidth(buf), list_start + i*fls, degrees_label);
  };
  
  return;
}

static void therm_handler(char input) {
  switch (input) {
    case 'D':
      change_to(PRINCIPAL);
      break;
    case 'B':
      active_item = return_active;
      change_to(return_link);
    default:
      break;
  }
  return;
}

static screen_program therm_program = { therm_draw, NULL, therm_handler };

#ifdef LANG_ES
static const u8g_pgm_uint8_t title_lights[] U8G_PROGMEM = "HORARIO ILUMINACI\323N";
static const u8g_pgm_uint8_t on_label[] U8G_PROGMEM = "Encendido";
static const u8g_pgm_uint8_t off_label[] U8G_PROGMEM = "Apagado";
#else
static const u8g_pgm_uint8_t title_lights[] U8G_PROGMEM = "LIGHTNING TIME";
static const u8g_pgm_uint8_t on_label[] U8G_PROGMEM = "On";
static const u8g_pgm_uint8_t off_label[] U8G_PROGMEM = "Off";
#endif

static void light_draw() {
  u8g_uint_t list_start;
  u8g_uint_t fls;
  int i;
  
  u8g.setFont(u8g_font_6x10);
  u8g.setFontPosTop();
  
  u8g.drawStrP(64 - u8g.getStrWidthP(title_lights)/2, 0, title_lights);
  list_start = u8g.getFontLineSpacing() + 1;
  u8g.drawHLine(0, list_start - 1, WIDTH);
  
  u8g.setFontPosTop();
  fls = u8g.getFontLineSpacing();
  u8g.drawStrP(0, list_start, on_label);
  u8g.drawStrP(0, list_start + fls, off_label);
  light_time(60, list_start, light_rise);
  light_time(60, list_start + fls, light_set);
  
  return;
}

static void light_time(int x, int y, int t) {
  int h = t / 60;
  int m = t - 60*h;
  char buf[10];
  
  sprintf(buf, "%02d:%02d", h, m);
  u8g.drawStr(x, y, buf);
  
  return;
}

static screen_program light_program = { light_draw, NULL, therm_handler };

#ifdef LANG_ES
static const u8g_pgm_uint8_t title_heater[] U8G_PROGMEM = "PROG. TEMPERATURA";
#else
static const u8g_pgm_uint8_t title_heater[] U8G_PROGMEM = "SET TEMPERATURE";
#endif

static void temp_draw() {
  u8g_uint_t list_start;
  char buf[20];
  
  u8g.setFont(u8g_font_6x10);
  u8g.setFontPosTop();
  
  u8g.drawStrP(64 - u8g.getStrWidthP(title_heater)/2, 0, title_heater);
  list_start = u8g.getFontLineSpacing() + 1;
  u8g.drawHLine(0, list_start - 1, WIDTH);
  
  u8g.setFontPosTop();
  temperature_to_str(heater_temp, buf);
  u8g.drawStr(5, list_start, buf);
  u8g.drawStrP(5 + u8g.getStrWidth(buf), list_start, degrees_label);
  
  return;
}

static screen_program temp_program = { temp_draw, NULL, therm_handler };

#ifdef LANG_ES
static const u8g_pgm_uint8_t title_feed[] U8G_PROGMEM = "PROG. ALIMENTACI\323N";
static const u8g_pgm_uint8_t interval_label[] U8G_PROGMEM = "Intervalo (horas)";
static const u8g_pgm_uint8_t dose_label[] U8G_PROGMEM = "Dosis (bolas)";
#else
static const u8g_pgm_uint8_t title_feed[] U8G_PROGMEM = "FEEDING SETTINGS";
static const u8g_pgm_uint8_t interval_label[] U8G_PROGMEM = "Interval (hours)";
static const u8g_pgm_uint8_t dose_label[] U8G_PROGMEM = "Dose (pellets)";
#endif

static void feed_draw() {
  u8g_uint_t list_start;
  u8g_uint_t fls;
  char buf[20];
  
  u8g.setFont(u8g_font_6x10);
  u8g.setFontPosTop();
  
  u8g.drawStrP(64 - u8g.getStrWidthP(title_feed)/2, 0, title_feed);
  list_start = u8g.getFontLineSpacing() + 1;
  u8g.drawHLine(0, list_start - 1, WIDTH);
  
  u8g.setFont(u8g_font_baby);
  u8g.setFontPosTop();
  fls = u8g.getFontLineSpacing();
  u8g.drawStrP(0, list_start, interval_label);
  u8g.drawStrP(0, list_start + fls, dose_label);
  itoa(feeding_interval, buf, 10);
  u8g.drawStr(100, list_start, buf);
  itoa(feeding_dose, buf, 10);
  u8g.drawStr(100, list_start + fls, buf);
  
  return;
}

static screen_program feed_program = { feed_draw, NULL, therm_handler };

static void put_menu(struct menu *m) {
  menu = m;
  program = &menu_program;
  return;
}

static void put_input(struct input *i) {
  input = i;
  input_init();
  program = &input_program;
  return;
}

static void change_to(int screen) {
  switch (screen) {
    case PRINCIPAL:
      program = &principal_program;
      break;
    case MENU:
      return_link = PRINCIPAL;
      return_active = 0;
      put_menu(&main_menu);
      break;
    case CLOCK_MENU:
      return_link = MENU;
      return_active = 0;
      put_menu(&clock_menu);
      break;
    case CLOCK_SET:
      return_link = PRINCIPAL;
      return_active = 0;
      put_input(&clock_set);
      break;
    case THERM_MENU:
      return_link = MENU;
      return_active = 1;
      put_menu(&therm_menu);
      break;
    case THERM_CONSULT:
      return_link = THERM_MENU;
      return_active = 0;
      program = &therm_program;
      discover_therms();
      break;
    case SET_THERM:
      return_link = THERM_MENU;
      return_active = 0;
      put_input(&therm_set);
      break;
    case ERASE_MENU:
      return_link = MENU;
      return_active = 5;
      put_menu(&erase_menu);
      break;
    case ERASE_EEPROM:
      erase_eeprom();
      change_to(MENU);
      break;
    case LIGHT_MENU:
      return_link = MENU;
      return_active = 2;
      put_menu(&light_menu);
      break;
    case LIGHT_SAVE:
      return_link = LIGHT_MENU;
      return_link = 0;
      put_input(&set_light);
      break;
    case LIGHT_SEE:
      return_link = LIGHT_MENU;
      return_active = 0;
      program = &light_program;
      break;
    case TEMP_MENU:
      return_link = MENU;
      return_active = 3;
      put_menu(&temp_menu);
      break;
    case TEMP_SET:
      return_link = TEMP_MENU;
      return_active = 1;
      put_input(&set_temp);
      break;
    case TEMP_SEE:
      return_link = TEMP_MENU;
      return_active = 0;
      program = &temp_program;
      break;
    case FEED_NOW:
      feed_now();
      change_to(PRINCIPAL);
      break;
    case FEED_MENU:
      return_link = MENU;
      return_active = 4;
      put_menu(&feed_menu);
      break;
    case FEED_INTERVAL:
      return_link = FEED_MENU;
      return_active = 0;
      put_input(&set_feed_interval);
      break;
    case FEED_DOSE:
      return_link = FEED_MENU;
      return_active = 0;
      put_input(&set_feed_dose);
      break;
    case FEED_SEE:
      return_link = FEED_MENU;
      return_active = 0;
      program = &feed_program;
      break;
    default:
      break;
  }
  
  picture_loop();
}

void update_principal() {
  if (program == &principal_program) {
    picture_loop();
  }
}

