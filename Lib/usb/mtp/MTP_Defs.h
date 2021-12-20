#ifndef MTP_DEFS_H
#define MTP_DEFS_H


/*
 * PUBLIC DEFINITIONS
 */

#define MTP_DATATYPE_INT8			                       0x0001
#define MTP_DATATYPE_UINT8			                       0x0002
#define MTP_DATATYPE_INT16			                       0x0003
#define MTP_DATATYPE_UINT16			                       0x0004
#define MTP_DATATYPE_INT32			                       0x0005
#define MTP_DATATYPE_UINT32			                       0x0006
#define MTP_DATATYPE_INT64			                       0x0007
#define MTP_DATATYPE_UINT64			                       0x0008
#define MTP_DATATYPE_UINT128			                   0x000A
#define MTP_DATATYPE_ARRAY			                       0x4000 // Can modify other types
#define MTP_DATATYPE_STR			                       0xFFFF

/* MTP Object format codes */
#define MTP_OBJ_FORMAT_UNDEFINED                           0x3000
#define MTP_OBJ_FORMAT_ASSOCIATION                         0x3001
#define MTP_OBJ_FORMAT_SCRIPT                              0x3002
#define MTP_OBJ_FORMAT_EXECUTABLE                          0x3003
#define MTP_OBJ_FORMAT_TEXT                                0x3004
#define MTP_OBJ_FORMAT_HTML                                0x3005
#define MTP_OBJ_FORMAT_DPOF                                0x3006
#define MTP_OBJ_FORMAT_AIFF                                0x3007
#define MTP_OBJ_FORMAT_WAV                                 0x3008
#define MTP_OBJ_FORMAT_MP3                                 0x3009
#define MTP_OBJ_FORMAT_AVI                                 0x300A
#define MTP_OBJ_FORMAT_MPEG                                0x300B
#define MTP_OBJ_FORMAT_ASF                                 0x300C
#define MTP_OBJ_FORMAT_DEFINED                             0x3800
#define MTP_OBJ_FORMAT_EXIF_JPEG                           0x3801
#define MTP_OBJ_FORMAT_TIFF_EP                             0x3802
#define MTP_OBJ_FORMAT_FLASHPIX                            0x3803
#define MTP_OBJ_FORMAT_BMP                                 0x3804
#define MTP_OBJ_FORMAT_CIFF                                0x3805
#define MTP_OBJ_FORMAT_UNDEFINED_RESERVED0                 0x3806
#define MTP_OBJ_FORMAT_GIF                                 0x3807
#define MTP_OBJ_FORMAT_JFIF                                0x3808
#define MTP_OBJ_FORMAT_CD                                  0x3809
#define MTP_OBJ_FORMAT_PICT                                0x380A
#define MTP_OBJ_FORMAT_PNG                                 0x380B
#define MTP_OBJ_FORMAT_UNDEFINED_RESERVED1                 0x380C
#define MTP_OBJ_FORMAT_TIFF                                0x380D
#define MTP_OBJ_FORMAT_TIFF_IT                             0x380E
#define MTP_OBJ_FORMAT_JP2                                 0x380F
#define MTP_OBJ_FORMAT_JPX                                 0x3810
#define MTP_OBJ_FORMAT_UNDEFINED_FIRMWARE                  0xB802
#define MTP_OBJ_FORMAT_WINDOWS_IMAGE_FORMAT                0xB881
#define MTP_OBJ_FORMAT_UNDEFINED_AUDIO                     0xB900
#define MTP_OBJ_FORMAT_WMA                                 0xB901
#define MTP_OBJ_FORMAT_OGG                                 0xB902
#define MTP_OBJ_FORMAT_AAC                                 0xB903
#define MTP_OBJ_FORMAT_AUDIBLE                             0xB904
#define MTP_OBJ_FORMAT_FLAC                                0xB906
#define MTP_OBJ_FORMAT_UNDEFINED_VIDEO                     0xB980
#define MTP_OBJ_FORMAT_WMV                                 0xB981
#define MTP_OBJ_FORMAT_MP4_CONTAINER                       0xB982
#define MTP_OBJ_FORMAT_MP2                                 0xB983
#define MTP_OBJ_FORMAT_3GP_CONTAINER                       0xB984


