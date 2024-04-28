#include "lcds.h"
#include "delay.h"

#define ST_DISP_ON 0xAF      // 显示开
#define ST_DISP_OFF 0xAE     // 显示关
#define ST_START_LINE 0x40   // 设置Start Line
#define ST_PAGE_ADDR 0xB0    // 设置Page Address
#define ST_COL_ADDRM 0x10    // 设置Column Address MSB
#define ST_COL_ADDRL 0x00    // 设置Column Address LSB
#define ST_DISP_NORMAL 0xA6  // 正常显示,1-显示
#define ST_DISP_REVERSE 0xA7 // 反向显示,0-显示
#define ST_DISP_ALL_OFF 0xA4 // 正常显示
#define ST_DISP_ALL_ON 0xA5  // 不管RAM内容，全部显示
#define ST_RESET 0xE2        // 软件复位
#define ST_BOOST 0x2F        // 内部升压
#define ST_DISP_0 0xC4       // 0度正常显示
#define ST_DISP_180 0xC2     // 旋转180度显示
#define ST_SET_EV 0x81       // 对比度控制
#pragma pack(1)
/*
    驱动使用的数据结构，不对外
*/
struct _cog_drv_data
{
    uint8_t gram[8][COL_DOT_MAX_192];

    /*刷新区域*/
    uint16_t sx;
    uint16_t ex;
    uint16_t sy;
    uint16_t ey;
    uint16_t disx;
    uint16_t disy;
};
#pragma pack()

/**
 * @brief 写命令
 * @param {uint8_t} cmd
 * @return {*}
 */
static void write_cmd(spi_t *handle, uint8_t cmd)
{
    handle->interface.u.lcd.write_cmd(handle, cmd);
}

static void write_data(spi_t *handle, uint8_t *data, uint16_t len)
{
    handle->interface.u.lcd.write_data(handle, data, len);
}
/**
 * @brief 设置起始页
 * @param {uint8_t} page
 * @return {*}
 */
static void start_page(spi_t *handle, uint8_t page)
{
    page &= 0x07;         // 定义低3 位
    page |= ST_PAGE_ADDR; // 第一页
    write_cmd(handle, page);
}

/**
 * @brief 设置起始列
 * @param {uint8_t} Column
 * @return {*}
 */
static void start_column(spi_t *handle, uint8_t column)
{
    uint8_t temp, cmd;

    temp = column;
    cmd = (temp & 0x0F); // 定义A3-A0;
    cmd += ST_COL_ADDRL;
    write_cmd(handle, cmd);

    temp >>= 4;
    cmd = (temp & 0x0F); // 定义A7-A4;
    cmd += ST_COL_ADDRM;
    write_cmd(handle, cmd);
}

/**
 * @brief 设置起始行
 * @param {uint8_t} line
 * @return {*}
 */
static void start_line(spi_t *handle, uint8_t line)
{
    line &= 0x3F;          // 定义低6 位
    line |= ST_START_LINE; // 第一行或
    write_cmd(handle, line);
}

/**
 * @brief 初始化，包括硬件初始化和软件初始化
 * @param {lcd_t} *lcd
 * @return {*}
 */
static int32_t _init(lcd_t *lcd)
{
    lcd->info.spi->gpios.rst->set(*lcd->info.spi->gpios.rst);
    delay_tick(50);
    lcd->info.spi->gpios.rst->reset(*lcd->info.spi->gpios.rst);
    delay_tick(50);
    lcd->info.spi->gpios.rst->set(*lcd->info.spi->gpios.rst);
    delay_tick(50);
    lcd->info.on = 0;
    lcd->info.pri = (void *)osel_mem_alloc(sizeof(struct _cog_drv_data));
    uint8_t *p = (uint8_t *)lcd->info.pri;
    osel_memset(p, 0x00, (lcd->info.height / 8) * lcd->info.width);

    write_cmd(lcd->info.spi, ST_RESET); // 软件复位
    delay_ms(20);
    write_cmd(lcd->info.spi, ST_BOOST); // 内部升压
    delay_ms(20);
    write_cmd(lcd->info.spi, ST_DISP_OFF); // display Off
    lcd->info.on = 0;
    write_cmd(lcd->info.spi, 0xa0);
    delay_ms(20);
    write_cmd(lcd->info.spi, ST_SET_EV);
    write_cmd(lcd->info.spi, 0x79);

    write_cmd(lcd->info.spi, 0xeb);
    start_line(lcd->info.spi, 0); // display start line=0

    write_cmd(lcd->info.spi, ST_DISP_NORMAL);  // 正常显示,1-显示
    write_cmd(lcd->info.spi, ST_DISP_ALL_OFF); // 正常显示

    write_cmd(lcd->info.spi, ST_DISP_0); // 0度正常显示
    lcd->info.scandir = 0;

    // 应用层去清空并显示
    //  lcd->driver.color_fill(lcd, 0, lcd->info.width, 0, lcd->info.height, WHITE);
    // lcd->driver.onoff(lcd, 1);

    return 0;
}

