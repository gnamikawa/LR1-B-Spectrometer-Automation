#ifndef LIBSHARED_AND_STATIC_EXPORT_H
#define LIBSHARED_AND_STATIC_EXPORT_H

#ifdef LIBSHARED_AND_STATIC_STATIC_DEFINE
#  define LIBSHARED_AND_STATIC_EXPORT
#  define LIBSHARED_AND_STATIC_NO_EXPORT
#else
#  ifndef LIBSHARED_AND_STATIC_EXPORT
#    ifdef spectrlib_shared_EXPORTS
#      define LIBSHARED_AND_STATIC_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define LIBSHARED_AND_STATIC_EXPORT __declspec(dllimport)
#    endif
#  endif

#  ifndef LIBSHARED_AND_STATIC_NO_EXPORT
#    define LIBSHARED_AND_STATIC_NO_EXPORT
#  endif
#endif

#ifndef LIBSHARED_AND_STATIC_DEPRECATED
#  define LIBSHARED_AND_STATIC_DEPRECATED __declspec(deprecated)
#  define LIBSHARED_AND_STATIC_DEPRECATED_EXPORT LIBSHARED_AND_STATIC_EXPORT __declspec(deprecated)
#  define LIBSHARED_AND_STATIC_DEPRECATED_NO_EXPORT LIBSHARED_AND_STATIC_NO_EXPORT __declspec(deprecated)
#endif

#define DEFINE_NO_DEPRECATED 0
#if DEFINE_NO_DEPRECATED
# define LIBSHARED_AND_STATIC_NO_DEPRECATED
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char u_int8_t;
typedef unsigned short u_int16_t;
typedef unsigned int u_int32_t;

LIBSHARED_AND_STATIC_EXPORT int connectToDevice(const char* serialNumber);
LIBSHARED_AND_STATIC_EXPORT void disconnectDevice();
LIBSHARED_AND_STATIC_EXPORT int setAcquisitionParameters(u_int16_t numOfScans, u_int16_t numOfBlankScans, u_int8_t scanMode, u_int32_t timeOfExposure);
LIBSHARED_AND_STATIC_EXPORT int setExposure(const u_int32_t timeOfExposure, const u_int8_t force);
LIBSHARED_AND_STATIC_EXPORT int setFrameFormat(const u_int16_t numOfStartElement, const u_int16_t numOfEndElement, const u_int8_t reductionMode, u_int16_t *numOfPixelsInFrame);
LIBSHARED_AND_STATIC_EXPORT int getFrameFormat(u_int16_t *numOfStartElement, u_int16_t *numOfEndElement, u_int8_t *reductionMode, u_int16_t *numOfPixelsInFrame);
LIBSHARED_AND_STATIC_EXPORT int triggerAcquisition();
LIBSHARED_AND_STATIC_EXPORT int setExternalTrigger(const u_int8_t enableMode, const u_int8_t signalFrontMode);
LIBSHARED_AND_STATIC_EXPORT int setOpticalTrigger(const u_int8_t enableMode, const u_int16_t pixel, const u_int16_t threshold);

LIBSHARED_AND_STATIC_EXPORT int getStatus(u_int8_t *statusFlags, u_int16_t *framesInMemory);
LIBSHARED_AND_STATIC_EXPORT int clearMemory();

LIBSHARED_AND_STATIC_EXPORT int getFrame(u_int16_t  *framePixelsBuffer, const u_int16_t numOfFrame);

LIBSHARED_AND_STATIC_EXPORT int eraseFlash();
LIBSHARED_AND_STATIC_EXPORT int readFlash(u_int8_t *buffer, u_int32_t absoluteOffset, u_int32_t bytesToRead);
LIBSHARED_AND_STATIC_EXPORT int writeFlash(u_int8_t *buffer, u_int32_t offset, u_int32_t bytesToWrite);

LIBSHARED_AND_STATIC_EXPORT int resetDevice();
LIBSHARED_AND_STATIC_EXPORT int detachDevice();

LIBSHARED_AND_STATIC_EXPORT int getAcquisitionParameters(u_int16_t* numOfScans, u_int16_t* numOfBlankScans, u_int8_t* scanMode, u_int32_t* timeOfExposure);

LIBSHARED_AND_STATIC_EXPORT int setAllParameters(u_int16_t numOfScans, u_int16_t numOfBlankScans, u_int8_t scanMode, u_int32_t timeOfExposure, u_int8_t enableMode, u_int8_t signalFrontMode);


#define OK 0
#define CONNECT_ERROR_WRONG_ID 500
#define CONNECT_ERROR_NOT_FOUND 501
#define CONNECT_ERROR_FAILED 502
#define DEVICE_NOT_INITIALIZED 503
#define WRITING_PROCESS_FAILED 504
#define READING_PROCESS_FAILED 505
#define WRONG_ANSWER 506
#define GET_FRAME_REMAINING_PACKETS_ERROR 507
#define NUM_OF_PACKETS_IN_FRAME_ERROR 508
#define INPUT_PARAMETER_NOT_INITIALIZED 509
#define READ_FLASH_REMAINING_PACKETS_ERROR 510

#ifdef __cplusplus
}
#endif

#endif