// MTP Object properties
#define MTP_OBJ_PROP_STORAGE_ID                            0xDC01
#define MTP_OBJ_PROP_OBJECT_FORMAT                         0xDC02
#define MTP_OBJ_PROP_PROTECTION_STATUS                     0xDC03
#define MTP_OBJ_PROP_OBJECT_SIZE                           0xDC04
#define MTP_OBJ_PROP_ASSOC_TYPE                            0xDC05
#define MTP_OBJ_PROP_ASSOC_DESC                            0xDC06
#define MTP_OBJ_PROP_OBJ_FILE_NAME                         0xDC07
#define MTP_OBJ_PROP_DATE_CREATED                          0xDC08
#define MTP_OBJ_PROP_DATE_MODIFIED                         0xDC09
#define MTP_OBJ_PROP_KEYWORDS                              0xDC0A
#define MTP_OBJ_PROP_PARENT_OBJECT                         0xDC0B
#define MTP_OBJ_PROP_ALLOWED_FOLD_CONTENTS                 0xDC0C
#define MTP_OBJ_PROP_HIDDEN                                0xDC0D
#define MTP_OBJ_PROP_SYSTEM_OBJECT                         0xDC0E
#define MTP_OBJ_PROP_PERS_UNIQ_OBJ_IDEN                    0xDC41
#define MTP_OBJ_PROP_SYNCID                                0xDC42
#define MTP_OBJ_PROP_PROPERTY_BAG                          0xDC43
#define MTP_OBJ_PROP_NAME                                  0xDC44
#define MTP_OBJ_PROP_CREATED_BY                            0xDC45
#define MTP_OBJ_PROP_ARTIST                                0xDC46
#define MTP_OBJ_PROP_DATE_AUTHORED                         0xDC47
#define MTP_OBJ_PROP_DESCRIPTION                           0xDC48
#define MTP_OBJ_PROP_URL_REFERENCE                         0xDC49
#define MTP_OBJ_PROP_LANGUAGELOCALE                        0xDC4A
#define MTP_OBJ_PROP_COPYRIGHT_INFORMATION                 0xDC4B
#define MTP_OBJ_PROP_SOURCE                                0xDC4C
#define MTP_OBJ_PROP_ORIGIN_LOCATION                       0xDC4D
#define MTP_OBJ_PROP_DATE_ADDED                            0xDC4E
#define MTP_OBJ_PROP_NON_CONSUMABLE                        0xDC4F
#define MTP_OBJ_PROP_CORRUPTUNPLAYABLE                     0xDC50
#define MTP_OBJ_PROP_PRODUCERSERIALNUMBER                  0xDC51
#define MTP_OBJ_PROP_REPRESENTATIVE_SAMPLE_FORMAT          0xDC81
#define MTP_OBJ_PROP_REPRESENTATIVE_SAMPLE_SIZE            0xDC82
#define MTP_OBJ_PROP_REPRESENTATIVE_SAMPLE_HEIGHT          0xDC83
#define MTP_OBJ_PROP_REPRESENTATIVE_SAMPLE_WIDTH           0xDC84
#define MTP_OBJ_PROP_REPRESENTATIVE_SAMPLE_DURATION        0xDC85
#define MTP_OBJ_PROP_REPRESENTATIVE_SAMPLE_DATA            0xDC86
#define MTP_OBJ_PROP_WIDTH                                 0xDC87
#define MTP_OBJ_PROP_HEIGHT                                0xDC88
#define MTP_OBJ_PROP_DURATION                              0xDC89
#define MTP_OBJ_PROP_RATING                                0xDC8A
#define MTP_OBJ_PROP_TRACK                                 0xDC8B
#define MTP_OBJ_PROP_GENRE                                 0xDC8C
#define MTP_OBJ_PROP_CREDITS                               0xDC8D
#define MTP_OBJ_PROP_LYRICS                                0xDC8E
#define MTP_OBJ_PROP_SUBSCRIPTION_CONTENT_ID               0xDC8F
#define MTP_OBJ_PROP_PRODUCED_BY                           0xDC90
#define MTP_OBJ_PROP_USE_COUNT                             0xDC91
#define MTP_OBJ_PROP_SKIP_COUNT                            0xDC92
#define MTP_OBJ_PROP_LAST_ACCESSED                         0xDC93
#define MTP_OBJ_PROP_PARENTAL_RATING                       0xDC94
#define MTP_OBJ_PROP_META_GENRE                            0xDC95
#define MTP_OBJ_PROP_COMPOSER                              0xDC96
#define MTP_OBJ_PROP_EFFECTIVE_RATING                      0xDC97
#define MTP_OBJ_PROP_SUBTITLE                              0xDC98
#define MTP_OBJ_PROP_ORIGINAL_RELEASE_DATE                 0xDC99
#define MTP_OBJ_PROP_ALBUM_NAME                            0xDC9A
#define MTP_OBJ_PROP_ALBUM_ARTIST                          0xDC9B
#define MTP_OBJ_PROP_MOOD                                  0xDC9C
#define MTP_OBJ_PROP_DRM_STATUS                            0xDC9D
#define MTP_OBJ_PROP_SUB_DESCRIPTION                       0xDC9E
#define MTP_OBJ_PROP_IS_CROPPED                            0xDCD1
#define MTP_OBJ_PROP_IS_COLOUR_CORRECTED                   0xDCD2
#define MTP_OBJ_PROP_IMAGE_BIT_DEPTH                       0xDCD3
#define MTP_OBJ_PROP_FNUMBER                               0xDCD4
#define MTP_OBJ_PROP_EXPOSURE_TIME                         0xDCD5
#define MTP_OBJ_PROP_EXPOSURE_INDEX                        0xDCD6
#define MTP_OBJ_PROP_TOTAL_BITRATE                         0xDE91
#define MTP_OBJ_PROP_BITRATE_TYPE                          0xDE92
#define MTP_OBJ_PROP_SAMPLE_RATE                           0xDE93
#define MTP_OBJ_PROP_NUMBER_OF_CHANNELS                    0xDE94
#define MTP_OBJ_PROP_AUDIO_BITDEPTH                        0xDE95
#define MTP_OBJ_PROP_SCAN_TYPE                             0xDE97
#define MTP_OBJ_PROP_AUDIO_WAVE_CODEC                      0xDE99
#define MTP_OBJ_PROP_AUDIO_BITRATE                         0xDE9A
#define MTP_OBJ_PROP_VIDEO_FOURCC_CODEC                    0xDE9B
#define MTP_OBJ_PROP_VIDEO_BITRATE                         0xDE9C
#define MTP_OBJ_PROP_FRAMES_PER_THOUSAND_SECONDS           0xDE9D
#define MTP_OBJ_PROP_KEYFRAME_DISTANCE                     0xDE9E
#define MTP_OBJ_PROP_BUFFER_SIZE                           0xDE9F
#define MTP_OBJ_PROP_ENCODING_QUALITY                      0xDEA0
#define MTP_OBJ_PROP_ENCODING_PROFILE                      0xDEA1
#define MTP_OBJ_PROP_DISPLAY_NAME                          0xDCE0
#define MTP_OBJ_PROP_BODY_TEXT                             0xDCE1
#define MTP_OBJ_PROP_SUBJECT                               0xDCE2
#define MTP_OBJ_PROP_PRIORITY                              0xDCE3
#define MTP_OBJ_PROP_GIVEN_NAME                            0xDD00
#define MTP_OBJ_PROP_MIDDLE_NAMES                          0xDD01
#define MTP_OBJ_PROP_FAMILY_NAME                           0xDD02
#define MTP_OBJ_PROP_PREFIX                                0xDD03
#define MTP_OBJ_PROP_SUFFIX                                0xDD04
#define MTP_OBJ_PROP_PHONETIC_GIVEN_NAME                   0xDD05
#define MTP_OBJ_PROP_PHONETIC_FAMILY_NAME                  0xDD06
#define MTP_OBJ_PROP_EMAIL_PRIMARY                         0xDD07
#define MTP_OBJ_PROP_EMAIL_PERSONAL_1                      0xDD08
#define MTP_OBJ_PROP_EMAIL_PERSONAL_2                      0xDD09
#define MTP_OBJ_PROP_EMAIL_BUSINESS_1                      0xDD0A
#define MTP_OBJ_PROP_EMAIL_BUSINESS_2                      0xDD0B
#define MTP_OBJ_PROP_EMAIL_OTHERS                          0xDD0C
#define MTP_OBJ_PROP_PHONE_NUMBER_PRIMARY                  0xDD0D
#define MTP_OBJ_PROP_PHONE_NUMBER_PERSONAL                 0xDD0E
#define MTP_OBJ_PROP_PHONE_NUMBER_PERSONAL_2               0xDD0F
#define MTP_OBJ_PROP_PHONE_NUMBER_BUSINESS                 0xDD10
#define MTP_OBJ_PROP_PHONE_NUMBER_BUSINESS_2               0xDD11
#define MTP_OBJ_PROP_PHONE_NUMBER_MOBILE                   0xDD12
#define MTP_OBJ_PROP_PHONE_NUMBER_MOBILE_2                 0xDD13
#define MTP_OBJ_PROP_FAX_NUMBER_PRIMARY                    0xDD14
#define MTP_OBJ_PROP_FAX_NUMBER_PERSONAL                   0xDD15
#define MTP_OBJ_PROP_FAX_NUMBER_BUSINESS                   0xDD16
#define MTP_OBJ_PROP_PAGER_NUMBER                          0xDD17
#define MTP_OBJ_PROP_PHONE_NUMBER_OTHERS                   0xDD18
#define MTP_OBJ_PROP_PRIMARY_WEB_ADDRESS                   0xDD19
#define MTP_OBJ_PROP_PERSONAL_WEB_ADDRESS                  0xDD1A
#define MTP_OBJ_PROP_BUSINESS_WEB_ADDRESS                  0xDD1B
#define MTP_OBJ_PROP_INSTANT_MESSENGER_ADDRESS             0xDD1C
#define MTP_OBJ_PROP_INSTANT_MESSENGER_ADDRESS_2           0xDD1D
#define MTP_OBJ_PROP_INSTANT_MESSENGER_ADDRESS_3           0xDD1E
#define MTP_OBJ_PROP_POSTAL_ADDRESS_PERSONAL_FULL          0xDD1F
#define MTP_OBJ_PROP_POSTAL_ADDRESS_PERSONAL_LINE_1        0xDD20
#define MTP_OBJ_PROP_POSTAL_ADDRESS_PERSONAL_LINE_2        0xDD21
#define MTP_OBJ_PROP_POSTAL_ADDRESS_PERSONAL_CITY          0xDD22
#define MTP_OBJ_PROP_POSTAL_ADDRESS_PERSONAL_REGION        0xDD23
#define MTP_OBJ_PROP_POSTAL_ADDRESS_PERSONAL_POSTAL_CODE   0xDD24
#define MTP_OBJ_PROP_POSTAL_ADDRESS_PERSONAL_COUNTRY       0xDD25
#define MTP_OBJ_PROP_POSTAL_ADDRESS_BUSINESS_FULL          0xDD26
#define MTP_OBJ_PROP_POSTAL_ADDRESS_BUSINESS_LINE_1        0xDD27
#define MTP_OBJ_PROP_POSTAL_ADDRESS_BUSINESS_LINE_2        0xDD28
#define MTP_OBJ_PROP_POSTAL_ADDRESS_BUSINESS_CITY          0xDD29
#define MTP_OBJ_PROP_POSTAL_ADDRESS_BUSINESS_REGION        0xDD2A
#define MTP_OBJ_PROP_POSTAL_ADDRESS_BUSINESS_POSTAL_CODE   0xDD2B
#define MTP_OBJ_PROP_POSTAL_ADDRESS_BUSINESS_COUNTRY       0xDD2C
#define MTP_OBJ_PROP_POSTAL_ADDRESS_OTHER_FULL             0xDD2D
#define MTP_OBJ_PROP_POSTAL_ADDRESS_OTHER_LINE_1           0xDD2E
#define MTP_OBJ_PROP_POSTAL_ADDRESS_OTHER_LINE_2           0xDD2F
#define MTP_OBJ_PROP_POSTAL_ADDRESS_OTHER_CITY             0xDD30
#define MTP_OBJ_PROP_POSTAL_ADDRESS_OTHER_REGION           0xDD31
#define MTP_OBJ_PROP_POSTAL_ADDRESS_OTHER_POSTAL_CODE      0xDD32
#define MTP_OBJ_PROP_POSTAL_ADDRESS_OTHER_COUNTRY          0xDD33
#define MTP_OBJ_PROP_ORGANIZATION_NAME                     0xDD34
#define MTP_OBJ_PROP_PHONETIC_ORGANIZATION_NAME            0xDD35
#define MTP_OBJ_PROP_ROLE                                  0xDD36
#define MTP_OBJ_PROP_BIRTHDATE                             0xDD37
#define MTP_OBJ_PROP_MESSAGE_TO                            0xDD40
#define MTP_OBJ_PROP_MESSAGE_CC                            0xDD41
#define MTP_OBJ_PROP_MESSAGE_BCC                           0xDD42
#define MTP_OBJ_PROP_MESSAGE_READ                          0xDD43
#define MTP_OBJ_PROP_MESSAGE_RECEIVED_TIME                 0xDD44
#define MTP_OBJ_PROP_MESSAGE_SENDER                        0xDD45
#define MTP_OBJ_PROP_ACT_BEGIN_TIME                        0xDD50
#define MTP_OBJ_PROP_ACT_END_TIME                          0xDD51
#define MTP_OBJ_PROP_ACT_LOCATION                          0xDD52
#define MTP_OBJ_PROP_ACT_REQUIRED_ATTENDEES                0xDD54
#define MTP_OBJ_PROP_ACT_OPTIONAL_ATTENDEES                0xDD55
#define MTP_OBJ_PROP_ACT_RESOURCES                         0xDD56
#define MTP_OBJ_PROP_ACT_ACCEPTED                          0xDD57
#define MTP_OBJ_PROP_OWNER                                 0xDD5D
#define MTP_OBJ_PROP_EDITOR                                0xDD5E
#define MTP_OBJ_PROP_WEBMASTER                             0xDD5F
#define MTP_OBJ_PROP_URL_SOURCE                            0xDD60
#define MTP_OBJ_PROP_URL_DESTINATION                       0xDD61
#define MTP_OBJ_PROP_TIME_BOOKMARK                         0xDD62
#define MTP_OBJ_PROP_OBJECT_BOOKMARK                       0xDD63
#define MTP_OBJ_PROP_BYTE_BOOKMARK                         0xDD64
#define MTP_OBJ_PROP_LAST_BUILD_DATE                       0xDD70
#define MTP_OBJ_PROP_TIME_TO_LIVE                          0xDD71
#define MTP_OBJ_PROP_MEDIA_GUID                            0xDD72

