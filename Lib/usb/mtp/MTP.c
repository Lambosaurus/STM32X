
#include "MTP.h"
#include "../USB_Defs.h"
#include <string.h>

/*
 * PRIVATE DEFINITIONS
 */

#define MTP_STORAGE_ID		0x00010001

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

/*
 * PRIVATE VARIABLES
 */

static inline uint8_t * MTP_Read16(uint8_t * src, uint16_t * value);
static inline uint8_t * MTP_Read32(uint8_t * src, uint32_t * value);
static inline uint8_t * MTP_ReadString(uint8_t * src, char * str);

static uint8_t * MTP_WriteType(uint8_t * dst, uint16_t type, uint32_t data);
static uint8_t * MTP_Write128(uint8_t * dst, uint32_t value);
static uint8_t * MTP_Write64(uint8_t * dst, uint64_t value);
static uint8_t * MTP_Write32(uint8_t * dst, uint32_t value);
static inline uint8_t * MTP_Write16(uint8_t * dst, uint16_t value);
static inline uint8_t * MTP_Write8(uint8_t * dst, uint8_t value);
static uint8_t * MTP_WriteArray32(uint8_t * dst, const uint32_t * array, uint32_t count);
static uint8_t * MTP_WriteArray16(uint8_t * dst, const uint16_t * array, uint32_t count);
static uint8_t * MTP_WriteString(uint8_t * dst, const char * str);

// TODO: Does giving these the same signature improve performance?

static MTP_State_t MTP_OpenSession(MTP_t * mtp, MTP_Container_t * container);
static MTP_State_t MTP_GetDeviceInfo(MTP_Container_t * container);
//static MTP_State_t MTP_GetDevicePropertyDescriptor(MTP_Container_t * container);
static MTP_State_t MTP_GetStorageIDs(MTP_Container_t * container);
static MTP_State_t MTP_GetStorageInfo(MTP_Container_t * container, uint32_t storage_id);
static MTP_State_t MTP_GetObjectHandles(MTP_t * mtp, MTP_Container_t * container);
static MTP_State_t MTP_GetObjectInfo(MTP_t * mtp, MTP_Container_t * container, uint32_t object_id);
static MTP_State_t MTP_GetObjectPropertiesSupported(MTP_Container_t * container, uint32_t object_type);
static MTP_State_t MTP_GetObjectPropertyDescriptor(MTP_Container_t * container, uint32_t property_id);
static MTP_State_t MTP_GetObjectPropertyList(MTP_t * mtp, MTP_Container_t * container, uint32_t object_id);
static MTP_State_t MTP_GetObjectPropertyValue(MTP_t * mtp, MTP_Container_t * container, uint32_t object_id, uint32_t property_id);
static MTP_State_t MTP_GetObject(MTP_t * mtp, MTP_Container_t * container, uint32_t object_id);
static MTP_State_t MTP_GetObjectReferences(MTP_t * mtp, MTP_Container_t * container, uint32_t object_id);

static MTP_State_t MTP_RecieveObjectInfo(MTP_t * mtp, MTP_Container_t * container, uint32_t storage_id, uint32_t parent_id);

static MTP_State_t MTP_SendResponse(MTP_Container_t * container, uint16_t code);
static MTP_State_t MTP_SendData(MTP_Container_t * container, uint32_t size);
static MTP_State_t MTP_ReceiveData(MTP_Container_t * container, uint32_t size);

/*
 * PUBLIC FUNCTIONS
 */

MTP_State_t MTP_Reset(MTP_t * mtp)
{
	mtp->in_session = false;
	mtp->transaction.remaining = 0;
	return MTP_State_RxOperation;
}

