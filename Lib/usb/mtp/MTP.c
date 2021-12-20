
#include "MTP.h"
#include "../USB_Defs.h"
#include <string.h>

/*
 * PRIVATE DEFINITIONS
 */

#define MTP_STORAGE_ID		0x00010001
#define MTP_STORAGE_SPACE	(1024 * 1024)

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

/*
 * PRIVATE VARIABLES
 */

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

static MTP_State_t MTP_GetDeviceInfo(MTP_Container_t * container);
static MTP_State_t MTP_GetDevicePropertyDescriptor(MTP_Container_t * container);
static MTP_State_t MTP_GetStorageIDs(MTP_Container_t * container);
static MTP_State_t MTP_GetStorageInfo(MTP_Container_t * container, uint32_t storage_id);
static MTP_State_t MTP_GetObjectHandles(MTP_t * mtp, MTP_Container_t * container);
static MTP_State_t MTP_GetObjectInfo(MTP_t * mtp, MTP_Container_t * container, uint32_t object_id);
static MTP_State_t MTP_GetObjectPropertiesSupported(MTP_Container_t * container, uint32_t object_type);
static MTP_State_t MTP_GetObjectPropertyDescriptor(MTP_Container_t * container, uint32_t property_id);
static MTP_State_t MTP_GetObjectPropertyList(MTP_t * mtp, MTP_Container_t * container, const MTP_Operation_t * op);
static MTP_State_t MTP_GetObjectReferences(MTP_t * mtp, MTP_Container_t * container, uint32_t object_id);
static MTP_State_t MTP_OpenSession(MTP_t * mtp, MTP_Container_t * container);

static MTP_State_t MTP_SendResponse(MTP_Container_t * container, uint16_t code);
static MTP_State_t MTP_SendData(MTP_Container_t * container, uint32_t size);

/*
 * PUBLIC FUNCTIONS
 */

MTP_State_t MTP_Reset(MTP_t * mtp)
{
	mtp->in_session = false;
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
		return MTP_GetObjectPropertyList(mtp, container, op);

	case MTP_OP_GET_OBJECT_PROP_VALUE:
		//USBD_MTP_OPT_GetObjectPropValue(pdev);
		//hmtp->MTP_ResponsePhase = MTP_RESPONSE_PHASE;
		break;

	case MTP_OP_GET_DEVICE_PROP_DESC:
		break;
		//return MTP_GetDevicePropertyDescriptor(container);

	case MTP_OP_GET_OBJECT:
		//USBD_MTP_OPT_GetObject(pdev);
		//hmtp->MTP_ResponsePhase = MTP_READ_DATA;
		break;

	case MTP_OP_SEND_OBJECT_INFO:
		//USBD_MTP_OPT_SendObjectInfo(pdev, (uint8_t *)(hmtp->rx_buff), MTP_DataLength.prv_len);
		//hmtp->MTP_ResponsePhase = MTP_RECEIVE_DATA;
		break;

	case MTP_OP_SEND_OBJECT:
		//USBD_MTP_OPT_SendObject(pdev, (uint8_t *)(hmtp->rx_buff), MTP_DataLength.rx_length);
		//hmtp->MTP_ResponsePhase = MTP_RECEIVE_DATA;
		break;

	case MTP_OP_DELETE_OBJECT:
		//USBD_MTP_OPT_DeleteObject(pdev);
		//hmtp->MTP_ResponsePhase = MTP_RESPONSE_PHASE;
		break;

	default:
		break;
	}

	return MTP_SendResponse(container, MTP_RESP_OPERATION_NOT_SUPPORTED);
}

MTP_State_t MTP_NextData(MTP_t * mtp, MTP_Container_t * container)
{
	return MTP_SendResponse(container, MTP_RESP_OK);
}


/*
 * PRIVATE FUNCTIONS
 */

static MTP_State_t MTP_SendResponse(MTP_Container_t * container, uint16_t code)
{
	container->code = code;
	container->type = MTP_CONT_TYPE_RESPONSE;
	container->length = MTP_CONT_HEADER_SIZE;
	return MTP_State_TxResponse;
}

