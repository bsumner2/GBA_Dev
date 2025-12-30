/** (C) 25 of December, 2025 Burt Sumner */
/** Free to use, but this copyright message must remain here */

#ifndef _GBADEV_SOUND_H_
#define _GBADEV_SOUND_H_

#include <GBAdev_memdef.h>
#include <GBAdev_memmap.h>

#ifdef __cplusplus
extern "C" {
#endif  /* C++ Name mangler guard */

#define DSOUND_CHANNEL_A ((u32)'A')
#define DSOUND_CHANNEL_B ((u32)'B')

#define DSOUND_VCHANNEL_CT 4

typedef enum e_dsound_buffer_size_preset {
  DSOUND_CHANNEL_BUFFER_SIZE_96_BYTES=0,
  DSOUND_CHANNEL_BUFFER_SIZE_176_BYTES,
  DSOUND_CHANNEL_BUFFER_SIZE_224_BYTES,
  DSOUND_CHANNEL_BUFFER_SIZE_304_BYTES,
  DSOUND_CHANNEL_BUFFER_SIZE_352_BYTES,
  DSOUND_CHANNEL_BUFFER_SIZE_448_BYTES,
  DSOUND_CHANNEL_BUFFER_SIZE_528_BYTES,
  DSOUND_CHANNEL_BUFFER_SIZE_608_BYTES,
  DSOUND_CHANNEL_BUFFER_SIZE_672_BYTES,
  DSOUND_CHANNEL_BUFFER_SIZE_704_BYTES,
  DSOUND_CHANNEL_BUFFER_SIZE_OPTION_COUNT,
  DSOUND_CHANNEL_BUFFER_MAX_SIZE=704
} DirectSound_BufferSize_Preset_e;

typedef struct s_dsound_virtual_channel DirectSound_VChannel_t;

/* @brief Initialize master sound enable flag,
 * and correct amplitude resolution value,
 * to their respective sound control registers
 * */
INLINE void DirectSound_Init(void);

#if __STDC_VERSION__>=202311L  /* Hoverdoc differs if stdcver >= c23 */
/**
 * @brief Set up direct sound with given settings for DirectSound channel A
 * ----------------------------------------------------------------------------
 * @param channel Which channel to set. Use one of the preset macros, 
 * DSOUND_CHANNEL_A or DSOUND_CHANNEL_B, for value passed thru this param.
 * ----------------------------------------------------------------------------
 * @param buffer_size Used to decide sample rate. Use one of the enum presets,
 * DSOUND_CHANNEL_BUFFER_SIZE_X_BYTES, for value passed thru this param.
 * ----------------------------------------------------------------------------
 * @param channel_settings Used to set channel settings. 
 * Pass value yielded by macro,
 * DSOUND_CHANNEL_SETTINGS(setting1, ..., setting_n_minus_1, setting_n).
 * Since settings include LEFT_ENABLE, RIGHT_ENABLE, setting must not be zero, 
 * as you at least need to enable left speaker or right speaker, if not both,
 * in settings.
 * ----------------------------------------------------------------------------
 * @param vol_ratio_full Used to set channel settings. If set to true, volume
 * ratio set to 100%, otherwise, if false, it's set to 50%.
 * ----------------------------------------------------------------------------
 * @return 0 on failure or hardware sample rate in hertz on success
 * ----------------------------------------------------------------------------
 * */
u32 DirectSound_Channel_Init(u32 channel,
                             u32 buffer_size,
                             u32 channel_settings,
                             BOOL vol_ratio_full);

#else
/**
 * @brief Set up direct sound with given settings for given DirectSound channel
 * ----------------------------------------------------------------------------
 * @param channel Which channel to set. Use either of the macros,
 * DSOUND_CHANNEL_A or DSOUND_CHANNEL_B, for value passed thru this param.
 * ----------------------------------------------------------------------------
 * @param buffer_size Used to decide sample rate. Use one of the enum presets,
 * DSOUND_CHANNEL_BUFFER_SIZE_X_BYTES, for value passed thru this param.
 * ----------------------------------------------------------------------------
 * @param channel_settings Used to set channel settings. Set by OR'ing channel
 * settings. Since settings include DSOUND_LEFT_ENABLE, DSOUND_RIGHT_ENABLE, 
 * setting must not be zero,  as you at least need to enable left speaker or 
 * right speaker, if not both, in settings.
 * ----------------------------------------------------------------------------
 * @param vol_ratio_full Used to set channel settings. If set to true, volume
 * ratio set to 100%, otherwise, if false, it's set to 50%.
 * ----------------------------------------------------------------------------
 * @return 0 on failure or hardware sample rate in hertz on success
 * ----------------------------------------------------------------------------
 * */
u32 DirectSound_Channel_Init(u32 channel,
                        u32 buffer_size,
                        u32 channel_settings,
                        BOOL vol_ratio_full);
#endif  /* The hoverdoc diffs between C versions (>=c23 vs <c23) */

/**
 * @brief Get the virtual channel by its given channel number.
 * ----------------------------------------------------------------------------
 * @param channel_no The channel number of the virtual channel you want to
 * retrieve. Valid channel numbers are between 0 and DSOUND_VCHANNEL_CT-1.
 * ----------------------------------------------------------------------------
 * @return Pointer to channel#channel_no.
 * ----------------------------------------------------------------------------
 * */
DirectSound_VChannel_t *DirectSound_VChannel_Get(int vchannel_no);

#define DSOUND_FEATURES_TO_BE_DEVELOPED

#define DSOUND_VCHANNEL_OUT_CHANNEL_A   0x4000
#define DSOUND_VCHANNEL_OUT_CHANNEL_B   0x2000

#define DSOUND_VCHANNEL_LOOPS           0x1000
#define DSOUND_VCHANNEL_NO_STEAL        0x0800
#define DSOUND_VCHANNEL_AUTO_DISABLE    0x0400

#ifndef DSOUND_FEATURES_TO_BE_DEVELOPED
#define DSOUND_VCHANNEL_REVERSE         0x0200
#define DSOUND_VCHANNEL_PINGPONG        0x0100

#define DSOUND_VCHANNEL_FORMAT_16BIT    0x0080
#define DSOUND_VCHANNEL_FADE_OUT        0x0040
#define DSOUND_VCHANNEL_NO_PAN          0x0020
#else
#define DSOUND_VCHANNEL_RESERVED_BIT9   0x0200
#define DSOUND_VCHANNEL_RESERVED_BIT8   0x0100

#define DSOUND_VCHANNEL_RESERVED_BIT7   0x0080
#define DSOUND_VCHANNEL_RESERVED_BIT6   0x0040
#define DSOUND_VCHANNEL_RESERVED_BIT5   0x0020
#endif  /* Keep these flags inactive till we finish developing the baseline */

/**
 * @brief Initialize the given virtual channel, with the given settings. This 
 * implicitly sets the enable flagbit for the virtual channel.
 * ----------------------------------------------------------------------------
 * @param audio_data Pointer to the actual audio track data.
 * ----------------------------------------------------------------------------
 * @param len Length of audio track as a 20.12 fixed point rational number.
 * ----------------------------------------------------------------------------
 * @param increment_size By how much should the channel's playback seek 
 * position be advanced per increment interval. Represented as a 20.12 fixed
 * point rational.
 * ----------------------------------------------------------------------------
 * @param loop_len Length of track to loop as a 20.12 fixed point rational.
 * ----------------------------------------------------------------------------
 * @param flags Channel Config flags. OR'd together. vchannel flags should be 
 * defined as macro constants of format DSOUND_VCHANNEL_<SETTING IN ALL CAPS>.
 * @param volume Volume level of track as 1.6 fixed point rational.
 * (i.e.: 64 = 0b1000000 = 0b1.000000 = 100% volume level,
 *        32 = 0b0100000 = 0b0.100000 = 50% volume lvl,
 *        4 = 0b00000100 = 0b0.000100 = 6.25% volume level)
 * ----------------------------------------------------------------------------
 * @return Same channel pointer given OR NULL if there was any invalid params.
 * ----------------------------------------------------------------------------
 * */
DirectSound_VChannel_t *DirectSound_VChannel_Init(
                                            DirectSound_VChannel_t *channel,
                                            void *audio_data,
                                            UFixed20_12_t len,
                                            UFixed20_12_t increment_size,
                                            UFixed20_12_t loop_len,
                                            u32 flags,
                                            UFixed1_6_t volume);
/**
 * @brief Disables the given channel, if channel is not NULL.
 * */
void DirectSound_VChannel_Stop(DirectSound_VChannel_t *channel);

void DirectSound_VChannel_Restart(DirectSound_VChannel_t *channel);

void DirectSound_VChannel_Resume(DirectSound_VChannel_t *channel);

/**
 * @brief Apply sound mixing from virtual channels, mapped to hardware channel 
 * A and enabled, onto channel A's buffer. IFF channel A Init'd. 
 * */
void DirectSound_VChannel_MixA(void);


void DirectSound_Init(void) {
  u16 snd_bias;
  REG_SND_STAT |= REG_FLAG(SND_STAT, MASTER_ENABLE);
  snd_bias = REG_SND_BIAS & ~SND_BIAS_AMPLITUDE_RESOLUTION_MASK;
  REG_SND_BIAS =    snd_bias | REG_VALUE(SND_BIAS, 
                                         AMPLITUDE_RESOLUTION,
                                         AMPLITUDE_RES(8, 65536));
}


#ifdef __cplusplus
}
#endif  /* C++ Name mangler guard */

#endif  /* _GBADEV_SOUND_H_ */
