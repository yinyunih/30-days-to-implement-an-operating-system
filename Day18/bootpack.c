#include "bootpack.h"
#include <stdio.h>

extern struct TIMERCTL timerctl;

void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
void console_task(struct SHEET *sheet, unsigned int memtotal);
int cons_newline(int cursor_y, struct SHEET *sheet);

void HariMain(void) {

	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	struct FIFO32 fifo, keycmd;
	char s[40];
	int fifobuf[128], keycmd_buf[32];
	int mx, my, i, cursor_x, cursor_c;
	struct MOUSE_DEC mdec;
	unsigned int memtotal;
 	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons;
	unsigned char *buf_back, buf_mouse[256], *buf_win, *buf_cons;
	struct TIMER *timer;
	static char keytable0[0x80] = {
		0,   0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0,   0,   ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0x5c, 0,  0,   0,   0,   0,   0,   0,   0,   0,   0x5c, 0,  0
	};
	static char keytable1[0x80] = {
		0,   0,   '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', 0,   0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0,   0,   'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0,   0,   '}', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ', 0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
		0,   0,   0,   '_', 0,   0,   0,   0,   0,   0,   0,   0,   0,   '|', 0,   0
	};
	struct TASK *task_a, *task_cons;
	int key_to = 0, key_shift = 0, key_leds = (binfo->leds >> 4) & 7, keycmd_wait = -1;

	/*GDT和IDT初始化,PIC初始化*/
	init_gdtidt();
	init_pic();
	io_sti();

	fifo32_init(&fifo, 128, fifobuf, 0);
	init_pit();
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);
	io_out8(PIC0_IMR, 0xf8); /* (11111000) */
	io_out8(PIC1_IMR, 0xef); /* (11101111) */
	fifo32_init(&keycmd, 32, keycmd_buf, 0);

	/* 内存管理 */
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);
	
	init_palette();
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 0);

	/* sht_back */
	sht_back = sheet_alloc(shtctl);
	buf_back = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	init_screen(buf_back, binfo->scrnx, binfo->scrny);

	/* sht_win */
	sht_win = sheet_alloc(shtctl);
	buf_win = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_win, buf_win, 144, 52, -1);
	make_window8(buf_win, 144, 52, "task_a", 1);
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;
	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);	
	
	/* sht_mouse */
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	init_mouse_cursor8(buf_mouse, 99);
	mx = (binfo->scrnx - 16) / 2;
	my = (binfo->scrny - 28 - 16) / 2;

	/* sht_cons */
	sht_cons = sheet_alloc(shtctl);
	buf_cons = (unsigned char *) memman_alloc_4k(memman, 256 * 165);
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1); /*无透明色*/
	make_window8(buf_cons, 256, 165, "console", 0);
	make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
	task_cons = task_alloc();
	task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
	task_cons->tss.eip = (int) &console_task;
	task_cons->tss.es = 1 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	*((int *) (task_cons->tss.esp + 4)) = (int) sht_cons;
	*((int *) (task_cons->tss.esp + 8)) = memtotal; /*这里！ */
	task_run(task_cons, 2, 2); /* level=2, priority=2 */
	
	sheet_slide(sht_back, 		0,   0);
	sheet_slide(sht_win, 		8,  76);
	sheet_slide(sht_mouse,     mx,  my);
	sheet_slide(sht_cons, 	   168, 76);
	sheet_updown(sht_back,	   0);
	sheet_updown(sht_cons,     1);
	sheet_updown(sht_win, 	   2);
	sheet_updown(sht_mouse,    3);

	fifo32_put(&keycmd, KEYCMD_LED);
	fifo32_put(&keycmd, key_leds);
	
	for(;;) {
		if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
            /*如果存在向键盘控制器发送的数据，则发送它  */
            keycmd_wait = fifo32_get(&keycmd);
            wait_KBC_sendready();
            io_out8(PORT_KEYDAT, keycmd_wait);
        }
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			task_sleep(task_a);
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (i >= 256 && i <= 511) {
				// 键盘数据
				if (i < 0x80 + 256) { 
					/*将按键编码转换为字符编码*/
                    if (key_shift == 0) {
                        s[0] = keytable0[i - 256];
                    } else {
                        s[0] = keytable1[i - 256];
                    }
                } else {
                    s[0] = 0;
                }
				if ('A' <= s[0] && s[0] <= 'Z') {   /*当输入字符为英文字母时*/
                    if (((key_leds & 4) == 0 && key_shift == 0) ||
                            ((key_leds & 4) != 0 && key_shift != 0)) {
                        s[0] += 0x20;   /*将大写字母转换为小写字母*/
                    }
			    }
				if (s[0] != 0) { /*一般字符*/
                    if (key_to == 0) {  /*发送给任务A */
                        if (cursor_x < 128) {
                            /*显示一个字符之后将光标后移一位*/
                            s[1] = 0;
                            putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
                            cursor_x += 8;
                        }
                    } else {    /*发送给命令行窗口*/
                        fifo32_put(&task_cons->fifo, s[0] + 256);
                    }
                }
				if (i == 256 + 0x0e) { /* 退格键  */
					if (key_to == 0) {
						if (cursor_x > 8) {
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
                			cursor_x -= 8;
						}
					} else {
						fifo32_put(&task_cons->fifo, 8 + 256);
					}
            	}
				if (i == 256 + 0x0f) { /* Tab键*/
                    if (key_to == 0) {
                        key_to = 1;
                        make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  0);
                        make_wtitle8(buf_cons, sht_cons->bxsize, "console", 1);
						cursor_c = -1;
						boxfill8(sht_win->buf, sht_win->bxsize, COL8_FFFFFF, cursor_x, 28, cursor_x + 7, 43);
						fifo32_put(&task_cons->fifo, 2);
                    } else {
                        key_to = 0;
                        make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  1);
                        make_wtitle8(buf_cons, sht_cons->bxsize, "console", 0);
						cursor_c = COL8_000000;
						fifo32_put(&task_cons->fifo, 3);
                    }
                    sheet_refresh(sht_win,  0, 0, sht_win->bxsize,  21);
                    sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
			    }
				if (i == 256 + 0x1c) {  /*回车键*/
                    if (key_to != 0) {  /*发送至命令行窗口*/
                        fifo32_put(&task_cons->fifo, 10 + 256);
                    }
			    }
				if (i == 256 + 0x2a) {  /*左Shift ON */
                    key_shift |= 1;
                }
                if (i == 256 + 0x36) {  /*右Shift ON */
                    key_shift |= 2;
                }
                if (i == 256 + 0xaa) {  /*左Shift OFF */
                    key_shift &= ~1;
                }
                if (i == 256 + 0xb6) {  /*右Shift OFF */
                    key_shift &= ~2;
                }
				if (i == 256 + 0x3a) {  /* CapsLock */
                    key_leds ^= 4;
                    fifo32_put(&keycmd, KEYCMD_LED);
                    fifo32_put(&keycmd, key_leds);
                }
                if (i == 256 + 0x45) {  /* NumLock */
                    key_leds ^= 2;
                    fifo32_put(&keycmd, KEYCMD_LED);
                    fifo32_put(&keycmd, key_leds);
                }
                if (i == 256 + 0x46) {  /* ScrollLock */
                    key_leds ^= 1;
                    fifo32_put(&keycmd, KEYCMD_LED);
                    fifo32_put(&keycmd, key_leds);
                }
                if (i == 256 + 0xfa) {  /*键盘成功接收到数据*/
                    keycmd_wait = -1;
                }
                if (i == 256 + 0xfe) {  /*键盘没有成功接收到数据*/
                    wait_KBC_sendready();
                    io_out8(PORT_KEYDAT, keycmd_wait);
			    }
            	/* 光标再显示 */
				if (cursor_c >= 0) {
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				}
            	sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			} else if (i >= 512 && i <= 767) {
				// 鼠标数据
 				if (mouse_decode(&mdec, (unsigned char)(i - 512)) != 0) {
 				/* 数据的3个字节都齐了，显示出来 */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 1) {
						mx = binfo->scrnx - 1;
					}
					if (my > binfo->scrny - 1) {
						my = binfo->scrny - 1;
					}
					sheet_slide(sht_mouse, mx, my);
					if ((mdec.btn & 0x01) != 0) {
 						sheet_slide(sht_win, mx - 80, my - 8);
 					}
 				}
			} else if (i <= 1) { /* 光标用定时器 */
            	if (i != 0) {
					timer_init(timer, &fifo, 0);
					if (cursor_c >= 0) {
						cursor_c = COL8_000000;
					}
				} else {
					timer_init(timer, &fifo, 1);
					if (cursor_c >= 0) {
						cursor_c = COL8_FFFFFF;
					}
				}
				timer_settime(timer, 50);
				if (cursor_c >= 0) {
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
					sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
				}
 			}
		}
	}
}

