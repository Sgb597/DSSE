/*
 * File: practica2b.c
 *
 * Description: Muestra el uso del sensor de color
 *
 * Author:  DSSE
 * Version: 1.0
 *
*/

#include <stdio.h>
#include <ev3c.h>

#define COLOR_SENSOR_NOT_AVAILABLE  -1
#define COLOR_SENSOR_PORT            3

#define MAX_ITERATIONS				15
#define SLEEP_DURATION              1       // seconds

typedef enum color_command_enum
{
    COL_REFLECT = 0, COL_AMBIENT = 1, COL_COLOR = 2
} color_command;

int main (void) {

    ev3_sensor_ptr sensors       = NULL; //  List of available sensors
    ev3_sensor_ptr color_sensor  = NULL;

    int index, data;
    char *mode;

    // Loading all sensors
    sensors = ev3_load_sensors();
    if (sensors == NULL) {
        printf ("Error on ev3_load_sensors\n");
        return COLOR_SENSOR_NOT_AVAILABLE;
    }

    // Get color sensor by port
    color_sensor = ev3_search_sensor_by_port (sensors, COLOR_SENSOR_PORT);
    if (color_sensor == NULL) {
        printf ("Error on ev3_search_sensor_by_port\n");
        return COLOR_SENSOR_NOT_AVAILABLE;
    }

    // Init sensor
    color_sensor = ev3_open_sensor (color_sensor);
    if (color_sensor == NULL) {
        printf ("Error on ev3_open_sensor\n");
        return COLOR_SENSOR_NOT_AVAILABLE;
    }

    // Set mode
    ev3_mode_sensor (color_sensor, COL_COLOR);
    mode = color_sensor->modes[color_sensor->mode];
    printf ("Color mode enabled: %s\n", mode);

    // Init LEDs
    ev3_init_led();

    for (index = 0; index < MAX_ITERATIONS; index++) {
        ev3_update_sensor_val (color_sensor);
        data = color_sensor->val_data[0].s32;

        switch (data) {
			case 3:
				ev3_set_led(LEFT_LED,GREEN_LED,255);
				ev3_set_led(RIGHT_LED,GREEN_LED,255);
				ev3_set_led(LEFT_LED,RED_LED,0);
				ev3_set_led(RIGHT_LED,RED_LED,0);
				printf ("The color detected is = green\n");
				break;
			case 4:
				ev3_set_led(LEFT_LED,GREEN_LED,255);
				ev3_set_led(RIGHT_LED,GREEN_LED,255);
				ev3_set_led(LEFT_LED,RED_LED,255);
				ev3_set_led(RIGHT_LED,RED_LED,255);
				printf ("The color detected is = yellow\n");
				break;
			case 5:
				ev3_set_led(LEFT_LED,GREEN_LED,0);
				ev3_set_led(RIGHT_LED,GREEN_LED,0);
				ev3_set_led(LEFT_LED,RED_LED,255);
				ev3_set_led(RIGHT_LED,RED_LED,255);
				printf ("The color detected is = red\n");
				break;
			default:
				ev3_set_led(LEFT_LED,GREEN_LED,0);
				ev3_set_led(LEFT_LED,RED_LED,0);
				ev3_set_led(RIGHT_LED,GREEN_LED,0);
				ev3_set_led(RIGHT_LED,RED_LED,0);
		}

        printf ("The color detected is = %d\n", data);
        sleep (SLEEP_DURATION);
    }

    //  Finish & close devices
    printf ("\n*** Finishing color sensor application... OK***\n");
    ev3_delete_sensors (sensors);
    ev3_quit_led();

    return 0;
}
