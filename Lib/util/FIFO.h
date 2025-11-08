#ifndef FIFO_H
#define FIFO_H

#include <stdint.h>
#include <string.h>

/*
 * PUBLIC DEFINITIONS
 */

#define FIFO_INDEX_ADD_MOD(_size, _offset, _step)		((_offset) + (_step) < (_size) ? (_offset) + (_step) : (_offset) + (_step) - (_size))
#define FIFO_INDEX_SUB_MOD(_size, _offset, _step)		((_offset) < (_step) ? (_offset) + (_size) - (_step) : (_offset) - (_step))

#ifdef DEBUG
#define FIFO_INDEX_ADD(_size, _offset, _step)			FIFO_INDEX_ADD_MOD(_size, _offset, _step)
#define FIFO_INDEX_SUB(_size, _offset, _step)			FIFO_INDEX_SUB_MOD(_size, _offset, _step)
#else
// In an unoptimized build, the below will probably not get dead-branch pruned.
#define FIFO_IS_POW2(_x) 						(((_x) & ((_x) - 1)) == 0)
#define FIFO_INDEX_ADD(_size, _offset, _step)		(FIFO_IS_POW2(_size) ? (((_offset) + (_step)) & ((_size) - 1)) : FIFO_INDEX_ADD_MOD(_size, _offset, _step) )
#define FIFO_INDEX_SUB(_size, _offset, _step)		(FIFO_IS_POW2(_size) ? (((_offset) - (_step)) & ((_size) - 1)) : FIFO_INDEX_SUB_MOD(_size, _offset, _step) )
#endif

#define FIFO_UNPACK(_fifo)				&(_fifo)->base, sizeof((_fifo)->bfr)


// Used to declare a new fifo with a given size
// A power-of-two size is preferred, it will provide faster indexing
#define FIFO_DECLARE(_name, _size)		\
	struct { 							\
		FIFO_t base;					\
		uint8_t bfr[_size];				\
	} _name								\

/*
 * PUBLIC TYPES
 */

typedef struct {
	uint16_t head;
	uint16_t tail;
	uint8_t bfr[];
} FIFO_t;

/*
 * PUBLIC FUNCTIONS
 */

// Init can be skipped if the memory is zeroed (ie, bss)
#define FIFO_Init(_fifo)							FIFO_INL_Init(FIFO_UNPACK(_fifo))

// Clears the FIFO
#define FIFO_Clear(_fifo)							FIFO_INL_Clear(FIFO_UNPACK(_fifo))

// Returns number of bytes that may be read from the fifo
#define FIFO_Count(_fifo)							FIFO_INL_Count(FIFO_UNPACK(_fifo))

// Returns number of bytes that may be written to the fifo
#define FIFO_Free(_fifo)							FIFO_INL_Free(FIFO_UNPACK(_fifo))

// Puts a single byte into the FIFO
// Returns true if the byte was written
#define FIFO_Put(_fifo, b)							FIFO_INL_Put(FIFO_UNPACK(_fifo), b)

// Pulls a single byte out of the FIFO
// Returns true if a byte was read
#define FIFO_Pop(_fifo, b)							FIFO_INL_Pop(FIFO_UNPACK(_fifo), b)

// Discards a number of bytes from the buffer.
// Returns the number of bytes discarded
#define FIFO_Discard(_fifo, _n)						FIFO_INL_Discard(FIFO_UNPACK(_fifo), _n)

// Writes bytes to the fifo.
// Returns the number of bytes written.
#define FIFO_Write(_fifo, _src, _src_size)			FIFO_INL_Write(FIFO_UNPACK(_fifo), _src, _src_size)

// Read bytes from the fifo.
// Returns the number of bytes read
#define FIFO_Read(_fifo, _dst, _dst_size)			FIFO_INL_Read(FIFO_UNPACK(_fifo), _dst, _dst_size)

// Treating the fifo like a queue, inserts an object
// Returns true if the object was written
#define FIFO_Enqueue(_fifo, _src, _src_size)		FIFO_INL_Enqueue(FIFO_UNPACK(_fifo), _src, _src_size)

// Treating the fifo like a queue, removes an object
// Returns true if the object was read
#define FIFO_Dequeue(_fifo, _dst, _dst_size)		FIFO_INL_Dequeue(FIFO_UNPACK(_fifo), _dst, _dst_size)

// Puts a single byte into the FIFO.
// WARN: This does not check bounds.
#define FIFO_BlindPut(_fifo, b)						FIFO_INL_BlindPut(FIFO_UNPACK(_fifo), b)

// Pulls a single byte out of the FIFO.
// Returns the next byte in the FIFO.
// WARN: This does not check bounds.
#define FIFO_BlindPop(_fifo)						FIFO_INL_BlindPop(FIFO_UNPACK(_fifo))

// Writes bytes into the fifo
// WARN: This does not check bounds.
#define FIFO_BlindWrite(_fifo, _src, _src_size)		FIFO_INL_BlindWrite(FIFO_UNPACK(_fifo), _src, _src_size)

// Reads bytes out of the fifo
// WARN: This does not check bounds.
#define FIFO_BlindRead(_fifo, _dst, _dst_size)		FIFO_INL_BlindRead(FIFO_UNPACK(_fifo), _dst, _dst_size)


// The prototypes referenced above.
// Prefer the macros - they make a cleaner API.
static inline void FIFO_INL_Init(FIFO_t * fifo, uint16_t size);
static inline void FIFO_INL_Clear(FIFO_t * fifo, uint16_t size);
static inline uint16_t FIFO_INL_Count(FIFO_t * fifo, uint16_t size);
static inline uint16_t FIFO_INL_Free(FIFO_t * fifo, uint16_t size);
static inline uint16_t FIFO_INL_Discard(FIFO_t * fifo, uint16_t size, uint16_t n);
static inline bool FIFO_INL_Put(FIFO_t * fifo, uint16_t size, uint8_t b);
static inline bool FIFO_INL_Pop(FIFO_t * fifo, uint16_t size, uint8_t * b);
static inline uint16_t FIFO_INL_Write(FIFO_t * fifo, uint16_t size, const uint8_t * src, uint16_t src_size);
static inline uint16_t FIFO_INL_Read(FIFO_t * fifo, uint16_t size, uint8_t * dst, uint16_t dst_size);
static inline bool FIFO_INL_Enqueue(FIFO_t * fifo, uint16_t size, const void * src, uint16_t src_size);
static inline bool FIFO_INL_Dequeue(FIFO_t * fifo, uint16_t size, void * dst, uint16_t dst_size);
static inline void FIFO_INL_BlindPut(FIFO_t * fifo, uint16_t size, uint8_t b);
static inline uint8_t FIFO_INL_BlindPop(FIFO_t * fifo, uint16_t size);
static inline void FIFO_INL_BlindWrite(FIFO_t * fifo, uint16_t size, const uint8_t * src, uint16_t src_size);
static inline void FIFO_INL_BlindRead(FIFO_t * fifo, uint16_t size, uint8_t * dst, uint16_t dst_size);;

/*
 * EXTERN DECLARATIONS
 */

#include "FIFO.inl.h"

#endif //FIFO_H
