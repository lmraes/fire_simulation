#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <locale.h>
#include "functions.h"

char forest_grid[SIZE][SIZE];
pthread_mutex_t grid_mutex[SIZE][SIZE];
pthread_cond_t fire_condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t fire_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t central_lock = PTHREAD_MUTEX_INITIALIZER;

int main() {
    setlocale(LC_ALL, "Portuguese");
    srand(time(NULL));

    for (int row = 0; row < SIZE; row++) {
        for (int col = 0; col < SIZE; col++) {
            forest_grid[row][col] = (rand() % 3 == 0) ? SENSOR : EMPTY;
            pthread_mutex_init(&grid_mutex[row][col], NULL);
        }
    }

    display_forest();

    pthread_t sensor_threads[SIZE][SIZE];
    for (int row = 0; row < SIZE; row++) {
        for (int col = 0; col < SIZE; col++) {
            if (forest_grid[row][col] == SENSOR) {
                SensorNode* new_sensor = (SensorNode*)malloc(sizeof(SensorNode));
                new_sensor->x = row;
                new_sensor->y = col;
                pthread_create(&sensor_threads[row][col], NULL, sensor_thread, (void*)new_sensor);
            }
        }
    }

    pthread_t fire_thread;
    pthread_create(&fire_thread, NULL, fire_starter, NULL);

    pthread_t control_thread;
    pthread_create(&control_thread, NULL, control_center_thread, NULL);

    pthread_join(fire_thread, NULL);
    pthread_join(control_thread, NULL);

    for (int row = 0; row < SIZE; row++) {
        for (int col = 0; col < SIZE; col++) {
            if (forest_grid[row][col] == SENSOR) {
                pthread_join(sensor_threads[row][col], NULL);
            }
        }
    }

    return 0;
}