void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l) {
	boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
	putfont8_asc(sht->buf, sht->bxsize, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
	return;
}

void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c)
{
	int x1 = x0 + sx, y1 = y0 + sy;
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, c,           x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
}

void console_task(struct SHEET *sheet, unsigned int memtotal) {

	struct TIMER *timer;
	struct TASK *task = task_now();
	int i, fifobuf[128], cursor_x = 16,cursor_y = 28, cursor_c = -1;
	char s[30], cmdline[30];
	int x, y;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct FILEINFO *finfo = (struct FILEINFO *) (ADR_DISKIMG + 0x002600);

	fifo32_init(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);

	putfonts8_asc_sht(sheet, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);

	for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {
			task_sleep(task);
			io_sti();
		} else {
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1) { /*光标用定时器*/
				if (i != 0) {
					timer_init(timer, &task->fifo, 0); /*下次置0 */
					if (cursor_c >= 0) {
						cursor_c = COL8_FFFFFF;
					}
				} else {
					timer_init(timer, &task->fifo, 1); /*下次置1 */
					if (cursor_c >= 0) {
						cursor_c = COL8_000000;
					}
				}
				timer_settime(timer, 50);
			} 
			if (i == 2) {
				cursor_c = COL8_FFFFFF;
			}
			if (i == 3) {
				boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
                cursor_c = -1;
			}
			if (256 <= i && i <= 511) { /*键盘数据（通过任务A） */
				if (i == 8 + 256) {
					/*退格键*/
					if (cursor_x > 16) {
                        /*用空白擦除光标后将光标前移一位*/
                        putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
                        cursor_x -= 8;
                    }
                } else if (i == 10 + 256) {
					/*回车键*/
					putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
					cmdline[cursor_x / 8 - 2] = 0;
					cursor_y = cons_newline(cursor_y, sheet);
					 if (strcmp(cmdline, "mem") == 0) {
                        /* mem命令*/
                        sprintf(s, "total   %dMB",  memtotal / (1024 * 1024));
                        putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
                        cursor_y = cons_newline(cursor_y, sheet);
                        sprintf(s, "free %dKB", memman_total(memman) / 1024);
                        putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
                        cursor_y = cons_newline(cursor_y, sheet);
                        cursor_y = cons_newline(cursor_y, sheet);
                    } else if (strcmp(cmdline, "cls") == 0) {
						/* cls命令 */
						for (y = 28; y < 28 + 128; y++) {
                            for (x = 8; x < 8 + 240; x++) {
                                sheet->buf[x + y * sheet->bxsize] = COL8_000000;
                            }
                        }
                        sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
						cursor_y = 28;
					}  else if (strcmp(cmdline, "ls") == 0) {
						/* dir命令 */
						for (x = 0; x < 224; x++) {
                            if (finfo[x].name[0] == 0x00) {
                                break;
                            }
                            if (finfo[x].name[0] != 0xe5) {
                                if ((finfo[x].type & 0x18) == 0) {
                                    sprintf(s, "filename.ext   %7d", finfo[x].size);
                                    for (y = 0; y < 8; y++) {
                                        s[y] = finfo[x].name[y];
                                    }
                                    s[9] = finfo[x].ext[0];
                                    s[10] = finfo[x].ext[1];
                                    s[11] = finfo[x].ext[2];
                                    putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
                                    cursor_y = cons_newline(cursor_y, sheet);
                                }
                            }
                        }
			            cursor_y = cons_newline(cursor_y, sheet);
					} else if (cmdline[0] != 0) {
                        /*不是命令，也不是空行  */
                        putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, "Bad command.", 12);
                        cursor_y = cons_newline(cursor_y, sheet);
                        cursor_y = cons_newline(cursor_y, sheet);
				    }
					putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, ">", 1);
					cursor_x = 16;
				} else {
                    /*一般字符*/
                    if (cursor_x < 240) {
                        /*显示一个字符之后将光标后移一位  */
                        s[0] = i - 256;
                        s[1] = 0;
						cmdline[cursor_x / 8 - 2] = i - 256;
                        putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
                        cursor_x += 8;
                    }
                }
            }
			if (cursor_c >= 0) {
				boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
			}
			sheet_refresh(sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
		}
	}
}

int cons_newline(int cursor_y, struct SHEET *sheet) {
	int x, y;
	if (cursor_y < 28 + 112) {
		cursor_y += 16; /*换行*/
	} else {
		/*滚动*/
		for (y = 28; y < 28 + 112; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
			}
		}
		for (y = 28 + 112; y < 28 + 128; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = COL8_000000;
			}
		}
		sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
	}
	return cursor_y;
}
