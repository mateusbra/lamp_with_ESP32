#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

#include "esp_wifi.h"
#include "nvs_flash.h"
#include "wifiCred.h" //Store SSID and PASS
#include "esp_netif.h"

#include "esp_http_client.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>

#include <driver/adc.h>
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

#include "driver/ledc.h"
#define LED_RED_CH      LEDC_CHANNEL_0
#define LED_GREEN_CH    LEDC_CHANNEL_1
#define LED_BLUE_CH     LEDC_CHANNEL_2

#define LED_RED_PIN     15
#define LED_GREEN_PIN    2
#define LED_BLUE_PIN     4

//static const char *TAG = "HTTP_CLIENT";
#define DEFAULT_RSSI -127
#define DEFAULT_AUTHMODE WIFI_AUTH_WPA2_PSK
#define DEFAULT_SCAN_METHOD WIFI_FAST_SCAN
#define DEFAULT_SORT_METHOD WIFI_CONNECT_AP_BY_SIGNAL

static const char *TAG = "WiFi";

int color_R = 8191; //8191 = 255
int color_G = 8191;
int color_B = 8191;
uint8_t mode = 0;
bool WiFi 	= 0;
bool Light 	= 0;
bool motion = 0;
bool manualMode = 0;
int ambientLight = 500; //500->high light | 4000->low light
bool lightBefore = 0;


