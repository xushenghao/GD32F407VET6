#ifndef __MENU_H__
#define __MENU_H__

#include "lib.h"

/******************************************* 配置项 ********************************************************************/

/* 定义 _MENU_USE_MALLOC_ 则采用 MALLOC/FREE 的方式实现多级菜单, 否则通过数组的形式 */
// #define _MENU_USE_MALLOC_

/* 定义 _MENU_USE_SHORTCUT_ 则启用快捷菜单选项进入功能 */
#define _MENU_USE_SHORTCUT_

/* 多级菜单深度 */
#define MENU_MAX_DEPTH 5

/* 菜单支持的最大选项数目 */
#define MENU_MAX_NUM 40

/* 菜单支持的语种数目 */
#define MENU_SUPPORT_LANGUAGE 2

/******************************************* 配置项 ********************************************************************/

/* exported types ----------------------------------------------------------------------------------------------------*/

#if MENU_MAX_NUM < 255
typedef uint8_t menusize_t;
#else
typedef uint16_t menusize_t;
#endif

typedef void (*menu_call_fun_f)(void);

typedef struct
{
    menusize_t items_num; /*!< 当前菜单中选项的总数目 */

    menusize_t select_item; /*!< 当前菜单中被选中的选项 */

    menusize_t show_base_item; /*!< 当前菜单首个显示的选项 */

    uint8_t page_no; /*!< 当前菜单的页码 */

    char *psz_desc; /*!< 当前菜单的字符串描述 */

    char *psz_items_desc[MENU_MAX_NUM]; /*!< 当前菜单中所有选项的字符串描述 */

    void *p_items_ex_data[MENU_MAX_NUM]; /*!< 当前菜单中所有选项注册时的扩展数据 */
} menu_show_t;

typedef void (*showmenu_call_fun_f)(menu_show_t *pt_show_info);

typedef struct
{
    const char *(psz_desc[MENU_SUPPORT_LANGUAGE]);
} menu_txt_t;

/**
 * @brief 菜单信息注册结构体
 *
 */
typedef struct
{
    uint16_t window_no; /*!< 当前菜单的窗口号 */

    BOOL single_page; /*!< 当前菜单是否为单页 */

    const char *(psz_desc[MENU_SUPPORT_LANGUAGE]); /*!< 当前选项的字符串描述(多语种) */

    menu_call_fun_f pfn_enter_call_fun; /*!< 当前菜单选项进入时(从父菜单进入)需要执行一次的函数, 为null不执行 */

    menu_call_fun_f pfn_exit_call_fun; /*!< 当前菜单选项进入后退出时(退出至父菜单)需要执行一次的函数, 为null不执行 */

    menu_call_fun_f pfn_load_call_fun; /*!< 当前菜单选项每次加载时(从父菜单进入或子菜单退出)需要执行一次的函数, 为null不执行 */

    menu_call_fun_f pfn_run_call_fun; /*!< 当前菜单选项的周期调度函数 */

    void *p_extend_data; /*!< 当前选项的菜单显示效果函数扩展数据入参, 可自行设置该内容 */
} menu_list_t, menu_item_t;

/**
 * @brief 菜单信息注册结构体
 *
 */
typedef struct
{
    const char *(psz_desc[MENU_SUPPORT_LANGUAGE]); /*!< 当前选项的字符串描述(多语种) */

    menu_call_fun_f pfn_enter_call_fun; /*!< 主前菜单进入时(进入菜单)需要执行一次的函数, 为null不执行 */

    menu_call_fun_f pfn_exit_call_fun; /*!< 主前菜单进入后退出时(退出菜单)需要执行一次的函数, 为null不执行 */

    menu_call_fun_f pfn_load_call_fun; /*!< 主菜单每次加载时需要执行一次的函数, 为null不执行 */

    menu_call_fun_f pfn_run_call_fun; /*!< 主菜单周期调度函数 */
} main_menu_cfg_t;

/* exported constants ------------------------------------------------------------------------------------------------*/
/* exported macro ----------------------------------------------------------------------------------------------------*/

#define COT_GET_MENU_NUM(x) (sizeof(x) / sizeof(menu_list_t))

/* exported functions ------------------------------------------------------------------------------------------------*/

/* 菜单初始化和反初始化 */

extern BOOL menu_init(const main_menu_cfg_t *p_main_menu);
extern BOOL menu_de_init(void);

extern BOOL menu_unbind(void);

extern BOOL menu_bind(uint16_t parent_window_no, const menu_list_t *p_menu_list, menusize_t menu_num, showmenu_call_fun_f pfn_show_menu_fun);

