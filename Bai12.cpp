#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* ===== Cấu trúc LED ===== */
typedef struct {
    uint32_t frequency; // Hz
    uint32_t duty;      // 0-100%
} Led_config_t;

/* ===== Queue ===== */
QueueHandle_t xQueue_led_config;

/* ===== Khởi tạo GPIO PB12 ===== */
void LED_GPIO_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   // Push-Pull output
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOB, GPIO_Pin_12); // Tắt LED lúc đầu
}

/* ===== Task 1: Nhấp nháy LED theo tham số ===== */
void vLEDTask(void *pvParameters)
{
    Led_config_t ledParams = {2, 50}; // mặc định 2Hz, 50%
    TickType_t T_period, T_on, T_off;
    BaseType_t xStatus;

    while(1)
    {
        if(ledParams.frequency == 0) ledParams.frequency = 1;
        if(ledParams.duty > 100) ledParams.duty = 100;

        T_period = configTICK_RATE_HZ / ledParams.frequency;
        T_on = T_period * ledParams.duty / 100;
        T_off = T_period - T_on;

        // Bật LED
        if(T_on > 0)
        {
            GPIO_ResetBits(GPIOB, GPIO_Pin_12); // Bật LED
            xStatus = xQueueReceive(xQueue_led_config, &ledParams, T_on);
            if(xStatus == pdPASS) continue; // nhận tham số mới -> bắt đầu chu kỳ mới
        }

        // Tắt LED
        if(T_off > 0)
        {
            GPIO_SetBits(GPIOB, GPIO_Pin_12); // Tắt LED
            xStatus = xQueueReceive(xQueue_led_config, &ledParams, T_off);
            if(xStatus == pdPASS) continue;
        }
    }
}

/* ===== Task 2: Cập nhật tham số định kỳ ===== */
void vUpdateTask(void *pvParameters)
{
    const uint32_t parameters[][2] = {
        {2, 10}, {4, 30}, {6, 50}, {8, 70}, {10, 90}
    };
    const uint8_t num_param_sets = sizeof(parameters) / sizeof(parameters[0]);
    uint8_t index = 0;
    Led_config_t params_to_send;

    while(1)
    {
        params_to_send.frequency = parameters[index][0];
        params_to_send.duty = parameters[index][1];

        xQueueOverwrite(xQueue_led_config, &params_to_send); // gửi Queue
        index++;
        if(index >= num_param_sets) index = 0; // quay lại đầu

        vTaskDelay(pdMS_TO_TICKS(5000)); // 5s
    }
}

int main(void)
{
    LED_GPIO_Init();

    xQueue_led_config = xQueueCreate(1, sizeof(Led_config_t));
    if(xQueue_led_config != NULL)
    {
        xTaskCreate(vLEDTask, "LED Task", 128, NULL, 1, NULL);
        xTaskCreate(vUpdateTask, "Update Task", 128, NULL, 2, NULL);
        vTaskStartScheduler();
    }

    while(1); // Scheduler chạy, không vào đây
}