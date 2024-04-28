/**
 * @file sqqueue.c
 * @author xxx
 * @date 2023-06-25 13:07:02
 * @brief 提供循环队列功能
 * @copyright Copyright (c) 2023 by xxx, All Rights Reserved.
 */

#include "../inc/sqqueue.h"
#include "../inc/osel_arch.h"

#define SQQ_ENTRY_SIZE (queue_ptr->entry_size)
#define SQQ_LEN (queue_ptr->sqq_len)

/**
 * @brief 初始化队列控制结构体
 * @param p_this 队列控制结构体的指针
 * @param entry_size 队列中每个元素的大小
 * @param sqq_len 队列的最大长度
 * @return {bool} 初始化成功返回true，否则返回false
 * @note 此函数用于初始化队列控制结构体，确保队列有足够的空间存储元素
 */
static bool sqqueue_init(sqqueue_ctrl_t *const p_this,
                         uint8_t entry_size,
                         uint16_t sqq_len)
{
    DBG_ASSERT(p_this != NULL __DBG_LINE);
    sqqueue_t *queue_ptr = &(p_this->sqq);

    if (p_this != NULL)
    {
        queue_ptr->entry_size = entry_size;
        queue_ptr->sqq_len = sqq_len + 1;
        queue_ptr->base = (uint8_t *)osel_mem_alloc(SQQ_LEN * SQQ_ENTRY_SIZE);
        if (queue_ptr->base == NULL)
        {
            return false;
        }
        queue_ptr->front = 0;
        queue_ptr->rear = 0;

        return true;
    }

    return false;
}

/**
 * @brief 获取队列的长度
 * @param {sqqueue_ctrl_t} *p_this 队列控制结构体的指针
 * @return {uint16_t} 队列的长度
 * @note 此函数用于获取队列的长度，包括队列中元素的数量和最大长度
 */
static uint16_t sqqueue_length(const sqqueue_ctrl_t *const p_this)
{
    DBG_ASSERT(p_this != NULL __DBG_LINE);
    const sqqueue_t *const queue_ptr = &(p_this->sqq);
    uint16_t length = 0;
    if (p_this != NULL)
    {
        length = (queue_ptr->rear + SQQ_LEN - queue_ptr->front);

        if (length >= SQQ_LEN)
        {
            length -= SQQ_LEN;
        }
    }

    return length;
}

/**
 * @brief 获取队列的长度
 * @param {sqqueue_ctrl_t} *p_this 队列控制结构体的指针
 * @return {uint16_t} 队列的长度
 * @note 此函数用于获取队列的长度，包括队列中元素的数量和最大长度
 */
static bool sqqueue_full(const sqqueue_ctrl_t *const p_this)
{
    uint16_t rear = 0;

    DBG_ASSERT(p_this != NULL __DBG_LINE);

    if (p_this != NULL)
    {
        const sqqueue_t *const queue_ptr = &(p_this->sqq);
        rear = queue_ptr->rear + 1;
        if (rear >= SQQ_LEN)
        {
            rear -= SQQ_LEN;
        }
        if (rear == queue_ptr->front)
        {
            return true;
        }
    }

    return false;
}

/**
 * @brief 将元素添加到队列中
 * @param {sqqueue_ctrl_t} *p_this 队列控制结构体的指针
 * @param {void} *e 要添加的元素
 * @return {bool} 添加成功返回true，否则返回false
 * @note 此函数用于将元素添加到队列中，确保队列有足够的空间存储元素
 */
static bool enter_sqqueue(sqqueue_ctrl_t *const p_this, const void *const e)
{
    uint16_t rear = 0;
    sqqueue_t *queue_ptr = &(p_this->sqq);

    if ((p_this != NULL) && (e != NULL))
    {
        rear = queue_ptr->rear + 1;
        if (rear >= SQQ_LEN)
        {
            rear -= SQQ_LEN;
        }

        if (rear == queue_ptr->front)
        {
            return false;
        }

        /* 根据e的长度进行内存拷贝 */
        DBG_ASSERT(queue_ptr->rear != SQQ_LEN __DBG_LINE);
        osel_memcpy(queue_ptr->base + (queue_ptr->rear * SQQ_ENTRY_SIZE),
                    e,
                    SQQ_ENTRY_SIZE);
        queue_ptr->rear = rear;

        return true;
    }
    return false;
}

