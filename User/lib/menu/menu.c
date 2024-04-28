#include "menu.h"
#include <string.h>
#include <stdio.h>
#ifdef _COT_MENU_USE_MALLOC_
#include <malloc.h>
#endif

#ifdef _COT_MENU_USE_SHORTCUT_
#include <stdarg.h>
#endif

/* private typedef ---------------------------------------------------------------------------------------------------*/
typedef struct menu_ctrl
{
    uint16_t parent_window_no;               /*!< 父菜单的窗口号 */
    struct menu_ctrl *p_parent_menu_ctrl;    /*!< 父菜单控制处理 */
    char *(psz_desc[MENU_SUPPORT_LANGUAGE]); /*!< 当前选项的字符串描述(多语种) */
    showmenu_call_fun_f pfn_show_menu_fun;   /*!< 当前菜单显示效果函数 */
    menu_list_t *p_menu_list;                /*!< 当前菜单列表 */
    menu_call_fun_f pfn_load_call_fun;       /*!< 当前菜单加载函数 */
    menu_call_fun_f pfn_run_call_fun;        /*!< 当前选项的非菜单功能函数 */
    menusize_t items_num;                    /*!< 当前菜单选项总数目 */
    menusize_t show_base_item;               /*!< 当前菜单首个显示的选项 */
    menusize_t select_item;                  /*!< 当前菜单选中的选项 */
    BOOL is_selected;                        /*!< 菜单选项是否已经被选择 */
} menu_ctrl_t;

typedef struct
{
    menu_ctrl_t *p_menu_ctrl;                /*!< 当前菜单控制处理 */
    menu_call_fun_f pfn_main_enter_call_fun; /*!< 主菜单进入时(进入菜单)需要执行一次的函数 */
    menu_call_fun_f pfn_main_exit_call_fun;  /*!< 主菜单进入后退出时(退出菜单)需要执行一次的函数 */
    menu_call_fun_f pfn_load_call_fun;       /*!< 重加载函数 */
    uint8_t language;                        /*!< 语种选择 */
    BOOL is_enter_main_menu : TRUE;          /*!< 是否进入了主菜单 */
} menu_manage_t;

/* private define ----------------------------------------------------------------------------------------------------*/
/* private macro -----------------------------------------------------------------------------------------------------*/
/* private variables -------------------------------------------------------------------------------------------------*/
static menu_manage_t sg_t_menu_manage;

#ifndef _menu_use_malloc_
static menu_ctrl_t sg_arr_menu_ctrl[MENU_MAX_DEPTH];
#endif

static uint8_t sg_curr_menu_depth = 0;

/* private function prototypes ---------------------------------------------------------------------------------------*/
static menu_ctrl_t *new_menu(void);
static void delete_menu(menu_ctrl_t *p_menu);

/* private function --------------------------------------------------------------------------------------------------*/
/**
 * @brief      新建菜单层级
 *
 * @return     menu_ctrl_t*
 */
static menu_ctrl_t *new_menu(void)
{
    menu_ctrl_t *p_menu_ctrl = NULL;

    if (sg_curr_menu_depth < MENU_MAX_DEPTH)
    {
#ifdef _menu_use_malloc_
        p_menu_ctrl = (menu_ctrl_t *)malloc(sizeof(menu_ctrl_t));
#else
        p_menu_ctrl = &sg_arr_menu_ctrl[sg_curr_menu_depth];
#endif
        sg_curr_menu_depth++;
    }

    return p_menu_ctrl;
}

/**
 * @brief      销毁菜单层级
 *
 * @param      p_menu
 */
static void delete_menu(menu_ctrl_t *p_menu)
{
#ifdef _menu_use_malloc_
    free(p_menu);
#endif
    if (sg_curr_menu_depth > 0)
    {
        sg_curr_menu_depth--;
    }
}

/**
 * @brief 解绑当前菜单
 * @return {*}
 * @note
 */
BOOL menu_unbind(void)
{
    if (sg_t_menu_manage.p_menu_ctrl == NULL)
    {
        return FALSE;
    }

    delete_menu(sg_t_menu_manage.p_menu_ctrl);
    return TRUE;
}

/**
 * @brief      菜单初始化
 *
 * @param[in]  p_main_menu        主菜单注册信息
 * @return     TRUE:成功；FALSE:失败
 */
