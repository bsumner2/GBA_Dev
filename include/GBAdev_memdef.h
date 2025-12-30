/** (C) 7 of September, 2025 Burt Sumner */
/** Free to use, but this copyright message must remain here */

#ifndef _GBADEV_MEMDEF_H_
#define _GBADEV_MEMDEF_H_

#ifdef __cplusplus
extern "C" {
#endif  /* C++ Name mangler guard */

/* MEMORY REGIONS */
#define MEM_IO            0x04000000
#define MEM_PAL           0x05000000
#define MEM_VRAM          0x06000000
#define MEM_OAM           0x07000000
#define MEM_ROM           0x08000000
#define MEM_SRAM          0x0E000000

/* MEMORY SEGMENT SIZES */
#define M4_PAGE_SIZE      0x9600
#define M5_PAGE_SIZE      0xA000
#define SRAM_SIZE         0x10000

#define PAL_BG_SIZE                 0x0200
#define PAL_OBJ_SIZE                0x0200

#define VRAM_BG_SIZE                0x10000
#define VRAM_OBJ_SIZE               0x08000

#define M3_SIZE                     0x12C00
#define M4_SIZE                     0x09600
#define M5_SIZE                     0x0A000
#define BMP_PAGE_SIZE               0x0A000

/* GBA BMP mode screen buffer dims */
#define SCREEN_HEIGHT               160
#define SCREEN_WIDTH                240

#define M3_SCREEN_HEIGHT            160
#define M3_SCREEN_WIDTH             240

#define M4_SCREEN_HEIGHT            160
#define M4_SCREEN_WIDTH             240

#define M5_SCREEN_HEIGHT            128
#define M5_SCREEN_WIDTH             160

/* REG_KEY_STAT */
#define KEY_STAT_KEYFLAGS_MASK      0x03FF
#define KEY_STAT_KEYFLAGS_SHAMT     0

/* Key flags */
#define KEY_A                       0x0001
#define KEY_B                       0x0002
#define KEY_SEL                     0x0004
#define KEY_START                   0x0008
#define KEY_RIGHT                   0x0010
#define KEY_LEFT                    0x0020
#define KEY_UP                      0x0040
#define KEY_DOWN                    0x0080
#define KEY_R                       0x0100
#define KEY_L                       0x0200
#define KEY_AB                      0x0003
#define KEY_STARTSEL                0x000C
#define KEY_DPAD_HORIZONTAL         0x0030
#define KEY_DPAD_VERTICAL           0x00C0
#define KEY_DPAD                    0x00F0
#define KEY_BUMPERS                 0x0300
#define KEY_ANY                     0x03FF
#define KEY_ALL                     0x03FF

/* REG_KEY_CNT */
#define KEY_CNT_KEYFLAGS_MASK       0x03FF
#define KEY_CNT_IRQ_FLAGBIT         0x4000
#define KEY_CNT_BWISE_AND_FLAGBIT   0x8000

#define KEY_CNT_KEYFLAGS_SHAMT      0

/* Display control reg bitfield flags and masks */
#define DPY_CNT_MODE_MASK           0x0007
#define DPY_CNT_GB_FLAGBIT          0x0008
#define DPY_CNT_PAGE_SEL_FLAGBIT    0x0010
#define DPY_CNT_OAM_HBL_FLAGBIT     0x0020
#define DPY_CNT_OBJ_1D_FLAGBIT      0x0040
#define DPY_CNT_BLANK_FLAGBIT       0x0080
#define DPY_CNT_BG0_FLAGBIT         0x0100
#define DPY_CNT_BG1_FLAGBIT         0x0200
#define DPY_CNT_BG2_FLAGBIT         0x0400
#define DPY_CNT_BG3_FLAGBIT         0x0800
#define DPY_CNT_OBJ_FLAGBIT         0x1000
#define DPY_CNT_WIN0_FLAGBIT        0x2000
#define DPY_CNT_WIN1_FLAGBIT        0x4000
#define DPY_CNT_WIN_OBJ_FLAGBIT     0x8000

#define DPY_CNT_MODE_SHAMT            0
#define DPY_STAT_VCOUNT_SHAMT         8

/* Display status reg bitfield flag and masks bit */
#define DPY_STAT_IN_VBL_FLAGBIT       0x0001
#define DPY_STAT_IN_HBL_FLAGBIT       0x0002
#define DPY_STAT_IN_VCOUNT_FLAGBIT    0x0004
#define DPY_STAT_VBL_IRQ_FLAGBIT      0x0008
#define DPY_STAT_HBL_IRQ_FLAGBIT      0x0010
#define DPY_STAT_VCOUNT_IRQ_FLAGBIT   0x0020
#define DPY_STAT_VCOUNT_MASK          0xFF00

/* Background control reg fields */
#define BG_CNT_PRIORITY_MASK          0x0003
#define BG_CNT_TILE_BLOCK_BASE_MASK   0x000C
#define BG_CNT_MOSAIC_FLAGBIT         0x0040
#define BG_CNT_8BPP_FLAGBIT           0x0080
#define BG_CNT_SCR_BLOCK_BASE_MASK    0x1F00
#define BG_CNT_AFFINE_WRAP_FLAGBIT    0x2000
#define BG_CNT_BG_SIZE_MASK           0xC000

#define BG_CNT_PRIORITY_SHAMT         0
#define BG_CNT_TILE_BLOCK_BASE_SHAMT  2
#define BG_CNT_SCR_BLOCK_BASE_SHAMT   8
#define BG_CNT_BG_SIZE_SHAMT          14

/* BLEND control and param registers */
#define BLEND_CNT_LAYER_A_BG0_FLAGBIT       0x0001
#define BLEND_CNT_LAYER_A_BG1_FLAGBIT       0x0002
#define BLEND_CNT_LAYER_A_BG2_FLAGBIT       0x0004
#define BLEND_CNT_LAYER_A_BG3_FLAGBIT       0x0008
#define BLEND_CNT_LAYER_A_OBJ_FLAGBIT       0x0010
#define BLEND_CNT_LAYER_A_BACKDROP_FLAGBIT  0x0020
#define BLEND_CNT_BLEND_MODE_MASK           0x00C0
#define BLEND_CNT_LAYER_B_BG0_FLAGBIT       0x0100
#define BLEND_CNT_LAYER_B_BG1_FLAGBIT       0x0200
#define BLEND_CNT_LAYER_B_BG2_FLAGBIT       0x0400
#define BLEND_CNT_LAYER_B_BG3_FLAGBIT       0x0800
#define BLEND_CNT_LAYER_B_OBJ_FLAGBIT       0x1000
#define BLEND_CNT_LAYER_B_BACKDROP_FLAGBIT  0x2000

/* Sound control registers */
#define DMG_CNT_LVOL_MASK                     0x0007
#define DMG_CNT_RVOL_MASK                     0x0070
#define DMG_CNT_LSQR1_FLAGBIT                 0x0100
#define DMG_CNT_LSQR2_FLAGBIT                 0x0200
#define DMG_CNT_LWAVE_FLAGBIT                 0x0400
#define DMG_CNT_LNOISE_FLAGBIT                0x0800
#define DMG_CNT_LEFT_CHANNELS_ENABLE_MASK     0x0F00
#define DMG_CNT_RSQR1_FLAGBIT                 0x1000
#define DMG_CNT_RSQR2_FLAGBIT                 0x2000
#define DMG_CNT_RWAVE_FLAGBIT                 0x4000
#define DMG_CNT_RNOISE_FLAGBIT                0x8000
#define DMG_CNT_RIGHT_CHANNELS_ENABLE_MASK    0xF000

#define DMG_CNT_LVOL_SHAMT                    0
#define DMG_CNT_RVOL_SHAMT                    4
#define DMG_CNT_LEFT_CHANNELS_ENABLE_SHAMT    8
#define DMG_CNT_RIGHT_CHANNELS_ENABLE_SHAMT   12


#define DSOUND_CNT_DMG_VOL_RATIO_MASK             0x0003
#define DSOUND_CNT_DSA_VOL_FULL_FLAGBIT           0x0004
#define DSOUND_CNT_DSB_VOL_FULL_FLAGBIT           0x0008
#define DSOUND_CNT_DSA_LEFT_ENABLE_FLAGBIT        0x0100
#define DSOUND_CNT_DSA_RIGHT_ENABLE_FLAGBIT       0x0200
#define DSOUND_CNT_DSA_USE_TIMER1_FLAGBIT         0x0400
#define DSOUND_CNT_DSA_RESET_FLAGBIT              0x0800
#define DSOUND_CNT_DSA_SETTINGS_MASK              0x0F00
#define DSOUND_CNT_DSB_LEFT_ENABLE_FLAGBIT        0x1000
#define DSOUND_CNT_DSB_RIGHT_ENABLE_FLAGBIT       0x2000
#define DSOUND_CNT_DSB_USE_TIMER1_FLAGBIT         0x4000
#define DSOUND_CNT_DSB_RESET_FLAGBIT              0x8000
#define DSOUND_CNT_DSB_SETTINGS_MASK              0xF000

#define DSOUND_CNT_DMG_VOL_RATIO_SHAMT            0
#define DSOUND_CNT_DSA_SETTINGS_SHAMT             8
#define DSOUND_CNT_DSB_SETTINGS_SHAMT             12


#define SND_CNT_LVOL_MASK                       0x00000007
#define SND_CNT_RVOL_MASK                       0x00000070
#define SND_CNT_LSQR1_FLAGBIT                   0x00000100
#define SND_CNT_LSQR2_FLAGBIT                   0x00000200
#define SND_CNT_LWAVE_FLAGBIT                   0x00000400
#define SND_CNT_LNOISE_FLAGBIT                  0x00000800
#define SND_CNT_LEFT_DMG_CHANNELS_ENABLE_MASK   0x00000F00
#define SND_CNT_RSQR1_FLAGBIT                   0x00001000
#define SND_CNT_RSQR2_FLAGBIT                   0x00002000
#define SND_CNT_RWAVE_FLAGBIT                   0x00004000
#define SND_CNT_RNOISE_FLAGBIT                  0x00008000
#define SND_CNT_RIGHT_DMG_CHANNELS_ENABLE_MASK  0x0000F000
#define SND_CNT_DMG_VOL_RATIO_MASK              0x00030000
#define SND_CNT_DSA_VOL_FULL_FLAGBIT            0x00040000
#define SND_CNT_DSB_VOL_FULL_FLAGBIT            0x00080000
#define SND_CNT_DSA_LEFT_ENABLE_FLAGBIT         0x01000000
#define SND_CNT_DSA_RIGHT_ENABLE_FLAGBIT        0x02000000
#define SND_CNT_DSA_USE_TIMER1_FLAGBIT          0x04000000
#define SND_CNT_DSA_RESET_FLAGBIT               0x08000000
#define SND_CNT_DSA_SETTINGS_MASK               0x0F000000
#define SND_CNT_DSB_LEFT_ENABLE_FLAGBIT         0x10000000
#define SND_CNT_DSB_RIGHT_ENABLE_FLAGBIT        0x20000000
#define SND_CNT_DSB_USE_TIMER1_FLAGBIT          0x40000000
#define SND_CNT_DSB_RESET_FLAGBIT               0x80000000
#define SND_CNT_DSB_SETTINGS_MASK               0xF0000000

#define SND_CNT_LVOL_SHAMT                          0
#define SND_CNT_RVOL_SHAMT                          4
#define SND_CNT_LEFT_DMG_CHANNELS_ENABLE_SHAMT      8
#define SND_CNT_RIGHT_DMG_CHANNELS_ENABLE_SHAMT     12
#define SND_CNT_DMG_VOL_RATIO_SHAMT                 16
#define SND_CNT_DSA_SETTINGS_SHAMT                  24
#define SND_CNT_DSB_SETTINGS_SHAMT                  28

#define SND_STAT_MASTER_ENABLE_FLAGBIT          0x0080

#define SND_BIAS_AMPLITUDE_RESOLUTION_MASK      0xC000
#define SND_BIAS_AMPLITUDE_RESOLUTION_SHAMT     14

/* BLEND control and param registers' fields */
#define BLEND_CNT_BLEND_MODE_SHAMT          6

#define BLEND_ALPHA_LAYER_A_WEIGHT_MASK     0x001F
#define BLEND_ALPHA_LAYER_B_WEIGHT_MASK     0x1F00

#define BLEND_ALPHA_LAYER_A_WEIGHT_SHAMT    0
#define BLEND_ALPHA_LAYER_B_WEIGHT_SHAMT    8

#define BLEND_Y_TOP_BLEND_FADE_MASK         0x001F
#define BLEND_Y_TOP_BLEND_FADE_SHAMT        0

#define BLEND_FADE_TOP_BLEND_FADE_MASK         0x001F
#define BLEND_FADE_TOP_BLEND_FADE_SHAMT        0

#define BLEND_MODE_OFF      0
#define BLEND_MODE_ALPHA    1
#define BLEND_MODE_WHITE    2
#define BLEND_MODE_BLACK    3

/* Sprite and BG tile block dims */
#define TILE_DIMS                     8
#define TILE4_DIMS                    8
#define TILE8_DIMS                    8
/* DMA stuff */
#define DMA_DST_INC         0b00
#define DMA_DST_DEC         0b01
#define DMA_DST_FIXED       0b10
#define DMA_DST_RELOAD      0b11

#define DMA_SRC_INC         0b00
#define DMA_SRC_DEC         0b01
#define DMA_SRC_FIXED       0b10
#define DMA_SRC_RElOAD  \
  _Pragma("GCC error \"No reload option for DMA source addr adjustment\"")
#define DMA_ADDR_ADJ(src_or_dst, type)  \
  DMA_##src_or_dst##_##type

#define DMA_TIMING_NOW           0b00
#define DMA_TIMING_AT_VBLANK     0b01
#define DMA_TIMING_AT_HBLANK     0b10
#define DMA_TIMING_FIFO          0b11
#define DMA_TIMING_MODE(mode) DMA_TIMING_##mode

/* Sound stuff */
#define DMG_SQR1_ENABLE               0b0001
#define DMG_SQR2_ENABLE               0b0010
#define DMG_WAVE_ENABLE               0b0100
#define DMG_NOISE_ENABLE              0b1000

#define DSOUND_LEFT_ENABLE            0b0001
#define DSOUND_RIGHT_ENABLE           0b0010
#define DSOUND_USE_TIMER1             0b0100
#define DSOUND_RESET_FIFO             0b1000

#define DMG_VOL_RATIO_25_PERCENT      0b00
#define DMG_VOL_RATIO_50_PERCENT      0b01
#define DMG_VOL_RATIO_75_PERCENT      _Pragma("GCC error \"75% ratio unavailable\"")
#define DMG_VOL_RATIO_100_PERCENT     0b10

#define DMG_VOL_RATIO(n) DMG_VOL_RATIO_##n##_PERCENT

#define AMPLITUDE_RES_9_BIT_AT_32768HZ    0b00
#define AMPLITUDE_RES_8_BIT_AT_65536HZ    0b01
#define AMPLITUDE_RES_7_BIT_AT_131072HZ   0b10
#define AMPLITUDE_RES_6_BIT_AT_262144HZ   0b11

#define TIMER_RANGE (1UL<<16)
#define CPU_FREQ 16777216UL
#define TIMER_PERIODICITY(cycles_per_tick, countdown_ticks)  \
  (TIMER_RANGE - (CPU_FREQ/(cycles_per_tick*countdown_ticks)))

#define DSOUND_TIMER_PRESET(sample_rate_hz)  \
  TIMER_PERIODICITY(1, sample_rate_hz)
#define AMPLITUDE_RES(bitrate, freq) AMPLITUDE_RES_##bitrate##_BIT_AT_##freq##HZ

#define REG_GET_FIELD(register_value, register_name, field_name)  \
  (((register_value) & register_name##_##field_name##_MASK)  \
    >>register_name##_##field_name##_SHAMT)

#define REG_GET_FLAG(register_value, register_name, flag_name)  \
  ((BOOL) (((register_value) & register_name##_##flag_name##_FLAGBIT)!=0))

#if __STDC_VERSION__==202311L
#define REG_FLAG_RESET_MASK(regname, flagname) (~REG_FLAG(regname, flagname))
#define REG_FLAGS_RESET_MASK(regname, flag1name, ...)  \
  (~REG_FLAGS(regname, flag1name, __VA_ARGS__))
#define REG_FIELD_RESET_MASK(regname, fieldname)  \
  (~REG_FIELD_MASK(regname, fieldname))
#define REG_FIELDS_RESET_MASK(regname, fieldname1, ...)  \
  (~REG_FIELD_MASKS(regname, fieldname1, __VA_ARGS__))
#define REG_FLAG(regname, flagname) REG_FLAGS(regname, flagname)
#define REG_FIELD_MASK(regname, fieldname) REG_FIELD_MASKS(regname, fieldname)
#define REG_VALUE(regname, fieldname, value) REG_VALUES(regname, fieldname, value)
#define IRQ_FLAG(type) IRQ_FLAGS(type)
#define KEY_FLAG(key) KEY_FLAGS(key)

#define REG_FLAGS(regname, flagname0, ...)  \
  (regname##_##flagname0##_FLAGBIT __VA_OPT__ (  \
    | __GBADEV_INTERNAL__EXP(__GBADEV_INTERNAL__REGFLAGS_VA_FOLD(regname,  \
        __VA_ARGS__))))

#define REG_FIELD_MASKS(regname, field1, ...)  \
  (regname##_##field1##_MASK __VA_OPT__ (  \
    | __GBADEV_INTERNAL__EXP(__GBADEV_INTERNAL__REGFMASKS_VA_FOLD(regname,  \
        __VA_ARGS__))))

#define REG_VALUES(regname, fieldname0, value0, ...)  \
  ((regname##_##fieldname0##_MASK&((value0)<<regname##_##fieldname0##_SHAMT))  \
   __VA_OPT__ (| __GBADEV_INTERNAL__EXP(__GBADEV_INTERNAL__REGVALUES_VA_FOLD(  \
         regname, __VA_ARGS__))))

#define IRQ_FLAGS(irq0, ...)  \
  ((1<<IRQ_##irq0)  \
   __VA_OPT__(| __GBADEV_INTERNAL__EXP(__GBADEV_INTERNAL__IRQFLAGS_VA_FOLD(  \
         __VA_ARGS__))))

#define KEY_FLAGS(key0, ...) \
  (KEY_##key0  \
   __VA_OPT__(| __GBADEV_INTERNAL__EXP(__GBADEV_INTERNAL__KEYFLAGS_VA_FOLD(  \
         __VA_ARGS__))))

#define DMG_CHANNELS_ENABLE(channel1, ...)  \
  (DMG_##channel1##_ENABLE  \
   __VA_OPT__(| __GBADEV_INTERNAL__EXP(__GBADEV_INTERNAL__DMGCHANEN_VA_FOLD(  \
         __VA_ARGS__))))
#define DSOUND_CHANNEL_SETTINGS(setting1, ...)  \
  (DSOUND_##setting1  \
   __VA_OPT__(| __GBADEV_INTERNAL__EXP(__GBADEV_INTERNAL__DSCHANSET_VA_FOLD(  \
         __VA_ARGS__))))

#else

#define REG_FLAG_RESET_MASK(regname, flagname) (~REG_FLAG(regname, flagname))
#define REG_FIELD_RESET_MASK(regname, fieldname)  \
  (~REG_FIELD_MASK(regname, fieldname))

#define REG_FLAG(regname, flagname) (regname##_##flagname##_FLAGBIT)
#define REG_FIELD_MASK(regname, fieldname) (regname##_##fieldname##_MASK)
#define REG_VALUE(regname, fieldname, value)  \
  (regname##_##fieldname##_MASK&((value)<<regname##_##fieldname##_SHAMT))
#define KEY_FLAG(key) KEY_##key
#define IRQ_FLAG(type) (1<<IRQ_##type)

/* KEY_FLAGS and IRQ_FLAGS don't have non-variadic counterparts because typing 
 * the base case is so trivial, it's easier to just type KEY_A instead of 
 * KEY_FLAG(A), for instance */
#endif  /* Variadic version of bitfield flagbit and value constructors only 
         * available with c23 */
/* Use these if you want more descriptive indicators when setting values in
 * one of the register bitfield struct bool type, 1b, fields. */
#define TRUE 1U
#define FALSE 0U

#define OBJ_GFX_MODE_NORMAL         0
#define OBJ_GFX_MODE_BLEND          1
#define OBJ_GFX_MODE_WINDOW         2

#define OBJ_SHAPE_SQUARE            0
#define OBJ_SHAPE_WIDE              1
#define OBJ_SHAPE_TALL              2

#define OBJ_SIZE_SQUARE_8x8         0
#define OBJ_SIZE_SQUARE_16x16       1
#define OBJ_SIZE_SQUARE_32x32       2
#define OBJ_SIZE_SQUARE_64x64       3

#define OBJ_SIZE_WIDE_16x8          0
#define OBJ_SIZE_WIDE_32x8          1
#define OBJ_SIZE_WIDE_32x16         2
#define OBJ_SIZE_WIDE_64x32         3

#define OBJ_SIZE_TALL_8x16          0
#define OBJ_SIZE_TALL_8x32          1
#define OBJ_SIZE_TALL_16x32         2
#define OBJ_SIZE_TALL_32x64         3

#define BG_REGULAR_32x32_BLOCKS     0
#define BG_REGULAR_64x32_BLOCKS     1
#define BG_REGULAR_32x64_BLOCKS     2
#define BG_REGULAR_64x64_BLOCKS     3

#define BG_AFFINE_16x16_BLOCKS      0
#define BG_AFFINE_32x32_BLOCKS      1
#define BG_AFFINE_64x64_BLOCKS      2
#define BG_AFFINE_128x128_BLOCKS    3

#define TIMER_FREQ_1HZ              0
#define TIMER_FREQ_64HZ             1
#define TIMER_FREQ_256HZ            2
#define TIMER_FREQ_1024HZ           3


// Ignore this stuff. It's all just helpers so the variadic macros actually
// work
#if __STDC_VERSION__==202311L
#define __GBADEV_INTERNAL__REGFLAGS_VA_FOLD(regname, flagname_n, ...)  \
  regname##_##flagname_n##_FLAGBIT  \
    __VA_OPT__(| __GBADEV_INTERNAL__RFVAF_3ARY __GBADEV_INTERNAL__PRNTHSIS  \
        (regname, __VA_ARGS__))

#define __GBADEV_INTERNAL__REGFMASKS_VA_FOLD(regname, fieldname_n, ...)  \
  regname##_##fieldname_n##_MASK  \
    __VA_OPT__(| __GBADEV_INTERNAL__RFMVAF_3ARY __GBADEV_INTERNAL__PRNTHSIS  \
          (regname, __VA_ARGS__))

#define __GBADEV_INTERNAL__REGVALUES_VA_FOLD(regname, fieldname_n, value_n, ...)  \
  (regname##_##fieldname_n##_MASK&((value_n)<<regname##_##fieldname_n##_SHAMT))  \
   __VA_OPT__(| __GBADEV_INTERNAL__RVVAF_3ARY __GBADEV_INTERNAL__PRNTHSIS  \
        (regname,__VA_ARGS__))
#define __GBADEV_INTERNAL__IRQFLAGS_VA_FOLD(irq_n, ...)  \
  (1<<IRQ_##irq_n)  \
  __VA_OPT__(| __GBADEV_INTERNAL__IFVAF_3ARY __GBADEV_INTERNAL__PRNTHSIS  \
        (__VA_ARGS__))

#define __GBADEV_INTERNAL__KEYFLAGS_VA_FOLD(key_n, ...)  \
  KEY_##key_n  \
  __VA_OPT__(| __GBADEV_INTERNAL__KFVAF_3ARY __GBADEV_INTERNAL__PRNTHSIS  \
      (__VA_ARGS__))

#define __GBADEV_INTERNAL__DMGCHANEN_VA_FOLD(channel_n, ...)  \
  DMG_##channel_n##_ENABLE  \
  __VA_OPT__(| __GBADEV_INTERNAL__DMGCHE_3ARY __GBADEV_INTERNAL__PRNTHSIS  \
      (__VA_ARGS__))

#define __GBADEV_INTERNAL__DSCHANSET_VA_FOLD(setting_n, ...)  \
  DSOUND_##setting_n  \
  __VA_OPT__(| __GBADEV_INTERNAL__DSCS_3ARY __GBADEV_INTERNAL__PRNTHSIS  \
      (__VA_ARGS__))

#define __GBADEV_INTERNAL__IFVAF_3ARY() __GBADEV_INTERNAL__IRQFLAGS_VA_FOLD
#define __GBADEV_INTERNAL__RFVAF_3ARY() __GBADEV_INTERNAL__REGFLAGS_VA_FOLD
#define __GBADEV_INTERNAL__RFMVAF_3ARY() __GBADEV_INTERNAL__REGFMASKS_VA_FOLD
#define __GBADEV_INTERNAL__RVVAF_3ARY() __GBADEV_INTERNAL__REGVALUES_VA_FOLD
#define __GBADEV_INTERNAL__KFVAF_3ARY() __GBADEV_INTERNAL__KEYFLAGS_VA_FOLD
#define __GBADEV_INTERNAL__DMGCHE_3ARY() __GBADEV_INTERNAL__DMGCHANEN_VA_FOLD
#define __GBADEV_INTERNAL__DSCS_3ARY() __GBADEV_INTERNAL__DSCHANSET_VA_FOLD


#define __GBADEV_INTERNAL__EXP(...) __GBADEV_INTERNAL__EXP4(__GBADEV_INTERNAL__EXP4(__GBADEV_INTERNAL__EXP4(__GBADEV_INTERNAL__EXP4(__VA_ARGS__))))
#define __GBADEV_INTERNAL__EXP4(...) __GBADEV_INTERNAL__EXP3(__GBADEV_INTERNAL__EXP3(__GBADEV_INTERNAL__EXP3(__GBADEV_INTERNAL__EXP3(__VA_ARGS__))))
#define __GBADEV_INTERNAL__EXP3(...) __GBADEV_INTERNAL__EXP2(__GBADEV_INTERNAL__EXP2(__GBADEV_INTERNAL__EXP2(__GBADEV_INTERNAL__EXP2(__VA_ARGS__))))
#define __GBADEV_INTERNAL__EXP2(...) __GBADEV_INTERNAL__EXP1(__GBADEV_INTERNAL__EXP1(__GBADEV_INTERNAL__EXP1(__GBADEV_INTERNAL__EXP1(__VA_ARGS__))))
#define __GBADEV_INTERNAL__EXP1(...) __VA_ARGS__
#define __GBADEV_INTERNAL__PRNTHSIS ()

#endif  /* c23 variadic helper funcs */
#ifdef __cplusplus
}
#endif  /* C++ Name mangler guard */

#endif  /* _GBADEV_MEMDEF_H_ */
