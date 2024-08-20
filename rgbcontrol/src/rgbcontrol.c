#pragma GCC optimize(3,"Ofast","inline")

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include "rgbcontrol.h"

static size_t write_file(const char* path , const char* buf , const size_t len,const char* mode) {
    FILE* fp = NULL;
    fp = fopen(path ,mode);
    if (fp == NULL) {
        return 1;
    }
    const size_t size = fwrite(buf, len, 1, fp);
    fclose(fp);
    return size;
}

static void calculate_duty_cycles(int color , const int period , char* duty_cycles, const int max_brightness) {
    int duty;
    const float proportion = max_brightness / 255.0;
    if(color > max_brightness) {
        color = max_brightness;
        duty = (int)trunc(color * proportion) * period / max_brightness;
    }else {
        duty = color * period / max_brightness;
    }
    snprintf(duty_cycles, 10, "%d", duty);
}

static void sig_handler(const int sig) {
    if (sig == SIGINT) {
        exit_sig=1;
        printf("\nExit now!\n");
    }
}

static void calculate_spwm_duty_cycles(const char* duty_cycles,int spwm_wave[]) {
    const int max_duty_cycle = atoi(duty_cycles);
    const float step = (2 * PI) / SPWM_POINT_NUM;
    const float exponent = max_duty_cycle < 20000 ? 1.0 : 1.8;
    for (int i = 1; i <= SPWM_POINT_NUM; i++) {
        const float sine_value = (sin(i * step) + 1) / 2.0;
        spwm_wave[i] = round(pow(sine_value, exponent) * max_duty_cycle);
    }
}

static int max_brightness_index(int spwm_wave[]) {
    int max_bringhtness_index = 0;
    for (int i=1;i<=SPWM_POINT_NUM;i++) {
        if(spwm_wave[i-1] < spwm_wave[i] && spwm_wave[i] > spwm_wave[i+1]) {
            max_bringhtness_index = i;
        }
    }
    return max_bringhtness_index;
}

static void* breathing_pwm_led(void* arg) {
    const breathing* para = (breathing*)arg;
    int spwm[MAX_LENGTH]={0};
    memcpy(spwm,para -> spwm_wave,sizeof(para -> spwm_wave));
    const int max_bringhtness_index = max_brightness_index(spwm);
    char path[MAX_LENGTH]={0};
    strcpy(path,para -> pwm_path);
    const useconds_t wait_time = 10000000 / SPWM_POINT_NUM;
    
    pthread_mutex_lock(&mutex);
    thread_ready++;
    if (thread_ready < THREAD_COUNT) {
        printf("Waiting all thread ready.\n");
        pthread_cond_wait(&condition, &mutex);
    } else {
        printf("All thread start ready!\n");
        pthread_cond_broadcast(&condition);
    }
    pthread_mutex_unlock(&mutex);
    
    usleep(10000);
    
    printf("Now breathing!\n");
    while (1) {
        for(int i=1;i<=SPWM_POINT_NUM;i++) {
            char pwm_duty_cycle[10]={0};
            // itoa(spwm[i], pwm_duty_cycle, 10);
            snprintf(pwm_duty_cycle, sizeof(pwm_duty_cycle), "%d", spwm[i]);
            pthread_mutex_lock(&mutex);
            write_file(path, pwm_duty_cycle, strlen(pwm_duty_cycle), WRITE_READ);
            write_ready++;
            if (write_ready < THREAD_COUNT) {
                pthread_cond_wait(&condition, &mutex);
            } else {
                write_ready=0;
                pthread_cond_broadcast(&condition);
            }
            pthread_mutex_unlock(&mutex);
            if(exit_sig) break;
            if(i==max_bringhtness_index) {
                usleep(3000000);
            }
            usleep(wait_time);
        }
        if(exit_sig) break;
    }
    pthread_exit(NULL);
    return NULL;
}