// Prop getter/setters
#define MTP_PROP_GET                                       0x00
#define MTP_PROP_GET_SET                                   0x01

#define MTP_OBJ_NO_PROTECTION                              0x0000
#define MTP_OBJ_READ_ONLY                                  0x0001
#define MTP_OBJ_READ_ONLY_DATA                             0x8002
#define MTP_OBJ_NON_TRANSFERRABLE_DATA                     0x8003

// Device properties
#define MTP_DEV_PROP_UNDEFINED                             0x5000
#define MTP_DEV_PROP_BATTERY_LEVEL                         0x5001
#define MTP_DEV_PROP_FUNCTIONAL_MODE                       0x5002
#define MTP_DEV_PROP_IMAGE_SIZE                            0x5003
#define MTP_DEV_PROP_COMPRESSION_SETTING                   0x5004
#define MTP_DEV_PROP_WHITE_BALANCE                         0x5005
#define MTP_DEV_PROP_RGB_GAIN                              0x5006
#define MTP_DEV_PROP_F_NUMBER                              0x5007
#define MTP_DEV_PROP_FOCAL_LENGTH                          0x5008
#define MTP_DEV_PROP_FOCUS_DISTANCE                        0x5009
#define MTP_DEV_PROP_FOCUS_MODE                            0x500A
#define MTP_DEV_PROP_EXPOSURE_METERING_MODE                0x500B
#define MTP_DEV_PROP_FLASH_MODE                            0x500C
#define MTP_DEV_PROP_EXPOSURE_TIME                         0x500D
#define MTP_DEV_PROP_EXPOSURE_PROGRAM_MODE                 0x500E
#define MTP_DEV_PROP_EXPOSURE_INDEX                        0x500F
#define MTP_DEV_PROP_EXPOSURE_BIAS_COMPENSATION            0x5010
#define MTP_DEV_PROP_DATETIME                              0x5011
#define MTP_DEV_PROP_CAPTURE_DELAY                         0x5012
#define MTP_DEV_PROP_STILL_CAPTURE_MODE                    0x5013
#define MTP_DEV_PROP_CONTRAST                              0x5014
#define MTP_DEV_PROP_SHARPNESS                             0x5015
#define MTP_DEV_PROP_DIGITAL_ZOOM                          0x5016
#define MTP_DEV_PROP_EFFECT_MODE                           0x5017
#define MTP_DEV_PROP_BURST_NUMBER                          0x5018
#define MTP_DEV_PROP_BURST_INTERVAL                        0x5019
#define MTP_DEV_PROP_TIMELAPSE_NUMBER                      0x501A
#define MTP_DEV_PROP_TIMELAPSE_INTERVAL                    0x501B
#define MTP_DEV_PROP_FOCUS_METERING_MODE                   0x501C
#define MTP_DEV_PROP_UPLOAD_URL                            0x501D
#define MTP_DEV_PROP_ARTIST                                0x501E
#define MTP_DEV_PROP_COPYRIGHT_INFO                        0x501F
#define MTP_DEV_PROP_SYNCHRONIZATION_PARTNER               0xD401
#define MTP_DEV_PROP_DEVICE_FRIENDLY_NAME                  0xD402
#define MTP_DEV_PROP_VOLUME                                0xD403
#define MTP_DEV_PROP_SUPPORTEDFORMATSORDERED               0xD404
#define MTP_DEV_PROP_DEVICEICON                            0xD405
#define MTP_DEV_PROP_PLAYBACK_RATE                         0xD410
#define MTP_DEV_PROP_PLAYBACK_OBJECT                       0xD411
#define MTP_DEV_PROP_PLAYBACK_CONTAINER                    0xD412
#define MTP_DEV_PROP_SESSION_INITIATOR_VERSION_INFO        0xD406
#define MTP_DEV_PROP_PERCEIVED_DEVICE_TYPE                 0xD407

