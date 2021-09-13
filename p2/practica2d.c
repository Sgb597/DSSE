#include "ev3c.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>


// These constants are used for the detection of a button press
#define MAX_SIZE_BYTES              24
#define MAX_ITERATIONS              115
#define SLEEP_DURATION              130000     // microseconds

#define COLOR_SENSOR_NOT_AVAILABLE  -1
#define COLOR_SENSOR_PORT            3
#define MAX_ITERATIONS_COLOR             15
#define SLEEP_DURATION_COLOR             1       // seconds

typedef enum color_command_enum {
	COL_REFLECT = 0, COL_AMBIENT = 1, COL_COLOR = 2
} color_command;

struct shared_data {
	pthread_mutex_t mutex;
	char *button_pressed;
	char *color_sensor;
	char *op_mode;
};

int is_button_pressed(void);
void* sensor_monitor(void *arg);
void* btn_monitor(void *arg);
void* reader(void *arg);
struct shared_data *results;

int main(void) {
	pthread_mutexattr_t mutex_attr;
	pthread_t color_thread, button_thread, reportero;
	pthread_attr_t attr;
	results = (struct shared_data*)malloc(sizeof(struct shared_data));
	results->button_pressed = "";
	results->color_sensor = "";
	results->op_mode = "";

	pthread_mutexattr_init(&mutex_attr);
	pthread_attr_init(&attr);

	pthread_create(&color_thread, &attr, sensor_monitor, &results);
	pthread_create(&button_thread, &attr, btn_monitor, &results);
	pthread_create(&reportero, &attr, reader, &results);

	pthread_join(color_thread, NULL);
	pthread_join(button_thread, NULL);
	pthread_join(reportero, NULL);

	pthread_attr_destroy(&attr);
	ev3_clear_lcd();
	ev3_quit_lcd();
	printf("The app has succesfully executed all the way through\n");

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
	pthread_mutex_lock(&results->mutex);
	results->op_mode = mode;
	pthread_mutex_unlock(&results->mutex);

	// Init LEDs
	ev3_init_led();

	printf("Color monitor is up and running\n");

	for (index = 0; index < MAX_ITERATIONS_COLOR; index++) {
		ev3_update_sensor_val (color_sensor);
		data = color_sensor->val_data[0].s32;

		switch (data) {
		case 3:
			ev3_set_led(LEFT_LED,GREEN_LED,255);
			ev3_set_led(RIGHT_LED,GREEN_LED,255);
			ev3_set_led(LEFT_LED,RED_LED,0);
			ev3_set_led(RIGHT_LED,RED_LED,0);
			pthread_mutex_lock(&results->mutex);
			results->color_sensor = "Green!";
			pthread_mutex_unlock(&results->mutex);
			break;
		case 4:
			ev3_set_led(LEFT_LED,GREEN_LED,255);
			ev3_set_led(RIGHT_LED,GREEN_LED,255);
			ev3_set_led(LEFT_LED,RED_LED,255);
			ev3_set_led(RIGHT_LED,RED_LED,255);
			pthread_mutex_lock(&results->mutex);
			results->color_sensor = "Yellow!";
			pthread_mutex_unlock(&results->mutex);
			break;
		case 5:
			ev3_set_led(LEFT_LED,GREEN_LED,0);
			ev3_set_led(RIGHT_LED,GREEN_LED,0);
			ev3_set_led(LEFT_LED,RED_LED,255);
			ev3_set_led(RIGHT_LED,RED_LED,255);
			pthread_mutex_lock(&results->mutex);
			results->color_sensor = "Red!";
			pthread_mutex_unlock(&results->mutex);
			break;
		default:
			ev3_set_led(LEFT_LED,GREEN_LED,0);
			ev3_set_led(LEFT_LED,RED_LED,0);
			ev3_set_led(RIGHT_LED,GREEN_LED,0);
			ev3_set_led(RIGHT_LED,RED_LED,0);
			pthread_mutex_lock(&results->mutex);
			results->color_sensor = "No color!";
			pthread_mutex_unlock(&results->mutex);
		}

		printf ("The color detected is = %d\n", data);
		sleep (SLEEP_DURATION_COLOR);
	}

	//  Finish & close devices
	ev3_set_led(LEFT_LED,GREEN_LED,0);
	ev3_set_led(LEFT_LED,RED_LED,0);
	ev3_set_led(RIGHT_LED,GREEN_LED,0);
	ev3_set_led(RIGHT_LED,RED_LED,0);
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
			pthread_mutex_lock(&results->mutex);
			results->button_pressed = "Button press!";
			pthread_mutex_unlock(&results->mutex);
			sprintf (button_text, "Button: %s", BUTTON [button_pressed + 1]);
			printf ("%s\n", button_text);
		} else {
			pthread_mutex_lock(&results->mutex);
			results->button_pressed = "No button press!";
			pthread_mutex_unlock(&results->mutex);
		}

		// pseudo-periodic activation
		usleep (SLEEP_DURATION);
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

void* reader(void *arg) {
	ev3_init_lcd();
	int index;

	printf("Reporter thread is up and running\n");

	for (index = 0; index < MAX_ITERATIONS_COLOR; index++) {
		ev3_clear_lcd();
		pthread_mutex_lock(&results->mutex);
		ev3_text_lcd_normal(0, 30, results->button_pressed);
		ev3_text_lcd_normal(0, 60, results->color_sensor);
		ev3_text_lcd_normal(0, 100, results->op_mode);
		pthread_mutex_unlock(&results->mutex);
		sleep(SLEEP_DURATION_COLOR);
	}
	ev3_clear_lcd();
	ev3_quit_lcd();
}
