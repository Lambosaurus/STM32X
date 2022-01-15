
#include "MTP_FS.h"
#include "MTP_Defs.h"
#include "MTP.h"

#include <string.h>

/*
 * PRIVATE DEFINITIONS
 */

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

void MTP_Init(MTP_t * mtp)
{
	bzero(mtp, sizeof(MTP_t));
	mtp->next_id = 1;
	mtp->in_session = false;
}

bool MTP_AddFile(MTP_t * mtp, MTP_File_t * file)
{
	// TODO: This is an interrupt hazard

	if (MTP_AddFileInternal(mtp, file))
	{
		if (file->mtp.type == 0)
		{
			// Assign the file format.
			file->mtp.type = MTP_OBJ_FORMAT_UNDEFINED; //MTP_OBJ_FORMAT_TEXT
		}
		file->mtp.id = mtp->next_id++;
		MTP_UpdateFileEvent(mtp, file, MTP_EVENT_OBJECTADDED);
		return true;
	}
	return false;
}

bool MTP_AddFileInternal(MTP_t * mtp, MTP_File_t * file)
{
	for (uint32_t i = 0; i < LENGTH(mtp->objects); i++)
	{
		// Find the next free slot to insert a file
		if (mtp->objects[i] == NULL)
		{
			mtp->objects[i] = file;
			return true;
		}
	}
	return false;
}

void MTP_RemoveFile(MTP_t * mtp, MTP_File_t * file)
{
	// TODO: This is an interrupt hazard

	uint32_t object_id = file->mtp.id;
	for (uint32_t i = 0; i < LENGTH(mtp->objects); i++)
	{
		// Locate the file so it may be deleted.
		if (mtp->objects[i]->mtp.id == object_id)
		{
			mtp->objects[i] = NULL;
			MTP_UpdateFileEvent(mtp, file, MTP_EVENT_OBJECTREMOVED);
			return;
		}
	}
}

void MTP_UpdateFile(MTP_t * mtp, MTP_File_t * file)
{
	MTP_UpdateFileEvent(mtp, file, MTP_EVENT_OBJECTINFOCHANGED);
}

MTP_File_t * MTP_GetObjectById(MTP_t * mtp, uint32_t object_id)
{
	for (uint32_t i = 0; i < LENGTH(mtp->objects); i++)
	{
		if (mtp->objects[i]->mtp.id == object_id)
		{
			return mtp->objects[i];
		}
	}
	return NULL;
}

void MTP_OnNewFile(MTP_t * mtp, MTP_NewFileCallback_t callback)
{
	mtp->new_file_callback = callback;
}

uint32_t MTP_FreeObjects(MTP_t * mtp)
{
	uint32_t free = MTP_MAX_OBJECTS;
	for (uint32_t i = 0; i < LENGTH(mtp->objects); i++)
	{
		if (mtp->objects[i] != NULL)
		{
			free--;
		}
	}
	return free;
}

/*
 * PRIVATE FUNCTIONS
 */