static rainbow_color hsl_to_rgb(const int hue) {
    const float chroma = (1 - fabs(2 * 0.5 - 1)) * 1.0;
    const float h_prime = hue / 60.0;
    const float x = chroma * (1 - fabs(fmod(h_prime, 2) - 1));
    const float m = 0.5 - chroma / 2.0;
    
    float red, green, blue;
    if (h_prime >= 0 && h_prime < 1) {
        red = chroma, green = x, blue = 0;
    } else if (h_prime >= 1 && h_prime < 2) {
        red = x, green = chroma, blue = 0;
    } else if (h_prime >= 2 && h_prime < 3) {
        red = 0, green = chroma, blue = x;
    } else if (h_prime >= 3 && h_prime < 4) {
        red = 0, green = x, blue = chroma;
    } else if (h_prime >= 4 && h_prime < 5) {
        red = x, green = 0, blue = chroma;
    } else {
        red = chroma, green = 0, blue = x;
    }
    
    rainbow_color color;
    color.rainbow_red = (red + m) * 255;
    color.rainbow_green = (green + m) * 255;
    color.rainbow_blue = (blue + m) * 255;
    
    return color;
}

int main(const int argc, char* argv[]) {
    signal(SIGINT, sig_handler);
    if(argc==1) {
        printf("Invalid arguments\n");
        printf("Try '%s -h' or '%s --help' for more information.\n", argv[0],argv[0]);
        return 1;
    }
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i],"--help") == 0) {
            ishelp=1;
            i++;
        } else if (strcmp(argv[i],"-P") == 0 || strcmp(argv[i],"--poweroff") == 0) {
            ispoweroff=1;
            i++;
        } else if ((strcmp(argv[i],"-p") == 0 || strcmp(argv[i],"--period") == 0) && i + 1 < argc) {
            period = argv[i + 1];
            i++;
        } else if ((strcmp(argv[i],"-c") == 0 || strcmp(argv[i],"--color") == 0) && i + 1 < argc) {
            color = argv[i + 1];
            i++;
        } else if ((strcmp(argv[i],"-a") == 0 || strcmp(argv[i],"--animation") == 0) && i + 1 < argc) {
            animation = argv[i + 1];
            i++;
        } else if (strcmp(argv[i],"--leds_brightness") == 0 && i + 1 < argc) {
            leds_brightness = argv[i + 1];
            i++;
        } else if (strcmp(argv[i],"--pwm_path") == 0 && i + 1 < argc) {
            pwm_path = argv[i + 1];
            i++;
        } else if (strcmp(argv[i],"--pwm_name") == 0 && i + 1 < argc) {
            pwm_name = argv[i + 1];
            i++;
        } else if (strcmp(argv[i],"--red_pwm_name") == 0 && i + 1 < argc) {
            red_pwm_name = argv[i + 1];
            i++;
        } else if (strcmp(argv[i],"--green_pwm_name") == 0 && i + 1 < argc) {
            green_pwm_name = argv[i + 1];
            i++;
        } else if (strcmp(argv[i],"--blue_pwm_name") == 0 && i + 1 < argc) {
            blue_pwm_name = argv[i + 1];
            i++;
        } else if (strcmp(argv[i],"--red_led_tweak") == 0 && i + 1 < argc) {
            red_max_brightness = strtol(argv[i + 1],NULL,10);
            i++;
        } else if (strcmp(argv[i],"--green_led_tweak") == 0 && i + 1 < argc) {
            green_max_brightness = strtol(argv[i + 1],NULL,10);
            i++;
        } else if (strcmp(argv[i],"--blue_led_tweak") == 0 && i + 1 < argc) {
            blue_max_brightness = strtol(argv[i + 1],NULL,10);
            i++;
        } else {
            printf("Invalid arguments\n");
            printf("Try '%s -h' or '%s --help' for more information.\n", argv[0],argv[0]);
            return 1;
        }
    }
    
    snprintf(pwm_chip_export, MAX_LENGTH, "%s/%s/export", pwm_path, pwm_name);
    snprintf(pwm_chip_unexport, MAX_LENGTH, "%s/%s/export", pwm_path, pwm_name);
    
    snprintf(red_pwm, MAX_LENGTH, "%s/%s/%s", pwm_path, pwm_name, red_pwm_name);
    snprintf(green_pwm, MAX_LENGTH, "%s/%s/%s", pwm_path, pwm_name, green_pwm_name);
    snprintf(blue_pwm, MAX_LENGTH, "%s/%s/%s", pwm_path, pwm_name, blue_pwm_name);
    
    snprintf(red_pwm_enable, MAX_LENGTH, "%s/%s/%s/enable", pwm_path, pwm_name, red_pwm_name);
    snprintf(green_pwm_enable, MAX_LENGTH, "%s/%s/%s/enable", pwm_path, pwm_name, green_pwm_name);
    snprintf(blue_pwm_enable, MAX_LENGTH, "%s/%s/%s/enable", pwm_path, pwm_name, blue_pwm_name);

    snprintf(red_pwm_period, MAX_LENGTH, "%s/%s/%s/period", pwm_path, pwm_name, red_pwm_name);
    snprintf(green_pwm_period, MAX_LENGTH, "%s/%s/%s/period", pwm_path, pwm_name, green_pwm_name);
    snprintf(blue_pwm_period, MAX_LENGTH, "%s/%s/%s/period", pwm_path, pwm_name, blue_pwm_name);

    snprintf(red_pwm_duty_cycle, MAX_LENGTH, "%s/%s/%s/duty_cycle", pwm_path, pwm_name, red_pwm_name);
    snprintf(green_pwm_duty_cycle, MAX_LENGTH, "%s/%s/%s/duty_cycle", pwm_path, pwm_name, green_pwm_name);
    snprintf(blue_pwm_duty_cycle, MAX_LENGTH, "%s/%s/%s/duty_cycle", pwm_path, pwm_name, blue_pwm_name);
    
    
    if (ishelp) {
        printf("%s Version 0.0.1 NekokeCore(Ashley Lee)@2024 nekokecore@emtips.net\n", argv[0]);
        printf("Usage: %s [OPTION]...\n", argv[0]);
        printf("General:\n");
        printf("  -h, --help          Display this help and exit.\n");
        printf("  -P, --poweroff      Poweroff all LEDs.\n");
        printf("  -p, --period        Set PWMs period.\n");
        printf("  -c, --color         Set HEX color code.\n");
        printf("  -a, --animation     Set RGB animation.\n");
        printf("        default       Keep LEDs on mode.\n");
        printf("        breathing     Breathing LEDs mode.\n");
        printf("        rainbow       Rainbow color LEDs mode.\n");
        printf("Example:\n  %s -p 51000 -c 00ff00 -a default.\n", argv[0]);
        printf("Advance:\n");
        printf("  --leds_brightness   Set LEDs max brightness.\n");
        printf("  --pwm_path          Set LEDs PWM path.\n");
        printf("  --pwm_name          Set LEDs PWM name.\n");
        printf("  --red_pwm_name      Set Red LED pin name.\n");
        printf("  --green_pwm_name    Set Green LED pin name.\n");
        printf("  --blue_pwm_name     Set Blue LED pin name.\n");
        printf("  --red_led_tweak     Set Red LED tweak.\n");
        printf("  --green_led_tweak   Set Green LED tweak.\n");
        printf("  --blue_led_tweak    Set Blue LED tweak.\n");
        return 0;
    }
    
    if (ispoweroff) {
        if (access(red_pwm, F_OK) == -1 || access(green_pwm, F_OK) == -1 || access(blue_pwm, F_OK) == -1 ) {
            printf("Registering PWMs.\n");
            write_file(pwm_chip_export,red_pwm_name+3,strlen(red_pwm_name),WRITE_ONLY);
            write_file(pwm_chip_export,green_pwm_name+3,strlen(green_pwm_name),WRITE_ONLY);
            write_file(pwm_chip_export,blue_pwm_name+3,strlen(blue_pwm_name),WRITE_ONLY);
        }
        
        printf("Poweroff all LEDs.\n");
        write_file(red_pwm_enable,DISABLE,strlen(DISABLE),WRITE_READ);
        write_file(green_pwm_enable,DISABLE,strlen(DISABLE),WRITE_READ);
        write_file(blue_pwm_enable,DISABLE,strlen(DISABLE),WRITE_READ);
        
        while (1) {
            sleep(1);
            if(exit_sig) break;
        }
        return 0;
    }
        
    int red;
    int green;
    int blue;
    sscanf(color,"%2x%2x%2x",&red,&green,&blue);

    if (access(red_pwm, F_OK) == -1 || access(green_pwm, F_OK) == -1 || access(blue_pwm, F_OK) == -1 ) {
        printf("Registering PWMs.\n");
        write_file(pwm_chip_export,red_pwm_name+3,strlen(red_pwm_name),WRITE_ONLY);
        write_file(pwm_chip_export,green_pwm_name+3,strlen(green_pwm_name),WRITE_ONLY);
        write_file(pwm_chip_export,blue_pwm_name+3,strlen(blue_pwm_name),WRITE_ONLY);
    }
    
    printf("Restting PWMs and LEDs.\n");
    write_file(red_pwm_period,DISABLE,strlen(DISABLE),WRITE_READ);
    write_file(green_pwm_period,DISABLE,strlen(DISABLE),WRITE_READ);
    write_file(blue_pwm_period,DISABLE,strlen(DISABLE),WRITE_READ);
        
    write_file(red_pwm_duty_cycle,DISABLE,strlen(DISABLE),WRITE_READ);
    write_file(green_pwm_duty_cycle,DISABLE,strlen(DISABLE),WRITE_READ);
    write_file(blue_pwm_duty_cycle,DISABLE,strlen(DISABLE),WRITE_READ);
        
    printf("Setting PWMs.\n");
    write_file(red_pwm_enable,ENABLE,strlen(ENABLE),WRITE_READ);
    write_file(green_pwm_enable,ENABLE,strlen(ENABLE),WRITE_READ);
    write_file(blue_pwm_enable,ENABLE,strlen(ENABLE),WRITE_READ);

    write_file(red_pwm_period,period,strlen(period),WRITE_READ);
    write_file(green_pwm_period,period,strlen(period),WRITE_READ);
    write_file(blue_pwm_period,period,strlen(period),WRITE_READ);
        
    if(strcmp(animation,"default") == 0) {
        printf("Setting LEDs.\n");
        char red_duty[10];
        char green_duty[10];
        char blue_duty[10];

        calculate_duty_cycles(red,strtol(period,NULL,10),red_duty,red_max_brightness * (strtol(leds_brightness,NULL,10) / 100));
        calculate_duty_cycles(green,strtol(period,NULL,10),green_duty,green_max_brightness * (strtol(leds_brightness,NULL,10) / 100));
        calculate_duty_cycles(blue,strtol(period,NULL,10),blue_duty,blue_max_brightness * (strtol(leds_brightness,NULL,10) / 100));
            
        write_file(red_pwm_duty_cycle,red_duty,strlen(red_duty),WRITE_READ);
        write_file(green_pwm_duty_cycle,green_duty,strlen(green_duty),WRITE_READ);
        write_file(blue_pwm_duty_cycle,blue_duty,strlen(blue_duty),WRITE_READ);

        while (1) {
            sleep(1);
            if(exit_sig) break;
        }
    }

     if(strcmp(animation,"breathing") == 0) {
         printf("Setting LEDs.\n");

         int pwm3_spwm_wave[MAX_LENGTH]={0};
         int pwm2_spwm_wave[MAX_LENGTH]={0};
         int pwm1_spwm_wave[MAX_LENGTH]={0};
         char red_duty[10];
         char green_duty[10];
         char blue_duty[10];

         calculate_duty_cycles(red,strtol(period,NULL,10),red_duty,red_max_brightness * (strtol(leds_brightness,NULL,10) / 100));
         calculate_duty_cycles(green,strtol(period,NULL,10),green_duty,green_max_brightness * (strtol(leds_brightness,NULL,10) / 100));
         calculate_duty_cycles(blue,strtol(period,NULL,10),blue_duty,blue_max_brightness * (strtol(leds_brightness,NULL,10) / 100));

         calculate_spwm_duty_cycles(red_duty,pwm3_spwm_wave);
         calculate_spwm_duty_cycles(green_duty,pwm2_spwm_wave);
         calculate_spwm_duty_cycles(blue_duty,pwm1_spwm_wave);
            
         breathing *breathing_red_led_args = malloc(sizeof(breathing));
         strcpy(breathing_red_led_args -> pwm_path,red_pwm_duty_cycle);
         memcpy(breathing_red_led_args -> spwm_wave,pwm3_spwm_wave,sizeof(pwm3_spwm_wave));
            
         breathing *breathing_green_led_args = malloc(sizeof(breathing));
         strcpy(breathing_green_led_args -> pwm_path,green_pwm_duty_cycle);
         memcpy(breathing_green_led_args -> spwm_wave,pwm2_spwm_wave,sizeof(pwm2_spwm_wave));
            
         breathing *breathing_blue_led_args = malloc(sizeof(breathing));
         strcpy(breathing_blue_led_args -> pwm_path,blue_pwm_duty_cycle);
         memcpy(breathing_blue_led_args -> spwm_wave,pwm1_spwm_wave,sizeof(pwm1_spwm_wave));

         pthread_t th1,th2,th3;
            
         pthread_create(&th1,NULL,breathing_pwm_led,breathing_red_led_args);
         pthread_create(&th2,NULL,breathing_pwm_led,breathing_green_led_args);
         pthread_create(&th3,NULL,breathing_pwm_led,breathing_blue_led_args);

         pthread_join(th1,NULL);
         pthread_join(th2,NULL);
         pthread_join(th3,NULL);

         pthread_mutex_destroy(&mutex);
         pthread_cond_destroy(&condition);

         free(breathing_red_led_args);
         free(breathing_green_led_args);
         free(breathing_blue_led_args);
     }

    if(strcmp(animation,"rainbow") == 0) {
        printf("Setting LEDs.\n");
        int rainbow_red_duty[365] = {0};
        int rainbow_green_duty[365] = {0};
        int rainbow_blue_duty[365] = {0};

        for (int i = 0; i <= RAINBOW_STEP; i++) {
            const rainbow_color color = hsl_to_rgb(i);
            char red_duty[10];
            char green_duty[10];
            char blue_duty[10];
                
            calculate_duty_cycles(color.rainbow_red,strtol(period,NULL,10),red_duty,MAX_BRIGHTNESS * (strtol(leds_brightness,NULL,10) / 100));
            calculate_duty_cycles(color.rainbow_green,strtol(period,NULL,10),green_duty,MAX_BRIGHTNESS * (strtol(leds_brightness,NULL,10) / 100));
            calculate_duty_cycles(color.rainbow_blue,strtol(period,NULL,10),blue_duty,MAX_BRIGHTNESS * (strtol(leds_brightness,NULL,10) / 100));
                
            rainbow_red_duty[i] = strtol(red_duty,NULL,10);
            rainbow_green_duty[i] = strtol(green_duty,NULL,10);
            rainbow_blue_duty[i] = strtol(blue_duty,NULL,10);
        }
        printf("Now turning on the rainbow.\n");
        while (1) {
            for (int i = 0; i <= RAINBOW_STEP; i++) {
                char red_duty[10];
                char green_duty[10];
                char blue_duty[10];
                    
                snprintf(red_duty,sizeof(red_duty),"%d",rainbow_red_duty[i]);
                snprintf(green_duty,sizeof(green_duty),"%d",rainbow_green_duty[i]);
                snprintf(blue_duty,sizeof(blue_duty),"%d",rainbow_blue_duty[i]);
                    
                write_file(red_pwm_duty_cycle,red_duty,strlen(red_duty),WRITE_READ);
                write_file(green_pwm_duty_cycle,green_duty,strlen(green_duty),WRITE_READ);
                write_file(blue_pwm_duty_cycle,blue_duty,strlen(blue_duty),WRITE_READ);
                    
                if(exit_sig) break;
                usleep(50000);
            }
            if(exit_sig) break;
        }
    }
    return 0;
}