/**
 * @brief 将字符串添加到队列中
 * @param {sqqueue_ctrl_t} *p_this 队列控制结构体的指针
 * @param {const void} *string 要添加的字符串
 * @param {uint16_t} cnt 要添加的字符串的长度
 * @return {bool} 添加成功返回true，否则返回false
 * @note 此函数用于将字符串添加到队列中，确保队列有足够的空间存储字符串
 */
static bool string_enter_sqqueue(sqqueue_ctrl_t *const p_this,
                                 const void *const string,
                                 uint16_t cnt)
{
    uint16_t rear = 0;
    uint16_t length = 0;
    sqqueue_t *queue_ptr = &(p_this->sqq);

    if ((p_this != NULL) && (string != NULL))
    {
        /* 判断是否超出队列长度 */
        length = sqqueue_length(p_this); // 已有元素个数
        if (length == 0xFFFF)
        {
            return false;
        }

        length = (SQQ_LEN - 1) - length; // 可写入个数
        if (length < cnt)
        {
            return false;
        }

        rear = queue_ptr->rear + cnt;
        if (rear >= SQQ_LEN)
        {
            rear -= SQQ_LEN;
            uint8_t half = SQQ_LEN - queue_ptr->rear;
            osel_memcpy(queue_ptr->base + (queue_ptr->rear * SQQ_ENTRY_SIZE),
                        string, half * SQQ_ENTRY_SIZE);
            uint8_t *half_p = (uint8_t *)string;
            osel_memcpy(queue_ptr->base, (uint8_t *)&half_p[half], rear * SQQ_ENTRY_SIZE);
        }
        else
        {
            osel_memcpy(queue_ptr->base + (queue_ptr->rear * SQQ_ENTRY_SIZE),
                        string, SQQ_ENTRY_SIZE * cnt);
        }

        queue_ptr->rear = rear;

        return true;
    }
    return false;
}

/**
 * @brief 从队列中删除元素
 * @param {sqqueue_ctrl_t} *p_this 队列控制结构体的指针
 * @return {void *} 删除的元素，如果队列为空则返回NULL
 * @note 此函数用于从队列中删除元素，确保队列中有足够的空间存储元素
 */
static void *delete_sqqueue(sqqueue_ctrl_t *const p_this)
{
    DBG_ASSERT(p_this != NULL __DBG_LINE);
    uint16_t front = 0;

    sqqueue_t *queue_ptr = NULL;

    if (p_this != NULL)
    {
        void *p_elem = NULL;
        queue_ptr = &(p_this->sqq);
        if (queue_ptr->rear == queue_ptr->front)
        {
            return NULL;
        }
        /* 根据元素类型大小计算出偏移量，得到该元素首地址 */
        p_elem = (void *)((queue_ptr->base) + (queue_ptr->front * SQQ_ENTRY_SIZE));
        front = queue_ptr->front + 1;
        if (front >= SQQ_LEN)
        {
            front -= SQQ_LEN;
        }
        queue_ptr->front = front;

        return p_elem;
    }
    return NULL;
}

/**
 * @brief 撤销队列中的一个元素
 * @param {sqqueue_ctrl_t} *p_this 队列控制结构体的指针
 * @return {*} 返回被撤销的元素，如果队列为空则返回NULL
 * @note
 */
static void *revoke_sqqueue(sqqueue_ctrl_t *const p_this)
{
    DBG_ASSERT(p_this != NULL __DBG_LINE);
    uint16_t rear = 0;
    sqqueue_t *queue_ptr = NULL;

    if (p_this != NULL)
    {
        void *p_elem = NULL;

        queue_ptr = &(p_this->sqq);
        if (queue_ptr->rear == queue_ptr->front)
        {
            return NULL;
        }

        rear = queue_ptr->rear;
        if (rear == 0)
        {
            rear = SQQ_LEN - 1;
        }
        else
        {
            rear--;
        }
        queue_ptr->rear = rear;
        /* 根据元素类型大小计算出偏移量，得到该元素首地址*/
        p_elem = (void *)((queue_ptr->base) + (queue_ptr->rear * SQQ_ENTRY_SIZE));

        return p_elem;
    }
    return NULL;
}

/**
 * @brief 清空队列
 * @param {sqqueue_ctrl_t} *p_this 队列控制结构体的指针
 * @return {void}
 * @note
 */