// Operation codes
#define MTP_OP_GET_DEVICE_INFO                             0x1001
#define MTP_OP_OPEN_SESSION                                0x1002
#define MTP_OP_CLOSE_SESSION                               0x1003
#define MTP_OP_GET_STORAGE_IDS                             0x1004
#define MTP_OP_GET_STORAGE_INFO                            0x1005
#define MTP_OP_GET_NUM_OBJECTS                             0x1006
#define MTP_OP_GET_OBJECT_HANDLES                          0x1007
#define MTP_OP_GET_OBJECT_INFO                             0x1008
#define MTP_OP_GET_OBJECT                                  0x1009
#define MTP_OP_GET_THUMB                                   0x100A
#define MTP_OP_DELETE_OBJECT                               0x100B
#define MTP_OP_SEND_OBJECT_INFO                            0x100C
#define MTP_OP_SEND_OBJECT                                 0x100D
#define MTP_OP_FORMAT_STORE                                0x100F
#define MTP_OP_RESET_DEVICE                                0x1010
#define MTP_OP_GET_DEVICE_PROP_DESC                        0x1014
#define MTP_OP_GET_DEVICE_PROP_VALUE                       0x1015
#define MTP_OP_SET_DEVICE_PROP_VALUE                       0x1016
#define MTP_OP_RESET_DEVICE_PROP_VALUE                     0x1017
#define MTP_OP_TERMINATE_OPEN_CAPTURE                      0x1018
#define MTP_OP_MOVE_OBJECT                                 0x1019
#define MTP_OP_COPY_OBJECT                                 0x101A
#define MTP_OP_GET_PARTIAL_OBJECT                          0x101B
#define MTP_OP_INITIATE_OPEN_CAPTURE                       0x101C
#define MTP_OP_GET_OBJECT_PROPS_SUPPORTED                  0x9801
#define MTP_OP_GET_OBJECT_PROP_DESC                        0x9802
#define MTP_OP_GET_OBJECT_PROP_VALUE                       0x9803
#define MTP_OP_SET_OBJECT_PROP_VALUE                       0x9804
#define MTP_OP_GET_OBJECT_PROPLIST                         0x9805
#define MTP_OP_GET_OBJECT_REFERENCES                       0x9810
#define MTP_OP_GETSERVICEIDS                               0x9301
#define MTP_OP_GETSERVICEINFO                              0x9302
#define MTP_OP_GETSERVICECAPABILITIES                      0x9303
#define MTP_OP_GETSERVICEPROPDESC                          0x9304