MTP_State_t MTP_HandleOperation(MTP_t * mtp, MTP_Operation_t * op, MTP_Container_t * container)
{
	// Blank out unused parameters - so they can default to zero if unspecified.
	uint32_t params = (op->length - 12) / 4;
	if (params < LENGTH(op->param))
	{
		for (uint32_t i = params; i < LENGTH(op->param); i++)
		{
			op->param[i] = 0;
		}
	}

	// code and transaction_id will be reused during data/response send
	container->code = op->code;
	container->transaction_id = op->transaction_id;

	switch (op->code)
	{
	case MTP_OP_GET_DEVICE_INFO:
		return MTP_GetDeviceInfo(container);

	case MTP_OP_OPEN_SESSION:
		return MTP_OpenSession(mtp, container);

	case MTP_OP_GET_STORAGE_IDS:
		return MTP_GetStorageIDs(container);

	case MTP_OP_GET_STORAGE_INFO:
		return MTP_GetStorageInfo(container, op->param[0]);

	case MTP_OP_GET_OBJECT_HANDLES:
		return MTP_GetObjectHandles(mtp, container);

	case MTP_OP_GET_OBJECT_INFO:
		return MTP_GetObjectInfo(mtp, container, op->param[0]);

	case MTP_OP_GET_OBJECT_REFERENCES:
		return MTP_GetObjectReferences(mtp, container, op->param[0]);

	case MTP_OP_GET_OBJECT_PROPS_SUPPORTED:
		return MTP_GetObjectPropertiesSupported(container, op->param[0]);

	case MTP_OP_GET_OBJECT_PROP_DESC:
		return MTP_GetObjectPropertyDescriptor(container, op->param[0]);

	case MTP_OP_GET_OBJECT_PROPLIST:
		return MTP_GetObjectPropertyList(mtp, container, op->param[0]);

	case MTP_OP_GET_OBJECT_PROP_VALUE:
		return MTP_GetObjectPropertyValue(mtp, container, op->param[0], op->param[1]);

	case MTP_OP_GET_DEVICE_PROP_DESC:
		break;
		//return MTP_GetDevicePropertyDescriptor(container);

	case MTP_OP_GET_OBJECT:
		return MTP_GetObject(mtp, container, op->param[0]);

	case MTP_OP_SEND_OBJECT_INFO:
		return MTP_ReceiveData(container, sizeof(container->data));

	case MTP_OP_SEND_OBJECT:
		// Our receive on this shall be aligned.
		//return MTP_ReceiveData(container, sizeof(container->data) - MTP_CONT_HEADER_SIZE);
		break;

	case MTP_OP_DELETE_OBJECT:
		//USBD_MTP_OPT_DeleteObject(pdev);
		//hmtp->MTP_ResponsePhase = MTP_RESPONSE_PHASE;
		break;

	default:
		break;
	}

	__BKPT();
	return MTP_SendResponse(container, MTP_RESP_OPERATION_NOT_SUPPORTED);
}

MTP_State_t MTP_NextData(MTP_t * mtp, MTP_Operation_t * op, MTP_Container_t * container)
{
	if (mtp->transaction.remaining)
	{
		// The read function overreads by up to 12 bytes.
		// Put these into the head of our container.
		memcpy(container, container->data + sizeof(container->data) - MTP_CONT_HEADER_SIZE, MTP_CONT_HEADER_SIZE);

		// If the remaining data is less than 12, we would have already sent it.
		uint32_t remaining = mtp->transaction.remaining - MTP_CONT_HEADER_SIZE;

		uint32_t chunk = MIN(remaining, sizeof(container->data));
		mtp->transaction.callback(container->data, mtp->transaction.offset, chunk);
		mtp->transaction.offset += chunk;

		if (chunk == remaining)
		{
			// We have read our last. Send it all.
			container->packet_size = remaining + MTP_CONT_HEADER_SIZE;
			mtp->transaction.remaining = 0;
			return MTP_State_TxDataLast;
		}

		// Otherwise we still need follow up packets.
		// Make sure we only send aligned packets.
		remaining -= sizeof(container->data) - MTP_CONT_HEADER_SIZE;
		container->packet_size = sizeof(container->data);
		mtp->transaction.remaining = remaining;
		return MTP_State_TxData;
	}

	container->code = op->code;
	container->transaction_id = op->transaction_id;
	return MTP_SendResponse(container, MTP_RESP_OK);
}

MTP_State_t MTP_HandleData(MTP_t * mtp, MTP_Operation_t * op, MTP_Container_t * container)
{
	switch (op->code)
	{
	case MTP_OP_SEND_OBJECT_INFO:
		return MTP_RecieveObjectInfo(mtp, container, op->param[0], op->param[1]);

	default:
		break;
	}
	// Should not occurr.
	return MTP_State_RxOperation;
}


/*
 * PRIVATE FUNCTIONS
 */

static MTP_State_t MTP_SendResponse(MTP_Container_t * container, uint16_t code)
{
	container->code = code;
	container->type = MTP_CONT_TYPE_RESPONSE;
	container->length = MTP_CONT_HEADER_SIZE;
	container->packet_size = MTP_CONT_HEADER_SIZE;
	return MTP_State_TxResponse;
}

