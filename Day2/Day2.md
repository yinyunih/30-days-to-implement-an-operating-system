# 30天自制操作系统---第二天

## 1. 继续开发汇编程序，进一步理解程序
* 将昨天的.asm文件中的16进制数翻译为指令，并且只需要制作512字节大小的程序

* 保存为ipl.asm文件

* 制作相应的镜像文件ipl.img文件

* 在VMware中选择该文件为软盘映像文件启动，成功显示*hello YunOS!*

## 收获
### 1. 学会了汇编语言的几个指令和语法
* ORG指令--程序执行开始时，告诉编译器这些机器语言指令装载到内存中的哪个地址（但是感觉这个说法不是很严谨，参考该[文章](https://blog.csdn.net/judyge/article/details/52333656)）
* MOV指令--赋值语句
* JMP指令--跳转语句
* [地址]--中括号代表取地址中的内容，其中的内容可以是数字也可以是寄存器（BX、BP、SI、DI）
    
        MOV BYTE [678],123     ;代表用678号内存单元存储123
        MOV BX, DX             ;这两行代表，DX内存的内容赋值给AL
        MOV AL, BYTE [BX]
* ADD指令--加法指令
* CMP指令和JE指令--比较指令和相等条件跳转指令

        CMP AL, 0              ;若寄存器AL中的内容等于0，则跳转到fin处
        JE fin
* INT指令--软件中断指令
* HIT指令--让CPU进入待机状态
### 2. 了解CPU中的寄存器
* 以下是16位（2字节）的寄存器

| 寄存器      | 全称               | 解释            | 
| :---------: | :---------:       | :---------:     |
| AX          | accumulator       |  累加寄存器      |
| CX          | counter           |  计数寄存器      |
| DX          | data              |  数据寄存器      |
| BX          | base              |  基址寄存器      |
| SP          | stack pointer     |  栈指针寄存器    |
| BP          | base pointer      |  基址指针寄存器  |
| SI          | source index      |  源变址寄存器    |
| DI          | destination index |  目的变址寄存器  |

* AX,CX,DX,BX寄存器具有两个8位的寄存器：
    AH、AL、CH、CL、DH、DL、BH、BL（H->高八位，L->低八位）

* 32位（4字节）寄存器：
    EAX、ECX、EDX、EBX、ESP、EBP、ESI、EDI（低16位对应相应的16位寄存器）

### 3. 了解BIOS
* 计算机出厂时就存在于ROM
* “basic input output system”的缩写
* 本质是为操作系统开发人员准备的各种函数的集合
* INT后面跟不同的中断号，可以调用不同的中断服务程序