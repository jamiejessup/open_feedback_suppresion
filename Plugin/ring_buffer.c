#include "ring_buffer.h"

int ring_buffer_init(RingBuffer *ring_buf, unsigned size)
{
    if( (ring_buf->elems = (double *) malloc(size*sizeof(double))) != NULL ){
        ring_buf->size = size;
        ring_buf->end = 0;
        ring_buf->start = 0;

        //init the buffer to zero
        for(unsigned i = 0; i<size; i++){
            ring_buf->elems[i] = 0;
        }

        return 0;
    } else
        return -1;
}

void ring_buffer_free(RingBuffer *ring_buf) {
    free(ring_buf->start_buf);
}

void ring_buffer_write(RingBuffer *ring_buf, double data) {
    ring_buf->elems[ring_buf->end] = data;
    ring_buf->end = (ring_buf->end + 1) % ring_buf->size;
    //check if start move is necessary
    if(ring_buf->fill_count < ring_buf->size){
        ring_buf->fill_count++;
    } else {
        ring_buf->start = (ring_buf->start +1) % ring_buf->size;
    }
}

void ring_buffer_copy(RingBuffer *ring_buf, double *dest) {

}
