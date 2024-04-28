#include "flow.h"

/* 1，初始化一个struct flow变量 */
static struct flow fl_led;

static char led_flash(struct flow *fl)
{
    FL_HEAD(fl);
    for (;;)
    {
        FL_LOCK_DELAY(fl, FL_CLOCK_SEC * 1U); /* 延时一秒 */
        led_open();
        FL_LOCK_DELAY(fl, FL_CLOCK_SEC * 1U); /* 延时一秒 */
        led_close();
    }
    FL_TAIL(fl);
}

int main(void)
{
    FL_INIT(&fl_led);
    for (;;)
    {
        led_flash(&fl_led);
        // other_process();
    }
    return 0;
}
