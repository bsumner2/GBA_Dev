#include "GBAdev_util_macros.h"
#include <GBAdev_stdio_portable.h>
#include <GBAdev_text_engine.h>
#include <GBAdev_memdef.h>
#include <stdio.h>
#include <sys/unistd.h>

extern IWRAM_CODE BOOL __GBADEV_INTERNAL__TextEngine_ProcessEscapeChars(
                                                         TextEngine_Ctx_t *ctx, 
                                                         Coord_t *cursor,
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


ssize_t __GBADEV_INTERNAL__TextEngine_StdioPortable_ProcStream(
                                                          struct _reent *reent,
                                                          void *fd,
                                                          const char *text,
                                                          size_t len) {
  TextEngine_Font_Glyph_t glyph_info;
  TextEngine_GlyphRender_cb ren_cb;
  TextEngine_Font_t *font;
  TextEngine_Ctx_t *ctx = NULL;
  const char *const BEGINP = text, *const ENDP = &text[len];
  Coord_t cursor;
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

  cursor = (Coord_t) { .x = ctx->cursor_x, .y = ctx->cursor_y };
  font = ctx->current_font;
  ren_cb = ctx->glyph_render_cb;
  if (NULL == font)
    return -1;
  if (NULL == ren_cb)
    return -1;

  while (c=*text++, '\0'!=c && text < ENDP) {
    if (c < font->glyph0_char_code) {
      if ('\x1b' == c) {
        if (!__GBADEV_INTERNAL__TextEngine_ProcessEscapeChars(ctx, 
                                                              &cursor, 
                                                              &text)) {
          err_occurred = TRUE;
          break;
        }
        continue;
      } else if ('\n' == c) {
        cursor.y += font->glyph_height;
        cursor.x = 0;
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
                      &cursor,
                      ctx->pal,
                      ctx->margins,
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
                &cursor,
                ctx->pal,
                ctx->margins,
                ctx->userdata)) {
      err_occurred = TRUE;
      break;
    }
  }

  ctx->cursor_x = cursor.x;
  ctx->cursor_y = cursor.y;
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
