#include "gps.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static float convert_to_decimal(const char *nmea_coord, const char *dir) {
    float degrees = 0.0f;
    float minutes = 0.0f;

    if (strlen(nmea_coord) < 6)
        return 0.0f;

    char deg[3] = {0};
    strncpy(deg, nmea_coord, (dir[0] == 'N' || dir[0] == 'S') ? 2 : 3);
    degrees = atof(deg);

    minutes = atof(nmea_coord + ((dir[0] == 'N' || dir[0] == 'S') ? 2 : 3));

    float decimal = degrees + minutes / 60.0f;
    if (dir[0] == 'S' || dir[0] == 'W') decimal *= -1.0f;

    return decimal;
}

bool parse_gpgga(const char *nmea, GPSData *data) {
    if (strncmp(nmea, "$GPGGA", 6) != 0)
        return false;

    char *tokens[15] = {0};
    char buf[128];
    strncpy(buf, nmea, sizeof(buf));
    buf[sizeof(buf)-1] = '\0';

    char *token = strtok(buf, ",");
    int index = 0;

    while (token && index < 15) {
        tokens[index++] = token;
        token = strtok(NULL, ",");
    }

    if (index < 6 || tokens[2] == NULL || tokens[4] == NULL)
        return false;

    data->latitude = convert_to_decimal(tokens[2], tokens[3]);
    data->longitude = convert_to_decimal(tokens[4], tokens[5]);

    return true;
}
