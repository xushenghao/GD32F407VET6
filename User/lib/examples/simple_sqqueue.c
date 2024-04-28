#include "../inc/data_type_def.h"
#include "../inc/log.h"
#include "../inc/osel_arch.h"
#include "../inc/sqqueue.h"

typedef struct
{
    uint8_t x;
    uint8_t y;
} element_t;

sqqueue_ctrl_t queue; // 创建队列对象

void traverse_cb(const void *e)
{
    element_t *p = (element_t *)e;
    LOG_PRINT("x = %d, y = %d", p->x, p->y);
}

int32_t main(void)
{
    int size = 10;
    // 初始化队列
    if (FALSE == sqqueue_ctrl_init(&queue, sizeof(element_t), size))
    {
        LOG_ERR("queue init failed!");
        return -1; // 创建失败
    }

    // 添加测试元素
    for (int i = 1; i <= 10; i++)
    {
        element_t element;
        element.x = i * 10;
        element.y = i * 10;
        queue.enter(&queue, &element); // 将成员插入到队列中
    }
    LOG_PRINT("add queue len = %d", queue.get_len(&queue)); // 获取队列长度

    queue.del(&queue);                                         // 移除首元素
    LOG_PRINT("del queue len = %d", queue.get_len(&queue));    // 获取队列长度
    queue.revoke(&queue);                                      // 移除尾元素
    LOG_PRINT("revoke queue len = %d", queue.get_len(&queue)); // 获取队列长度
    queue.remove(&queue, 3);                                   // 删除相对队头指定偏移位置的元素
    LOG_PRINT("remove queue len = %d", queue.get_len(&queue)); // 获取队列长度

    LOG_PRINT("queue traverse:");
    queue.traverse(&queue, traverse_cb); // 遍历队列

    queue.clear_sqq(&queue);                                  // 清空队列
    LOG_PRINT("clear queue len = %d", queue.get_len(&queue)); // 获取队列长度
    return 0;
}
