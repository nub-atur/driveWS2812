    /*+----------+
      |          |
      |   ESP    |----GPIO------->ledDataCount1{pixels[0], pixels[1], pixels[2]}---------------+
      |          |                                                                             |
      |          |             +---ledDataCount2{pixels[3], pixels[4], pixels[5]}<-------------+
      +----------+            |
                              +--->ledDataCount2{pixels[6], pixels[7], pixels[8]}-----------> ..........
  */

#include <stdio.h>
#include "main.h"
#include "TIM_RTOS.h"

#include "driver/gpio.h"
#include "driver/timer.h"

#include "freertos/queue.h"
#include <freertos/portmacro.h>

#include <soc/sens_reg.h>
#include <soc/sens_struct.h>

#define QUEUE_LENGTH 10
#define ITEM_SIZE sizeof(uint8_t)
#define INPUT_PIN GPIO_NUM_34

//-----------------------SOME TASKS---------------------------------------
TaskHandle_t rainBow;
TaskHandle_t flop;
TaskHandle_t readInput;
QueueHandle_t myQueue;

void rainBow_led_task(void *pvParameter)
{
    uint8_t green = 0;
    uint8_t red = 255;
    uint8_t blue = 0;

    uint32_t rxData;
	while(1)
	{   
        uint16_t res = 500;
        if (xQueueReceive(myQueue, &rxData, portMAX_DELAY)){
            res = (rxData > (int)(4095/2))? 1:500;
        }

        while (blue < 255){
            for (uint8_t i = 0; i < NUM_COLS*NUM_ROWS; i++){
                setLedColor(i, green, red, blue);
            }   
            blue++;
            red--;
            led_act();
            vTaskDelay(res / portTICK_PERIOD_MS);
        } 

        while(green < 255){
            for (uint8_t i = 0; i < NUM_COLS*NUM_ROWS; i++){
                setLedColor(i, green, red, blue);
            }   
            green++;
            blue--;
            led_act();
            vTaskDelay(res / portTICK_PERIOD_MS);
        }

        while(red < 255){
            for (uint8_t i = 0; i < NUM_COLS*NUM_ROWS; i++){
                setLedColor(i, green, red, blue);
            }   
            red++;
            green--;
            led_act();
            vTaskDelay(res / portTICK_PERIOD_MS);
        }

        while(green < 255){
            for (uint8_t i = 0; i < NUM_COLS*NUM_ROWS; i++){
                setLedColor(i, green, red, blue);
            }   
            green++;
            red--;
            led_act();
            vTaskDelay(res / portTICK_PERIOD_MS);
        }

        while(blue < 255){
            for (uint8_t i = 0; i < NUM_COLS*NUM_ROWS; i++){
                setLedColor(i, green, red, blue);
            }   
            blue++;
            green--;
            led_act();
            vTaskDelay(res / portTICK_PERIOD_MS);
        }

        // vTaskDelay(res / portTICK_PERIOD_MS);

        while(red < 255){
            for (uint8_t i = 0; i < NUM_COLS*NUM_ROWS; i++){
                setLedColor(i, green, red, blue);
            }   
            red++;
            blue--;
            led_act();
            vTaskDelay(res / portTICK_PERIOD_MS);
        }

        // vTaskDelay(res / portTICK_PERIOD_MS);
	}
}

void vFlop(void *pvParameter){
    for(;;){
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        vTaskSuspend(rainBow);
        uint16_t res = 1000;
        
        for (uint8_t i=0; i< NUM_COLS*NUM_ROWS; i++){
            setLedColor(i, 255, 255, 255);
            led_act();
        }

        for (uint8_t i=0; i< NUM_COLS*NUM_ROWS; i++){
            setLedColor(i, 0, 0, 0);
            led_act();
        }

        vTaskResume(rainBow);
        vTaskDelay(res / portTICK_PERIOD_MS);
    }
}

void vReadLM393(void *pvParameter){
    uint32_t input;

    adc1_config_width(ADC_WIDTH_BIT_12);

    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11); //attenuation

    for(;;){
        input = adc1_get_raw(ADC1_CHANNEL_6);
        printf("%ld\n", input);
        xQueueSend(myQueue, &input, 100);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

//------------------------------------------------------------------------
void app_main() {
    Init_TIM_RTOS();

    bitbang_initialize(pins);

    myQueue = xQueueCreate(QUEUE_LENGTH, ITEM_SIZE);

    xTaskCreate(rainBow_led_task, "rainbow", 1024*3, NULL, 1, &rainBow);
    xTaskCreate(vReadLM393, "read", 1024*3, NULL, 2, &readInput);
    xTaskCreate(vFlop, "jumping", 1024*3, NULL, 0, &flop);

    while(1){
        // printf("Restarting now.\n");
        // fflush(stdout);
        // esp_restart();
    }
}






// #define ADC_SAMPLES_COUNT 1000
// int16_t abuf[ADC_SAMPLES_COUNT];
// int16_t abufPos = 0;
//---------------------------SamplingCalc---------------------------
// portMUX_TYPE DRAM_ATTR timerMux = portMUX_INITIALIZER_UNLOCKED; 
// TaskHandle_t complexHandlerTask;
// timer_t * adcTimer = NULL; // our timer

// void complexHandler(void *param) {
//   while (true) {
//     // Sleep until the ISR gives us something to do, or for 1 second
//     uint32_t tcount = ulTaskNotifyTake(pdFALSE, pdMS_TO_TICKS(1000));  
//     if (check_for_work) {
//       // Do something complex and CPU-intensive
//     }
//   }
// }

// void IRAM_ATTR onTimer(void *args) {
//   portENTER_CRITICAL_ISR(&timerMux);

//   abuf[abufPos++] = local_adc1_read(ADC1_CHANNEL_0);
  
//   if (abufPos >= ADC_SAMPLES_COUNT) { 
//     abufPos = 0;

//     // Notify adcTask that the buffer is full.
//     BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//     vTaskNotifyGiveFromISR(flop, &xHigherPriorityTaskWoken);
//     if (xHigherPriorityTaskWoken) {
//       portYIELD_FROM_ISR();
//     }
//   }
//   portEXIT_CRITICAL_ISR(&timerMux);
// }

// int IRAM_ATTR local_adc1_read(int channel) {
//     uint16_t adc_value;
//     SENS.sar_meas_start1.sar1_en_pad = (1 << channel); // only one channel is selected
//     while (SENS.sar_slave_addr1.meas_status != 0);
//     SENS.sar_meas_start1.meas1_start_sar = 0;
//     SENS.sar_meas_start1.meas1_start_sar = 1;
//     while (SENS.sar_meas_start1.meas1_done_sar == 0);
//     adc_value = SENS.sar_meas_start1.meas1_data_sar;
//     return adc_value;
// }
