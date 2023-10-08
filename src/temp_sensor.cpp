#include "temp_sensor.h"

bool TempSensor::quit = false;

TempSensor::TempSensor() : th(TempSensor::read_sensor) {
}

void TempSensor::sigusr1_handler(int signal) {
    TempSensor::quit = true;
}

void* TempSensor::read_sensor(void *args) {
    int fd;
    Sem sem(SEM_PATH, SEM_ID, false);
    SharedMemory<serverData> shm(SHM_PATH, SHM_ID);
    int current_sample = 0;
    char str_temp[20];
    float *temps = NULL;
    float temp = 0;
    int samples = 0, period = 0, prev_samples = 0;

    Signal::set_handler(SIGUSR1, TempSensor::sigusr1_handler);

    if ((fd = open(TEMP_SENSOR_DEVICE, O_RDWR)) == -1) {
        perror(ERROR("Couldn't open temperature sensor device"));
        return NULL;
    }

    while(!TempSensor::quit) {
        samples = shm[0].samples_moving_average_filter;
        period = shm[0].sensor_period;

        if (samples == 0) {
            goto next;
        }

        if (prev_samples != samples) {
            temps = (float*) reallocarray(temps, samples, sizeof(float));
            if (prev_samples == 0) {
                for (int i = 0; i < samples; i++) {
                    temps[i] = 0;
                }
            }
            prev_samples = samples;
        }

        read(fd, str_temp, sizeof(str_temp));
        temps[current_sample] = atof(str_temp);
        temp = 0;
        for (int i = 0; i < samples; i++) {
            temp += temps[i];
        }
        temp /= samples;
        sem--;
        shm[0].temp = temp;
        sem++;

        next:
        current_sample++;
        if (current_sample >= samples) {
            current_sample = 0;
        }
        Signal::set_timer_single_shot(period * 1000);
        Signal::wait_and_ignore(SIGALRM);
    }
    return NULL;
}

TempSensor::~TempSensor(void) {
    this->th.send_signal(SIGUSR1);
    this->th.join();
}