/* 菜单功能设置 */

extern BOOL menu_select_language(uint8_t language_idx);

/* 菜单选项显示时需要使用的功能扩展函数 */

extern BOOL menu_limit_show_list_num(menu_show_t *pt_menu_show, menusize_t *p_show_num);
extern BOOL menu_query_parent_menu(menu_show_t *pt_menu_show, uint8_t level);

/* 菜单操作 */

/**
 * @brief 进入主菜单
 * @return 进入成功返回TRUE，否则返回FALSE
 */
extern BOOL menu_main_enter(void);

/**
 * @brief 退出主菜单
 * @return 退出成功返回TRUE，否则返回FALSE
 */
extern BOOL menu_main_exit(void);

/**
 * @brief 重置菜单
 * @return 重置成功返回TRUE，否则返回FALSE
 */
extern BOOL menu_reset(void);

/**
 * @brief 进入菜单
 * @param p 指向菜单的指针
 * @return 进入成功返回TRUE，否则返回FALSE
 */
extern BOOL menu_enter(void *p);

/**
 * @brief 退出菜单
 * @param is_reset 是否重置菜单
 * @return 退出成功返回TRUE，否则返回FALSE
 */
extern BOOL menu_exit(BOOL is_reset);

/**
 * @brief 选择上一个菜单项
 * @param is_allow_roll 是否允许循环选择
 * @return 选择成功返回TRUE，否则返回FALSE
 */
extern BOOL menu_select_previous(BOOL is_allow_roll);

/**
 * @brief 选择上一个菜单页
 * @param {uint8_t} show_num 每页菜单项数目
 * @return {*}
 * @note
 */
extern BOOL menu_select_previous_page(uint8_t show_num);

/**
 * @brief 选择下一个菜单项
 * @param is_allow_roll 是否允许循环选择
 * @return 选择成功返回TRUE，否则返回FALSE
 */
extern BOOL menu_select_next(BOOL is_allow_roll);

/**
 * @brief 选择下一个菜单页
 * @param {uint8_t} show_num 每页菜单项数目
 * @return {*}
 * @note
 */
extern BOOL menu_select_next_page(uint8_t show_num);

/**
 * @brief 跳转到指定菜单页
 * @param {uint8_t} index 菜单项
 * @return {*}
 * @note
 */
extern BOOL menu_jump_item(uint8_t index);
/**
 * @brief 进入菜单快捷方式
 *
 * 该函数用于进入菜单的快捷方式。
 *
 * @param is_absolute 是否为绝对路径
 * @param deep 菜单路径的深度
 * @param ... 菜单路径的参数列表
 * @return BOOL 进入菜单是否成功
 */
extern BOOL menu_shortcut_enter(BOOL is_absolute, uint8_t deep, ...);

/**
 * @brief 获取菜单描述的最大长度
 *
 * 该函数用于获取菜单描述的最大长度。
 *
 * @param pt_show_info 菜单显示信息的指针
 * @return uint8_t 菜单描述的最大长度
 */
extern uint8_t menu_psz_desc_max_size(menu_show_t *pt_show_info);

/**
 * @brief 显示菜单文本
 *
 * 该函数用于显示菜单的文本。
 *
 * @param buf 存储菜单文本的缓冲区
 * @param m_txt 菜单文本的指针
 */
extern void menu_txt_show(char *buf, const menu_txt_t *m_txt);

/**
 * @brief 进入指定窗口的菜单
 *
 * 该函数用于进入指定窗口的菜单。
 *
 * @param no 窗口编号
 * @return BOOL 进入菜单是否成功
 */
extern BOOL menu_enter_with_window_no(uint8_t no);

/**
 * @brief 获取当前父窗口的编号
 *
 * 该函数用于获取当前父窗口的编号。
 *
 * @return uint16_t 当前父窗口的编号
 */
extern uint16_t menu_current_parent_window_no(void);

/**
 * @brief 获取当前窗口的编号
 *
 * 该函数用于获取当前窗口的编号。
 *
 * @return uint16_t 当前窗口的编号
 */
extern uint16_t menu_current_window_no(void);

/**
 * @brief 获取当前窗口的信息
 *
 * 该函数用于获取当前窗口的信息。
 *
 * @param info 存储当前窗口信息的指针
 * @return BOOL 获取当前窗口信息是否成功
 */
extern BOOL menu_get_current_window_info(menu_show_t *info);
/* 菜单轮询处理任务 */

extern BOOL menu_task(void);

#endif // __MENU_H__