static MTP_State_t MTP_SendData(MTP_Container_t * container, uint32_t size)
{
	container->type = MTP_CONT_TYPE_DATA;
	container->length = MTP_CONT_HEADER_SIZE + size;
	return MTP_State_TxData;
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


//*

static const uint16_t cSuppOps[] = { MTP_OP_GET_DEVICE_INFO, MTP_OP_OPEN_SESSION, MTP_OP_CLOSE_SESSION,
                                   MTP_OP_GET_STORAGE_IDS, MTP_OP_GET_STORAGE_INFO, MTP_OP_GET_NUM_OBJECTS,
                                   MTP_OP_GET_OBJECT_HANDLES, MTP_OP_GET_OBJECT_INFO, MTP_OP_GET_OBJECT,
                                   MTP_OP_DELETE_OBJECT, MTP_OP_SEND_OBJECT_INFO, MTP_OP_SEND_OBJECT,
                                   MTP_OP_GET_DEVICE_PROP_DESC, MTP_OP_GET_DEVICE_PROP_VALUE,
                                   MTP_OP_SET_OBJECT_PROP_VALUE, MTP_OP_GET_OBJECT_PROP_VALUE,
                                   MTP_OP_GET_OBJECT_PROPS_SUPPORTED, MTP_OP_GET_OBJECT_PROPLIST,
                                   MTP_OP_GET_OBJECT_PROP_DESC, MTP_OP_GET_OBJECT_REFERENCES
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
	// Standard version: 1.0
	dst = MTP_Write16(dst, 100);
	// MTP vendor extension: None
	dst = MTP_Write32(dst, 0x06);
	// MTP version: 1.0
	dst = MTP_Write16(dst, 100);
	// MTP extentions: windows compatability garbage.....
	dst = MTP_WriteString(dst, "microsoft.com: 1.0; ");
	// Functional mode: Standard
	dst = MTP_Write16(dst, 0);

	// Supported operations
	dst = MTP_WriteArray16(dst, cSuppOps, LENGTH(cSuppOps));
	// Supported events
	dst = MTP_WriteArray16(dst, cSuppEvents, LENGTH(cSuppEvents));
	// Device properties
	dst = MTP_WriteArray16(dst, NULL, 0);

	// Capture formats (formats emitted by the device)
	dst = MTP_WriteArray16(dst, cSuppObjects, LENGTH(cSuppObjects));
	// Image formats (formats supported by the device)
	dst = MTP_WriteArray16(dst, cSuppObjects, LENGTH(cSuppObjects));

	// Manufacturer
	dst = MTP_WriteString(dst, USB_MANUFACTURER_STRING);
	// Model
	dst = MTP_WriteString(dst, USB_PRODUCT_STRING);
	// Device version
	dst = MTP_WriteString(dst, "V1.00");
	// Device serial: Must be a 32 char hex string. Probably windows compatability garbage.
	dst = MTP_WriteString(dst, "00000000000000000000000000000000");
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

	uint8_t get_set = (property_id == MTP_OBJ_PROP_PROTECTION_STATUS) ? MTP_PROP_GET_SET : MTP_PROP_GET;

	uint8_t * dst = container->data;
	dst = MTP_Write16(dst, property_id);
	dst = MTP_Write16(dst, data_type);
	dst = MTP_Write8(dst, get_set);
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

static uint8_t * MTP_GetObjectPropertyListItem(uint8_t * dst, MTP_File_t * file, uint32_t object_id, uint32_t property_id)
{
	uint32_t data;
	uint16_t data_type = MTP_GetObjectProperty(file, property_id, &data);
	if (data_type != 0)
	{
		dst = MTP_Write32(dst, object_id);
		dst = MTP_Write16(dst, property_id);
		dst = MTP_Write16(dst, data_type);
		dst = MTP_WriteType(dst, data_type, data);
	}
	return dst;
}

static MTP_State_t MTP_GetObjectPropertyList(MTP_t * mtp, MTP_Container_t * container, const MTP_Operation_t * op)
{
	// params are..
	// object_id
	// [object_format]
	// property_id
	// [property_group_code]
	// [depth]

	uint32_t property_id = op->param[2];
	if (property_id == 0)
	{
		return MTP_SendResponse(container, MTP_RESP_SPECIFICATION_BY_GROUP_UNSUPPORTED);
	}

	if (op->param[4] != 0)
	{
		// Specification by depth.
		// The object_id is the 'head' (0 for root), and all items below up to the depth are requested.
		// Hard to implement....
		return MTP_SendResponse(container, MTP_RESP_SPECIFICATION_BY_DEPTH_UNSUPPORTED);
	}

	uint8_t * dst = container->data + sizeof(uint32_t); // We need to amend this field later
	uint32_t object_id = op->param[0];
	uint32_t item_count = 0;
	if (object_id)
	{
		// A specific object has been specified.
		MTP_File_t * file = MTP_GetObjectById(mtp, object_id);
		if (file == NULL)
		{
			return MTP_SendResponse(container, MTP_RESP_INVALID_OBJECT_HANDLE);
		}
		item_count += 1;
		dst = MTP_GetObjectPropertyListItem(dst, file, object_id, property_id);
	}
	else // All objects are specified..
	{
		uint32_t object_format = op->param[1];
		for (uint32_t i = 0; i < LENGTH(mtp->objects); i++)
		{
			MTP_File_t * file = mtp->objects[i];
			if ( file != NULL && (object_format == 0 || file->type == object_format))
			{
				object_id = i + 1;
				item_count += 1;
				dst = MTP_GetObjectPropertyListItem(dst, file, object_id, property_id);
			}
		}
	}

	MTP_Write32(container->data, item_count);

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

	// A bunch of PTP garbage. Zero it.
	// Thumb format (16)
	// Thumb compressed size (32)
	// Thumb pix width (32)
	// Thumb pix height (32)
	// Image pix width (32)
	// Image pix height (32)
	// Image bit depth (32)
	// Parent object (32)
	// Association type (16)
	// Association description (32)
	// Sequence number (32)
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
	//__BKPT();
	return MTP_SendResponse(container, MTP_RESP_INVALID_OBJECT_HANDLE);
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
