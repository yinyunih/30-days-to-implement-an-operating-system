#include "bootpack.h"
#include<stdio.h>

void HariMain(void) {

	struct BOOTINFO *binfo;
	char s[40], mcursor[256];
	int mx, my;

	binfo = (struct BOOTINFO *) 0x0ff0;

	init_gdtidt();
	init_pic();
	io_sti();

	init_palette();
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
	putfont8_asc(binfo->vram, binfo->scrnx, 8, 8, COL8_FFFFFF, "YinYunOS");
	putfont8_asc(binfo->vram, binfo->scrnx, 31, 31, COL8_000000, "Hi,YinYun!");
	putfont8_asc(binfo->vram, binfo->scrnx, 30, 30, COL8_FFFFFF, "Hi,YinYun!");
	mx = (binfo->scrnx - 16) / 2;
	my = (binfo->scrny - 28 - 16) / 2;
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	sprintf(s, "scrnx = %d", binfo->scrnx);
	putfont8_asc(binfo->vram, binfo->scrnx, 16, 64, COL8_FFFFFF, s);

	io_out8(PIC0_IMR, 0xf9); /* (11111001) */
	io_out8(PIC1_IMR, 0xef); /* (11101111) */
	
	for(;;) {
		io_hlt();
	}
	
}
