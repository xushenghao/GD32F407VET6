[TOC]


### clist 简单链表
#### 说明
- clist是list的简化版，下面给出常用的接口说明
- 接口说明：
```code

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

```
#### DEMO
- 详细使用请见<a href="examples/simple_clist.c" target="_blank">simple_clist.c</a>
- examples目录内执行```make clist && make run```


### cmd 命令解析器
#### 说明
- cmd是一个非常简单好用的命令解析器。
> 简单来说，我希望我的开发板，可以通过命令执行一些处理，比如说我用串口发一个命令A，开发板就执行A的一些处理，或者，在调试某些AT模组的时候，当我收到模组返回的一些指令后，自动执行一些处理。
- 接口说明：
```code
REGISTER_CMD(cmd, handler, desc)  ///< 注册命令

void cmd_init(void);         ///< 初始化命令

void cmd_parsing(char *str); ///< 命令解析
```

#### DEMO
- 详细使用请见<a href="examples/simple_cmd.c" target="_blank">simple_cmd.c</a>
- 该模块不提供GCC编译，只提供KEIL编译和IAR编译



### data_analysis 数据分析器
#### 说明
- data_analysis 是用来处理串口或者其他方式获得的数据，具有识别数据帧起始和结束的功能，只需要定义好识别符以及长度，该模块会将完整的数据帧处理好
- 接口说明：
```code
typedef void (*data_interupt_cb_t)(uint8_t id, uint8_t ch); ///< 中断回调函数，数据从这里写入

extern uint8_t data_read(uint8_t id, void *buffer, uint16_t len); ///< 读取数据

extern void data_write(uint8_t id, uint8_t *const string, uint16_t len); ///< TODO 写入数据

extern void lock_data(uint8_t data_id); ///< 锁定数据，防止中断写入数据

extern void unlock_data(uint8_t data_id); ///< 解锁数据

extern data_interupt_cb_t data_fsm_init(uint8_t data_id); ///< 初始化数据状态机

extern bool data_reg(uint8_t id, data_reg_t reg); ///< 注册数据

extern void data_unreg(uint8_t id); ///< 注销数据
```
#### DEMO
- 详细使用请见<a href="examples/simple_data_analysis.c" target="_blank">simple_data_analysis.c</a>
- examples目录内执行```make data_analysis && make run```


### sqqueue(队列)
#### 说明
- sqqueue 是一个成员变量固定长度的队列
- 使用的时候先创建一个"sqqueue_ctrl_t"类型的队列对象，调用初始化函数后就可以使用对象中的功能了

#### DEMO
- 详细使用请见<a href="examples/simple_sqqueue.c" target="_blank">simple_sqqueue.c</a>
- examples目录内执行```make sqqueue && make run```


### aes(加密)
<a href="http://baike.baidu.com/link?url=WnbPeKCwba_BGRIBls-MNxyghGRrAI3Ubu4bntdlnjqHKzOjEl5a3-w-E-XF0Mi-A8dEaFpC31ofGw-GOaC_ea" target="_blank">百度百科</a>

#### 说明

- 对不同长度的密钥，AES采用不同的加密轮次：

|  128位        | 192位        | 256位      |
| ------------- | -----------: | :--------: |
| 10            | 12           | 14         |

- 密钥扩展：简单说就是将原来的密钥扩展到足够用的长度：

    - 128位密钥： 扩展到 11x4x4
    - 192位密钥： 扩展到 13x4x4
    - 256位密钥： 扩展到 15x4x4

- AES加密过程是在一个4×4的字节矩阵上运作,所以一次加解密传入的最小块单位为16字节


#### 代码流程

```flow
st=>start: 加密开始
e=>end: 加密结束
op1=>operation: 加密的数据[简称密文]、密钥（16个字节）、输入数据块和输出数据块
op2=>operation: 调用接口"aes_set_key",完成密钥的扩展
op3=>operation: 调用接口"aes_encrypt",完成数据的加密
st->op1->op2->op3->e
```
```flow
st=>start: 解密开始
e=>end: 解密结束
op1=>operation: 解密的数据[简称密文]、密钥（16个字节）、输入数据块和输出数据块
op2=>operation: 调用接口"aes_set_key",完成密钥的扩展
op3=>operation: 调用接口"aes_encrypt",完成数据的解密
st->op1->op2->op3->e
```
#### DEMO
- demo中会使用aes中的接口完成简单的加密和解密数据
- **将aes.h中的AES_ENC_PREKEYED和AES_DEC_PREKEYED置1**
- 详细使用请见<a href="../../apps/simple_aes.c" target="_blank">simple_aes.c</a>
- examples目录内执行```make aes && make run```

### cmac(类CRC)
#### 说明

- cmac是一种数据传输检错功能，以保证数据传输的正确性和完整性
- cmac基本原理是：通过对需要校验的数据奇偶校验、位移、AES加密获取一段16个字节大小的数组
- **lorawan使用cmac模块给MAC数据和加密数据做校验的优点：效率很高、安全性很高（在校验数据之前调用"AES_CMAC_Update"二次，增加了异变因子）**
- **困惑：为什么不使用CRC32**
- demo中只对需要校验的数据使用"AES_CMAC_Update"一次，无法确定lorawan增加一次"AES_CMAC_Update"的效果如何


#### 代码流程

```flow
st=>start: 校验开始
e=>end: 校验结束
op1=>operation: 需要校验的数据、密钥、密钥扩展表
op2=>operation: 调用接口"AES_CMAC_Init",完成密钥扩展表的初始化
op3=>operation: 调用接口"AES_CMAC_SetKey",完成密钥扩展表数据
op4=>operation: 调用接口"AES_CMAC_Update",完成数据的奇偶校验
op5=>operation: 调用接口"AES_CMAC_Final",生成16个字节的校验表
op6=>operation: 取表4个字节作为校验码
st->op1->op2->op3->op4->op5->op6->e
```

#### DEMO
- 详细使用请见<a href="../../apps/simple_cmac.c" target="_blank">simple_cmac.c</a>
- examples目录内执行```make cmac && make run```
