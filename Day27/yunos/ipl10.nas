; hello-os
; TAB=4

CYLS	EQU		10

		ORG		0x7c00			; 指明程序装载地址

; 标准FAT12格式软盘专用的代码 Stand FAT12 format floppy code

		JMP		entry
		DB		0x90
		DB		"HELLOIPL"		; 启动扇区名称（8字节）
		DW		512				; 每个扇区（sector）大小（必须512字节）
		DB		1				; 簇（cluster）大小（必须为1个扇区）
		DW		1				; FAT起始位置（一般为第一个扇区）
		DB		2				; FAT个数（必须为2）
		DW		224				; 根目录大小（一般为224项）
		DW		2880			; 该磁盘大小（必须为2880扇区1440*1024/512）
		DB		0xf0			; 磁盘类型（必须为0xf0）
		DW		9				; FAT的长度（必须是9扇区）
		DW		18				; 一个磁道（track）有几个扇区（必须为18）
		DW		2				; 磁头数（必须是2）
		DD		0				; 不使用分区，必须是0
		DD		2880			; 重写一次磁盘大小
		DB		0,0,0x29		; 意义不明（固定）
		DD		0xffffffff		; （可能是）卷标号码
		DB		"HELLO-OS   "	; 磁盘的名称（必须为11字节，不足填空格）
		DB		"FAT12   "		; 磁盘格式名称（必须是8字节，不足填空格）
		TIMES	18	DB	0		; 先空出18字节

; 程序主体

entry:
		MOV		AX,0			; 初始化寄存器
		MOV		SS,AX
		MOV		SP,0x7c00
		MOV		DS,AX

		MOV		AX,0x0820
		MOV		ES,AX
		MOV		CH,0			; 柱面号 0
		MOV		DH,0			; 磁头号 0
		MOV		CL,2			; 扇区号 2
		
readloop:
		MOV		SI,0			; 记录失败次数寄存器
		
retry:
		MOV		AH,0x02			; AH=0x02---读入磁盘
		MOV		AL,1			; 一次处理1个扇区
		MOV		BX,0			; 缓冲地址[ES:BS]
		MOV		DL,0x00			; 0号软盘驱动器
		INT		0x13			; 调用磁盘BIOS
		JNC		next			; 没错误就跳转到next
		ADD		SI,1			; 失败数+1
		CMP		SI,5			
		JAE		errors			; 失败数>=5
		MOV		AH,0x00
		MOV		DL,0x00
		INT		0x13			; 重置驱动器
		JMP		retry
		
next:
		MOV		AX,ES
		ADD		AX,0x0020
		MOV		ES,AX			; 内存地址后移512字节
		ADD		CL,1			; 扇区号+1
		CMP		CL,18
		JBE		readloop		; 扇区号<=18 readloop
		MOV		CL,1
		ADD		DH,1
		CMP		DH,2
		JB		readloop		; 磁头号<2 readloop
		MOV		DH,0
		ADD		CH,1
		CMP		CH,CYLS
		JB		readloop		; CH < CYLS readloop
		
		MOV		[0x0ff0],CH
		JMP		0xc200

errors:
		MOV		SI,msg
		
putloop:
		MOV		AL,[SI]
		ADD		SI,1			; 给SI加1
		CMP		AL,0
		JE		fin
		MOV		AH,0x0e			; 显示一个文字
		MOV		BX,15			; 指定字符颜色
		INT		0x10			; 调用显卡BIOS
		JMP		putloop
fin:
		HLT						; 让CPU停止，等待指令
		JMP		fin				; 无限循环

msg:
		DB		0x0a, 0x0a		; 换行两次
		DB		"load faild!"
		DB		0x0a			; 换行
		DB		0

		TIMES	0x1fe-($-$$)	DB	0; 填写0x00直到0x001fe

		DB		0x55, 0xaa