BOOL menu_init(const main_menu_cfg_t *p_main_menu)
{
    int i;
    menu_ctrl_t *p_new_menu_ctrl = NULL;

    if (sg_t_menu_manage.p_menu_ctrl != NULL)
    {
        return FALSE;
    }

#if MENU_MAX_DEPTH != 0
    sg_curr_menu_depth = 0;
#endif

    if ((p_new_menu_ctrl = new_menu()) == NULL)
    {
        return FALSE;
    }

    sg_t_menu_manage.language = 0;

    for (i = 0; i < MENU_SUPPORT_LANGUAGE; i++)
    {
        p_new_menu_ctrl->psz_desc[i] = (char *)p_main_menu->psz_desc[i];
    }

    p_new_menu_ctrl->p_parent_menu_ctrl = NULL;
    p_new_menu_ctrl->pfn_load_call_fun = p_main_menu->pfn_load_call_fun;
    p_new_menu_ctrl->pfn_show_menu_fun = NULL;
    p_new_menu_ctrl->pfn_run_call_fun = p_main_menu->pfn_run_call_fun;

    p_new_menu_ctrl->p_menu_list = NULL;
    p_new_menu_ctrl->items_num = 0;
    p_new_menu_ctrl->select_item = 0;
    p_new_menu_ctrl->show_base_item = 0;

    sg_t_menu_manage.p_menu_ctrl = p_new_menu_ctrl;
    sg_t_menu_manage.is_enter_main_menu = 0;
    sg_t_menu_manage.pfn_main_enter_call_fun = p_main_menu->pfn_enter_call_fun;
    sg_t_menu_manage.pfn_main_exit_call_fun = p_main_menu->pfn_exit_call_fun;
    sg_t_menu_manage.pfn_load_call_fun = p_new_menu_ctrl->pfn_load_call_fun;

    return TRUE;
}

/**
 * @brief  菜单反初始化
 *
 * @attention  不管处于任何界面都会逐级退出到主菜单后（会调用退出函数），再退出主菜单，最后反初始化
 * @return TRUE:成功；FALSE:失败
 */
BOOL menu_de_init(void)
{
    if (sg_t_menu_manage.p_menu_ctrl == NULL)
    {
        return FALSE;
    }

    menu_main_exit();

    delete_menu(sg_t_menu_manage.p_menu_ctrl);
    sg_t_menu_manage.p_menu_ctrl = NULL;
    sg_t_menu_manage.language = 0;
    sg_t_menu_manage.is_enter_main_menu = 0;
    sg_t_menu_manage.pfn_main_enter_call_fun = NULL;
    sg_t_menu_manage.pfn_main_exit_call_fun = NULL;
    sg_t_menu_manage.pfn_load_call_fun = NULL;

    return TRUE;
}

/**
 * @brief      子菜单绑定当前菜单选项
 *
 * @param      parent_window_no 父菜单选项的窗口号
 * @param      p_menu_list       新的菜单列表
 * @param      menu_num         新的菜单列表数目
 * @param      pfn_show_menu_fun  新的菜单列表显示效果回调函数, 为NULL则延续上级菜单显示效果
 * @return     BOOL
 */
BOOL menu_bind(uint16_t parent_window_no, const menu_list_t *p_menu_list, menusize_t menu_num, showmenu_call_fun_f pfn_show_menu_fun)
{
    if (sg_t_menu_manage.p_menu_ctrl == NULL)
    {
        return FALSE;
    }

    if (sg_t_menu_manage.p_menu_ctrl->p_menu_list != NULL)
    {
        return TRUE;
    }

    sg_t_menu_manage.p_menu_ctrl->p_menu_list = (menu_list_t *)p_menu_list;
    sg_t_menu_manage.p_menu_ctrl->items_num = menu_num;
    sg_t_menu_manage.p_menu_ctrl->parent_window_no = parent_window_no;

    if (pfn_show_menu_fun != NULL)
    {
        sg_t_menu_manage.p_menu_ctrl->pfn_show_menu_fun = pfn_show_menu_fun;
    }

    return TRUE;
}

/**
 * @brief      选择语种
 *
 * @param[in]  language_idx 语种索引
 * @return     TRUE:成功；FALSE:失败
 */
BOOL menu_select_language(uint8_t language_idx)
{
    if (MENU_SUPPORT_LANGUAGE <= language_idx)
    {
        return FALSE;
    }

    sg_t_menu_manage.language = language_idx;
    return TRUE;
}

