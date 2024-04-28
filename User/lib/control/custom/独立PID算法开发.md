# 架构
|文件|路径| <div style="width:300px">说明</div>|
|:--|:--|:--|
|pid|User\lib\control\src|PID算法模块|
|execute|User\application\src|执行器|
|app_flow|User|任务流程控制|

## APP_FLOW任务流程控制
> adjust_inspection 中在没有检测到调试信号时执行<b style="color:blue">execute_dac,EXECUTE_PLAN</b>定义了需要执行的算法任务计划

## PID算法模块
文件内容：
|文件| <div style="width:300px">说明</div>|
|:--|:--|
|pid.c|构造算法的入口|
|pid_common.c|普通算法|
|pid_neural.c|神经网络算法|
|pid_fuzzy.c|模糊算法|

<b style="color:blue">custom 目录下为各自算法实现</b>


## EXECUTE执行器
> execute_pid_init中定义了初始化参数
> execute_dac中定义了执行器