static MTP_State_t MTP_SendResponseParams(MTP_Container_t * container, uint32_t * params, uint32_t count)
{
	uint32_t param_size = sizeof(uint32_t) * count;
	container->code = MTP_RESP_OK;
	container->type = MTP_CONT_TYPE_RESPONSE;
	container->length = MTP_CONT_HEADER_SIZE + param_size;
	container->packet_size = container->length;
	memcpy(container->data, params, param_size);
	return MTP_State_TxResponse;
}

static MTP_State_t MTP_SendData(MTP_Container_t * container, uint32_t size)
{
	container->type = MTP_CONT_TYPE_DATA;
	container->length = MTP_CONT_HEADER_SIZE + size;
	container->packet_size = container->length;
	return MTP_State_TxDataLast;
}

static MTP_State_t MTP_ReceiveData(MTP_Container_t * container, uint32_t size)
{
	container->packet_size = MTP_CONT_HEADER_SIZE + size;
	return MTP_State_RxData;
}

static MTP_State_t MTP_OpenSession(MTP_t * mtp, MTP_Container_t * container)
{
	if (mtp->in_session)
	{
		return MTP_SendResponse(container, MTP_RESP_SESSION_ALREADY_OPEN);
	}
	else
	{
		mtp->in_session = true;
		return MTP_SendResponse(container, MTP_RESP_OK);
	}
}


static const uint16_t cSuppOps[] = { MTP_OP_GET_DEVICE_INFO, MTP_OP_OPEN_SESSION, MTP_OP_CLOSE_SESSION,
								   MTP_OP_GET_STORAGE_IDS, MTP_OP_GET_STORAGE_INFO, MTP_OP_GET_NUM_OBJECTS,
								   MTP_OP_GET_OBJECT_HANDLES, MTP_OP_GET_OBJECT_INFO, MTP_OP_GET_OBJECT,
								   MTP_OP_DELETE_OBJECT, MTP_OP_SEND_OBJECT_INFO, MTP_OP_SEND_OBJECT,
								   MTP_OP_GET_DEVICE_PROP_DESC, MTP_OP_GET_DEVICE_PROP_VALUE,
								   MTP_OP_GET_OBJECT_PROP_VALUE, MTP_OP_GET_OBJECT_PROPS_SUPPORTED,
								   MTP_OP_GET_OBJECT_PROPLIST, MTP_OP_GET_OBJECT_PROP_DESC, MTP_OP_GET_OBJECT_REFERENCES
								 };

static const uint16_t cSuppEvents[] = { MTP_EVENT_OBJECTADDED };

static const uint16_t cSuppObjects[] = { MTP_OBJ_FORMAT_UNDEFINED, MTP_OBJ_FORMAT_ASSOCIATION, MTP_OBJ_FORMAT_TEXT };

static const uint16_t cSuppObjProps[] = {  MTP_OBJ_PROP_STORAGE_ID, MTP_OBJ_PROP_OBJECT_FORMAT, MTP_OBJ_PROP_OBJECT_SIZE,
										MTP_OBJ_PROP_OBJ_FILE_NAME, MTP_OBJ_PROP_PARENT_OBJECT, MTP_OBJ_PROP_NAME,
										MTP_OBJ_PROP_PERS_UNIQ_OBJ_IDEN, MTP_OBJ_PROP_PROTECTION_STATUS
									   };

/*
static const uint16_t cSuppDevProps[] = { MTP_DEV_PROP_DEVICE_FRIENDLY_NAME, MTP_DEV_PROP_BATTERY_LEVEL };

static const uint16_t cSuppImgFormat[] = {MTP_OBJ_FORMAT_UNDEFINED, MTP_OBJ_FORMAT_TEXT, MTP_OBJ_FORMAT_ASSOCIATION,
										 MTP_OBJ_FORMAT_EXECUTABLE, MTP_OBJ_FORMAT_WAV, MTP_OBJ_FORMAT_MP3,
										 MTP_OBJ_FORMAT_EXIF_JPEG, MTP_OBJ_FORMAT_MPEG, MTP_OBJ_FORMAT_MP4_CONTAINER,
										 MTP_OBJ_FORMAT_WINDOWS_IMAGE_FORMAT, MTP_OBJ_FORMAT_PNG, MTP_OBJ_FORMAT_WMA,
										 MTP_OBJ_FORMAT_WMV
										};
*/


