#include <DS1307RTC.h>
#include <Time.h>
#include <Timezone.h>

TimeChangeRule CEST = {"CEST", Fourth, Sun, Mar, 2, +120};
TimeChangeRule CET = {"CET", Fourth, Sun, Oct, 3, +60};
Timezone tz(CEST, CET);

boolean boot_clock() { 
  setSyncProvider(RTC.get);
  
  return timeStatus() == timeSet;
}
