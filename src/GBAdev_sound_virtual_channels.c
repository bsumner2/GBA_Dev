#include <GBAdev_sound.h>
#include <GBAdev_functions.h>
#include <GBAdev_types.h>
#include "GBAdev_sound_private.h"
#include <stddef.h>

static IWRAM_BSS DirectSound_VChannel_t 
                      VIRTUAL_CHANNELS[DSOUND_VCHANNEL_CT] = {0};


DirectSound_VChannel_t *DirectSound_VChannel_Get(int channel_no) {
  if (0 > channel_no || DSOUND_VCHANNEL_CT <= channel_no)
    return NULL;
  return &VIRTUAL_CHANNELS[channel_no];
}

DirectSound_VChannel_t *DirectSound_VChannel_Init(
                                            DirectSound_VChannel_t *channel,
                                            void *audio_data,
                                            UFixed20_12_t len,
                                            UFixed20_12_t increment_size,
                                            UFixed20_12_t loop_len,
                                            u32 flags,
                                            UFixed1_6_t volume) {
  if (NULL == channel
      || NULL == audio_data
      || 0UL == len
      || 0UL == increment_size)
    return NULL;
  if ((UIPTR_T)VIRTUAL_CHANNELS > (UIPTR_T)channel
      || (((UIPTR_T)VIRTUAL_CHANNELS) 
            + sizeof(VIRTUAL_CHANNELS)) <= (UIPTR_T)channel)
    return NULL;

  if ((0!=(flags&DSOUND_VCHANNEL_LOOPS)) ^ (0 != loop_len))
    return NULL;

  if (64 < volume)
    volume = 64;

  *channel = (DirectSound_VChannel_t) {
    .data = audio_data,
    .position = 0UL,
    .increment = increment_size,
    .length = len,
    .loop_length = loop_len,
    .volume = volume,
    .pan = 0U,  // Placeholder for now. Panning support will be added l8r
    .flags = (flags&DSOUND_VCHANNEL_USER_FLAGS_MASK)|DSOUND_VCHANNEL_ENABLE
  };
  return channel;

}

void DirectSound_VChannel_Stop(DirectSound_VChannel_t *channel) {
  if (NULL != channel)
    channel->flags &= ~DSOUND_VCHANNEL_ENABLE;
}

void DirectSound_VChannel_Restart(DirectSound_VChannel_t *channel) {
  if (NULL==channel)
    return;
  if (NULL == channel->data || 0 == channel->length)
    return;
  if (0 == (channel->flags & (DSOUND_VCHANNEL_OUT_CHANNEL_A|DSOUND_VCHANNEL_OUT_CHANNEL_B)))
    return;

  if (DSOUND_VCHANNEL_ENABLE & channel->flags)
    channel->flags ^= DSOUND_VCHANNEL_ENABLE;  // temp disable
  channel->position = 0;
  channel->flags |= DSOUND_VCHANNEL_ENABLE;  // (re-)enable
}

void DirectSound_VChannel_Resume(DirectSound_VChannel_t *channel) {
  if (NULL==channel)
    return;
  if (NULL == channel->data || 0 == channel->length)
    return;
  if (DSOUND_VCHANNEL_ENABLE & channel->flags)
    return;
  channel->flags |= DSOUND_VCHANNEL_ENABLE;
}

void DirectSound_VChannel_MixA(void) {
  if (!__GBADEV_INTERNAL__dsa_buf.enabled)
    return;
  const i8 *cur_channel_data;
  const u32 DSA_BUF_SZ = __GBADEV_INTERNAL__dsa_buf.mix_buf_sz;
  i16 tmp_buf[DSA_BUF_SZ];
  DirectSound_VChannel_t *channel;
  u32 flags;
  u32 i, cur_channel;

  
  i = 0;
  Fast_Memset32(tmp_buf, 0, sizeof(tmp_buf)/(sizeof(WORD)));

  for (cur_channel = 0; DSOUND_VCHANNEL_CT > cur_channel; ++cur_channel) {
    channel = &VIRTUAL_CHANNELS[cur_channel];
    cur_channel_data = channel->data;
    if (NULL == cur_channel_data)
      continue;

    flags = channel->flags;
    if (0 == (flags & DSOUND_VCHANNEL_OUT_CHANNEL_A))
      continue;
    if (0 == (flags & DSOUND_VCHANNEL_ENABLE))
      continue;
    for (i=0; DSA_BUF_SZ > i; ++i) {
      tmp_buf[i] += cur_channel_data[channel->position>>12] * channel->volume;
      channel->position += channel->increment;
      if (channel->position >= channel->length) {
        if (DSOUND_VCHANNEL_LOOPS & flags) {
          while (channel->position >= channel->length)
            channel->position -= channel->loop_length;
        }
        if (DSOUND_VCHANNEL_AUTO_DISABLE & flags) {
          channel->flags ^= DSOUND_VCHANNEL_ENABLE;
          break;
        }
      }
    }
  }
  i8 *cur_mix_buf = __GBADEV_INTERNAL__dsa_buf.cur_mix_buf;
  for (i = 0; DSA_BUF_SZ > i; ++i) {
    cur_mix_buf[i] = tmp_buf[i]>>8;
  }
}

