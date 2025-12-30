#include <GBAdev_types.h>
#include <GBAdev_memmap.h>
#include <GBAdev_util_macros.h>
#define __GBADEV_INTERNAL__FIFO_BUFFERS_BASE_DECL
#include "GBAdev_sound_private.h"

IWRAM_CODE void __GBADEV_INTERNAL__DirectSound_VSync_Callback(void) {
  if (__GBADEV_INTERNAL__dsa_buf.enabled) {
    if (1==__GBADEV_INTERNAL__dsa_buf.active_buf) {
      REG_DMA[1].cnt_reg.raw.fields = 0;
      REG_DMA[1].src = __GBADEV_INTERNAL__dsa_buf.mix_buf_base;
      REG_DMA[1].cnt_reg.raw.fields = 0xB640;
      __GBADEV_INTERNAL__dsa_buf.cur_mix_buf 
        = __GBADEV_INTERNAL__dsa_buf.mix_buf_base 
          + __GBADEV_INTERNAL__dsa_buf.mix_buf_sz;
      __GBADEV_INTERNAL__dsa_buf.active_buf = 0;
    } else {
      __GBADEV_INTERNAL__dsa_buf.cur_mix_buf 
        = __GBADEV_INTERNAL__dsa_buf.mix_buf_base;
      __GBADEV_INTERNAL__dsa_buf.active_buf = 1;
    }
  }
  if (__GBADEV_INTERNAL__dsb_buf.enabled) {
    if (1==__GBADEV_INTERNAL__dsb_buf.active_buf) {
      REG_DMA[2].cnt_reg.raw.fields = 0;
      REG_DMA[2].src = __GBADEV_INTERNAL__dsb_buf.mix_buf_base;
      REG_DMA[2].cnt_reg.raw.fields = 0xB640;
      __GBADEV_INTERNAL__dsb_buf.cur_mix_buf 
        = __GBADEV_INTERNAL__dsb_buf.mix_buf_base 
          + __GBADEV_INTERNAL__dsb_buf.mix_buf_sz;
      __GBADEV_INTERNAL__dsb_buf.active_buf = 0;
    } else {
      __GBADEV_INTERNAL__dsb_buf.cur_mix_buf
        = __GBADEV_INTERNAL__dsb_buf.mix_buf_base;
      __GBADEV_INTERNAL__dsb_buf.active_buf = 1;
    }
  }
}
