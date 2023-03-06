#ifndef __ENE_KB3930_H__
#define __ENE_KB3930_H__
/* I2C registers that are multiplexing access to the EC RAM. */
enum {
  EC_DATA_IN	= 0x00,
  EC_RAM_OUT	= 0x80,
  EC_RAM_IN	= 0x81,
};
/* EC RAM registers. */
enum {
  EC_MODEL	  = 0x30,
  EC_VERSION_MAJ  = 0x31,
  EC_VERSION_MIN  = 0x32,
};
#endif

