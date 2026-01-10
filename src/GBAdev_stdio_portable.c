#include "GBAdev_util_macros.h"
#include <GBAdev_stdio_portable.h>
#include <GBAdev_text_engine.h>
#include <GBAdev_memdef.h>
#include <stdio.h>
#include <sys/unistd.h>

extern BOOL __GBADEV_INTERNAL__TextEngine_ProcessEscapeChars_ANSI(
                                                         TextEngine_Ctx_t *ctx, 
                                                         const char **iostr);

extern BOOL __GBADEV_INTERNAL__TextEngine_ProcessEscapeChars_GBADev(
                                                         TextEngine_Ctx_t *ctx, 
                                                         const char **iostr);

extern TextEngine_Font_Glyph_t *__GBADEV_INTERNAL__TextEngine_LookupGlyph(
                                                TextEngine_Font_Glyph_t *dst,
                                                const TextEngine_Font_t *font,
                                                u32 code_pt);




static IWRAM_BSS devoptab_t 
                  __GBADEV_INTERNAL__TextEngine_STDOUT_DevOpTab = {0},
                  __GBADEV_INTERNAL__TextEngine_STDERR_DevOpTab = {0};

static IWRAM_BSS TextEngine_Ctx_t 
              *__GBADEV_INTERNAL__TextEngine_STDOUT_CtxP = NULL,
              *__GBADEV_INTERNAL__TextEngine_STDERR_CtxP = NULL;

                      

static const char 
                *__GBADEV_INTERNAL__TextEngine_STDOUT_DevOpTab_NameStr
                         = "INTERNAL__TextEngine_STDOUT_DevOpTab",
                *__GBADEV_INTERNAL__TextEngine_STDERR_DevOpTab_NameStr
                         = "INTERNAL__TextEngine_STDERR_DevOpTab";

ssize_t __GBADEV_INTERNAL__TextEngine_StdioPortable_ProcStream(
                                                          struct _reent *reent,
                                                          void *fd,
                                                          const char *text,
                                                          size_t len);

static ssize_t __GBADEV_INTERNAL__TextEngine_StdioPortable_ProcStreamStdout(
                                                          struct _reent *reent,
                                                          void *fd,
                                                          const char *text,
                                                          size_t len);




static ssize_t __GBADEV_INTERNAL__TextEngine_StdioPortable_ProcStreamStderr(
                                                          struct _reent *reent,
                                                          void *fd,
                                                          const char *text,
                                                          size_t len);
#define EXPAND_THEN_STRINGIFY(macro) #macro
#define ASM_TAIL_CALL_ARG_SETUP(filedes)  \
  __asm volatile ("MOV r1, #" EXPAND_THEN_STRINGIFY(filedes))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
__attribute__ (( __naked__ ))
ssize_t __GBADEV_INTERNAL__TextEngine_StdioPortable_ProcStreamStderr(
                                           struct _reent *reent,
                                           void *fd,
                                           const char *text,
                                           size_t len) {
  ASM_TAIL_CALL_ARG_SETUP(STDERR_FILENO);
  ASM("B __GBADEV_INTERNAL__TextEngine_StdioPortable_ProcStream");
}

__attribute__ (( __naked__ ))
ssize_t __GBADEV_INTERNAL__TextEngine_StdioPortable_ProcStreamStdout(
                                           struct _reent *reent,
                                           void *fd,
                                           const char *text,
                                           size_t len) {
  ASM_TAIL_CALL_ARG_SETUP(STDOUT_FILENO);
  ASM("B __GBADEV_INTERNAL__TextEngine_StdioPortable_ProcStream");
}
#pragma GCC diagnostic pop


