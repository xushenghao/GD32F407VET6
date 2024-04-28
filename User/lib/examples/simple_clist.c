#include "../inc/data_type_def.h"
#include "../inc/clist.h"

int32_t main(void)
{
    clist_node_t *head = NULL; // 创建头指针，初始化为NULL
    clist_init(&head);         // 初始化指针（可有可无）

    // 1：添加数据
    for (int32_t i = 0; i < 30; i++)
    {
        if (i > 10)
            clist_push_front(&head, (cnode)i); // 头部插入
        else
            clist_push_back(&head, (cnode)i); // 尾部插入
    }

    LOG_PRINT("\n 1:  count:%d \n", clist_node_count(head)); // 获取链表节点数，打印
    clist_print(head);                                       // 打印链表

    // 2：删除数据
    for (int32_t i = 0; i < 10; i++)
    {
        if (i > 5)
            clist_pop_back(&head); // 删除尾部
        else
            clist_pop_front(&head); // 头部删除
    }
    LOG_PRINT("\n 2:  count:%d \n", clist_node_count(head));
    clist_print(head);

    // 3：插入数据
    clist_insert(&head, 5, (cnode)1111);
    clist_insert_for_node(&head, head->Next->Next->Next->Next->Next, (cnode)10000);
    clist_insert(&head, 1000, (cnode)2222); // 无效插入
    LOG_PRINT("\n 3:  count:%d \n", clist_node_count(head));
    clist_print(head);

    // 4：删除指定节点
    clist_remove(&head, (cnode)5);
    clist_erase_for_node(&head, head->Next->Next);
    clist_remove(&head, (cnode)1000); // 无效删除
    clist_print(head);
    LOG_PRINT("\n 4:  count:%d \n", clist_node_count(head));
    clist_print(head);

    // 5：删除所有节点
    clist_destroy(&head);
    LOG_PRINT("\n 5:  count:%d    ", clist_node_count(head));
    clist_print(head);
    return 0;
}
