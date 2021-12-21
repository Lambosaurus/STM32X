
#include "MTP_FS.h"
#include "MTP_Defs.h"

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
}

bool MTP_AddFile(MTP_t * mtp, MTP_File_t * file)
{
	for (uint32_t i = 0; i < LENGTH(mtp->objects); i++)
	{
		// Find the next free slot to insert a file
		if (mtp->objects[i] == NULL)
		{
			if (file->type == 0)
			{
				// Assign the file format.
				file->type = MTP_OBJ_FORMAT_TEXT; //MTP_OBJ_FORMAT_UNDEFINED;
			}
			mtp->objects[i] = file;
			return true;
		}
	}
	return false;
}

void MTP_RemoveFile(MTP_t * mtp, MTP_File_t * file)
{
	for (uint32_t i = 0; i < LENGTH(mtp->objects); i++)
	{
		// Locate the file so it may be deleted.
		if (mtp->objects[i] == file)
		{
			mtp->objects[i] = NULL;
			return;
		}
	}
}

MTP_File_t * MTP_GetObjectById(MTP_t * mtp, uint32_t object_id)
{
	object_id -= 1;
	if (object_id < LENGTH(mtp->objects))
	{
		return mtp->objects[object_id];
	}
	return NULL;
}

/*
 * PRIVATE FUNCTIONS
 */
