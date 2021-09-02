/*
 * File: practica3.c
 *
 * Description: Muestra el uso de los motores
 *
 * Author:  DSSE
 * Version: 1.0
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include "ev3c.h"

#define BELT_MOTOR_PORT             'A'     // Motor ports are named using characters
#define FULL_SPEED_LARGE_MOTOR      900     // units: deg/seg
#define FULL_SPEED_MEDIUM_MOTOR     1200    // units: deg/seg
#define SUSPENSION_TIME             2000    // units: usecs

typedef enum stop_mode_enum {COAST, BRAKE, HOLD} stop_mode;
static char *STOP_MODE_STRING[] = {"coast", "brake", "hold"};

typedef enum commands_enum {RUN_ABS_POS, RUN_REL_POS, RUN_TIMED, STOP} commands;
static char *COMMANDS_STRING[] = {"run-to-abs-pos", "run-to-rel-pos", "run-timed", "stop"};

int main (void) {
    //  Regular variables
    int index            = 1;    // Loop variable
    ev3_motor_ptr motors = NULL; // List of motors

    printf ("*** Practica 3: Motors on EV3DEV ***\n");

    motors  = ev3_load_motors(); // Loading all motors
    if (motors == NULL) {
        printf ("Error on ev3_load_motors\n");
        return EXIT_FAILURE;
    }

    // Get the target motor
    ev3_motor_ptr belt_motor = ev3_search_motor_by_port (motors, BELT_MOTOR_PORT);

    // Init motor
    ev3_reset_motor (belt_motor);
    ev3_open_motor (belt_motor);

    // Motor configuration
    ev3_stop_command_motor_by_name (belt_motor, STOP_MODE_STRING[COAST]);
    ev3_set_duty_cycle_sp (belt_motor, 25);  // Power to the engine
    ev3_set_position (belt_motor, 0);

    printf ("Start position %d (degrees)\n", ev3_get_position (belt_motor));

    while (index < 4) { // Main loop
        ev3_set_time_sp (belt_motor, index * 300); // Running time (milliseconds)
        ev3_command_motor_by_name (belt_motor, COMMANDS_STRING[RUN_TIMED]); // Action!
        index++;
        usleep (SUSPENSION_TIME);
        while ((ev3_motor_state (belt_motor) & MOTOR_RUNNING)); // Wait until motor action is complete
        printf ("Current position %d (degrees)\n", ev3_get_position (belt_motor));
        sleep (1); // Wait until next period (pseudo-periodic)
    }

    //Let's reset and close the motors
    ev3_set_position_sp (belt_motor, 0);
    ev3_command_motor_by_name (belt_motor, COMMANDS_STRING[RUN_ABS_POS]);
    usleep (SUSPENSION_TIME);
    while ((ev3_motor_state (belt_motor) & MOTOR_RUNNING));

    printf ("Final position %d (degrees)\n", ev3_get_position (belt_motor));
    ev3_reset_motor (belt_motor);
    ev3_delete_motors (motors);
    return EXIT_SUCCESS;
}
