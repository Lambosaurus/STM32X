#ifndef USB_MSC_STORAGE_H
#define USB_MSC_STORAGE_H

#include <stdint.h>
#include <stdbool.h>

/*
 * PUBLIC DEFINITIONS
 */

#define USB_MSC_BLOCK_SIZE		512

/*
 * PUBLIC TYPES
 */

typedef struct {
	bool (*open)(uint32_t * blk_count);
	bool (*read)(uint8_t * bfr, uint32_t blk_addr, uint16_t blk_count);
	bool (*write)(uint8_t * bfr, uint32_t blk_addr, uint16_t blk_count);
}USB_MSC_Storage_t;



#endif //USB_MSC_STORAGE_H
