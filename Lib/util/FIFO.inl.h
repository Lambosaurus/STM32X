#ifndef FIFO_INL_H
#define FIFO_INL_H

/*
 * PRIVATE DEFINITIONS
 */

/*
 * INLINE PROTOTYPES
 */

/*
 * INLINE FUNCTION DEFINITIONS
 */

static inline void FIFO_INL_Init(FIFO_t * fifo, uint32_t size)
{
	fifo->tail = fifo->head = 0;
}

static inline void FIFO_INL_Clear(FIFO_t * fifo, uint32_t size)
{
	fifo->tail = fifo->head;
}

static inline uint32_t FIFO_INL_Count(FIFO_t * fifo, uint32_t size)
{
	return FIFO_INDEX_SUB(size, fifo->head, fifo->tail);
}

static inline uint32_t FIFO_INL_Free(FIFO_t * fifo, uint32_t size)
{
	return (size - 1) - FIFO_INDEX_SUB(size, fifo->head, fifo->tail);
}

static inline uint32_t FIFO_INL_Discard(FIFO_t * fifo, uint32_t size, uint32_t n)
{
	uint32_t count = FIFO_INL_Count(fifo, size);
	uint32_t to_discard = count < n ? count : n;
	fifo->tail = FIFO_INDEX_ADD(size, fifo->tail, to_discard);
	return to_discard;
}

static inline bool FIFO_INL_Put(FIFO_t * fifo, uint32_t size, uint8_t b)
{
	uint32_t next_head = FIFO_INDEX_ADD(size, fifo->head, 1);
	if (next_head != fifo->tail)
	{
		fifo->bfr[fifo->head] = b;
		fifo->head = next_head;
		return true;
	}
	return false;
}

static inline bool FIFO_INL_Pop(FIFO_t * fifo, uint32_t size, uint8_t * b)
{
	uint32_t tail = fifo->tail;
	if (tail != fifo->head)
	{
		*b = fifo->bfr[tail];
		fifo->tail = FIFO_INDEX_ADD(size, tail, 1);
		return true;
	}
	return false;
}

static inline uint32_t FIFO_INL_Write(FIFO_t * fifo, uint32_t size, const uint8_t * src, uint32_t src_size)
{
	uint32_t space = FIFO_INL_Free(fifo, size);
	uint32_t to_write = space < src_size ? space : src_size;
	if (to_write > 0)
	{
		FIFO_INL_BlindWrite(fifo, size, src, to_write);
	}
	return to_write;
}

static inline uint32_t FIFO_INL_Read(FIFO_t * fifo, uint32_t size, uint8_t * dst, uint32_t dst_size)
{
	uint32_t count = FIFO_INL_Count(fifo, size);
	uint32_t to_read = count < dst_size ? count : dst_size;
	if (to_read > 0)
	{
		FIFO_INL_BlindRead(fifo, size, dst, to_read);
	}
	return to_read;
}

static inline bool FIFO_INL_Enqueue(FIFO_t * fifo, uint32_t size, const void * src, uint32_t src_size)
{
	if (FIFO_INL_Free(fifo, size) >= src_size)
	{
		FIFO_INL_BlindWrite(fifo, size, src, src_size);
		return true;
	}
	return false;
}

static inline bool FIFO_INL_Dequeue(FIFO_t * fifo, uint32_t size, void * dst, uint32_t dst_size)
{
	if (FIFO_INL_Count(fifo, size) >= dst_size)
	{
		FIFO_INL_BlindRead(fifo, size, dst, dst_size);
		return true;
	}
	return false;
}


static inline void FIFO_INL_BlindPut(FIFO_t * fifo, uint32_t size, uint8_t b)
{
	uint32_t head = fifo->head;
	fifo->bfr[head] = b;
	fifo->head = FIFO_INDEX_ADD(size, head, 1);
}

static inline uint8_t FIFO_INL_BlindPop(FIFO_t * fifo, uint32_t size)
{
	uint32_t tail = fifo->tail;
	uint8_t b = fifo->bfr[tail];
	fifo->tail = FIFO_INDEX_ADD(size, tail, 1);
	return b;
}

static inline void FIFO_INL_BlindWrite(FIFO_t * fifo, uint32_t size, const uint8_t * src, uint32_t src_size)
{
	uint32_t head = fifo->head;
	uint32_t next_head = FIFO_INDEX_ADD(size, head, src_size);
	if (next_head > head)
	{
		// We can write continuously into the buffer
		memcpy(fifo->bfr + head, src, src_size);
	}
	else
	{
		// We write to end of buffer, then write from the start
		uint32_t chunk = size - head;
		memcpy(fifo->bfr + head, src, chunk);
		memcpy(fifo->bfr, src + chunk, src_size - chunk);
	}

	fifo->head = next_head;
}

static inline void FIFO_INL_BlindRead(FIFO_t * fifo, uint32_t size, uint8_t * dst, uint32_t dst_size)
{
	uint32_t tail = fifo->tail;
	uint32_t next_tail = FIFO_INDEX_ADD(size, tail, dst_size);
	if (next_tail > tail)
	{
		// We can read continuously from the buffer
		memcpy(dst, fifo->bfr + tail, dst_size);
	}
	else
	{
		// We read to end of buffer, then read from the start
		uint32_t chunk = size - tail;
		memcpy(dst, fifo->bfr + tail, chunk);
		memcpy(dst + chunk, fifo->bfr, dst_size - chunk);
	}

	fifo->tail = next_tail;
}

#endif //FIFO_INL_H
