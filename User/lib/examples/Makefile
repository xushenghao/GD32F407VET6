# 变量BIN: 给定的是我们想要生成的可执行文件的名称
BIN = run.exe
SO = lib.dll

# 变量SRC中给的是所有的想要编译的.c源文件，与makefile在同一目录下可直接写（如这里的main.c），否则需要写明相对路径（如这里的其余源文件都在目录src下）。
# 多文件时，选择用"\"进行分行处理
SRC = \
	../src/malloc.c	\
	../src/sqqueue.c	\
	../src/mlist.c	\
	../src/debug.c	\
	../src/data_analysis.c	\
	../src/filter.c	\
	../src/clist.c	\
    ../src/aes.c    \
    ../src/cmac.c    \
	../src/lib.c

EXAMPLE = \
	./simple_clist.c \
	./simple_data_analysis.c \
	./simple_sqqueue.c \
	./simple_aes.c \
    ./simple_cmac.c

CPLUS_INCLUDE_PATH= -I ../inc

# 变量CC：给定编译器名gcc
# 变量CFLAGS：传给编译器的某些编译参数，看需求添加
CC       = gcc
CFLAGS   = -m32 -std=c99
# 变量GDB：给定debugger名gdb
# 变量RM：给定删除文件方式，用于后面删除所有编译所得的.o文件,linux下使用rm -rf
GDB    = gdb
RM     = rm -rf
# 变量OBJS：将变量SRC中所有的.c文件替换成以.o结尾，即将.c源文件编译成.o文件
OBJS = $(SRC:%.c=%.o)
EXAPMLES = $(EXAMPLE:%.c=%.o)

$(SO): $(OBJS) $(EXAPMLES)


# pull in dependencies for .o files
-include $(OBJS:.o=.d)

%.o: %.c
	$(CC) $(CPLUS_INCLUDE_PATH) $(CFLAGS) -c $< -o $@

.PHONY: all clean clist data_analysis

all: $(SO)

rm:
	$(RM) $(OBJS)

#简单链表
clist: $(SO)
	$(CC) $(CPLUS_INCLUDE_PATH) $(CFLAGS) $(OBJS) ./simple_clist.o -o $(BIN)
	$(RM) $(OBJS) $(EXAPMLES)

#数据分析器
data_analysis: $(SO)
	$(CC) $(CPLUS_INCLUDE_PATH) $(CFLAGS) $(OBJS) ./simple_data_analysis.o -o $(BIN)
	$(RM) $(OBJS) $(EXAPMLES)

#队列
sqqueue: $(SO)
	$(CC) $(CPLUS_INCLUDE_PATH) $(CFLAGS) $(OBJS) ./simple_sqqueue.o -o $(BIN)
	$(RM) $(OBJS) $(EXAPMLES)

#aes加密
aes: $(SO)
	$(CC) $(CPLUS_INCLUDE_PATH) $(CFLAGS) $(OBJS) ./simple_aes.o -o $(BIN)
	$(RM) $(OBJS) $(EXAPMLES)

#cmac类CRC
cmac: $(SO)
	$(CC) $(CPLUS_INCLUDE_PATH) $(CFLAGS) $(OBJS) ./simple_cmac.o -o $(BIN)
	$(RM) $(OBJS) $(EXAPMLES)

#运行程序
run:
	./run.exe

clean:
	$(RM) $(OBJS) $(EXAPMLES) $(BIN)
