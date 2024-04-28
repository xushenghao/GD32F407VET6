/**
 * @file clist.h
 * @author xxx
 * @date 2023-08-08 23:18:15
 * @brief 简单链表  使用方法 lib\examples\simple_clist.c
 * @copyright Copyright (c) 2023 by xxx, All Rights Reserved.
 */

#ifndef __CLIST_H
#define __CLIST_H
#include "lib.h"
typedef void *cnode;

/// 链表中一个节点的结构体
typedef struct CLIST_NODE
{
    cnode data;              /// 值
    struct CLIST_NODE *Next; /// 指向下一个结点
} clist_node_t;

void clist_init(clist_node_t **ppFirst); ///< 初始化 ,构造一条空的链表

void clist_print(clist_node_t *First); ///< 打印链表

uint32_t clist_node_count(clist_node_t *First); ///< 获取链表节点数

void clist_push_back(clist_node_t **ppFirst, cnode data); ///< 尾部插入

void clist_push_front(clist_node_t **ppFirst, cnode data); ///< 头部插入

void clist_pop_back(clist_node_t **ppFirst); ///< 尾部删除

void clist_pop_front(clist_node_t **ppFirst); ///< 头部删除

void clist_insert_for_node(clist_node_t **ppFirst, clist_node_t *pPos, cnode data); ///< 给定结点插入，插入到结点前

int32_t clist_insert(clist_node_t **ppFirst, int32_t Pos, cnode data); ///< 按位置插入

void clist_erase_for_node(clist_node_t **ppFirst, clist_node_t *pPos); ///< 给定结点删除

void clist_remove(clist_node_t **ppFirst, cnode data); ///< 按值删除，只删遇到的第一个

void clist_remove_all(clist_node_t **ppFirst, cnode data); ///< 按值删除，删除所有的

void clist_destroy(clist_node_t **ppFirst); ///< 销毁 ，需要销毁每一个节点

clist_node_t *clist_find(clist_node_t *pFirst, cnode data); ///< 按值查找，返回第一个找到的结点指针，如果没找到，返回 NULL

#endif //__CLIST_H