// Response codes
#define MTP_RESP_OK                                        0x2001
#define MTP_RESP_GENERAL_ERROR                             0x2002
#define MTP_RESP_SESSION_NOT_OPEN                          0x2003
#define MTP_RESP_OPERATION_NOT_SUPPORTED                   0x2005
#define MTP_RESP_PARAMETER_NOT_SUPPORTED                   0x2006
#define MTP_RESP_INCOMPLETE_TRANSFER                       0x2007
#define MTP_RESP_INVALID_STORAGE_ID                        0x2008
#define MTP_RESP_INVALID_OBJECT_HANDLE                     0x2009
#define MTP_RESP_DEVICEPROP_NOT_SUPPORTED                  0x200A
#define MTP_RESP_INVALID_OBJECT_FORMAT_CODE                0x200B
#define MTP_RESP_STORE_FULL                                0x200C
#define MTP_RESP_OBJECT_WRITE_PROTECTED                    0x200D
#define MTP_RESP_STORE_READ_ONLY                           0x200E
#define MTP_RESP_ACCESS_DENIED                             0x200F
#define MTP_RESP_STORE_NOT_AVAILABLE                       0x2013
#define MTP_RESP_SPECIFICATION_BY_FORMAT_NOT_SUPPORTED     0x2014
#define MTP_RESP_NO_VALID_OBJECT_INFO                      0x2015
#define MTP_RESP_DEVICE_BUSY                               0x2019
#define MTP_RESP_INVALID_PARENT_OBJECT                     0x201A
#define MTP_RESP_INVALID_PARAMETER                         0x201D
#define MTP_RESP_SESSION_ALREADY_OPEN                      0x201E
#define MTP_RESP_TRANSACTION_CANCELLED                     0x201F
#define MTP_RESP_INVALID_OBJECT_PROP_CODE                  0xA801
#define MTP_RESP_SPECIFICATION_BY_GROUP_UNSUPPORTED        0xA807
#define MTP_RESP_SPECIFICATION_BY_DEPTH_UNSUPPORTED        0xA808
#define MTP_RESP_OBJECT_TOO_LARGE                          0xA809
#define MTP_RESP_OBJECT_PROP_NOT_SUPPORTED                 0xA80A


