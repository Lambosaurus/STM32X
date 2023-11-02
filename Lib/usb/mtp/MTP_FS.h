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
	struct {
		uint32_t id;
		uint16_t type;
	} mtp;

	const char * name;
	uint32_t size;
	bool (*read)(uint8_t * bfr, uint32_t pos, uint32_t count);
	bool (*write)(const uint8_t * bfr, uint32_t pos, uint32_t count);
} MTP_File_t;

typedef MTP_File_t * (*MTP_NewFileCallback_t)(const char * name, uint32_t size);

typedef struct {
	bool in_session;
	uint32_t next_id;
	MTP_File_t * objects[MTP_MAX_OBJECTS];
	MTP_NewFileCallback_t new_file_callback;
	struct {
		uint32_t offset;
		uint32_t remaining;
		bool (*callback)(uint8_t * bfr, uint32_t pos, uint32_t count);
	} transaction;
	MTP_File_t * new_file;
} MTP_t;

/*
 * PUBLIC FUNCTIONS
 */

void MTP_Init(MTP_t * mtp);
bool MTP_AddFile(MTP_t * mtp, MTP_File_t * file);
uint32_t MTP_FreeObjects(MTP_t * mtp);
void MTP_RemoveFile(MTP_t * mtp, MTP_File_t * file);
void MTP_UpdateFile(MTP_t * mtp, MTP_File_t * file);

void MTP_OnNewFile(MTP_t * mtp, MTP_NewFileCallback_t callback);

MTP_File_t * MTP_GetObjectById(MTP_t * mtp, uint32_t object_id);
bool MTP_AddFileInternal(MTP_t * mtp, MTP_File_t * file);

/*
 * EXTERN DECLARATIONS
 */

#endif //MTP_FS_H
