#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#define SIZE 30

#define EMPTY '-'
#define SENSOR 'T'
#define FIRE '@'
#define BURNED '/'

typedef struct {
    int x;
    int y;
} SensorNode;

void display_forest();
void* sensor_thread(void* arg);
void* fire_starter(void* arg);
void* control_center_thread(void* arg);
void communicate_with_neighbors(int x, int y);
void extinguish_fire(int x, int y);

#endif