/**
 *@brief      refresh_gram
 *@details:       刷新指定区域到屏幕上
                  坐标是横屏模式坐标
 *@param[in]   uint16_t sc
               uint16_t ec
               uint16_t sp
               uint16_t ep
 *@param[out]  无
 *@retval:     static
 */
static uint32_t refresh_gram(lcd_t *lcd, uint16_t sc, uint16_t ec, uint16_t sp, uint16_t ep)
{
    struct _cog_drv_data *drvdata;
    uint8_t i;

    drvdata = (struct _cog_drv_data *)lcd->info.pri;

    for (i = sp / 8; i <= ep / 8; i++)
    {
        // 设置页地址。每页是8行。一个画面的64行被分成8个页。我们平常所说的第1页，在LCD驱动IC里是第0页
        start_page(lcd->info.spi, i);
        // 设置列地址的高4位 和列地址的低4位
        start_column(lcd->info.spi, sc);

        write_data(lcd->info.spi, &(drvdata->gram[i][sc]), (ec - sc + 1));
    }
    return 0;
}

/**
 * @brief   画点
 * @param {lcd_t} *lcd
 * @param {uint16_t} x
 * @param {uint16_t} y
 * @param {uint16_t} color
 * @return {*}
 */
static int32_t _draw_point(lcd_t *lcd, uint16_t x, uint16_t y, uint16_t color)
{
    uint16_t xtmp, ytmp;
    uint16_t page, colum;

    struct _cog_drv_data *drvdata;

    drvdata = (struct _cog_drv_data *)lcd->info.pri;

    if (x > lcd->info.width)
        return -1;
    if (y > lcd->info.height)
        return -1;

    if (lcd->info.dir == W_LCD)
    {
        xtmp = x;
        ytmp = y;
    }
    else // 如果是竖屏，XY轴跟显存的映射要对调
    {
        xtmp = y;
        ytmp = lcd->info.width - 1 - x;
    }

    page = ytmp / 8; // 页地址
    colum = xtmp;    // 列地址

    if (color == BLACK)
    {
        drvdata->gram[page][colum] |= (0x01 << (ytmp % 8));
    }
    else
    {
        drvdata->gram[page][colum] &= ~(0x01 << (ytmp % 8));
    }

    // 设置页地址。每页是8行。一个画面的64行被分成8个页。我们平常所说的第1页，在LCD驱动IC里是第0页
    start_page(lcd->info.spi, page);
    // 设置列地址的高4位 和列地址的低4位
    start_column(lcd->info.spi, colum);

    write_data(lcd->info.spi, &drvdata->gram[page][colum], 1);

    return 0;
}

/**
 * @brief 将一块区域设定为某种颜色
 * @param {lcd_t} *lcd
 * @param {uint16_t} sx
 * @param {uint16_t} ex
 * @param {uint16_t} sy
 * @param {uint16_t} ey
 * @param {uint16_t} color
 * @return {*}
 */
static int32_t _color_fill(lcd_t *lcd, uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey, uint16_t color)
{
    uint16_t i, j;
    uint16_t xtmp, ytmp;
    uint16_t page, colum;

    struct _cog_drv_data *drvdata;

    drvdata = (struct _cog_drv_data *)lcd->info.pri;

    /*防止坐标溢出*/
    if (sy >= lcd->info.height)
    {
        sy = lcd->info.height - 1;
    }
    if (sx >= lcd->info.width)
    {
        sx = lcd->info.width - 1;
    }

    if (ey >= lcd->info.height)
    {
        ey = lcd->info.height - 1;
    }
    if (ex >= lcd->info.width)
    {
        ex = lcd->info.width - 1;
    }

    for (j = sy; j <= ey; j++)
    {
        for (i = sx; i <= ex; i++)
        {

            if (lcd->info.dir == W_LCD)
            {
                xtmp = i;
                ytmp = j;
            }
            else // 如果是竖屏，XY轴跟显存的映射要对调
            {
                xtmp = j;
                ytmp = lcd->info.width - 1 - i;
            }

            page = ytmp / 8; // 页地址
            colum = xtmp;    // 列地址

            if (color == BLACK)
            {
                drvdata->gram[page][colum] |= (0x01 << (ytmp % 8));
            }
            else
            {
                drvdata->gram[page][colum] &= ~(0x01 << (ytmp % 8));
            }
        }
    }

    /*
        只刷新需要刷新的区域
        坐标范围是横屏模式
    */
    if (lcd->info.dir == W_LCD)
    {
        refresh_gram(lcd, sx, ex, sy, ey);
    }
    else
    {
        refresh_gram(lcd, sy, ey, lcd->info.width - ex - 1, lcd->info.width - sx - 1);
    }
    return 0;
}

