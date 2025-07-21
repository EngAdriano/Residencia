#ifndef GPS_H
#define GPS_H

#include <stdbool.h>

typedef struct {
    float latitude;
    float longitude;
} GPSData;

bool parse_gpgga(const char *nmea, GPSData *data);

#endif