static MTP_State_t MTP_GetDeviceInfo(MTP_Container_t * container)
{
	uint8_t * dst = container->data;

	dst = MTP_Write16(dst, 100); // Standard version: 1.0
	dst = MTP_Write32(dst, 0x06); // MTP vendor extension: None
	dst = MTP_Write16(dst, 100); // MTP version: 1.0
	dst = MTP_WriteString(dst, "microsoft.com: 1.0; "); // MTP extentions: windows compatability garbage.....
	dst = MTP_Write16(dst, 0); // Functional mode: Standard
	dst = MTP_WriteArray16(dst, cSuppOps, LENGTH(cSuppOps)); // Supported operations
	dst = MTP_WriteArray16(dst, cSuppEvents, LENGTH(cSuppEvents)); // Supported events
	dst = MTP_WriteArray16(dst, NULL, 0); // Device properties
	dst = MTP_WriteArray16(dst, cSuppObjects, LENGTH(cSuppObjects)); // Capture formats (formats emitted by the device)
	dst = MTP_WriteArray16(dst, cSuppObjects, LENGTH(cSuppObjects)); // Image formats (formats supported by the device)
	dst = MTP_WriteString(dst, USB_MANUFACTURER_STRING); // Manufacturer
	dst = MTP_WriteString(dst, USB_PRODUCT_STRING); // Model
	dst = MTP_WriteString(dst, "V1.00"); // Device version
	dst = MTP_WriteString(dst, "00000000000000000000000000000000"); // Device serial: Must be a 32 char hex string.

	uint32_t size = dst - container->data;
	return MTP_SendData(container, size);
}

static MTP_State_t MTP_GetStorageIDs(MTP_Container_t * container)
{
	uint32_t storage_ids[] = { MTP_STORAGE_ID };
	uint8_t * dst = MTP_WriteArray32(container->data, storage_ids, LENGTH(storage_ids));
	uint32_t size = dst - container->data;
	return MTP_SendData(container, size);
}

static MTP_State_t MTP_GetStorageInfo(MTP_Container_t * container, uint32_t storage_id)
{
	if (storage_id == MTP_STORAGE_ID)
	{
		uint8_t * dst = container->data;
		dst = MTP_Write16(dst, MTP_STORAGE_FIXED_RAM);
		dst = MTP_Write16(dst, MTP_FILESYSTEM_GENERIC_FLAT);
		dst = MTP_Write16(dst, MTP_ACCESS_CAP_RW);
		dst = MTP_Write64(dst, 0xFFFFFFFF);// MTP_STORAGE_SPACE); // Max capacity
		dst = MTP_Write64(dst, 0xFFFFFFFF); //0xFFFFFFFFFFFFFFFF); // Free space
		dst = MTP_Write32(dst, MTP_MAX_OBJECTS); // Free objects TODO: (NOT MAX)
		dst = MTP_WriteString(dst, NULL);
		dst = MTP_WriteString(dst, NULL);
		uint32_t size = dst - container->data;
		return MTP_SendData(container, size);
	}
	else
	{
		return MTP_SendResponse(container, MTP_RESP_INVALID_STORAGE_ID);
	}
}

static MTP_State_t MTP_GetObjectPropertiesSupported(MTP_Container_t * container, uint32_t object_type)
{
	uint8_t * dst = container->data;
	dst = MTP_WriteArray16(dst, cSuppObjProps, LENGTH(cSuppObjProps));
	uint32_t size = dst - container->data;
	return MTP_SendData(container, size);
}

static uint16_t MTP_GetObjectProperty(const MTP_File_t * file, uint32_t property_id, uint32_t * data)
{
	switch (property_id)
	{
	case MTP_OBJ_PROP_OBJECT_FORMAT :
		*data = file->type;
		return MTP_DATATYPE_UINT16;

	case MTP_OBJ_PROP_STORAGE_ID :
		*data = MTP_STORAGE_ID;
		return MTP_DATATYPE_UINT32;

	case MTP_OBJ_PROP_PARENT_OBJECT:
		*data = (uint32_t)NULL;
		return MTP_DATATYPE_STR;

	case MTP_OBJ_PROP_OBJECT_SIZE:
		*data = file->size;
		return MTP_DATATYPE_UINT64;

	case MTP_OBJ_PROP_OBJ_FILE_NAME:
	case MTP_OBJ_PROP_NAME:
		*data = (uint32_t)file->name;
		return MTP_DATATYPE_STR;

	case MTP_OBJ_PROP_PERS_UNIQ_OBJ_IDEN:
		*data = 0;
		return MTP_DATATYPE_UINT128;

	case MTP_OBJ_PROP_PROTECTION_STATUS:
		*data = ((file->write) ? MTP_OBJ_NO_PROTECTION : MTP_OBJ_READ_ONLY);
		return MTP_DATATYPE_UINT16;

	default:
		return 0;
	}
}