ssize_t __GBADEV_INTERNAL__TextEngine_StdioPortable_ProcStream(
                                                          struct _reent *reent,
                                                          void *fd,
                                                          const char *text,
                                                          size_t len) {
  TextEngine_Font_Glyph_t glyph_info; 
  TextEngine_GlyphRender_cb ren_cb;
  const TextEngine_Font_t *font;
  TextEngine_Ctx_t *ctx = NULL;
  const char *const BEGINP = text, *const ENDP = &text[len];
  Coord_t *cursor;
  int c;
  BOOL err_occurred = FALSE;
  if (NULL == text || len <= 0 || NULL == reent)
    return -1;
  switch ((int)fd) {
  case STDOUT_FILENO:
    ctx = __GBADEV_INTERNAL__TextEngine_STDOUT_CtxP;
    break;
  case STDERR_FILENO:
    ctx = __GBADEV_INTERNAL__TextEngine_STDERR_CtxP;
    break;
  default:
    return -1;
  }
  cursor = &ctx->surface.cursor;
  font = ctx->current_font;
  ren_cb = ctx->glyph_render_cb;
  if (NULL == font)
    return -1;
  glyph_info = (TextEngine_Font_Glyph_t) {
    .data = NULL,
    .width = font->glyph_width,
    .height = font->glyph_height,
    .max_width = font->glyph_width,
    .max_height = font->glyph_height,
    .cell_pitch = font->cell_pitch,
    .bpp = font->bitdepth,
    .bitpack_big_endian = font->bitpack_big_endian
  };

  if (NULL == ren_cb)
    return -1;

  for (c=*text; '\0'!=c && text < ENDP; c=*++text) {
    if (c < font->glyph0_char_code) {
      if ((unsigned)(c-'\x1b') <= 1) {
        ++text;
        switch (c) {
        case '\x1b':
          if (!__GBADEV_INTERNAL__TextEngine_ProcessEscapeChars_ANSI(ctx, 
                                                                     &text)) {
            break;
          }
          continue;
        case '\x1c':
          if (!__GBADEV_INTERNAL__TextEngine_ProcessEscapeChars_GBADev(ctx,
                                                                       &text)) {
            break;
          }
          continue;
        default:
          break;
        }
        // If we broke switch without continuing, then failure case
        err_occurred = TRUE;
        break;
      } else if ('\n' == c) {
        cursor->y += font->glyph_height;
        cursor->x = 0;
        continue;
      } else if ('\t' == c) {
        if (NULL == __GBADEV_INTERNAL__TextEngine_LookupGlyph(&glyph_info,
                                                              font,
                                                              ' ')) {
          err_occurred = TRUE;
          break;
        }
        for (c = 0; 4>c; ++c) {
          if (!ren_cb(&glyph_info, 
                      &ctx->surface,
                      ctx->userdata)) {
            err_occurred = TRUE;
            break;
          }
        }
        if (err_occurred)
          break;
        else
         continue;
      } else {
        err_occurred = TRUE;
        break;
      }
    }
    if (NULL == __GBADEV_INTERNAL__TextEngine_LookupGlyph(&glyph_info,
                                                          font,
                                                          c)) {
      err_occurred = TRUE;
      break;
    }
    if (!ren_cb(&glyph_info,
                &ctx->surface,
                ctx->userdata)) {
      err_occurred = TRUE;
      break;
    }
  }

  if (err_occurred)
    return -1;
  else
   return text - BEGINP;
  


}

BOOL TextEngine_StdioPortable_PipeOutstreamToTextCtx(int fd,
                                                     TextEngine_Ctx_t *ctx) {


  devoptab_t *dst_tab;
  if (NULL == ctx)
    return FALSE;
  if (NULL == ctx->current_font)
    return FALSE;
  if (NULL == ctx->glyph_render_cb)
    return FALSE;
  if (NULL == ctx->clear_render_cb)
    return FALSE;
  switch (fd) {
  case STDOUT_FILENO:
    dst_tab = &__GBADEV_INTERNAL__TextEngine_STDOUT_DevOpTab;
    dst_tab->name = __GBADEV_INTERNAL__TextEngine_STDOUT_DevOpTab_NameStr;
    dst_tab->write_r 
      = __GBADEV_INTERNAL__TextEngine_StdioPortable_ProcStreamStdout;
    __GBADEV_INTERNAL__TextEngine_STDOUT_CtxP = ctx;
    devoptab_list[STD_OUT] = dst_tab;
    break;
  case STDERR_FILENO:
    dst_tab = &__GBADEV_INTERNAL__TextEngine_STDERR_DevOpTab;
    dst_tab->name = __GBADEV_INTERNAL__TextEngine_STDERR_DevOpTab_NameStr;
    dst_tab->write_r
      = __GBADEV_INTERNAL__TextEngine_StdioPortable_ProcStreamStderr;
    __GBADEV_INTERNAL__TextEngine_STDERR_CtxP = ctx;
    devoptab_list[STD_ERR] = dst_tab;
    break;
  default:
    return FALSE;
  }
  setvbuf(stderr, NULL, _IONBF, 0);
  return TRUE;
}
