/*
 * File: practica2a.c
 *
 * Description: Muestra el uso de la botonera (por completar)
 *
 * Author:  DSSE
 * Version: 1.0
 *
*/

#include <stdio.h>
#include <unistd.h>
#include "ev3c.h"

#define MAX_SIZE_BYTES              24
#define MAX_ITERATIONS              100
#define SLEEP_DURATION              130000       // microseconds


int is_button_pressed (void);

int main (void) {

    // Local variables
    int index, button_pressed;
    static const char *BUTTON[] = { "NO_BUTTON", "BUTTON_LEFT", "BUTTON_UP",
                                    "BUTTON_RIGHT", "BUTTON_DOWN", "BUTTON_CENTER", "BUTTON_BACK" };
    char button_text [MAX_SIZE_BYTES]; // char array to store the output string

    // Init devices
    ev3_init_button();

    // Main loop
    for (index = 0; index < MAX_ITERATIONS; index++) {
        button_pressed = is_button_pressed();
        if (button_pressed >= 0) {
            sprintf (button_text, "Button: %s", BUTTON [button_pressed + 1]);
            printf ("%s\n", button_text);
        }
        // pseudo-periodic activation
        usleep (SLEEP_DURATION);
    }

    //  Finish & close devices
    printf ("\n*** Finishing button application... ");
    ev3_quit_button();

    printf ("OK***\n");
    return 0;
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
