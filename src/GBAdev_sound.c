#include <GBAdev_sound.h>
#include <GBAdev_IRQ.h>
#include <GBAdev_memdef.h>
#include <GBAdev_memmap.h>
#include <GBAdev_types.h>
#include "GBAdev_sound_private.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
extern IWRAM_CODE void __GBADEV_INTERNAL__DirectSound_VSync_Callback(void);

static const u32 __GBADEV_INTERNAL__BUFFER_SIZES[DSOUND_CHANNEL_BUFFER_SIZE_OPTION_COUNT] = {
  96, 176, 224, 304, 352, 448, 528, 608, 672, 704
};



static const u32 __GBADEV_INTERNAL__SAMPLE_RATES[
                                  DSOUND_CHANNEL_BUFFER_SIZE_OPTION_COUNT] = {
  5734,
  10512,
  13379,
  18157,
  21024,
  26758,
  31536,
  36314,
  40137,
  42048,
};

static EWRAM_BSS i8 
  __GBADEV_INTERNAL__DSOUND_BUFFER_A[DSOUND_CHANNEL_BUFFER_MAX_SIZE*2]={0},
  __GBADEV_INTERNAL__DSOUND_BUFFER_B[DSOUND_CHANNEL_BUFFER_MAX_SIZE*2]={0};



