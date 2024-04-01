#include "bootpack.h"
#include <stdio.h>

extern struct FIFO8 keyfifo, mousefifo;

void HariMain(void) {

	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my, i;
	struct MOUSE_DEC mdec;
	unsigned int memtotal;
 	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;

	init_gdtidt();
	init_pic();
	io_sti();

	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	io_out8(PIC0_IMR, 0xf9); /* (11111001) */
	io_out8(PIC1_IMR, 0xef); /* (11101111) */

	init_keyboard();

	init_palette();
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
	// putfont8_asc(binfo->vram, binfo->scrnx, 8, 8, COL8_FFFFFF, "YinYunOS");
	// putfont8_asc(binfo->vram, binfo->scrnx, 31, 31, COL8_000000, "Hi,YinYun!");
	// putfont8_asc(binfo->vram, binfo->scrnx, 30, 30, COL8_FFFFFF, "Hi,YinYun!");
	
	mx = (binfo->scrnx - 16) / 2;
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	sprintf(s, "(%d, %d)", mx, my);
	putfont8_asc(binfo->vram, binfo->scrnx, 16, 64, COL8_FFFFFF, s);

	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	enable_mouse(&mdec);
	
	for(;;) {
		io_cli();
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_stihlt();
		} else {
			if (fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 72, 15, 87);
				putfont8_asc(binfo->vram, binfo->scrnx, 0, 72, COL8_FFFFFF, s);
			} else if (fifo8_status(&mousefifo) != 0) {
				i = fifo8_get(&mousefifo);
 				io_sti();
 				if (mouse_decode(&mdec, i) != 0) {
 				/* 数据的3个字节都齐了，显示出来 */
 					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
 					if ((mdec.btn & 0x01) != 0) {
 						s[1] = 'L';
 					}
 					if ((mdec.btn & 0x02) != 0) {
 						s[3] = 'R';
 					}
 					if ((mdec.btn & 0x04) != 0) {
 						s[2] = 'C';
 					}
 					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 48, 32 + 15 * 8 - 1, 63);
 					putfont8_asc(binfo->vram, binfo->scrnx, 32, 48, COL8_FFFFFF, s);
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15); /* 隐藏鼠标 */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 16) {
						mx = binfo->scrnx - 16;
					}
					if (my > binfo->scrny - 16) {
						my = binfo->scrny - 16;
					}
					sprintf(s, "(%3d, %3d)", mx, my);
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15); /* 隐藏坐标  */
					putfont8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s); /* 显示坐标 */
					putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); /*描画鼠标 */
					sprintf(s, "memory %dMB   free : %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
    				putfont8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
 				}
			}
		}
	}
}
