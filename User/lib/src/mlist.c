/*
 * @Author:
 * @Date: 2023-04-04 08:39:23
 * @LastEditors: xxx
 * @LastEditTime: 2023-04-21 12:08:31
 * @Description:
 * email:
 * Copyright (c) 2023 by xxx, All Rights Reserved.
 */

#include "../inc/mlist.h"

#ifndef NULL
#define NULL ((void *)0)
#endif

void list_init(list_head_t *const ptr)
{
    (ptr)->next = (ptr);
    (ptr)->prev = (ptr);
}

/*
 * 在两个连续的链表元素中插入一个新的元素
 */
static void __list_add(list_head_t *const new_entry,
                       list_head_t *const prev,
                       list_head_t *const next)
{
    next->prev = new_entry;
    new_entry->next = next;
    new_entry->prev = prev;
    prev->next = new_entry;
}

/**
 * 在指定的位置之前插入一个元素
 */
void list_insert_forwards(list_head_t *const new_entry, list_head_t *const pos)
{
    __list_add(new_entry, pos->prev, pos);
}

/**
 * 在指定的位置之后插入一个元素
 */
void list_insert_backwards(list_head_t *const new_entry, list_head_t *const pos)
{
    __list_add(new_entry, pos, pos->next);
}

/**
 * 在链表尾部插入新的元素
 */
void list_add_to_tail(list_head_t *const new_entry, list_head_t *const list)
{
    __list_add(new_entry, list->prev, list);
}

/**
 * 在链表头后插入新的元素
 */
void list_add_to_head(list_head_t *const new_entry, list_head_t *const list)
{
    __list_add(new_entry, list, list->next);
}

static void __list_del(list_head_t *const prev, list_head_t *const next)
{
    next->prev = prev;
    prev->next = next;
}

/**
 * 删除指定的链表元素
 */
void list_del(list_head_t *const elem)
{
    __list_del(elem->prev, elem->next);
    elem->next = (list_head_t *)NULL;
    elem->prev = (list_head_t *)NULL;
}

/*
 * 删除并返回链表尾元素
 */
list_head_t *list_curtail(const list_head_t *const head)
{
    list_head_t *tail = head->prev;
    list_del(tail);
    return tail;
}

/**
 * 判断链表是否为空
 */
bool list_empty(const list_head_t *const head)
{
    return (((head)->next == head) || (head->next == NULL));
}

/**
 * 获取链表第一个元素
 */
list_head_t *list_first_elem_look(const list_head_t *const head)
{
    if (!list_empty(head))
    {
        return head->next;
    }
    return NULL;
}

/**
 * 从制定位置后取出并删除该元素
 */
list_head_t *list_next_elem_get(const list_head_t *const pos)
{
    if (pos == NULL)
    {
        return NULL;
    }

    list_head_t *temp = (pos)->next;
    if (temp != NULL)
    {
        list_del(temp);
    }

    return temp;
}

/**
 * 将链表元素从一个队列移出，再添加到另外一个队列中
 */
void list_move_to_another_head(list_head_t *const elem, list_head_t *const head)
{
    __list_del(elem->prev, elem->next);
    list_add_to_head(elem, head);
}

/**
 * 将元素从一个队列中取出，然后再放入另外一个队列的尾部；
 */
void list_move_to_another_tail(list_head_t *const elem, list_head_t *const head)
{
    __list_del(elem->prev, elem->next);
    list_add_to_tail(elem, head);
}
