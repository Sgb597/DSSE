#include "ev3c.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#define SLEEP_DURATION           1       // seconds
#define MAX_ITERATIONS           15
#define MAX_SIZE_BYTES              24
#define COLOR_SENSOR_NOT_AVAILABLE  -1
#define COLOR_SENSOR_PORT            3

// Enum used in color detection
typedef enum color_command_enum
{
    COL_REFLECT = 0, COL_AMBIENT = 1, COL_COLOR = 2
} color_command;


// Functions needed to verify the button press
int is_button_pressed (void);
//void ev3_init_button();
//int32_t ev3_button_pressed(enum ev3_button_identifier button);
//void ev3_quit_button();
void* sensor_monitor(void* arg);
void* btn_monitor(void* arg);

int main(){

	pthread_t sensor_thread, button_thread;

	pthread_create(&sensor_thread, NULL, sensor_monitor, NULL);
	pthread_create(&button_thread, NULL, btn_monitor, NULL);

	pthread_join(sensor_thread, NULL);
	pthread_join(button_thread, NULL);
	printf("Closing the app from the main thread\n");

	return 0;
}

// Function that encapsulates logic needed to identify colors w/ sensor
void* sensor_monitor(void* arg) {

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

    printf("Color monitor is up and running\n");

    for (index = 0; index < MAX_ITERATIONS; index++) {
        ev3_update_sensor_val (color_sensor);
        data = color_sensor->val_data[0].s32;

        switch (data) {
			case 3:
				ev3_set_led(LEFT_LED,GREEN_LED,255);
				ev3_set_led(RIGHT_LED,GREEN_LED,255);
				ev3_set_led(LEFT_LED,RED_LED,0);
				ev3_set_led(RIGHT_LED,RED_LED,0);
				break;
			case 4:
				ev3_set_led(LEFT_LED,GREEN_LED,255);
				ev3_set_led(RIGHT_LED,GREEN_LED,255);
				ev3_set_led(LEFT_LED,RED_LED,255);
				ev3_set_led(RIGHT_LED,RED_LED,255);
				break;
			case 5:
				ev3_set_led(LEFT_LED,GREEN_LED,0);
				ev3_set_led(RIGHT_LED,GREEN_LED,0);
				ev3_set_led(LEFT_LED,RED_LED,255);
				ev3_set_led(RIGHT_LED,RED_LED,255);
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

    pthread_exit(NULL);
}


// Function that encapsulates the logic to detect a button press and print what button was pressed

void* btn_monitor(void* arg) {

	// Local variables
	int index, button_pressed;
	static const char *BUTTON[] = { "NO_BUTTON", "BUTTON_LEFT", "BUTTON_UP",
									"BUTTON_RIGHT", "BUTTON_DOWN", "BUTTON_CENTER", "BUTTON_BACK" };
	char button_text [MAX_SIZE_BYTES]; // char array to store the output string

	// Init devices
	ev3_init_button();

	printf("Button monitor is up and running\n");

	// Main loop
	for (index = 0; index < MAX_ITERATIONS; index++) {
		button_pressed = is_button_pressed();
		if (button_pressed >= 0) {
			sprintf (button_text, "Button: %s", BUTTON [button_pressed + 1]);
			printf ("%s\n", button_text);
		}

		// pseudo-periodic activation
		sleep (SLEEP_DURATION);
	}

	//  Finish & close devices
	printf ("\n*** Finishing button application... ");
	ev3_quit_button();


	pthread_exit(NULL);
}

int is_button_pressed (void) {
    int button_pressed = -1;

    for (int i = BUTTON_LEFT; i<= BUTTON_BACK; i++){
    	if(ev3_button_pressed(i) == 1){
    		button_pressed = i;
    	}
    }
    return button_pressed;
}
