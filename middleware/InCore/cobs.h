#ifndef COBS_H
#define COBS_H
#include <stdint.h>

/**
  *@brief returns the maximum length of encoded data
  *@param len the length of the unencoded data
  *@return maximum length of data once it is encoded
  */
#define COBS_MAX_LEN(x) ((x) + (((x)+255)/254) + 1)

  typedef enum{
    COBS_OK=0,
    COBS_ERROR = -1
  }cobs_status_e;


/**
  *@brief Performs Byte stuffing and stores encoded data
  *@param source ptr to data to encode
  *@param len length of raw data
  *@param destination ptr to store endcoded data
  *@return length of encoded data
  */
int COBS_Encode(const uint8_t *source, int len, uint8_t *destination);

/**
  *@brief Decodes data by reversing the byte stuffing
  *@param ptr ptr to data to decode
  *@param len length of encoded data 
  *@param dst ptr to store decoded data
  *@return length of decoded data
  */
int COBS_Decode(const uint8_t* ptr, int len, uint8_t* dst);


#endif // COBS_H