/** (C) 28 of December, 2025 Burt Sumner */
/** Free to use, but this copyright message must remain here */

#ifndef _GBADEV_SOUND_PRIVATE_H_
#define _GBADEV_SOUND_PRIVATE_H_

#include <GBAdev_types.h>

#ifdef __cplusplus
extern "C" {
#endif  /* C++ Name mangler guard */
#define DSOUND_VCHANNEL_USER_FLAGS_MASK 0x7FFF
#define DSOUND_VCHANNEL_ENABLE          0x8000

typedef struct s_dsound_fifo_buf {
  i8 *mix_buf_base;
  i8 *cur_mix_buf;
  u32 mix_buf_sz;
  u8 active_buf;
  bool enabled;
} DirectSound_Buffer_t;



struct s_dsound_virtual_channel {
  i8 *data;
  UFixed20_12_t position;
  UFixed20_12_t increment;
  UFixed20_12_t length;
  UFixed20_12_t loop_length;
  UFixed1_6_t volume;
  UFixed1_6_t pan;
  u16 flags;
} ALIGN(4);

#ifdef __GBADEV_INTERNAL__FIFO_BUFFERS_BASE_DECL
DirectSound_Buffer_t __GBADEV_INTERNAL__dsa_buf = { 0 }, 
                     __GBADEV_INTERNAL__dsb_buf = { 0 };
#else
extern DirectSound_Buffer_t __GBADEV_INTERNAL__dsa_buf, 
                            __GBADEV_INTERNAL__dsb_buf;
#endif  /* Whether or not the FIFO buffers are declared here or externed */


#ifdef __cplusplus
}
#endif  /* C++ Name mangler guard */

#endif  /* _GBADEV_SOUND_PRIVATE_H_ */