u32 DirectSound_Channel_Init(u32 channel,
                             u32 buffer_size,
                             u32 channel_settings,
                             BOOL vol_ratio_full) {
  static const Timer_Cnt_t tm = {
    .fields = {
      .enable = TRUE,
      .freq = TIMER_FREQ_1HZ,
      .interrupt_upon_completion = FALSE,
      .cascade_mode = FALSE,
    }
  };
  
  assert(tm.raw == 0x0080);
  const size_t BUF_SZ = __GBADEV_INTERNAL__BUFFER_SIZES[buffer_size];
  const u32 SAMPLE_RATE = __GBADEV_INTERNAL__SAMPLE_RATES[buffer_size];
  int timer_idx = (DSOUND_USE_TIMER1 & channel_settings) ? 1 : 0;
  BOOL using_same_timer;
  if (buffer_size >= (u32)DSOUND_CHANNEL_BUFFER_SIZE_OPTION_COUNT)
    return FALSE;
  if (0==(REG_SND_STAT & REG_FLAG(SND_STAT, MASTER_ENABLE)))
    DirectSound_Init();
  if (channel >= 'a')
    channel -= ('a' - 'A');
  if (0 == ((DSOUND_LEFT_ENABLE | DSOUND_RIGHT_ENABLE) & channel_settings)) {
    return FALSE;
  }
  if (DSOUND_CHANNEL_A == channel) {
    // We use this BOOL more than once later, so just set it at the start
    // of each if case (for when channel is A: check DSB_USES_TIMER1 reg flag
    using_same_timer = (((REG_SND_DSOUND_CNT >> DSOUND_CNT_DSB_SETTINGS_SHAMT)
            & DSOUND_USE_TIMER1)
          == (DSOUND_USE_TIMER1 & channel_settings));
    if (using_same_timer && __GBADEV_INTERNAL__dsb_buf.enabled 
        && BUF_SZ != __GBADEV_INTERNAL__dsb_buf.mix_buf_sz) {
      return FALSE;
    }

    // Reset DS Channel A flags and fields
    REG_SND_DSOUND_CNT &= REG_FLAG_RESET_MASK(DSOUND_CNT, DSA_VOL_FULL)
                            & REG_FIELD_RESET_MASK(DSOUND_CNT, DSA_SETTINGS);

    // Set 100% vol flagbit if vol_ratio_full
    if (vol_ratio_full)
      REG_SND_DSOUND_CNT |= REG_FLAG(DSOUND_CNT, DSA_VOL_FULL);
    // Set channel settings
    REG_SND_DSOUND_CNT |= REG_VALUE(DSOUND_CNT,
                                    DSA_SETTINGS,
                                    channel_settings);
    __GBADEV_INTERNAL__dsa_buf = (DirectSound_Buffer_t) {
      .mix_buf_base = __GBADEV_INTERNAL__DSOUND_BUFFER_A,
      .cur_mix_buf = __GBADEV_INTERNAL__DSOUND_BUFFER_A+BUF_SZ,
      .mix_buf_sz = BUF_SZ,
      .active_buf = 0,
      .enabled = TRUE,
    };
    if (!using_same_timer || !__GBADEV_INTERNAL__dsb_buf.enabled) {
      REG_TM[timer_idx] = (Timer_Handle_t) {
        .cnt_reg = tm,
        .data = DSOUND_TIMER_PRESET(SAMPLE_RATE)
      };
    }
    REG_DMA[1].src = (void*)__GBADEV_INTERNAL__DSOUND_BUFFER_A; 
    REG_DMA[1].cnt_reg.raw.fields = 0xB640;
    REG_DMA[1].dst = (void*)(&REG_FIFO_A);
  } else if (DSOUND_CHANNEL_B == channel) {
    using_same_timer = (((REG_SND_DSOUND_CNT >> DSOUND_CNT_DSA_SETTINGS_SHAMT)
            & DSOUND_USE_TIMER1)
          == (DSOUND_USE_TIMER1 & channel_settings));
    if (using_same_timer && __GBADEV_INTERNAL__dsa_buf.enabled 
        && BUF_SZ != __GBADEV_INTERNAL__dsa_buf.mix_buf_sz) {
      return FALSE;
    }
    // Reset DS Channel B flags and fields
    REG_SND_DSOUND_CNT &= REG_FLAG_RESET_MASK(DSOUND_CNT, DSB_VOL_FULL)
                            & REG_FIELD_RESET_MASK(DSOUND_CNT, DSB_SETTINGS);
    // Set 100% vol flagbit if vol_ratio_full
    if (vol_ratio_full)
      REG_SND_DSOUND_CNT |= REG_VALUE(DSOUND_CNT, 
                                      DSB_SETTINGS,
                                      channel_settings);
    // Set channel settings
    REG_SND_DSOUND_CNT |= REG_VALUE(DSOUND_CNT,
                                    DSB_SETTINGS,
                                    channel_settings);

    __GBADEV_INTERNAL__dsb_buf = (DirectSound_Buffer_t) {
      .mix_buf_base = __GBADEV_INTERNAL__DSOUND_BUFFER_B,
      .cur_mix_buf = __GBADEV_INTERNAL__DSOUND_BUFFER_B,
      .mix_buf_sz = BUF_SZ,
      .active_buf = 0,
      .enabled = TRUE
    };

    if (!using_same_timer || !__GBADEV_INTERNAL__dsa_buf.enabled)
      REG_TM[timer_idx] = (Timer_Handle_t) {
        .cnt_reg = tm,
        .data = DSOUND_TIMER_PRESET(SAMPLE_RATE),
      };
    REG_DMA[2].src = (void*)__GBADEV_INTERNAL__DSOUND_BUFFER_B;
    REG_DMA[2].cnt_reg.raw.fields = 0xB640;
    REG_DMA[2].dst = (void*)(&REG_FIFO_B);
  } else {
    return FALSE;
  }


  if (0 == REG_IME) {
    REG_ISR_MAIN = ISR_Handler_Switchboard;
  } else if (ISR_Handler_Basic == REG_ISR_MAIN) {
    REG_IME = 0;
    REG_ISR_MAIN = ISR_Handler_Switchboard;
  } else {
    REG_IME = 0;
  }
  REG_IE |= IRQ_FLAG(VBLANK);
  REG_DPY_STAT |= REG_FLAG(DPY_STAT, VBL_IRQ);
  ISR_Add_Callback(IRQ_VBLANK,
                   UINT32_MAX,
                   __GBADEV_INTERNAL__DirectSound_VSync_Callback);
  REG_IME = 1;
  return TRUE;
}
