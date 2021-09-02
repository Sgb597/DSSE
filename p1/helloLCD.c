#include <stdio.h>
#include <ev3c.h>

int main(){
	ev3_init_lcd();
	ev3_text_lcd_normal(0,50,"Hello, World!");
	ev3_quit_lcd();
}