// Event codes
#define MTP_EVENT_UNDEFINED                                0x4000
#define MTP_EVENT_CANCELTRANSACTION                        0x4001
#define MTP_EVENT_OBJECTADDED                              0x4002
#define MTP_EVENT_OBJECTREMOVED                            0x4003
#define MTP_EVENT_STOREADDED                               0x4004
#define MTP_EVENT_STOREREMOVED                             0x4005
#define MTP_EVENT_DEVICEPROPCHANGED                        0x4006
#define MTP_EVENT_OBJECTINFOCHANGED                        0x4007
#define MTP_EVENT_DEVICEINFOCHANGED                        0x4008
#define MTP_EVENT_REQUESTOBJECTTRANSFER                    0x4009
#define MTP_EVENT_STOREFULL                                0x400A
#define MTP_EVENT_DEVICERESET                              0x400B
#define MTP_EVENT_STORAGEINFOCHANGED                       0x400C
#define MTP_EVENT_CAPTURECOMPLETE                          0x400D
#define MTP_EVENT_UNREPORTEDSTATUS                         0x400E
#define MTP_EVENT_OBJECTPROPCHANGED                        0xC801
#define MTP_EVENT_OBJECTPROPDESCCHANGED                    0xC802
#define MTP_EVENT_OBJECTREFERENCESCHANGED                  0xC803

