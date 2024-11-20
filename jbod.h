#ifndef JBOD_H_
#define JBOD_H_

#include <stdint.h>

#define JBOD_NUM_DISKS            16         // Total number of disks
#define JBOD_DISK_SIZE            65536      // Each disk is 65,536 bytes (256 blocks * 256 bytes per block)
#define JBOD_BLOCK_SIZE           256        // Each block is 256 bytes
#define JBOD_NUM_BLOCKS_PER_DISK  256        // Total blocks per disk (65536 / 256)


typedef enum {
  JBOD_MOUNT                   = 0x00,
  JBOD_UNMOUNT                 = 0x01,
  JBOD_SEEK_TO_DISK            = 0x02,
  JBOD_SEEK_TO_BLOCK           = 0x03,
  JBOD_READ_BLOCK              = 0x04,
  JBOD_WRITE_BLOCK             = 0x05, // Corrected value
  JBOD_SIGN_BLOCK              = 0x06, // Corrected value
  JBOD_RESERVED                = 0x07,
  JBOD_WRITE_PERMISSION        = 0x08, // Corrected value
  JBOD_REVOKE_WRITE_PERMISSION = 0x09, // Corrected value
  JBOD_NUM_CMDS,
} jbod_cmd_t;

typedef enum {
  JBOD_NO_ERROR,
  JBOD_UNMOUNTED,
  JBOD_ALREADY_MOUNTED,
  JBOD_ALREADY_UNMOUNTED,
  JBOD_CACHELOAD_FAIL,
  JBOD_CACHEWRITE_FAIL,
  JBOD_BAD_CMD,
  JBOD_BAD_DISK_NUM,
  JBOD_BAD_BLOCK_NUM,
  JBOD_BAD_READ,
  JBOD_BAD_WRITE,
  JBOD_WRITE_PERMISSION_ALREADY_GRANTED,
  JBOD_WRITE_PERMISSION_ALREADY_REVOKED,
  JBOD_NUM_ERRNOS,
} jbod_error_t;

int jbod_operation(uint32_t op, uint8_t *block);

extern jbod_error_t jbod_error;
const char *jbod_error_string(int eno);

#endif
