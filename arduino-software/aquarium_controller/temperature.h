#ifndef TEMPERATURE_H
#define TEMPERATURE_H

#include <OneWire.h>

#define WATER_SENSOR 0
#define AIR_SENSOR 1

#define SENSORS 2

#define ADDR_SIZE 8

extern int temperatures[SENSORS];
extern boolean temperatures_valid[SENSORS];
extern int discovered_temps[];
extern int n_discovered;
extern byte discovered_therms[][ADDR_SIZE];

#endif