/**
 * @brief      复位菜单, 回到主菜单界面
 *
 * @note       该复位回到主菜单不会执行退出所需要执行的回调函数
 * @return     TRUE:成功；FALSE:失败
 */
BOOL menu_reset(void)
{
    if (sg_t_menu_manage.p_menu_ctrl == NULL || sg_t_menu_manage.is_enter_main_menu == 0)
    {
        return FALSE;
    }

    while (sg_t_menu_manage.p_menu_ctrl->p_parent_menu_ctrl != NULL)
    {
        menu_ctrl_t *p_menu_ctrl = sg_t_menu_manage.p_menu_ctrl;

        sg_t_menu_manage.p_menu_ctrl = sg_t_menu_manage.p_menu_ctrl->p_parent_menu_ctrl;
        delete_menu(p_menu_ctrl);
    }

    sg_t_menu_manage.p_menu_ctrl->select_item = 0;

    return TRUE;
}

/**
 * @brief      主菜单进入
 *
 * @return     TRUE:成功；FALSE:失败
 */
BOOL menu_main_enter(void)
{
    if (sg_t_menu_manage.p_menu_ctrl == NULL || sg_t_menu_manage.is_enter_main_menu == 1)
    {
        return FALSE;
    }

    if (sg_t_menu_manage.pfn_main_enter_call_fun != NULL)
    {
        sg_t_menu_manage.pfn_main_enter_call_fun();
    }

    sg_t_menu_manage.is_enter_main_menu = 1;
    sg_t_menu_manage.pfn_load_call_fun = sg_t_menu_manage.p_menu_ctrl->pfn_load_call_fun;

    return TRUE;
}

/**
 * @brief      主菜单退出
 *
 * @attention  不管处于任何界面都会逐级退出到主菜单后（会调用退出函数），再退出主菜单
 * @return     TRUE:成功；FALSE:失败
 */
BOOL menu_main_exit(void)
{
    if (sg_t_menu_manage.p_menu_ctrl == NULL || sg_t_menu_manage.is_enter_main_menu == 0)
    {
        return FALSE;
    }

    while (menu_exit(1) == 0)
    {
    }

    if (sg_t_menu_manage.pfn_main_exit_call_fun != NULL)
    {
        sg_t_menu_manage.pfn_main_exit_call_fun();
    }

    sg_t_menu_manage.is_enter_main_menu = 0;

    return TRUE;
}

/**
 * @brief      进入当前菜单选项
 *
 * @param[in]  p 传递给进入函数的参数 != NULL 执行进入函数
 * @return     TRUE:成功；FALSE:失败
 */
BOOL menu_enter(void *p)
{
    int i;
    menu_ctrl_t *p_new_menu_ctrl = NULL;
    menu_ctrl_t *p_curr_menu_ctrl = sg_t_menu_manage.p_menu_ctrl;

    if (sg_t_menu_manage.p_menu_ctrl == NULL || sg_t_menu_manage.is_enter_main_menu == 0)
    {
        return FALSE;
    }

    if ((p_new_menu_ctrl = new_menu()) == NULL)
    {
        return FALSE;
    }

    for (i = 0; i < MENU_SUPPORT_LANGUAGE; i++)
    {
        p_new_menu_ctrl->psz_desc[i] = (char *)p_curr_menu_ctrl->p_menu_list[p_curr_menu_ctrl->select_item].psz_desc[i];
    }

    p_new_menu_ctrl->p_menu_list = NULL;
    p_new_menu_ctrl->items_num = 0;
    p_new_menu_ctrl->pfn_show_menu_fun = p_curr_menu_ctrl->pfn_show_menu_fun;
    p_new_menu_ctrl->pfn_load_call_fun = p_curr_menu_ctrl->p_menu_list[p_curr_menu_ctrl->select_item].pfn_load_call_fun;
    p_new_menu_ctrl->pfn_run_call_fun = p_curr_menu_ctrl->p_menu_list[p_curr_menu_ctrl->select_item].pfn_run_call_fun;
    p_new_menu_ctrl->select_item = 0;
    p_new_menu_ctrl->is_selected = TRUE;
    p_new_menu_ctrl->p_parent_menu_ctrl = p_curr_menu_ctrl;

    sg_t_menu_manage.p_menu_ctrl = p_new_menu_ctrl;
    sg_t_menu_manage.pfn_load_call_fun = p_new_menu_ctrl->pfn_load_call_fun;

    if (p_curr_menu_ctrl->p_menu_list[p_curr_menu_ctrl->select_item].pfn_enter_call_fun != NULL)
    {
        p_curr_menu_ctrl->p_menu_list[p_curr_menu_ctrl->select_item].pfn_enter_call_fun();
    }

    return TRUE;
}

