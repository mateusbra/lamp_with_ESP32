#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_RED_CH      LEDC_CHANNEL_0
#define LED_GREEN_CH    LEDC_CHANNEL_1
#define LED_BLUE_CH     LEDC_CHANNEL_2

#define LED_RED_PIN     15//27
#define LED_GREEN_PIN   2//14
#define LED_BLUE_PIN    4//12

void ledc_init()
{
    ledc_timer_config_t ledc_timer = 
	{
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_13_BIT,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_red_chan = 
	{
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LED_RED_CH,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LED_RED_PIN,
        .duty = 0,
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_red_chan));

    ledc_channel_config_t ledc_green_chan = 
	{
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LED_GREEN_CH,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LED_GREEN_PIN,
        .duty = 0,
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_green_chan));

    ledc_channel_config_t ledc_blue_chan = 
	{
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LED_BLUE_CH,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LED_BLUE_PIN,
        .duty = 0,
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_blue_chan));

    ledc_fade_func_install(0);
}

void app_main(void)
{

    ledc_init();

    while(1)
	{
		/*
        //blink red
        for(int i=0; i<3; i++)
		{ 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_RED_CH,8191,0); 
			vTaskDelay(400/portTICK_PERIOD_MS); 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_RED_CH,0,0); 
			vTaskDelay(400/portTICK_PERIOD_MS); 
		} 
		
		//blink green 
		for(int i=0; i<3; i++)
		{ 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_GREEN_CH,8191,0);
			vTaskDelay(400/portTICK_PERIOD_MS); 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_GREEN_CH,0,0); 
			vTaskDelay(400/portTICK_PERIOD_MS); 
		} 
		
		//blink blue 
		for(int i=0; i<3; i++)
		{ 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_BLUE_CH,8191,0); 
			vTaskDelay(400/portTICK_PERIOD_MS); 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_BLUE_CH,0,0); 
			vTaskDelay(400/portTICK_PERIOD_MS); 
		} 
		
		//fade 
		ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE,LED_RED_CH,8191,3000,LEDC_FADE_WAIT_DONE); 
		ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE,LED_RED_CH,0,3000,LEDC_FADE_WAIT_DONE); 
		ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE,LED_GREEN_CH,8191,3000,LEDC_FADE_WAIT_DONE); 
		ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE,LED_GREEN_CH,0,3000,LEDC_FADE_WAIT_DONE); 
		ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE,LED_BLUE_CH,8191,3000,LEDC_FADE_WAIT_DONE); 
		ledc_set_fade_time_and_start(LEDC_LOW_SPEED_MODE,LED_BLUE_CH,0,3000,LEDC_FADE_WAIT_DONE); */
		
		//yello color 
		/*ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_RED_CH,8191,0); 
		ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_GREEN_CH,0,0); 
		ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_BLUE_CH,8191,0);		
		vTaskDelay(10000/portTICK_PERIOD_MS); */
		
		for(int i=0; i<8191; i++)
		{ 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_RED_CH,	 8191,		0); 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_GREEN_CH,		0,		0); 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_BLUE_CH,		i,		0); 
			
			//vTaskDelay(100/portTICK_PERIOD_MS); 
		} 
		
		for(int i=0; i<8191; i++)
		{ 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_RED_CH,	 8191 - i,		0); 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_GREEN_CH,		0,		0); 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_BLUE_CH,	 8191,		0); 
			
			//vTaskDelay(100/portTICK_PERIOD_MS); 
		} 
		
		for(int i=0; i<8191; i++)
		{ 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_RED_CH,	 	0,		0); 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_GREEN_CH,		i,		0); 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_BLUE_CH,	 8191,		0); 
			
			//vTaskDelay(100/portTICK_PERIOD_MS); 
		} 
				for(int i=0; i<8191; i++)
		{ 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_RED_CH,	 	0,		0); 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_GREEN_CH,	 8191,		0); 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_BLUE_CH,	 8191-i,		0); 
			
			//vTaskDelay(100/portTICK_PERIOD_MS); 
		} 
						for(int i=0; i<8191; i++)
		{ 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_RED_CH,	 	i,		0); 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_GREEN_CH,	 8191,		0); 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_BLUE_CH,	 	0,		0); 
			
			//vTaskDelay(100/portTICK_PERIOD_MS); 
		} 
		for(int i=0; i<8191; i++)
				{ 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_RED_CH,	   8191,		0); 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_GREEN_CH,	 8191-i,		0); 
			ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_BLUE_CH,	 	  0,		0); 
			
			//vTaskDelay(100/portTICK_PERIOD_MS); 
		} 
		/*ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_RED_CH,4095,0); 
		ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_GREEN_CH,4095,0); 
		ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_BLUE_CH,0,0); 
		vTaskDelay(2000/portTICK_PERIOD_MS); 
		
		ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_RED_CH,100,0); 
		ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_GREEN_CH,100,0); 
		ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_BLUE_CH,0,0); 
		vTaskDelay(2000/portTICK_PERIOD_MS); */
		
		//close 
		ledc_stop(LEDC_LOW_SPEED_MODE,LED_RED_CH,0); 
		ledc_stop(LEDC_LOW_SPEED_MODE,LED_GREEN_CH,0); 
		ledc_stop(LEDC_LOW_SPEED_MODE,LED_BLUE_CH,0); 
		vTaskDelay(1000/portTICK_PERIOD_MS); 
	} 
}