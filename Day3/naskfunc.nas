; naskfunc
; TAB=4

[FORMAT "WCOFF"]				; 制作目标文件的模式
[BITS 32]						; 32位模式的机器语言


; 制作目标文件的信息

[FILE "naskfunc.nas"]			; 源文件名信息

		GLOBAL	_io_hlt			; 程序中的函数名
		
		
; 实际的函数

[SECTION .text]		; 目标文件中写了这些后再写程序

_io_hlt:	; 对应C语言中 void io_hlt(void);
		HLT
		RET