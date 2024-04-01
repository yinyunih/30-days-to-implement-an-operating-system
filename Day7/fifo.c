#include "bootpack.h"

void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf) {
    /* fifo的初始化 */
    fifo->size = size;
    fifo->buf = buf;
    fifo->free = size;
    fifo->flags = 0;
    fifo->next = 0;
    fifo->pre = 0;
}

int fifo8_put(struct FIFO8 *fifo, unsigned char data) {
    /* 向fifo传送数据 */
    if (fifo->free == 0) {
        /* 溢出 */
        fifo->flags |= FLAGS_OVERRUN;
        return -1;
    }
    fifo->buf[fifo->next] = data;
    fifo->next++;
    if (fifo->next == fifo->size) {
        fifo->next = 0;
    }
    fifo->free--;
    return 0;
}

int fifo8_get(struct FIFO8 *fifo) {
    /* 向fifo取数据 */
    int data;
    if (fifo->free == fifo->size) {
        /* 空 */
        return -1;
    }
    data = fifo->buf[fifo->pre];
    fifo->pre++;
    if (fifo->pre == fifo->size) {
        fifo->pre = 0;
    }
    fifo->free++;
    return data;
}

int fifo8_status(struct FIFO8 *fifo) {
    /* 查看当前的数据数量 */
    return fifo->size - fifo->free;
}