/**
 * @brief      退出当前选项并返回上一层菜单
 *
 * @param[in]  is_reset 菜单选项是否从头选择
 * @return     TRUE:成功；FALSE:失败, 即目前处于主菜单, 无法返回
 */
BOOL menu_exit(BOOL is_reset)
{
    menu_ctrl_t *p_menu_ctrl = sg_t_menu_manage.p_menu_ctrl;

    if (sg_t_menu_manage.p_menu_ctrl == NULL || sg_t_menu_manage.is_enter_main_menu == 0)
    {
        return FALSE;
    }

    if (sg_t_menu_manage.p_menu_ctrl->p_parent_menu_ctrl == NULL)
    {
        return FALSE;
    }

    sg_t_menu_manage.p_menu_ctrl = sg_t_menu_manage.p_menu_ctrl->p_parent_menu_ctrl;
    sg_t_menu_manage.pfn_load_call_fun = sg_t_menu_manage.p_menu_ctrl->pfn_load_call_fun;
    delete_menu(p_menu_ctrl);
    p_menu_ctrl = NULL;

    if (sg_t_menu_manage.p_menu_ctrl->p_menu_list[sg_t_menu_manage.p_menu_ctrl->select_item].pfn_exit_call_fun != NULL)
    {
        sg_t_menu_manage.p_menu_ctrl->is_selected = FALSE;
        sg_t_menu_manage.p_menu_ctrl->p_menu_list[sg_t_menu_manage.p_menu_ctrl->select_item].pfn_exit_call_fun();
    }

    if (is_reset)
    {
        sg_t_menu_manage.p_menu_ctrl->select_item = 0;
    }

    return TRUE;
}

/**
 * @brief      选择上一个菜单选项
 *
 * @param[in]  is_allow_roll 第一个选项时是否从跳转到最后一个选项
 * @return     TRUE:成功；FALSE:失败
 */
BOOL menu_select_previous(BOOL is_allow_roll)
{
    if (sg_t_menu_manage.p_menu_ctrl == NULL || sg_t_menu_manage.is_enter_main_menu == 0)
    {
        return FALSE;
    }

    if (sg_t_menu_manage.p_menu_ctrl->select_item > 0)
    {
        sg_t_menu_manage.p_menu_ctrl->select_item--;
    }
    else
    {
        if (is_allow_roll)
        {
            sg_t_menu_manage.p_menu_ctrl->select_item = sg_t_menu_manage.p_menu_ctrl->items_num - 1;
        }
        else
        {
            sg_t_menu_manage.p_menu_ctrl->select_item = 0;
            return FALSE;
        }
    }

    return TRUE;
}

/**
 * @brief 选择菜单的上一页
 * @param[in]  show_num 每页菜单项数目
 * @return {*}
 * @note
 */
BOOL menu_select_previous_page(uint8_t show_num)
{
    if (sg_t_menu_manage.p_menu_ctrl == NULL || sg_t_menu_manage.is_enter_main_menu == 0)
    {
        return FALSE;
    }
    uint8_t page_no = sg_t_menu_manage.p_menu_ctrl->select_item / show_num;
    if (page_no > 0)
    {
        sg_t_menu_manage.p_menu_ctrl->select_item = (page_no - 1) * show_num;
        sg_t_menu_manage.p_menu_ctrl->show_base_item = 0;
    }
    else
    {
        sg_t_menu_manage.p_menu_ctrl->select_item = ((sg_t_menu_manage.p_menu_ctrl->items_num - 1) / show_num) * show_num;
        sg_t_menu_manage.p_menu_ctrl->show_base_item = 0;
    }
    return TRUE;
}

/**
 * @brief      选择下一个菜单选项
 *
 * @param[in]  is_allow_roll 最后一个选项时是否跳转到第一个选项
 * @return     TRUE:成功；FALSE:失败
 */
