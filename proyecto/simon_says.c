#include <stdio.h>
#include <ev3c.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

#define MAX_ITERATIONS              15
#define MAX_ITERATIONS_MVMNT        100
#define SLEEP_DURATION              130000     // microseconds

#define COLOR_SENSOR_NOT_AVAILABLE  -1
#define COLOR_SENSOR_PORT            3

#define MAX_ITERATIONS_COLOR        5
#define SLEEP_DURATION_COLOR        1    // seconds
#define SCREEN_SLEEP				4	// seconds

#define MAX_SIZE_BYTES              50

int scoreboard = 0;
bool gameover = false;

typedef enum color_command_enum {
	COL_REFLECT = 0, COL_AMBIENT = 1, COL_COLOR = 2
} color_command;

int is_button_pressed(void);
int movement_checker();
int color_game();
void button_game();
int welcome();
void wrong_answer();
void right_answer();
void* reader(void *arg);

pthread_mutex_t mutex;

int main(){
	char buffer[MAX_SIZE_BYTES];
	int start_button = 0;

	pthread_t reportero;
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	//  ALERT!!! STOP  CLEARING THE LCD WHEN IT'S ALREADY BEEN CLEARED

	start_button = welcome(start_button, 4);

	if (start_button == 4) {

		ev3_init_lcd();
		ev3_text_lcd_normal(50,50,"Get Ready!!");
		sleep(SCREEN_SLEEP);
		ev3_clear_lcd();

		pthread_create(&reportero, &attr, reader, NULL);

//		color_game();
		button_game();
		gameover = true;
		printf("BACK TO MAIN\n");
		sleep(SLEEP_DURATION_COLOR);

		printf("about to join thread reportero\n");

		pthread_join(reportero, NULL);
		pthread_attr_destroy(&attr);
		printf("REPORTERO DESTOYED\n");

		sprintf(buffer, "Your final score is: %d\n", scoreboard);
		printf("buffer created\n");
		printf("%s", buffer);
		ev3_text_lcd_normal(00,50,buffer);
		sleep(SCREEN_SLEEP);

		ev3_clear_lcd();
		ev3_quit_lcd();

	} else {

		ev3_init_lcd();
		ev3_text_lcd_normal(20,50,"See you next time!");

		sleep(SCREEN_SLEEP);
		ev3_clear_lcd();
		ev3_quit_lcd();

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int welcome(int button_pressed){
	ev3_init_lcd();
	ev3_text_lcd_normal(15,50,"Welcome to:");
	ev3_text_lcd_normal(15,70,"The Robot Says Game!");

	sleep(SCREEN_SLEEP);
	ev3_clear_lcd();

	ev3_text_lcd_normal(25,50,"Press the center");
	ev3_text_lcd_normal(25,70,"button to start!");
	button_pressed = movement_checker(button_pressed);


	ev3_clear_lcd();
	ev3_quit_lcd();

	return button_pressed;
}

void button_game(){

	int button_pressed = -1;

	// Init devices
	ev3_init_button();
	ev3_init_lcd();

    // GAME LOGIC
    printf("Starting BUTTON game!\n");
    ev3_text_lcd_normal(25,50,"For the next game");
    ev3_text_lcd_normal(25,70,"use the buttons");
    ev3_text_lcd_normal(25,90,"to play! :^)");
    sleep(SCREEN_SLEEP);
    ev3_clear_lcd();

	ev3_text_lcd_normal(35,50,"Simon says");
	ev3_text_lcd_normal(35,70,"Jump Up!");

	button_pressed = movement_checker(button_pressed, 1);
	printf ("button_game() 2 position number will be saved %d\n", button_pressed);
	ev3_clear_lcd();

	if(button_pressed == 1){
		right_answer();
		button_pressed = -1;
	} else {
		wrong_answer();
		button_pressed = -1;
	}

	ev3_text_lcd_normal(35,50,"Simon says");
	ev3_text_lcd_normal(35,70,"To the right!");

	button_pressed = movement_checker(button_pressed, 2);
	printf ("button_game() 2 position number will be saved %d\n", button_pressed);

	ev3_clear_lcd();

	if(button_pressed == 2){
		right_answer();
		button_pressed = -1;
	} else {
		wrong_answer();
		button_pressed = -1;
	}

	ev3_text_lcd_normal(35,70,"To the left!");

	button_pressed = movement_checker(button_pressed, 0);
	printf ("button_game() 3 position number will be saved %d\n", button_pressed);

	ev3_clear_lcd();

	if(button_pressed == 0){
		wrong_answer();
		button_pressed = -1;
	} else {
		right_answer();
		button_pressed = -1;
	}

	ev3_text_lcd_normal(35,50,"Simon says");
	ev3_text_lcd_normal(35,70,"Get down!");

	button_pressed = movement_checker(button_pressed, 3);
	printf ("button_game() 4 position number will be saved %d\n", button_pressed);

	ev3_clear_lcd();

	if(button_pressed == 3){
		right_answer();
		button_pressed = -1;
	} else {
		wrong_answer();
		button_pressed = -1;
	}

	//  Finish & close devices
	ev3_quit_button();
	ev3_quit_lcd();
}

// This is a function similar to btn_monitor but it has been adapted specially for the button game implemented into the robot.
// It'll check the button pressed by the user but it'll stop checking once it has verified the desired button has been pressed
int movement_checker(int btn_passed){

	// Local variables
	int index, button_pressed;

	// Init devices
	ev3_init_button();

	printf ("btn_passed b4 checking in mvmnt checker %d\n", btn_passed);

	// Main loop
	for (index = 0; index < MAX_ITERATIONS_MVMNT; index++) {
		button_pressed = is_button_pressed();
		if (button_pressed >= 0) {
			btn_passed = button_pressed;
			printf ("btn_passed value in mvmnt checker %d\n", btn_passed);
			printf ("button_pressed value in mvmnt checker %d\n", button_pressed);
			break;
		}
		// pseudo-periodic activation
		usleep (SLEEP_DURATION);
	}

	//  Finish & close devices
	ev3_quit_button();

	return btn_passed;
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

int color_game() {

	ev3_sensor_ptr sensors       = NULL; //  List of available sensors
    ev3_sensor_ptr color_sensor  = NULL;
    ev3_init_lcd();

    int data;
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

    // GAME LOGIC
    printf("Starting color game!\n");
    ev3_text_lcd_normal(15,50,"Get your fruits");
    ev3_text_lcd_normal(15,70,"and vegetables for");
    ev3_text_lcd_normal(15,90,"the following game!");
    sleep(SCREEN_SLEEP);
    ev3_clear_lcd();

	ev3_text_lcd_normal(35,50,"Simon says");
	ev3_text_lcd_normal(35,70,"Get me a banana!");

	for(int i = 0; i < MAX_ITERATIONS_COLOR; i++){
		ev3_update_sensor_val (color_sensor);
		data = color_sensor->val_data[0].s32;
		printf ("The color detected is = %d\n", data);
		sleep(SLEEP_DURATION_COLOR);
	}
	ev3_clear_lcd();

	if(data == 4){
		right_answer();
	} else {
		wrong_answer();
	}

	ev3_text_lcd_normal(35,50,"Simon says");
	ev3_text_lcd_normal(35,70,"Get me an apple!");

	for(int i = 0; i < MAX_ITERATIONS_COLOR; i++){
		ev3_update_sensor_val (color_sensor);
		data = color_sensor->val_data[0].s32;
		printf ("The color detected is = %d\n", data);
		sleep(SLEEP_DURATION_COLOR);
	}
	ev3_clear_lcd();

	if(data == 5){
		right_answer();
	} else {
		wrong_answer();
	}

	ev3_text_lcd_normal(35,50,"Get me a");
	ev3_text_lcd_normal(35,70,"Green Veg!");

	for(int i = 0; i < MAX_ITERATIONS_COLOR; i++){
		ev3_update_sensor_val (color_sensor);
		data = color_sensor->val_data[0].s32;
		printf ("The color detected is = %d\n", data);
		sleep(SLEEP_DURATION_COLOR);
	}
	ev3_clear_lcd();

	if(data == 3){
		wrong_answer();
	} else {
		right_answer();
	}


    //  Finish & close devices
    printf ("\n*** Finishing color sensor application... OK***\n");

    ev3_clear_lcd();
    ev3_quit_lcd();
    ev3_delete_sensors (sensors);
}

void wrong_answer(){

	// Init LEDs and LCD
	ev3_init_led();

	ev3_text_lcd_normal(50,50,"Oops!");
	ev3_text_lcd_normal(50,70,"Pay attention");
	ev3_text_lcd_normal(50,90,"next time!");

	ev3_set_led(LEFT_LED,GREEN_LED,0);
	ev3_set_led(RIGHT_LED,GREEN_LED,0);
	ev3_set_led(LEFT_LED,RED_LED,255);
	ev3_set_led(RIGHT_LED,RED_LED,255);

	sleep(SCREEN_SLEEP);

	ev3_set_led(LEFT_LED,GREEN_LED,0);
	ev3_set_led(LEFT_LED,RED_LED,0);
	ev3_set_led(RIGHT_LED,GREEN_LED,0);
	ev3_set_led(RIGHT_LED,RED_LED,0);

	if (scoreboard != 0) {
		pthread_mutex_lock(&mutex);
		scoreboard--;
		pthread_mutex_unlock(&mutex);
	}

	ev3_clear_lcd();
	ev3_quit_led();
}

void right_answer(){

	// Init LEDs and LCD
	ev3_init_led();

	ev3_text_lcd_normal(50,50,"Good one!");
	ev3_text_lcd_normal(50,70,"onto the");
	ev3_text_lcd_normal(50,90,"next one!");

	ev3_set_led(LEFT_LED,GREEN_LED,255);
	ev3_set_led(RIGHT_LED,GREEN_LED,255);
	ev3_set_led(LEFT_LED,RED_LED,0);
	ev3_set_led(RIGHT_LED,RED_LED,0);

	sleep(SCREEN_SLEEP);

	ev3_set_led(LEFT_LED,GREEN_LED,0);
	ev3_set_led(LEFT_LED,RED_LED,0);
	ev3_set_led(RIGHT_LED,GREEN_LED,0);
	ev3_set_led(RIGHT_LED,RED_LED,0);

	pthread_mutex_lock(&mutex);
	scoreboard++;
	pthread_mutex_unlock(&mutex);

	ev3_clear_lcd();
	ev3_quit_led();
}

void* reader(void *arg) {
	//ev3_init_lcd();

	printf("Reporter thread is up and running\n");

	while (gameover != true){
		//ev3_clear_lcd();
		pthread_mutex_lock(&mutex);
		printf("Your scoreboard so far is = %d \n", scoreboard);
		pthread_mutex_unlock(&mutex);
		sleep(SLEEP_DURATION_COLOR);
	}

	printf("Game Over: %s\n", gameover ? "true" : "false");

	//ev3_clear_lcd();
	//ev3_quit_lcd();
}

