# 30天自制操作系统---第一天

## 1. 编写二进制文件，打印*hello world*
* 安装"二进制编译器"----[BZ](http://www.vcraft.jp/soft/bz.html)

* 打开BZ编写二进制程序

* 保存为helloos.img (img格式为盘映像文件)

* 在VMware中选择该文件为软盘映像文件启动，成功显示*hello world*！（**喜**）

## 2. 编写汇编程序
* 新建一个记事本，写入相应的汇编代码，保存为.asm（汇编语言源程序的扩展名文件）

* 安装汇编语言编译器----[NASM](https://www.nasm.us/pub/nasm/releasebuilds/2.14.02/)
    
* 在保存.asm文件的路径打开终端，输入如下代码（xx是.asm的文件名）

        nasm xx.asm -o xx.img

* 产生了如下报错（**悲**），解决方法参考该[文章](https://admarimoin.hatenablog.com/entry/2018/09/03/225406)

        hello.asm:41: error: attempt to reserve non-constant quantity of BSS space

* 解决报错后成功生成.img文件，成功在VMware上显示了自己设置的字符串（**喜**）

## 收获
### 1. 了解二进制程序和汇编程序，理解其他语言程序与二进制程序的关系
### 2. 学会使用二进制编辑器
### 3. 学会了汇编语言的几个指令和变量
* DB指令--"define byte"的缩写,是往文件里直接写入1个字节的指令。
* RESB指令--是"reserve byte"的略写,可以实习从现在的地址开始空出指定数量的字节，并且在空出的地址上写入0x00。
* DW指令和DD指令--它们分别是“define word”和“ddefine double-word”的缩写，和DB指令类似，分别是2个字节和4个字节。
* '$$'和'$'--貌似和程序的位置相关（**没能完全理解，悲！**）
### 4. 了解Boot Sector
* 计算机打开时，加电自检（POST）寻找启动盘，如果是软盘启动，就会去读取第一个扇区（0面0磁道1扇区），每个扇区512B，只有当最后两个字节是0xAA55时，才会被认为时Boot Sector（引导扇区），然后开始执行这个程序。如果最后两个字节不是0xAA55，则会认为没有引导扇区，报一个无法启动的错误！