//
// Created by nekok on 24-8-11.
//

#ifndef MAIN_H
#define MAIN_H

#define MAX_LENGTH 200
#define SPWM_POINT_NUM 128
#define PI 3.14159265
#define THREAD_COUNT 3
#define RAINBOW_STEP 360
#define MAX_BRIGHTNESS 255
#define ENABLE "1"
#define DISABLE "0"
#define WRITE_READ "w+"
#define WRITE_ONLY "w"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
int thread_ready = 0;
int write_ready = 0;

int exit_sig=0;

int ishelp=0;
int ispoweroff=0;
int red_max_brightness = 48;
int green_max_brightness = 48;
int blue_max_brightness = 255;
char* period="51000";
char* color="FFFFFF";
char* animation="default";
char* pwm_path="/sys/class/pwm";
char* leds_brightness="100";
char* pwm_name="pwmchip0";
char* red_pwm_name="pwm1";
char* green_pwm_name="pwm2";
char* blue_pwm_name="pwm3";
char pwm_chip_export[MAX_LENGTH] = {0};
char pwm_chip_unexport[MAX_LENGTH] = {0};
char red_pwm[MAX_LENGTH] = {0};
char green_pwm[MAX_LENGTH] = {0};
char blue_pwm[MAX_LENGTH] = {0};
char red_pwm_enable[MAX_LENGTH] = {0};
char green_pwm_enable[MAX_LENGTH] = {0};
char blue_pwm_enable[MAX_LENGTH] = {0};
char red_pwm_period[MAX_LENGTH] = {0};
char green_pwm_period[MAX_LENGTH] = {0};
char blue_pwm_period[MAX_LENGTH] = {0};
char red_pwm_duty_cycle[MAX_LENGTH] = {0};
char green_pwm_duty_cycle[MAX_LENGTH] = {0};
char blue_pwm_duty_cycle[MAX_LENGTH] = {0};

typedef struct breathing_pwm_led_args {
    char pwm_path[MAX_LENGTH];
    int spwm_wave[MAX_LENGTH];
}breathing;

typedef struct {
    int rainbow_red;
    int rainbow_green;
    int rainbow_blue;
} rainbow_color;

#endif //MAIN_H
