/**
 * @file lcds.h
 * @author xxx
 * @date 2023-11-16 16:29:36
 * @brief
 * @copyright Copyright (c) 2023 by xxx, All Rights Reserved.
 */
/**
 * 16*16字体，宽高16*16，宋体、粗体、11
欢迎使用智能定位器版本江苏巨石数字技术有限公司选择操作短按长返回进入下一个上取消确认图表页面电流温度℃↑↓←→√×序列号硬件软固设备型安装方向行程类气动正反直角开关单双地址工模式自手测试非待机写保护启闭压力传感状态蓝牙输出块通讯显示语言中文英算法置恒控制变频速域整已未编辑首基准驱存储磁条小路源偏差摩擦弹簧率内累计次时间故障量
 */

/**
 * 24*24字体，宽高24*24，宋体、粗体、18
主菜单设备重启自检保存成功失败加载中置已取消欢迎使用智能定位器版本江苏巨石数字技术有限公司电流温度故障超出规格需要维护正常一键整信息厂阀门诊断行程特性测试输入密码错误编辑当前不满足动条件未知秒后返回首页选择项目查请求
 */

#ifndef __LCDS_H__
#define __LCDS_H__

#include "lib.h"
#include "spis.h"

#define USE_UCGUI (1) // 0：不使用；1:使用uc-gui

// 列点阵数
#define COL_DOT_MAX_128 128
#define COL_DOT_MAX_192 192
#define COL_DOT_MAX_400 400
// 行点阵数
#define LIN_DOT_MAX_64 64
#define LIN_DOT_MAX_240 240

#define H_LCD 0 // 竖屏
#define W_LCD 1 // 横屏

// 扫描方向定义
// BIT 0 标识LR，1 R-L，0 L-R
// BIT 1 标识UD，1 D-U，0 U-D
// BIT 2 标识LR/UD，1 DU-LR，0 LR-DU
// #define LR_BIT_MASK 0X01
// #define UD_BIT_MASK 0X02
// #define LRUD_BIT_MASK 0X04

// #define L2R_U2D (0)                             // 从左到右,从上到下
// #define L2R_D2U (0 + UD_BIT_MASK)               // 从左到右,从下到上
// #define R2L_U2D (0 + LR_BIT_MASK)               // 从右到左,从上到下
// #define R2L_D2U (0 + UD_BIT_MASK + LR_BIT_MASK) // 从右到左,从下到上

// #define U2D_L2R (LRUD_BIT_MASK)                             // 从上到下,从左到右
// #define U2D_R2L (LRUD_BIT_MASK + LR_BIT_MASK)               // 从上到下,从右到左
// #define D2U_L2R (LRUD_BIT_MASK + UD_BIT_MASK)               // 从下到上,从左到右
// #define D2U_R2L (LRUD_BIT_MASK + UD_BIT_MASK + LR_BIT_MASK) // 从下到上,从右到左

// 画笔颜色
/*
    对于黑白屏
    WHITE就是不显示，清空
    BLACK就是显示
*/
#define WHITE 0xFFFF
#define BLACK 0x0000

#define BLUE 0x001F
#define GREEN 0x07E0
#define RED 0xF800

#define BRED 0XF81F
#define GRED 0XFFE0
#define GBLUE 0X07FF
#define MAGENTA 0xF81F
#define CYAN 0x7FFF
#define YELLOW 0xFFE0
#define BROWN 0XBC40      // 棕色
#define BRRED 0XFC07      // 棕红色
#define GRAY 0X8430       // 灰色
#define DARKBLUE 0X01CF   // 深蓝色
#define LIGHTBLUE 0X7D7C  // 浅蓝色
#define GRAYBLUE 0X5458   // 灰蓝色
#define LIGHTGREEN 0X841F // 浅绿色
#define LIGHTGRAY 0XEF5B  // 浅灰色(PANNEL)
#define LGRAY 0XC618      // 浅灰色(PANNEL),窗体背景色
#define LGRAYBLUE 0XA651  // 浅灰蓝色(中间层颜色)
#define LBBLUE 0X2B12     // 浅棕蓝色(选择条目的反色)

