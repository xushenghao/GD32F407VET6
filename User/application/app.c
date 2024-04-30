#include "main.h"
#include "app.h"
#include "step_motor.h"

ALIGN(RT_ALIGN_SIZE)
static char led_thread_stack[RT_MAIN_THREAD_STACK_SIZE];
static struct rt_thread led_thread;

static char step_motor_thread_stack[RT_MAIN_THREAD_STACK_SIZE];
static struct rt_thread step_motor_thread;

void led_thread_entry(void *paramenter)
{
    while (1)
    {
        GPIO_TOGGLE(LED_GPIO_Port, LED_Pin);
        rt_thread_mdelay(1000);
    }
}

void step_motor_thread_entry(void *paramenter)
{
    while (1)
    {
        step_motor_start(STEP_MOTOR_1);
        rt_thread_mdelay(5000);
    }
}

void app_init(void)
{
    step_motor_hardware_t hardware = {
        .en = gpio_create(TMC_EN_GPIO_Port, TMC_EN_Pin),
        .dir = gpio_create(TMC_DIR_GPIO_Port, TMC_DIR_Pin),
        .tim = &htim4,
        .chan = TIM_CHANNEL_1,
    };
    step_motor_init(STEP_MOTOR_1, hardware);

    rt_thread_init(&led_thread, "led task", led_thread_entry, RT_NULL, &led_thread_stack[0], sizeof(led_thread_stack), RT_THREAD_PRIORITY_MAX - 1, 10);
    rt_thread_startup(&led_thread);

    rt_thread_init(&step_motor_thread,
                   "step motor task",
                   step_motor_thread_entry,
                   RT_NULL,
                   &step_motor_thread_stack[0],
                   sizeof(step_motor_thread_stack),
                   RT_THREAD_PRIORITY_MAX - 2, 10);
    rt_thread_startup(&step_motor_thread);
}
