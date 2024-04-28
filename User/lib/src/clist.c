/**
 * @file clist.c
 * @author xxx
 * @date 2023-08-08 23:18:09
 * @brief
 * @copyright Copyright (c) 2023 by xxx, All Rights Reserved.
 */

#include "clist.h"

/**
 * @brief 初始化链表
 * @param {clist_node_t} **ppFirst 指向链表头节点的指针
 * @return void
 * @note 初始化链表，将链表头节点设置为空
 */
void clist_init(clist_node_t **ppFirst)
{
    DBG_ASSERT(ppFirst != NULL __DBG_LINE);
    *ppFirst = NULL;
}

/**
 * @brief 打印链表
 * @param {clist_node_t} First 链表头节点
 * @return void
 * @note 打印链表中的所有节点数据
 */
void clist_print(clist_node_t *First)
{
    LOG_PRINT("list: ");
    for (clist_node_t *p = First; p != NULL; p = p->Next)
        LOG_PRINT("%d-->", p->data);
    LOG_PRINT("\n");
}

/**
 * @brief 获取链表节点数
 * @param {clist_node_t} First 链表头节点
 * @return {uint32_t} 链表节点数
 * @note 遍历链表，计算节点数
 */
uint32_t clist_node_count(clist_node_t *First)
{
    int32_t count = 0;
    for (clist_node_t *p = First; p != NULL; p = p->Next)
        count++;
    return count;
}

/**
 * @brief 申请新节点
 * @param {cnode} data 节点数据
 * @return {clist_node_t *} 新节点指针
 * @note 分配内存，创建新节点
 */
static clist_node_t *CreateNode(cnode data)
{
    clist_node_t *node = (clist_node_t *)osel_mem_alloc(sizeof(clist_node_t));
    node->data = data;
    node->Next = NULL;
    return node;
}

/**
 * @brief 尾部插入
 * @param {clist_node_t} **ppFirst 指向链表头节点的指针
 * @param {cnode} data 要插入的节点数据
 * @return void
 * @note 在链表末尾插入一个新节点，新节点数据为data
 */
void clist_push_back(clist_node_t **ppFirst, cnode data)
{
    DBG_ASSERT(ppFirst != NULL __DBG_LINE);
    clist_node_t *node = CreateNode(data);
    if (*ppFirst == NULL) // 判断链表不为空
    {
        *ppFirst = node;
        return;
    }
    // 找链表中的最后一个节点
    clist_node_t *p = *ppFirst;
    while (p->Next != NULL)
        p = p->Next;
    p->Next = node; // 插入新申请的节点
}

/**
 * @brief 头部插入
 * @param {clist_node_t} **ppFirst 指向链表头节点的指针
 * @param {cnode} data 要插入的节点数据
 * @return void
 * @note 在链表头部插入一个新节点，新节点数据为data
 */
void clist_push_front(clist_node_t **ppFirst, cnode data)
{
    DBG_ASSERT(ppFirst != NULL __DBG_LINE);
    clist_node_t *node = CreateNode(data);
    node->Next = *ppFirst;
    *ppFirst = node;
}

/**
 * @brief 尾部删除
 * @param {clist_node_t} **ppFirst 指向链表头节点的指针
 * @return void
 * @note 删除链表中最后一个节点
 */
void clist_pop_back(clist_node_t **ppFirst) // 尾部删除
{
    DBG_ASSERT(ppFirst != NULL __DBG_LINE);
    DBG_ASSERT(*ppFirst != NULL __DBG_LINE);
    if ((*ppFirst)->Next == NULL)
    {
        osel_mem_free(*ppFirst);
        *ppFirst = NULL;
        return;
    }
    clist_node_t *p = *ppFirst;
    while (p->Next->Next != NULL)
        p = p->Next;
    osel_mem_free(p->Next);
    p->Next = NULL;
}

/**
 * @brief 头部删除
 * @param {clist_node_t} **ppFirst 指向链表头节点的指针
 * @return void
 * @note 删除链表中第一个节点
 */
void clist_pop_front(clist_node_t **ppFirst)
{
    DBG_ASSERT(ppFirst != NULL __DBG_LINE);
    DBG_ASSERT(*ppFirst != NULL __DBG_LINE); // 链表不是空链表
    clist_node_t *first = *ppFirst;
    *ppFirst = (*ppFirst)->Next;
    osel_mem_free(first);
}
/**
 * @brief 按节点指针插入
 * @param {clist_node_t} **ppFirst 指向链表头节点的指针
 * @param {clist_node_t} *pPos 要插入的节点位置
 * @param {cnode} data 要插入的节点数据
 * @return void
 * @note 在指定节点位置插入一个新节点，新节点数据为data
 */