#if !USE_UCGUI
// 对齐方式
#define GUI_TA_LEFT BIT1    // 左对齐
#define GUI_TA_HCENTER BIT2 // 居中对齐
#define GUI_TA_RIGHT BIT3   // 右对齐
#define GUI_TA_NORMOL BIT4  // 不对齐
// 显示模式
#define GUI_TEXTMODE_NORMAL BIT0    // 正常
#define GUI_TEXTMODE_REVERSE BIT1   // 反显
#define GUI_TEXTMODE_UNDERLINE BIT2 // 下线
#define GUI_TEXTMODE_XOR BIT3       //
#else
#include "GUI.h"
#define GUI_TA_NORMOL BIT4 // 不对齐
#endif

typedef enum
{
    LCD_ST7525,
    LCD_SHARP,
} lcd_type_e;

typedef enum
{
    FONT_SMALL,  // 16
    FONT_MEDIUM, // 24
    FONT_LARGE,  // 32
} font_type_e;

typedef enum
{
    TEMPLATE_COPY_FROM_PRI, // 拷贝pri到pri_template
    TEMPLATE_COPY_TO_PRI,   // 拷贝pri_template到pri
    TEMPLATE_COPY_CLEAR,    // 清空pri_template
} template_copy_type_e;     //  0,拷贝pri_template到pri；1，拷贝pri到pri_template; 其余清空

typedef struct LCDS lcd_t;
typedef struct
{
    lcd_type_e type;
    spi_t *spi;
    gpio_t *disp;

    /*驱动需要的变量*/
    uint8_t dir;     // 横屏还是竖屏控制：0，竖屏；1，横屏。
    uint16_t width;  // LCDS 宽度
    uint16_t height; // LCDS 高度

    // private:
    uint8_t on;      // 开关 0关，1开
    uint8_t scandir; // 扫描方向：0, 0度扫描，1， 180度扫描
    volatile BOOL need_build;
    BOOL clear_flag;    // 清屏标志
    void *pri;          // 私有数据，黑白屏跟OLED屏在初始化的时候会开辟显存
    void *pri_send;     // DMA发送时的私有数据
    void *pri_template; // 模板私有数据
    uint32_t flush_use_time;
} lcd_info_t;

typedef struct
{
    int32_t (*init)(lcd_t *lcd);

    int32_t (*set_point)(lcd_t *lcd, uint16_t x, uint16_t y, uint16_t color);                                  // 画点
    int32_t (*get_point)(lcd_t *lcd, uint16_t x, uint16_t y);                                                  // 读点
    int32_t (*color_fill)(lcd_t *lcd, uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey, uint16_t color);     // 颜色填充
    int32_t (*fill)(lcd_t *lcd, uint16_t x, uint16_t y, uint8_t font_width, uint8_t font_hight, uint8_t *buf); // 填充

    int32_t (*prepare_display)(lcd_t *lcd, uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey); // 准备显示
    void (*flush)(lcd_t *lcd);                                                                  // 刷新显示
    void (*flush_clear)(lcd_t *lcd);                                                            // 刷新显示并清空缓存

    int32_t (*onoff)(lcd_t *lcd, uint8_t sta);     // 开关
    void (*set_dir)(lcd_t *lcd, uint8_t scan_dir); // 设置扫描方向
    void (*backlight)(lcd_t *lcd, uint8_t sta);    // 背光控制
    void (*full_fill)(lcd_t *lcd, uint16_t color);
    int32_t (*clear)(lcd_t *lcd);                                      // 清屏
    void (*set_clear_flag)(lcd_t *lcd);                                // 设置清屏标志
    void (*copy_templete)(lcd_t *lcd, template_copy_type_e dir);       // 拷贝模板 0,拷贝pri_template到pri；1，拷贝pri到pri_template; 其余清空
    void (*clear_ram)(lcd_t *lcd, uint16_t min_row, uint16_t max_row); // 清除显存

    // 获取接口
    uint8_t (*get_dir)(lcd_t *lcd); // 获取扫描方向
    BOOL(*get_clear_flag)
    (lcd_t *lcd); // 获取清屏标志
} lcd_driver_t;

struct LCDS
{
    lcd_info_t info;
    lcd_driver_t driver;
};

extern lcd_t *lcd_create(lcd_info_t info);
extern void lcd_free(lcd_t *handle);
extern void lcd_set_font_size(font_type_e font);
extern int32_t lcd_put_alignment_string(lcd_t *lcd, font_type_e font, int32_t row, char *s, uint8_t alignment, BOOL reverse, uint16_t color); // 对齐显示字符串
#endif                                                                                                                                        // __LCDS_H__