const MTP_File_t cDefaultFile = {
		.name = "New Folder",
		.read = (void*)1,
		.write = (void*)1,
		.size = 0,
		.type = MTP_OBJ_FORMAT_UNDEFINED,
};

static MTP_State_t MTP_GetObjectPropertyDescriptor(MTP_Container_t * container, uint32_t property_id)
{
	uint32_t data;
	uint16_t data_type = MTP_GetObjectProperty(&cDefaultFile, property_id, &data);

	if (data_type == 0)
	{
		return MTP_SendResponse(container, MTP_RESP_OBJECT_PROP_NOT_SUPPORTED);
	}

	uint8_t * dst = container->data;
	dst = MTP_Write16(dst, property_id);
	dst = MTP_Write16(dst, data_type);
	dst = MTP_Write8(dst, MTP_PROP_GET);
	dst = MTP_WriteType(dst, data_type, data);
	dst = MTP_Write32(dst, 0); // GroupCode
	dst = MTP_Write16(dst, 0); // FormFlag
	uint32_t size = dst - container->data;
	return MTP_SendData(container, size);
}

static MTP_State_t MTP_GetObjectHandles(MTP_t * mtp, MTP_Container_t * container)
{
	uint32_t count = 0;
	uint32_t handles[LENGTH(mtp->objects)];
	for (uint32_t i = 0; i < LENGTH(mtp->objects); i++)
	{
		if (mtp->objects[i] != NULL)
		{
			handles[count++] = i + 1;
		}
	}
	uint8_t * dst = MTP_WriteArray32(container->data, handles, count);
	uint32_t size = dst - container->data;
	return MTP_SendData(container, size);
}

static MTP_State_t MTP_GetObjectPropertyValue(MTP_t * mtp, MTP_Container_t * container, uint32_t object_id, uint32_t property_id)
{
	MTP_File_t * file = MTP_GetObjectById(mtp, object_id);
	if (file == NULL)
	{
		return MTP_SendResponse(container, MTP_RESP_INVALID_OBJECT_HANDLE);
	}

	uint32_t data;
	uint16_t data_type = MTP_GetObjectProperty(file, property_id, &data);
	if (data_type == 0)
	{
		return MTP_SendResponse(container, MTP_RESP_OBJECT_PROP_NOT_SUPPORTED);
	}

	uint8_t * dst = MTP_WriteType(container->data, data_type, data);
	uint32_t size = dst - container->data;
	return MTP_SendData(container, size);
}

static MTP_State_t MTP_GetObjectPropertyList(MTP_t * mtp, MTP_Container_t * container, uint32_t object_id)
{
	// We are going to ignore all the garbage in the spec about group specification, ect.

	// The object may be 0 for 'all', but we dont want to overflow our packet buffer.
	// Lets just let those conditions throw INVALID_OBJECT_HANDLE....

	MTP_File_t * file = MTP_GetObjectById(mtp, object_id);
	if (file == NULL)
	{
		return MTP_SendResponse(container, MTP_RESP_INVALID_OBJECT_HANDLE);
	}

	uint8_t * dst = container->data;
	dst = MTP_Write32(container->data, LENGTH(cSuppObjProps));
	for (uint32_t i = 0; i < LENGTH(cSuppObjProps); i++)
	{
		uint32_t property_id = cSuppObjProps[i];
		uint32_t data;
		uint16_t data_type = MTP_GetObjectProperty(file, property_id, &data);
		if (data_type)
		{
			dst = MTP_Write32(dst, object_id);
			dst = MTP_Write16(dst, property_id);
			dst = MTP_Write16(dst, data_type);
			dst = MTP_WriteType(dst, data_type, data);
		}
	}

	uint32_t size = dst - container->data;
	return MTP_SendData(container, size);
}

