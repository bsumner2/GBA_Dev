#include <GBAdev_types.h>
#include <GBAdev_text_engine.h>
#include <GBAdev_memdef.h>
#include <GBAdev_memmap.h>
#include <GBAdev_IRQ.h>
#include <GBAdev_functions.h>
#include <GBAdev_default_fonts_Verdana_Monochrome.h>




__attribute__ (( __weak__, __noreturn__ ))
void __assert_func (const char *file, 
                    int line,
                    const char *caller,
                    const char *expr) {
  TextEngine_Ctx_t ctx = {0};
  u16 pal_buf[2];
  ctx.current_font = &GBADev_Verdana_Monochrome_Font;
  TextEngine_RenderCallbacks_UseDefault_BMP(&ctx, pal_buf, 3, 2, 0);
  Fast_Memset32(VRAM_M3,
                0,
                sizeof(u16)*M3_SCREEN_WIDTH*M3_SCREEN_HEIGHT/sizeof(WORD));

  // Use Puts as much as possible because failure case could potentially be
  // heap issues (e.g.: fragmentation, out of mem), so in order to minimize 
  // amount of heap storage used for printing, use Puts instead of Printf,
  // wherever possible
  TextEngine_Puts("\x1c[$PAL.1=0x7FFF $PAL.0=0x001F][Error]: "
      "\x1b[37;40mAssertion expression,\x1b[46m ", &ctx);
  TextEngine_Puts(expr, &ctx);
  TextEngine_Puts("\x1b[40m, in\n\x1c[$PAL.0=0x0000]\t\t"
      "\x1c[$PAL.0=0x3E00]", &ctx);
  TextEngine_Puts(file, &ctx);
  TextEngine_Putchar('#', &ctx);
  TextEngine_Puts(caller, &ctx);
  TextEngine_Printf(&ctx, ":%d", line);
  TextEngine_Puts("%s#%s:%d\x1b[37;40m, has failed!", &ctx);
  
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