static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) 
	{
		WiFi = 0;
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
	{
		WiFi = 0;
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
	{
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		WiFi = 1;
    }
}

static void wifi_connection()
{	
    ESP_ERROR_CHECK(esp_netif_init());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    // Initialize default station as network interface instance (esp-netif)
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    // Initialize and start WiFi
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = SSID,
            .password = PASS,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

/*void rest_get(){
    
    esp_http_client_config_t config_get = {
      .url = "http://worldclockapi.com/api/json/utc/now",
        .method = HTTP_METHOD_GET,
        .cert_pem = NULL,
        .event_handler = _http_event_handler
        };
        
    esp_http_client_handle_t client = esp_http_client_init(&config_get);
    ESP_ERROR_CHECK(esp_http_client_perform(client));
    ESP_ERROR_CHECK(esp_http_client_cleanup(client));
}*/ //Teste

void rest_post(){
	int tempAmbientLight = 0;
	char * url = "https://backendesp.vercel.app/setMotion?motion=true";
	const char * auxUrl = "https://backendesp.vercel.app/setLumin?luminosity=";
    
	for (int i = 0; i< 2; i++)
	{	
		if (i == 0)
		{
			if (motion)
				url = "https://backendesp.vercel.app/setMotion?motion=true";
			else 
				url = "https://backendesp.vercel.app/setMotion?motion=false";
		} else if (i == 1) 
		{
			tempAmbientLight = (ambientLight * 100)/4000;
			tempAmbientLight =100 - tempAmbientLight;
			
			int len_ = snprintf(NULL, 0, "%d", tempAmbientLight);
			char *tempServer = (char *)malloc(len_ + 1);
			snprintf(tempServer, len_+1, "%d", tempAmbientLight);
			
			ESP_LOGI(TAG, "VALOR LUZ AMBIENTE %s\n", tempServer);

			url = (char *)malloc(strlen(auxUrl)+strlen(tempServer));
			
			strcpy(url, auxUrl);
			strcat(url, tempServer);
		}
		else if (i == 2)
		{
			if (Light)
				url = "https://backendesp.vercel.app/setLight?light=true";
			else 
				url = "https://backendesp.vercel.app/setLight?light=false";
		}
		
		esp_http_client_config_t config_post = {
			.url = url,
			.method = HTTP_METHOD_POST,
			.event_handler = NULL
		};
		
		esp_http_client_handle_t client = esp_http_client_init(&config_post);
		esp_http_client_perform(client);
		esp_http_client_cleanup(client);
	}
	free(url);
}

static void taskLDR(void)
{
	adc1_config_width(ADC_WIDTH_12Bit);
	adc1_config_channel_atten(ADC1_CHANNEL_5, ADC_ATTEN_6db);
	while(1) 
	{
		ambientLight = adc1_get_raw(ADC1_CHANNEL_5);
		
		if (ambientLight > 4000)
			ambientLight = 4000;
		
		ESP_LOGI(TAG, "Value %d\n", ambientLight);
		vTaskDelay(2000/portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}

static void taskPIR(void)
{
	gpio_set_direction(GPIO_NUM_12, GPIO_MODE_INPUT);
	while(1) 
	{
		motion = gpio_get_level(GPIO_NUM_12);
		
		printf("valor:%d\n", motion);
		
		vTaskDelay(2000/portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL); 
}

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

void ledc_turn_off()
{
		ledc_stop(LEDC_LOW_SPEED_MODE,LED_RED_CH,0); 
		ledc_stop(LEDC_LOW_SPEED_MODE,LED_GREEN_CH,0); 
		ledc_stop(LEDC_LOW_SPEED_MODE,LED_BLUE_CH,0); 
}

static void taskLED(void)
{
	ledc_init();
	
	while(1)
	{
		if (manualMode)
		{
			if (Light)
			{
				ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_RED_CH,	color_R,	0); 
				ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_GREEN_CH,	color_G,	0); 
				ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_BLUE_CH,	color_B,	0);
			}	else 
			{
				ledc_turn_off();
			}			
		} else {
			if (((ambientLight > 2000) || lightBefore) && motion)
			{
				lightBefore = 1;
				ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_RED_CH,	color_R,	0); 
				ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_GREEN_CH,	color_G,	0); 
				ledc_set_duty_and_update(LEDC_LOW_SPEED_MODE,LED_BLUE_CH,	color_B,	0);
				vTaskDelay(10000/portTICK_PERIOD_MS); //Definir tempo que a luz ficará acessa
			} 	else {
				ledc_turn_off();
				lightBefore = 0;
			}
		}
		vTaskDelay(5000/portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
		case HTTP_EVENT_ON_DATA:

			if (mode == 1)
			{
				printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
				const char * buff = (char *)evt->data;
				const cJSON * raw_data = NULL;
				cJSON *_json = cJSON_Parse(buff);
				
				raw_data = cJSON_GetObjectItemCaseSensitive(_json, "rgb");
				
				const char * data = (char *)raw_data->valuestring;
				printf("Checking Color \"%s\"\n", data);
				
				char temp[3];
				int temp_color = 0;
				
				for(int i = 0; i<3; i++)
				{
					strncpy ( temp, data+(i*2), 2);
					temp_color = strtol(temp, NULL, 16);
					if(i==0)
						color_R = (8191 * temp_color)/255;
					else if (i==1)
						color_G = (8191 * temp_color)/255;
					else
						color_B = (8191 * temp_color)/255;
				}
				
				cJSON_Delete(_json);
			} else if (mode == 2) 
			{
				printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
				const char * buff = (char *)evt->data;
				const cJSON * raw_data = NULL;
				cJSON *_json = cJSON_Parse(buff);
				
				raw_data = cJSON_GetObjectItemCaseSensitive(_json, "light");
				Light = (bool)raw_data->valueint;
				//printf("Checking Light \"%d\"\n", data);
				cJSON_Delete(_json);
			} else if (mode == 3)
			{
				printf("HTTP_EVENT_ON_DATA: %.*s\n", evt->data_len, (char *)evt->data);
				const char * buff = (char *)evt->data;
				const cJSON * raw_data = NULL;
				cJSON *_json = cJSON_Parse(buff);
				
				raw_data = cJSON_GetObjectItemCaseSensitive(_json, "mode");
				manualMode = (bool)raw_data->valueint;
				//printf("Checking mode \"%d\"\n", data);
				cJSON_Delete(_json);
			}

			break;

		default:
			break;
    }
    return ESP_OK;   
}

void rest_get()
{
	char * url = "https://backendesp.vercel.app/getRgb";
	switch (mode)
    {
		case 1:
			url = "https://backendesp.vercel.app/getRgb";
			break;
		case 2:
			url = "https://backendesp.vercel.app/getLight";
			break;
		case 3:
			url = "https://backendesp.vercel.app/getMode";
			break;
		default:
			url = "https://backendesp.vercel.app/getRgb";
			break;
	}
	
    esp_http_client_config_t config_get = {
      .url = url,
        .method = HTTP_METHOD_GET,
        .cert_pem = NULL,
        .event_handler = _http_event_handler
    };
        
    esp_http_client_handle_t client = esp_http_client_init(&config_get);
    ESP_ERROR_CHECK(esp_http_client_perform(client));
    ESP_ERROR_CHECK(esp_http_client_cleanup(client));
}

static void taskServerUpdate(void)
{
	while(1)
	{
		if (WiFi)
		{
			mode = 1;
			rest_get();
			
			if (manualMode)
			{
				mode = 2;
				rest_get();
			}
			
			mode = 3;
			rest_get();
			
			rest_post();
		} else 
		{
			ESP_LOGI(TAG, "WiFi Off... trying to reconnect...");
			vTaskDelay(5000/portTICK_PERIOD_MS);
		}
		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
	vTaskDelete(NULL);
}

void app_main(void)
{
	esp_err_t	ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
	{
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
	ESP_ERROR_CHECK(ret);
	
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	// Override global log level
    esp_log_level_set("*", ESP_LOG_INFO);
    
	wifi_connection();
    vTaskDelay(5000.0 / portTICK_PERIOD_MS);
	
	xTaskCreatePinnedToCore(taskPIR, 			"taskPIR", 			10000, NULL, 3, NULL, 0);//ESP_LOGI(TAG, "BTN high watermark %d\n", uxTaskGetStackHighWaterMark( NULL ) );
	xTaskCreatePinnedToCore(taskLDR, 			"taskLDR", 			10000, NULL, 3, NULL, 0);
	xTaskCreatePinnedToCore(taskLED, 			"taskLED", 			10000, NULL, 2, NULL, 1);
	xTaskCreatePinnedToCore(taskServerUpdate, 	"taskServerUpdate", 10000, NULL, 5, NULL, 1);
}