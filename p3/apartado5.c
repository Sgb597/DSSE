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
#include <time.h>
#include "ev3c.h"

#define BELT_MOTOR_PORT             'A'     // Motor ports are named using characters
#define FULL_SPEED_LARGE_MOTOR      900     // units: deg/seg
#define FULL_SPEED_MEDIUM_MOTOR     1200    // units: deg/seg
#define SUSPENSION_TIME             500   // units: usecs
#define RUN_TIME 					15000	// units: ms
#define SLEEP 						1		// units: seconds

typedef enum stop_mode_enum {COAST, BRAKE, HOLD} stop_mode;
static char *STOP_MODE_STRING[] = {"coast", "brake", "hold"};

typedef enum commands_enum {RUN_ABS_POS, RUN_REL_POS, RUN_TIMED, STOP} commands;
static char *COMMANDS_STRING[] = {"run-to-abs-pos", "run-to-rel-pos", "run-timed", "stop"};

int main (void) {

    //  Regular variables
    int index            = 0;    // Loop variable
    ev3_motor_ptr motors = NULL; // List of motors
    double exec_time = RUN_TIME + (SUSPENSION_TIME/1000); // time to execute motors accounting for sleeps
    double v_avg = 0; // Variable where velicities will be summed to and divided by the number of samples

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
    ev3_set_speed_sp(belt_motor, 169.108492);
    ev3_set_position (belt_motor, 0);

    printf ("Start position %d (degrees)\n", ev3_get_position (belt_motor));
    printf("The motor will execute for: %f ms\n", exec_time);


	ev3_set_time_sp (belt_motor, exec_time); // Running time (milliseconds)
	ev3_command_motor_by_name (belt_motor, COMMANDS_STRING[RUN_TIMED]); // Action!


	usleep (SUSPENSION_TIME);

	while ((ev3_motor_state (belt_motor) & MOTOR_RUNNING) && !(ev3_motor_state (belt_motor) & MOTOR_HOLDING)){ // Wait until motor action is complete
		v_avg += (double)ev3_get_speed(belt_motor);
		index++;
	}

	printf ("Current position %d (degrees)\n", ev3_get_position (belt_motor));

    v_avg = v_avg/index;
    printf("Average velocity for motor is: %f determined with %d samples\n", v_avg, index);

    //Let's reset and close the motors
    ev3_set_position_sp (belt_motor, 0);
    ev3_command_motor_by_name (belt_motor, COMMANDS_STRING[RUN_ABS_POS]);
    usleep (SUSPENSION_TIME);
    while ((ev3_motor_state (belt_motor) & MOTOR_RUNNING) && !(ev3_motor_state (belt_motor) & MOTOR_HOLDING));

    sleep(1); // Este sleep permite leer la verdadera posición final.

    printf ("Final position %d (degrees)\n", ev3_get_position (belt_motor));
    ev3_reset_motor (belt_motor);
    ev3_delete_motors (motors);
    return EXIT_SUCCESS;
}