// Container Types
#define MTP_CONT_TYPE_UNDEFINED                            0
#define MTP_CONT_TYPE_COMMAND                              1
#define MTP_CONT_TYPE_DATA                                 2
#define MTP_CONT_TYPE_RESPONSE                             3
#define MTP_CONT_TYPE_EVENT                                4


#define MTP_SPACE_IN_OBJ_NOT_USED                          0xFFFFFFFF

// MTP storage type
#define MTP_STORAGE_UNDEFINED                              0x0000
#define MTP_STORAGE_FIXED_ROM                              0x0001
#define MTP_STORAGE_REMOVABLE_ROM                          0x0002
#define MTP_STORAGE_FIXED_RAM                              0x0003
#define MTP_STORAGE_REMOVABLE_RAM                          0x0004

// MTP file system type
#define MTP_FILESYSTEM_UNDEFINED                           0x0000
#define MTP_FILESYSTEM_GENERIC_FLAT                        0x0001
#define MTP_FILESYSTEM_GENERIC_HIERARCH                    0x0002
#define MTP_FILESYSTEM_DCF                                 0x0003

// MTP access capability
#define MTP_ACCESS_CAP_RW                                  0x0000
#define MTP_ACCESS_CAP_RO_WITHOUT_DEL                      0x0001
#define MTP_ACCESS_CAP_RO_WITH_DEL                         0x0002


// Packet sizes
#define MTP_OPERATION_SIZE					sizeof(MTP_Operation_t)
#define MTP_CONT_HEADER_SIZE				12
#define MTP_CMD_SIZE						8
#define MTP_MEDIA_PACKET					512

/*
 * PUBLIC TYPES
 */

// Used for both operations and responses.
typedef struct __attribute((packed)) {
	uint32_t length;
	uint16_t type;
	uint16_t code;
	uint32_t transaction_id;
	uint32_t param[5];
} MTP_Operation_t;

typedef struct __attribute((packed)) {
	uint32_t length;
	uint16_t type;
	uint16_t code;
	uint32_t transaction_id;
	uint8_t  data[MTP_MEDIA_PACKET];
} MTP_Container_t;

/*
 * PUBLIC FUNCTIONS
 */

/*
 * EXTERN DECLARATIONS
 */

#endif //MTP_DEFS_H