/**
 * @brief 填充矩形区域,这个函数是用来填充数据的
 * @param {lcd_t} *lcd
 * @param {uint16_t} sx
 * @param {uint16_t} ex
 * @param {uint16_t} sy
 * @param {uint16_t} ey
 * @param {uint16_t} *color
 * @return {*}
 */
static int32_t _fill(lcd_t *lcd, uint16_t x, uint16_t y, uint8_t font_width, uint8_t font_hight, uint8_t *buf)
{
    uint8_t i, j, n;
    n = y;
    for (i = 0; i < font_hight / 8; i++)
    {
        start_page(lcd->info.spi, x);
        for (j = 0; j < font_width; j++)
        {
            start_column(lcd->info.spi, y);
            write_data(lcd->info.spi, &buf[j + i * font_width], 1);
            y++;
        }
        x++;
        y = n;
    }
    return 0;
}

/**
 * @brief
 * @param {lcd_t} *lcd
 * @param {uint16_t} sx
 * @param {uint16_t} ex
 * @param {uint16_t} sy
 * @param {uint16_t} ey
 * @return {*}
 */
static int32_t _prepare_display(lcd_t *lcd, uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey)
{
    return 0;
}

/**
 * @brief
 * @param {lcd_t} *lcd
 * @param {uint16_t} *color
 * @param {uint32_t} len
 * @return {*}
 */
static void _flush(lcd_t *lcd)
{
}

/**
 * @brief 显示或关闭
 * @param {lcd_t} *lcd
 * @param {uint8_t} sta
 * @return {*}
 */
static int32_t _onoff(lcd_t *lcd, uint8_t sta)
{
    if (lcd->info.on != sta)
    {
        lcd->info.on = sta;
        if (sta == 1)
        {
            write_cmd(lcd->info.spi, ST_DISP_ON); // display On
        }
        else
        {
            write_cmd(lcd->info.spi, ST_DISP_OFF); // display Off
        }
    }

    return 0;
}

/**
 * @brief 设置显存扫描方向
 * @param {lcd_t} *lcd
 * @param {uint8_t} scan_dir 0 = 0度扫描，1 = 180度扫描
 * @return {*}
 */
void _set_dir(lcd_t *lcd, uint8_t scan_dir)
{
    if (lcd->info.scandir != scan_dir)
    {
        lcd->info.scandir = scan_dir;
        if (scan_dir == 0)
        {
            write_cmd(lcd->info.spi, ST_DISP_0); // 0度扫描
        }
        else
        {
            write_cmd(lcd->info.spi, ST_DISP_180); // 180度扫描
        }
    }
}

void _full_fill(lcd_t *lcd, uint16_t color)
{
    uint16_t i;
    uint8_t val;
    val = color > 0 ? 0x00 : 0xff;
    for (i = 0; i < lcd->info.height / 8; i++)
    {
        // 设置页地址。每页是8行。一个画面的64行被分成8个页。我们平常所说的第1页，在LCD驱动IC里是第0页
        start_page(lcd->info.spi, i);
        // 设置列地址的高4位 和列地址的低4位
        start_column(lcd->info.spi, 0);

        write_data(lcd->info.spi, &val, lcd->info.width);
    }
}

/**
 * @brief
 * @param {lcd_t} *lcd
 * @param {uint8_t} sta
 * @return {*}
 */
void _backlight(lcd_t *lcd, uint8_t sta) {}

void lcd_st7525_init(lcd_driver_t *driver)
{
    driver->init = _init;             // 已实现
    driver->set_point = _draw_point;  // 已实现
    driver->color_fill = _color_fill; // 已实现
    driver->fill = _fill;             // 已实现
    driver->prepare_display = _prepare_display;
    driver->flush = _flush;
    driver->onoff = _onoff; // 已实现
    driver->set_dir = _set_dir;
    driver->backlight = _backlight;
    driver->full_fill = _full_fill;
}