BOOL menu_select_next(BOOL is_allow_roll)
{
    if (sg_t_menu_manage.p_menu_ctrl == NULL || sg_t_menu_manage.is_enter_main_menu == 0)
    {
        return FALSE;
    }

    if (sg_t_menu_manage.p_menu_ctrl->select_item < (sg_t_menu_manage.p_menu_ctrl->items_num - 1))
    {
        sg_t_menu_manage.p_menu_ctrl->select_item++;
    }
    else
    {
        if (is_allow_roll)
        {
            sg_t_menu_manage.p_menu_ctrl->select_item = 0;
        }
        else
        {
            sg_t_menu_manage.p_menu_ctrl->select_item = sg_t_menu_manage.p_menu_ctrl->items_num - 1;
            return FALSE;
        }
    }

    return TRUE;
}

/**
 * @brief 选择菜单的下一页
 * @param[in]  show_num 每页菜单项数目
 * @return {*}
 * @note
 */
BOOL menu_select_next_page(uint8_t show_num)
{
    if (sg_t_menu_manage.p_menu_ctrl == NULL || sg_t_menu_manage.is_enter_main_menu == 0)
    {
        return FALSE;
    }
    uint8_t page_no = sg_t_menu_manage.p_menu_ctrl->select_item / show_num;
    if (page_no < ((sg_t_menu_manage.p_menu_ctrl->items_num - 1) / show_num))
    {
        sg_t_menu_manage.p_menu_ctrl->select_item = (page_no + 1) * show_num;
        sg_t_menu_manage.p_menu_ctrl->show_base_item = 0;
    }
    else
    {
        sg_t_menu_manage.p_menu_ctrl->select_item = 0;
        sg_t_menu_manage.p_menu_ctrl->show_base_item = 0;
    }
    return TRUE;
}

/**
 * @brief      跳转菜单项
 *
 * @param[in]  index 菜单项
 * @return     TRUE:成功；FALSE:失败
 */
BOOL menu_jump_item(uint8_t index)
{
    if (sg_t_menu_manage.p_menu_ctrl == NULL || sg_t_menu_manage.is_enter_main_menu == 0)
    {
        return FALSE;
    }
    sg_t_menu_manage.p_menu_ctrl->select_item = index;
    sg_t_menu_manage.p_menu_ctrl->show_base_item = 0;
    return TRUE;
}

#ifdef _menu_use_shortcut_

/**
 * @brief      相对主菜单或当前菜单通过下级各菜单索引快速进入指定选项
 *
 * @param[in]  is_absolute 是否采用绝对菜单索引（从主菜单开始）
 * @param[in]  deep 菜单深度，大于 0
 * @param[in]  ...  各级菜单索引值(从0开始), 入参个数由 deep 的值决定
 * @return     TRUE:成功；FALSE:失败
 */
BOOL menu_shortcut_enter(BOOL is_absolute, uint8_t deep, ...)
{
    uint8_t select_deep = 0;
    va_list p_item_list;
    menusize_t select_item;

    if (sg_t_menu_manage.p_menu_ctrl == NULL || sg_t_menu_manage.is_enter_main_menu == 0)
    {
        return FALSE;
    }

    if (is_absolute)
    {
        menu_reset();
    }

    va_start(p_item_list, deep);

    while (select_deep < deep)
    {
        select_item = va_arg(p_item_list, int);

        if (select_item >= sg_t_menu_manage.p_menu_ctrl->items_num)
        {
            va_end(p_item_list);
            return FALSE;
        }

        sg_t_menu_manage.p_menu_ctrl->select_item = select_item;
        menu_enter(NULL);
        select_deep++;
    }

    va_end(p_item_list);

    return TRUE;
}

#endif

/**
 * @brief      限制当前菜单界面最多显示的菜单数目
 *
 * @note       在菜单显示效果回调函数中使用, 使用成员变量 show_base_item 得到显示界面的第一个选项索引
 * @param[in,out]  t_menu_show   当前菜单显示信息
 * @param[in,out]  show_num     当前菜单中需要显示的选项数目, 根据当前菜单选项的总数得到最终的显示的选项数目
 * @return     TRUE:成功；FALSE:失败
 */