static void clear_sqq(sqqueue_ctrl_t *const p_this)
{
    DBG_ASSERT(p_this != NULL __DBG_LINE);

    sqqueue_t *queue_ptr = &(p_this->sqq);
    if (p_this != NULL)
    {
        queue_ptr->front = 0;
        queue_ptr->rear = 0;
    }
}

/**
 * @brief 遍历队列
 * @param {sqqueue_ctrl_t} *p_this 队列控制结构体的指针
 * @param {void (*)(const void *e)} vi 遍历函数，参数为队列中的一个元素
 * @return {void}
 * @note
 */
static void traverse(sqqueue_ctrl_t *const p_this, void (*vi)(const void *e))
{
    DBG_ASSERT(p_this != NULL __DBG_LINE);
    sqqueue_t *queue_ptr = NULL;
    uint16_t i = 0;

    if (p_this != NULL)
    {
        queue_ptr = &(p_this->sqq);

        if (queue_ptr->rear == queue_ptr->front)
        {
            return;
        }

        i = queue_ptr->front;
        while (i != queue_ptr->rear)
        {
            vi((void *)((queue_ptr->base) + (i * SQQ_ENTRY_SIZE)));
            if (++i >= SQQ_LEN)
            {
                i = 0;
            }
        }
    }
}

/**
 * @brief 从队列中删除一个元素
 * @param {sqqueue_ctrl_t} *p_this 队列控制结构体的指针
 * @param {uint16_t} offset_to_front 要删除的元素在队列中的偏移量，从队头开始
 * @return {void}
 * @note
 */
static void qremove(sqqueue_ctrl_t *const p_this, uint16_t offset_to_front)
{
    DBG_ASSERT(p_this != NULL __DBG_LINE);
    sqqueue_t *queue_ptr = NULL;
    uint16_t i = 0;

    if (p_this != NULL)
    {
        queue_ptr = &(p_this->sqq);
        DBG_ASSERT(offset_to_front < SQQ_LEN __DBG_LINE);

        if (queue_ptr->rear == queue_ptr->front)
        {
            return;
        }

        uint16_t j = 0;

        for (i = offset_to_front; i > 0; i--)
        {
            /* 定位待删除元素在队列中的位置 */
            j = queue_ptr->front + i;

            if (j >= SQQ_LEN)
            {
                j -= SQQ_LEN;
            }

            if (j == 0) // 在翻转位置特殊处理拷贝的源地址
            {
                osel_memcpy(queue_ptr->base + (0 * SQQ_ENTRY_SIZE),
                            queue_ptr->base + ((SQQ_LEN - 1) * SQQ_ENTRY_SIZE),
                            SQQ_ENTRY_SIZE);
            }
            else
            {
                osel_memcpy(queue_ptr->base + (j * SQQ_ENTRY_SIZE),
                            queue_ptr->base + ((j - 1) * SQQ_ENTRY_SIZE),
                            SQQ_ENTRY_SIZE);
            }
        }

        /* 减少队列长度 */
        uint16_t front = queue_ptr->front + 1;
        if (front >= SQQ_LEN)
        {
            front -= SQQ_LEN;
        }

        queue_ptr->front = front;
    }
}

/**
 * @brief 初始化队列控制结构体
 * @param {sqqueue_ctrl_t} *p_this 队列控制结构体的指针
 * @param {uint8_t} entry_size 队列中每个元素的类型大小
 * @param {uint16_t} sqq_len 队列的最大长度
 * @return {bool} 初始化成功返回true，否则返回false
 * @note
 */
bool sqqueue_ctrl_init(sqqueue_ctrl_t *const p_this,
                       uint8_t entry_size,
                       uint16_t sqq_len)
{
    DBG_ASSERT(p_this != NULL __DBG_LINE);

    if (p_this != NULL)
    {
        if (sqqueue_init(p_this, entry_size, sqq_len) != false)
        {
            p_this->enter = enter_sqqueue;
            p_this->string_enter = string_enter_sqqueue;
            p_this->del = delete_sqqueue;
            p_this->revoke = revoke_sqqueue;
            p_this->get_len = sqqueue_length;
            p_this->full = sqqueue_full;
            p_this->clear_sqq = clear_sqq;
            p_this->traverse = traverse;
            p_this->remove = qremove;
            return true;
        }
    }
    return false;
}
