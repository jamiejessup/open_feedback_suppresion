#ifndef RING_BUFFER_H
#define RING_BUFFER_H

typedef struct {
    int size;
    int start;
    int end;
    int fill_count;
    double *elems;
} RingBuffer;

int ring_buffer_init(RingBuffer *ring_buf, unsigned size);
void ring_buffer_free(RingBuffer *ring_buf);
void ring_buffer_write(RingBuffer *ring_buf, double data);
void ring_buffer_copy(RingBuffer *ring_buf, double *dest);


#endif // RING_BUFFER_H
