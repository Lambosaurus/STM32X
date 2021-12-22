#ifndef MTP_FS_H
#define MTP_FS_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

#define MTP_MAX_OBJECTS			16

/*
 * PUBLIC TYPES
 */

typedef struct {
	uint16_t type;
	const char * name;
	uint32_t size;
	bool (*read)(uint8_t * bfr, uint32_t pos, uint32_t count);
	bool (*write)(const uint8_t * bfr, uint32_t pos, uint32_t count);
} MTP_File_t;

typedef struct {
	bool in_session;
	MTP_File_t * objects[MTP_MAX_OBJECTS];
	struct {
		uint32_t offset;
		uint32_t remaining;
		bool (*callback)(uint8_t * bfr, uint32_t pos, uint32_t count);
	} transaction;
} MTP_t;

/*
 * PUBLIC FUNCTIONS
 */

void MTP_Init(MTP_t * mtp);
bool MTP_AddFile(MTP_t * mtp, MTP_File_t * file);
void MTP_RemoveFile(MTP_t * mtp, MTP_File_t * file);
MTP_File_t * MTP_GetObjectById(MTP_t * mtp, uint32_t object_id);

/*
 * EXTERN DECLARATIONS
 */

#endif //MTP_FS_H