static MTP_State_t MTP_GetObjectInfo(MTP_t * mtp, MTP_Container_t * container, uint32_t object_id)
{
	MTP_File_t * file = MTP_GetObjectById(mtp, object_id);
	if (file == NULL)
	{
		return MTP_SendResponse(container, MTP_RESP_INVALID_OBJECT_HANDLE);
	}

	uint8_t * dst = container->data;
	dst = MTP_Write32(dst, MTP_STORAGE_ID);
	dst = MTP_Write16(dst, file->type);
	dst = MTP_Write16(dst, (file->write) ? MTP_OBJ_NO_PROTECTION : MTP_OBJ_READ_ONLY);
	dst = MTP_Write32(dst, file->size);

	// A bunch of PTP garbage regarding thumbnail and image properties. Zero it.
	bzero(dst, 40);
	dst += 40;

	dst = MTP_WriteString(dst, file->name);
	dst = MTP_WriteString(dst, NULL); // Capture date
	dst = MTP_WriteString(dst, NULL); // Modified date
	dst = MTP_WriteString(dst, NULL); // Keywords

	uint32_t size = dst - container->data;
	return MTP_SendData(container, size);
}

static MTP_State_t MTP_GetObjectReferences(MTP_t * mtp, MTP_Container_t * container, uint32_t object_id)
{
	// I dont know why windows hits this....
	return MTP_SendResponse(container, MTP_RESP_INVALID_OBJECT_HANDLE);
}

static MTP_State_t MTP_GetObject(MTP_t * mtp, MTP_Container_t * container, uint32_t object_id)
{
	MTP_File_t * file = MTP_GetObjectById(mtp, object_id);
	if (file == NULL)
	{
		return MTP_SendResponse(container, MTP_RESP_INVALID_OBJECT_HANDLE);
	}

	if (file->read == NULL)
	{
		return MTP_SendResponse(container, MTP_RESP_ACCESS_DENIED);
	}

	// We align our reads to 512 byte chunks to ease compatibility with other systems.
	uint32_t chunk = MIN(file->size, sizeof(container->data));
	bool success = file->read(container->data, 0, chunk);
	if (!success)
	{
		return MTP_SendResponse(container, MTP_RESP_GENERAL_ERROR);
	}

	if (chunk == file->size)
	{
		// We can send this all in one go. Do it.
		return MTP_SendData(container, chunk);
	}

	// Otherwise we must stagger this over multiple writes.
	// Our writes MUST align to 64 byte chunks - including the header.
	// Otherwise we would send a short packet - which signals the end of the data phase.

	// We only send 500 bytes in the first packet, because of the header.
	// We will send the remaining 12 bytes later.
	mtp->transaction.offset = chunk;
	mtp->transaction.remaining = file->size - (chunk - MTP_CONT_HEADER_SIZE); // Remaining includes any overread bytes.
	mtp->transaction.callback = file->read;

	container->type = MTP_CONT_TYPE_DATA;
	container->length = MTP_CONT_HEADER_SIZE + file->size;
	container->packet_size = sizeof(container->data);

	return MTP_State_TxData;
}

static MTP_State_t MTP_RecieveObjectInfo(MTP_t * mtp, MTP_Container_t * container, uint32_t storage_id, uint32_t parent_id)
{
	if (parent_id != 0xFFFFFFFF)
	{
		return MTP_SendResponse(container, MTP_RESP_INVALID_PARENT_OBJECT);
	}

	uint16_t obj_type;
	MTP_Read16(container->data + 4, &obj_type);
	uint32_t size;
	MTP_Read32(container->data + 8, &size);
	char name[MTP_STRING_MAX];
	MTP_ReadString(container->data + 52, name);

	uint32_t object_id = 0;

	// First.. find a spare object id.
	for (uint32_t i = 0; i < LENGTH(mtp->objects); i++)
	{
		if (mtp->objects[i] == NULL)
		{
			object_id = i + 1;
			break;
		}
	}

	if (object_id == 0)
	{
		// No space.
		return MTP_SendResponse(container, MTP_RESP_STORE_FULL);
	}

	// Next.. see if the user wants the file.
	if (mtp->new_file_callback)
	{
		MTP_File_t * file = mtp->new_file_callback(name, size);
		if (file && file->write)
		{
			mtp->objects[object_id - 1] = file;
			mtp->new_file = file; // This is where a subsequent send data will occurr.

			file->size = size;
			file->type = obj_type;

			uint32_t params[] = {
				MTP_STORAGE_ID,
				parent_id,
				object_id,
			};

			return MTP_SendResponseParams(container, params, LENGTH(params));
		}
	}
	// Reject the file.
	return MTP_SendResponse(container, MTP_RESP_INVALID_DATASET);
}


