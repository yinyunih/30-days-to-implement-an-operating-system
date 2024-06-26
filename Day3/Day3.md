# 30天自制操作系统---第三天

## 1. 制作真正的IPL并读入10个柱面
* 在昨天的ipl.asm文件中添加代码，其中包含调用BIOS13号中断服务程序的代码

* 新建一个为haribote.nas的文件，在其中写入操作系统代码

* 利用作者开发的汇编编译器nask，实现haribote.nas=>haribote.sys 
(.sys结尾，作者这样定义是由于该文件是由操作系统的代码编译形成，个人猜测)

* 然后将这个文件保存到磁盘映像文件haribote.img，这个也只能通过作者提供的工具完成
（为了用作者的工具，将文件名ipl.asm改为了ipl.ask）

* 在VMware中选择该文件为软盘映像文件启动，屏幕一篇漆黑代表成功！（**开怀大笑**）

## 2.进入32位模式、导入C语言以及实现HLT
* 由于haribote.sys的前半部分是汇编语言，后半部分是C语言，原来文件haribote.nas变为了asmhead.nas，并且增加了100多行代码。作者还没对其作出解释？！

* 新建一个bootpack.c文件，写入与操作系统相关的c语言代码。（欧拉！看久了汇编语言，才发现c语言是那么和蔼可亲！）

* （~~由于作者的强迫症~~）今天还没结束！新建了一个naskfunc.nas，用来实现一个调用HLT指令的函数，在bootpack.c文件中调用函数即可以实现HLT。

* 利用作者的制作工具，生成haribote.img，运行后屏幕一篇漆黑！一天结束了！（舒一口气）

## 收获
### 1. 学会了汇编语言的几个指令和语法
* JC指令--如果进位标志（carry flag）是1就跳转。
* [地址]--这个语法昨天学习过，进一步补充，指定内存时需要指定一个段寄存器，[SI] == [DS:SI]，DS是默认的段寄存器。最终的地址=段寄存器 * 16 + 段偏移
* 一系列条件跳转指令--JNC,JAE,JBE,JB
* EQU指令--等同于C语言的#define命令
* RET指令--等同于C语言的return

### 2. 了解磁盘的结构
* 由以下几个重要的参数（姑且这么称呼）:柱面，磁头，扇区
* 柱面一个磁盘有80个
* 磁头一个磁盘有2个
* 扇区一个柱面18个，一个大小为512B
* 那么总存储量为 80 * 2 * 18 * 512B = 1440KB

### 3. 对32位系统和16位系统有初步的概念
* 作者似乎对这块含糊其词，大致介绍了一下
* 不同位数的系统机器语言的代码不一样
* 不同位数的系统更方便使用对应位数的寄存器
* 32位系统能使用CPU的自我保护功能

### 4.具体了解了C语言代码的编译过程
* 该[文章](https://www.cnblogs.com/yucloud/p/11041861.html)中有作者C语言程序编译的流程
* 原文中的介绍也很细致

### 5.解决一个疑惑
* “0x8000～0x81ff这512字节是留给启动区的，要将启动区的内容读到那里”
* “0x7c00～0x7dff用于启动区，0x7e00以后直到0x9fbff为止的区域都没有特别的用途”
* 以上是作者原文中的两句话，让人摸不着头脑（**恼火！**）
* __以下是查询后得到的结论：__ BIOS完成相关工作以后会选择一个启动盘加载第一个扇区（MBR）数据到内存0x7C00处，并从这里开始执行。MBR的大小固定为512字节，因为只有512字节，所以它要做的第一件事就是把第二个扇区的512字节读到0x8000处，然后jmp到该处执行。