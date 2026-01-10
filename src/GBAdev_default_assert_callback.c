#include <GBAdev_types.h>
#include <GBAdev_text_engine.h>
#include <GBAdev_memdef.h>
#include <GBAdev_memmap.h>
#include <GBAdev_IRQ.h>
#include <GBAdev_functions.h>
#include "GBAdev_deffont_verdana_priv.h"




__attribute__ (( __weak__, __noreturn__ ))
void __assert_func (const char *file, 
                    int line,
                    const char *caller,
                    const char *expr) {
  TextEngine_Ctx_t ctx = {0};
  const TextEngine_Font_t verdana = {
    .glyph_data = __GBADEV_INTERNAL__Verdana_Glyph_Data,
    .pal = __GBADEV_INTERNAL__Verdana_Pal,
    .glyph_widths = __GBADEV_INTERNAL__Verdana_Glyph_Widths,
    .glyph_heights = (void*)0,
    .glyph0_char_code = ' ',
    .errglyph_idx = 0,
    .glyph_count = __GBADEV_INTERNAL__Verdana_Glyph_Count,
    .pal_color_count = __GBADEV_INTERNAL__Verdana_Pal_Color_Count,
    .glyph_width = __GBADEV_INTERNAL__Verdana_Glyph_Cell_Width,
    .glyph_height = __GBADEV_INTERNAL__Verdana_Glyph_Height,
    .cell_width = __GBADEV_INTERNAL__Verdana_Glyph_Cell_Width,
    .cell_height = __GBADEV_INTERNAL__Verdana_Glyph_Cell_Height,
    .cell_size = __GBADEV_INTERNAL__Verdana_Glyph_Cell_Size,
    .cell_pitch = __GBADEV_INTERNAL__Verdana_Glyph_Cell_Pitch,
    .bitdepth = __GBADEV_INTERNAL__Verdana_Glyph_Bpp,
    .bitpack_big_endian = __GBADEV_INTERNAL__Verdana_BitPack_BigEndian
  };
  u16 pal_buf[2];
  ctx.current_font = &verdana;
  TextEngine_RenderCallbacks_UseDefault_BMP(&ctx, pal_buf, 3, 2, 0);
  Fast_Memset32(VRAM_M3,
                0,
                sizeof(u16)*M3_SCREEN_WIDTH*M3_SCREEN_HEIGHT/sizeof(WORD));
  TextEngine_Puts("\x1c[$PAL.1=0x7FFF $PAL.0=0x001F][Error]: "
      "\x1b[37;40mAssertion expression,\x1b[46m ", &ctx);
  TextEngine_Printf(&ctx,
      "%s\x1b[40m, in\n\x1c[$PAL.0=0x0000]\t\t"
      "\x1c[$PAL.0=0x3E00]%s#%s:%d\x1b[37;40m, "
      "has failed!",
      expr, file, caller, line);
  if (0==REG_IME) {
    REG_IE |= IRQ_FLAG(VBLANK);
    REG_ISR_MAIN = ISR_Handler_Basic;
    REG_IME = 1;
  } else {
    REG_IME = 0;
    REG_IE &= IRQ_FLAG(VBLANK);
    if (0==(REG_IE&IRQ_FLAG(VBLANK)))
      REG_IE |= IRQ_FLAG(VBLANK);
    REG_ISR_MAIN = ISR_Handler_Basic;
    REG_IME = 1;
  }
  do SUPERVISOR_CALL(0x05); while (1);
}
