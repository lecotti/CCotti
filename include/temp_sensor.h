#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "tools.h"
#include "thread.h"
#include "http_types.h"
#include "sem.h"
#include "shared_memory.h"
#include "bmp280_types.h"


#define TEMP_SENSOR_DEVICE "/dev/temp-sensor"

class TempSensor {
protected:
    Thread th;

public:
    TempSensor();
    ~TempSensor();
    static void* read_sensor(void *args);
    static void sigusr1_handler(int signal);
    static bool quit;
};

#endif // TEMP_SENSOR_H