void clist_insert_for_node(clist_node_t **ppFirst, clist_node_t *pPos, cnode data)
{
    DBG_ASSERT(ppFirst != NULL __DBG_LINE);
    if (*ppFirst == pPos)
    {
        clist_push_front(ppFirst, data);
        return;
    }
    clist_node_t *newNode = CreateNode(data);
    clist_node_t *p;

    for (p = *ppFirst; p->Next != pPos; p = p->Next)
    {
    }                  // 找到pos前的一个节点
    p->Next = newNode; // 改变的是字段内的值，而不是指针的值
    newNode->Next = pPos;
}
/**
 * @brief 按位置插入
 * @param {clist_node_t} **ppFirst 指向链表头节点的指针
 * @param {int32_t} Pos 插入位置
 * @param {cnode} data 要插入的节点数据
 * @return {int32_t} 插入成功返回1，否则返回0
 * @note 在指定位置插入一个新节点，新节点数据为data
 */
int32_t clist_insert(clist_node_t **ppFirst, int32_t Pos, cnode data) // 按位置插入
{
    clist_node_t *p = *ppFirst;
    for (int32_t i = 0; i < Pos; i++)
    {
        if (p == NULL)
            return 0;
        p = p->Next;
    }
    clist_insert_for_node(ppFirst, p, data);
    return 1;
}

/**
 * @brief 按位置删除
 * @param {clist_node_t} **ppFirst 指向链表头节点的指针
 * @param {int32_t} Pos 要删除的节点位置
 * @return {int32_t} 删除成功返回1，否则返回0
 * @note 从指定位置删除一个节点
 */
int32_t cListErase(clist_node_t **ppFirst, int32_t Pos)
{
    clist_node_t *p = *ppFirst;
    for (int32_t i = 0; i < Pos; i++)
    {
        if (p == NULL)
            return 0;
        p = p->Next;
    }
    clist_erase_for_node(ppFirst, p);
    return 1;
}

/**
 * @brief 删除给定结点之后的所有结点
 * @param {clist_node_t **} ppFirst 指向链表头结点的指针
 * @param {clist_node_t *} pPos 要删除的结点
 * @return void
 * @note
 */
void clist_erase_for_node(clist_node_t **ppFirst, clist_node_t *pPos)
{
    if (*ppFirst == pPos)
    {
        clist_pop_front(ppFirst);
        return;
    }
    clist_node_t *p = *ppFirst;
    while (p->Next != pPos)
        p = p->Next;
    p->Next = pPos->Next;
    osel_mem_free(pPos);
}

/**
 * @brief 删除指定值的结点
 * @param {clist_node_t **} ppFirst 指向链表头结点的指针
 * @param {cnode} data 要删除的结点数据
 * @return void
 * @note
 */
void clist_remove(clist_node_t **ppFirst, cnode data)
{
    clist_node_t *p = *ppFirst;
    clist_node_t *prev = NULL;
    DBG_ASSERT(ppFirst != NULL __DBG_LINE);
    if (*ppFirst == NULL)
        return;
    while (p)
    {
        if (p->data == data)
        {
            if (*ppFirst == p) // 删除的是第一个节点
            {
                *ppFirst = p->Next;
                osel_mem_free(p);
                p = NULL;
            }
            else // 删除中间节点
            {
                prev->Next = p->Next;
                osel_mem_free(p);
                p = NULL;
            }
            break;
        }
        prev = p;
        p = p->Next;
    }
}

/**
 * @brief 删除指定值的所有结点
 * @param {clist_node_t **} ppFirst 指向链表头结点的指针
 * @param {cnode} data 要删除的结点数据
 * @return void
 * @note
 */
void clist_remove_all(clist_node_t **ppFirst, cnode data)
{
    clist_node_t *p = NULL;
    clist_node_t *prev = NULL;
    DBG_ASSERT(ppFirst != NULL __DBG_LINE);
    if (*ppFirst == NULL)
        return;
    p = *ppFirst;
    while (p)
    {
        if (p->data == data)
        {
            if (*ppFirst == p) // 删除的是第一个节点
            {
                *ppFirst = p->Next;
                osel_mem_free(p);
                p = *ppFirst;
            }
            else // 删除中间节点
            {
                prev->Next = p->Next;
                osel_mem_free(p);
                p = prev;
            }
        }
        prev = p;
        p = p->Next;
    }
}

/**
 * @brief 销毁链表，每个节点都要销毁
 * @param {clist_node_t **} ppFirst 指向链表头结点的指针
 * @return void
 * @note
 */
void clist_destroy(clist_node_t **ppFirst)
{
    clist_node_t *p = NULL;
    clist_node_t *del = NULL;
    DBG_ASSERT(ppFirst != NULL __DBG_LINE);
    p = *ppFirst;
    while (p)
    {
        del = p;
        p = p->Next;
        osel_mem_free(del);
        del = NULL;
    }
    *ppFirst = NULL;
}

/**
 * @brief 按值查找，返回第一个找到的结点指针，如果没找到，返回 NULL
 * @param {clist_node_t *} pFirst 指向链表头结点的指针
 * @param {cnode} data 要查找的结点数据
 * @return {clist_node_t *} 找到的结点指针，如果没有找到，返回 NULL
 * @note
 */
clist_node_t *clist_find(clist_node_t *pFirst, cnode data)
{
    for (clist_node_t *p = pFirst; p != NULL; p = p->Next)
    {
        if (p->data == data)
            return p;
    }
    return NULL;
}