BOOL menu_limit_show_list_num(menu_show_t *pt_menu_show, menusize_t *p_show_num)
{
    if (pt_menu_show == NULL || p_show_num == NULL)
    {
        return FALSE;
    }

    if (*p_show_num > pt_menu_show->items_num)
    {
        *p_show_num = pt_menu_show->items_num;
    }

    if (pt_menu_show->select_item < pt_menu_show->show_base_item)
    {
        pt_menu_show->show_base_item = pt_menu_show->select_item;
    }
    else if ((pt_menu_show->select_item - pt_menu_show->show_base_item) >= *p_show_num)
    {
        pt_menu_show->show_base_item = pt_menu_show->select_item - *p_show_num + 1;
    }
    else
    {
        // 保持
    }
    pt_menu_show->page_no = pt_menu_show->select_item / *p_show_num;
    return TRUE;
}

/**
 * @brief       获取当前父菜单显示信息
 *              如获取当前菜单的二级父菜单信息，level 为2
 *
 * @param[out]  pt_menu_show 父 n 级菜单显示信息
 * @param[in]   level      n 级, 大于 0
 * @return int
 */
BOOL menu_query_parent_menu(menu_show_t *pt_menu_show, uint8_t level)
{
    int i;
    menu_list_t *p_menu;
    menu_ctrl_t *p_menu_ctrl = NULL;

    if (sg_t_menu_manage.p_menu_ctrl == NULL || sg_t_menu_manage.is_enter_main_menu == 0)
    {
        return FALSE;
    }

    p_menu_ctrl = sg_t_menu_manage.p_menu_ctrl->p_parent_menu_ctrl;

    while (level && p_menu_ctrl != NULL)
    {
        p_menu = p_menu_ctrl->p_menu_list;
        pt_menu_show->items_num = p_menu_ctrl->items_num;
        pt_menu_show->select_item = p_menu_ctrl->select_item;
        pt_menu_show->show_base_item = p_menu_ctrl->show_base_item;

        pt_menu_show->psz_desc = sg_t_menu_manage.p_menu_ctrl->psz_desc[sg_t_menu_manage.language];

        for (i = 0; i < pt_menu_show->items_num && i < MENU_MAX_NUM; i++)
        {
            pt_menu_show->psz_items_desc[i] = (char *)p_menu[i].psz_desc[sg_t_menu_manage.language];
            pt_menu_show->p_items_ex_data[i] = p_menu[i].p_extend_data;
        }

        p_menu_ctrl = p_menu_ctrl->p_parent_menu_ctrl;
        level--;
    }

    if (level != 0 && p_menu_ctrl == NULL)
    {
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief      获取当前菜单选项的描述字符串最大长度
 *
 * @param[in]  pt_show_info 当前菜单显示信息
 * @return     uint8_t
 */
uint8_t menu_psz_desc_max_size(menu_show_t *pt_show_info)
{
    uint8_t i;
    uint8_t max_size = 0;

    if (pt_show_info == NULL)
    {
        return 0;
    }

    for (i = 0; i < pt_show_info->items_num; i++)
    {
        if (strlen(pt_show_info->psz_items_desc[i]) > max_size)
        {
            max_size = strlen(pt_show_info->psz_items_desc[i]);
        }
    }

    return max_size;
}

/**
 * @brief 获取文本信息
 * @param {char} *s 返回的文本
 * @param {menu_txt_t} *m_txt 文本内容
 * @return {*}
 * @note
 */
void menu_txt_show(char *buf, const menu_txt_t *m_txt)
{
    DBG_ASSERT(buf != NULL __DBG_LINE);
    DBG_ASSERT(m_txt != NULL __DBG_LINE);
    sprintf(buf, "%s", m_txt->psz_desc[sg_t_menu_manage.language]);
}

/**
 * @brief  通过当前窗口编号进入菜单
 *
 * @param[in]  no 选项号
 */
BOOL menu_enter_with_window_no(uint8_t no)
{
    if (sg_t_menu_manage.p_menu_ctrl == NULL || sg_t_menu_manage.p_menu_ctrl->items_num == 0)
    {
        return FALSE;
    }

    for (uint8_t i = 0; i < sg_t_menu_manage.p_menu_ctrl->items_num; i++)
    {
        if (sg_t_menu_manage.p_menu_ctrl->p_menu_list[i].window_no == no)
        {
            sg_t_menu_manage.p_menu_ctrl->select_item = i;
            menu_enter(NULL);
            return TRUE;
        }
        else
        {
            if (sg_t_menu_manage.p_menu_ctrl->p_menu_list[i].window_no != 0 &&
                sg_t_menu_manage.p_menu_ctrl->p_menu_list[i].single_page != TRUE)
            {
                sg_t_menu_manage.p_menu_ctrl->select_item = i;

                menu_enter(NULL);
                if (menu_enter_with_window_no(no) == FALSE)
                {
                    menu_exit(FALSE);
                }
                else
                {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}

/**
 * @brief  获取当前窗口号
 *
 * @return uint16_t
 */
uint16_t menu_current_window_no(void)
{
    if (sg_t_menu_manage.p_menu_ctrl == NULL)
    {
        return 0;
    }

    return sg_t_menu_manage.p_menu_ctrl->p_menu_list[sg_t_menu_manage.p_menu_ctrl->select_item].window_no;
}

/**
 * @brief  获取当前父窗口号
 *
 * @return uint16_t
 */
uint16_t menu_current_parent_window_no(void)
{
    if (sg_t_menu_manage.p_menu_ctrl == NULL)
    {
        return 0;
    }
    return sg_t_menu_manage.p_menu_ctrl->parent_window_no;
}

/**
 * @brief  获取当前窗口信息
 *
 * @param[out] info
 * @return BOOL
 */
BOOL menu_get_current_window_info(menu_show_t *info)
{
    DBG_ASSERT(info != NULL __DBG_LINE);
    int i;
    menu_list_t *p_menu_list;
    if (sg_t_menu_manage.p_menu_ctrl != NULL)
    {
        p_menu_list = sg_t_menu_manage.p_menu_ctrl->p_menu_list;
        info->items_num = sg_t_menu_manage.p_menu_ctrl->items_num;
        info->select_item = sg_t_menu_manage.p_menu_ctrl->select_item;
        info->show_base_item = sg_t_menu_manage.p_menu_ctrl->show_base_item;

        info->psz_desc = sg_t_menu_manage.p_menu_ctrl->psz_desc[sg_t_menu_manage.language];
        if (p_menu_list != NULL)
        {
            for (i = 0; i < info->items_num && i < MENU_MAX_NUM; i++)
            {
                info->psz_items_desc[i] = (char *)p_menu_list[i].psz_desc[sg_t_menu_manage.language];
                info->p_items_ex_data[i] = p_menu_list[i].p_extend_data;
            }
        }

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/**
 * @brief  菜单任务
 *
 * @return 0,成功, 处于菜单模式下; -1,失败, 未处于菜单模式下
 */
BOOL menu_task(void)
{
    int i;
    menu_list_t *p_menu_list;
    menu_show_t t_menu_show;

    if (sg_t_menu_manage.p_menu_ctrl == NULL || sg_t_menu_manage.is_enter_main_menu == 0)
    {
        return FALSE;
    }

    if (sg_t_menu_manage.pfn_load_call_fun != NULL)
    {
        sg_t_menu_manage.pfn_load_call_fun();
        sg_t_menu_manage.pfn_load_call_fun = NULL;
    }

    if (sg_t_menu_manage.p_menu_ctrl->p_menu_list != NULL)
    {
        p_menu_list = sg_t_menu_manage.p_menu_ctrl->p_menu_list;
        t_menu_show.items_num = sg_t_menu_manage.p_menu_ctrl->items_num;
        t_menu_show.select_item = sg_t_menu_manage.p_menu_ctrl->select_item;
        t_menu_show.show_base_item = sg_t_menu_manage.p_menu_ctrl->show_base_item;

        t_menu_show.psz_desc = sg_t_menu_manage.p_menu_ctrl->psz_desc[sg_t_menu_manage.language];

        for (i = 0; i < t_menu_show.items_num && i < MENU_MAX_NUM; i++)
        {
            t_menu_show.psz_items_desc[i] = (char *)p_menu_list[i].psz_desc[sg_t_menu_manage.language];
            t_menu_show.p_items_ex_data[i] = p_menu_list[i].p_extend_data;
        }

        if (sg_t_menu_manage.p_menu_ctrl->pfn_show_menu_fun != NULL)
        {
            sg_t_menu_manage.p_menu_ctrl->pfn_show_menu_fun(&t_menu_show);
        }

        sg_t_menu_manage.p_menu_ctrl->show_base_item = t_menu_show.show_base_item;
    }

    if (sg_t_menu_manage.p_menu_ctrl->pfn_run_call_fun != NULL)
    {
        sg_t_menu_manage.p_menu_ctrl->pfn_run_call_fun();
    }

    return TRUE;
}
