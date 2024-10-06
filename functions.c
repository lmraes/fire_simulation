#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include "functions.h"

extern char forest_grid[SIZE][SIZE];
extern pthread_mutex_t grid_mutex[SIZE][SIZE];
extern pthread_cond_t fire_condition;
extern pthread_mutex_t fire_mutex;
extern pthread_mutex_t central_lock;

void display_forest() {
    pthread_mutex_lock(&central_lock);
    printf("Floresta:\n");
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            char cell_state = forest_grid[i][j];
            printf("%c ", cell_state);
        }
        printf("\n");
    }
    printf("\n");
    pthread_mutex_unlock(&central_lock);
}


_Bool sensor_identified_fire[SIZE][SIZE];

void* sensor_thread(void* arg) {
    SensorNode* sensor = (SensorNode*)arg;

    while (1) {
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                int neighbor_x = sensor->x + i;
                int neighbor_y = sensor->y + j;

                if (neighbor_x >= 0 && neighbor_x < SIZE && neighbor_y >= 0 && neighbor_y < SIZE) {
                    pthread_mutex_lock(&grid_mutex[neighbor_x][neighbor_y]);
                    if (forest_grid[neighbor_x][neighbor_y] == FIRE && !sensor_identified_fire[sensor->x][sensor->y]) {
                        // Check if this sensor is the closest sensor to the fire
                        int closest_sensor_x = sensor->x;
                        int closest_sensor_y = sensor->y;
                        int min_distance = INT_MAX;

                        for (int k = 0; k < SIZE; k++) {
                            for (int l = 0; l < SIZE; l++) {
                                if (forest_grid[k][l] == SENSOR) {
                                    int distance = abs(k - neighbor_x) + abs(l - neighbor_y);
                                    if (distance < min_distance) {
                                        min_distance = distance;
                                        closest_sensor_x = k;
                                        closest_sensor_y = l;
                                    }
                                }
                            }
                        }

                        if (closest_sensor_x == sensor->x && closest_sensor_y == sensor->y) {
                            printf("O sensor [%d, %d] identificou incêndio na posição [%d, %d]!\n", sensor->x, sensor->y, neighbor_x, neighbor_y);

                            sensor_identified_fire[sensor->x][sensor->y] = 1;

                            // Se o sensor for de borda, acione a central diretamente
                            if (is_border_sensor(sensor->x, sensor->y)) {
                                pthread_mutex_lock(&fire_mutex);
                                pthread_cond_signal(&fire_condition);
                                pthread_mutex_unlock(&fire_mutex);
                            } else {
                                // Alertar os vizinhos do sensor
                                communicate_with_neighbors(sensor->x, sensor->y);
                            }
                        }
                    }
                    pthread_mutex_unlock(&grid_mutex[neighbor_x][neighbor_y]);
                }
            }
        }
        sleep(1);
    }

    return NULL;
}

void communicate_with_neighbors(int x, int y) {
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int neighbor_x = x + i;
            int neighbor_y = y + j;

            if (neighbor_x >= 0 && neighbor_x < SIZE && neighbor_y >= 0 && neighbor_y < SIZE) {
                pthread_mutex_lock(&grid_mutex[neighbor_x][neighbor_y]);
                if (forest_grid[neighbor_x][neighbor_y] == SENSOR && (neighbor_x != x || neighbor_y != y)) {
                    printf("O sensor [%d, %d] notificou sensor [%d, %d] sobre o incêndio!\n", x, y, neighbor_x, neighbor_y);

                    if (is_border_sensor(neighbor_x, neighbor_y)) {
                        // Acionar a central de controle
                        pthread_mutex_lock(&fire_mutex);
                        pthread_cond_signal(&fire_condition);
                        pthread_mutex_unlock(&fire_mutex);
                    } else {
                        // Alertar os vizinhos do sensor vizinho
                        communicate_with_neighbors(neighbor_x, neighbor_y);
                    }
                }
                pthread_mutex_unlock(&grid_mutex[neighbor_x][neighbor_y]);
            }
        }
    }
}

void* fire_starter(void* arg) {
    while (1) {
        int x = rand() % SIZE;
        int y = rand() % SIZE;

        pthread_mutex_lock(&grid_mutex[x][y]);
        if (forest_grid[x][y] == EMPTY) {
            forest_grid[x][y] = FIRE;
            printf("Incêndio iniciado na posição [%d, %d]!\n", x, y);
            display_forest();
        }
        pthread_mutex_unlock(&grid_mutex[x][y]);

        pthread_mutex_lock(&central_lock);
        pthread_mutex_unlock(&central_lock);

        sleep(3);
    }

    return NULL;
}

void* control_center_thread(void* arg) {
    while (1) {
        pthread_mutex_lock(&fire_mutex);
        pthread_cond_wait(&fire_condition, &fire_mutex);
        pthread_mutex_unlock(&fire_mutex);

        printf("A central de controle foi notificada sobre o incêndio!\n");

        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                pthread_mutex_lock(&grid_mutex[i][j]);
                if (forest_grid[i][j] == FIRE) {
                    printf("Central de controle está apagando o incêndio na posição [%d, %d]!\n", i, j);
                    forest_grid[i][j] = BURNED;
                }
                pthread_mutex_unlock(&grid_mutex[i][j]);
            }
        }

        pthread_mutex_lock(&central_lock);
        display_forest();
        pthread_mutex_unlock(&central_lock);
    }

    return NULL;
}

int is_border_sensor(int x, int y) {
    return (x == 0 || x == SIZE - 1 || y == 0 || y == SIZE - 1);
}


