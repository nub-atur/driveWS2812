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

void vRainBow(void *pvParameter)
{
    uint8_t green;
    uint8_t red;
    uint8_t blue;
    uint16_t i;
    uint16_t j = 0;
    uint32_t rxData;
    uint16_t res = 1000;
	while(1)
	{  
        if (xQueueReceive(myQueue, &rxData, portMAX_DELAY))
        {
            res = (rxData > (uint16_t)(4095/2))? 1:1000;
        }
        
        for (i = 0; i < 256; ++i) 
        {
            if (j == NUM_COLS*NUM_ROWS) j = 0; 

            // Red goes from 0 to 255
            red = i;
            // Green goes from 0 to 255
            green = (i + 85) % 256; // 85 is 1/3 of 255
            // Blue goes from 255 to 0
            blue = (256 - i) % 256;

            // Print color
            setLedColor(j, green, red, blue);
            led_act();

            j++;
            // Sleep for a short duration to control the speed of the effect
        }
        vTaskDelay(res / portTICK_PERIOD_MS);
	}
}

void vFlop(void *pvParameter)
{
    uint16_t res = 1000;
    for(;;)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        vTaskSuspend(rainBow);

        for (uint8_t i=0; i< NUM_COLS*NUM_ROWS; i++)
        {
            setLedColor(i, 0, 0, 0);
            led_act();
        }
        vTaskDelay(res / portTICK_PERIOD_MS);
    }
}

void vReadLM393(void *pvParameter)
{
    uint32_t input;

    adc1_config_width(ADC_WIDTH_BIT_12);

    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11); //attenuation

    for(;;)
    {
        input = adc1_get_raw(ADC1_CHANNEL_6);
        printf("%ld\n", input);
        xQueueSend(myQueue, &input, 10);
        vTaskResume(rainBow);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

//------------------------------------------------------------------------
void app_main() 
{
    Init_TIM_RTOS();

    bitbang_initialize(pins);

    myQueue = xQueueCreate(QUEUE_LENGTH, ITEM_SIZE);

    xTaskCreate(vRainBow, "rainbow", 1024*3, NULL, 0, &rainBow);
    xTaskCreate(vReadLM393, "read", 1024*3, NULL, 2, &readInput);
    xTaskCreate(vFlop, "jumping", 1024*3, NULL, 1, &flop);

    while(1)
    {
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