// Primitive writing structures

static uint8_t * MTP_WriteType(uint8_t * dst, uint16_t type, uint32_t data)
{
	switch (type)
	{
	case MTP_DATATYPE_UINT8:
		return MTP_Write8(dst, (uint8_t)data);

	case MTP_DATATYPE_UINT16:
		return MTP_Write16(dst, (uint16_t)data);

	case MTP_DATATYPE_UINT32:
		return MTP_Write32(dst, data);

	case MTP_DATATYPE_UINT64:
		return MTP_Write64(dst, data);

	case MTP_DATATYPE_UINT128:
		return MTP_Write128(dst, data);

	case MTP_DATATYPE_STR:
		return MTP_WriteString(dst, (const char *)data);

	default:
		return dst;
	}
}

static uint8_t * MTP_Write128(uint8_t * dst, uint32_t value)
{
	// Fuck it. Who really needs 128 bits?
	dst = MTP_Write32(dst, (uint32_t)value);
	dst = MTP_Write32(dst, 0);
	dst = MTP_Write32(dst, 0);
	dst = MTP_Write32(dst, 0);
	return dst;
}

static uint8_t * MTP_Write64(uint8_t * dst, uint64_t value)
{
	dst = MTP_Write32(dst, (uint32_t)value);
	dst = MTP_Write32(dst, (uint32_t)(value >> 32));
	return dst;
}

static uint8_t * MTP_Write32(uint8_t * dst, uint32_t value)
{
	*dst++ = (uint8_t)(value);
	*dst++ = (uint8_t)(value >> 8);
	*dst++ = (uint8_t)(value >> 16);
	*dst++ = (uint8_t)(value >> 24);
	return dst;
}

static inline uint8_t * MTP_Write16(uint8_t * dst, uint16_t value)
{
	*dst++ = (uint8_t)(value);
	*dst++ = (uint8_t)(value >> 8);
	return dst;
}

static inline uint8_t * MTP_Write8(uint8_t * dst, uint8_t value)
{
	*dst++ = value;
	return dst;
}

static uint8_t * MTP_WriteArray32(uint8_t * dst, const uint32_t * array, uint32_t count)
{
	dst = MTP_Write32(dst, count);
	while(count--)
	{
		dst = MTP_Write32(dst, *array++);
	}
	return dst;
}

static uint8_t * MTP_WriteArray16(uint8_t * dst, const uint16_t * array, uint32_t count)
{
	dst = MTP_Write32(dst, count);
	while(count--)
	{
		dst = MTP_Write16(dst, *array++);
	}
	return dst;
}

static uint8_t * MTP_WriteString(uint8_t * dst, const char * str)
{
	if (str == NULL)
	{
		*dst++ = 0;
	}
	else
	{
		// The null character should be written if string is not empty
		uint8_t len = strlen(str) + 1;
		*dst++ = len;
		while (len--)
		{
			// Unicode format
			*dst++ = *str++;
			*dst++ = 0;
		}
	}
	return dst;
}

static inline uint8_t * MTP_Read16(uint8_t * src, uint16_t * value)
{
	*value = src[0] | (src[1] << 8);
	return src + 2;
}

static inline uint8_t * MTP_Read32(uint8_t * src, uint32_t * value)
{
	*value = src[0] | (src[1] << 8) | (src[2] << 16) | (src[3] << 24);
	return src + 4;
}

static uint8_t * MTP_ReadString(uint8_t * src, char * str)
{
	uint32_t length = *src++;

	if (length == 0)
	{
		*str = 0;
		return src;
	}

	uint32_t skip = 1; // Skip the null char.
	if (length >= MTP_STRING_MAX)
	{
		// We are going to just truncate it for now.
		length = MTP_STRING_MAX - 1;
		skip += length - MTP_STRING_MAX;
	}

	while (length--)
	{
		*str++ = *src;
		src += 2;
	}

	*str = 0;
	return src + (skip * 2);
}
