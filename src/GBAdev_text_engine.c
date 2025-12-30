#include <GBAdev_text_engine.h>
#include <GBAdev_types.h>
#include <GBAdev_memdef.h>
#include <GBAdev_memmap.h>
#include <GBAdev_functions.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



extern IWRAM_CODE BOOL __GBADEV_INTERNAL__TextEngine_ProcessEscapeChars(
                                                         TextEngine_Ctx_t *ctx, 
                                                         Coord_t *cursor,
                                                         const char **iostr);

static TextEngine_TilemapState_t __GBADEV_INTERNAL__txt_engine_def_udata;


static int TextEngine_ProcessString(TextEngine_Ctx_t *ctx, const char *str);

static TextEngine_Font_Glyph_t *TextEngine_LookupGlyph(
                                                  TextEngine_Font_Glyph_t *dst,
                                                  const TextEngine_Font_t *font,
                                                  u32 code_pt);

TextEngine_Font_Glyph_t *TextEngine_LookupGlyph(TextEngine_Font_Glyph_t *dst,
                                                const TextEngine_Font_t *font,
                                                u32 code_pt) {
  int idx = code_pt - font->glyph0_char_code;
  if (0 > idx || font->glyph_count <= idx) {
    idx = font->errglyph_idx;
    if (font->glyph_count <= idx)
      return NULL;
  }
  dst->bpp = font->bitdepth;
  dst->cell_pitch = font->cell_pitch;
  dst->data = (const void*)
    (((const uint8_t*)font->glyph_data) + idx*font->cell_size);
  dst->width = font->glyph_widths
                        ? font->glyph_widths[idx]
                        : font->glyph_width;
  dst->height = font->glyph_heights
                        ? font->glyph_heights[idx]
                        : font->glyph_height;
  dst->max_width = font->glyph_width;
  dst->max_height = font->glyph_height;
  dst->bitpack_big_endian = font->bitpack_big_endian;
  return dst;
}

int TextEngine_ProcessString(TextEngine_Ctx_t *ctx, const char *str) {
  if (NULL == ctx || NULL == str)
    return -1;
  const TextEngine_Font_t *font = ctx->current_font;
  if (NULL == font)
    return -1;
  TextEngine_GlyphRender_cb render_cb = ctx->glyph_render_cb;
  if (NULL == render_cb)
    return -1;
  TextEngine_Font_Glyph_t glyph_info;
  Coord_t cursor = { .x = ctx->cursor_x, .y = ctx->cursor_y };
  u32 code_pt;
  int ret = 0;
  while (code_pt = *str++, 0!=code_pt) {
    if (code_pt < 32) {
      if ('\t' != code_pt) {
        if (!__GBADEV_INTERNAL__TextEngine_ProcessEscapeChars(ctx,
                                                              &cursor,
                                                              &str)) {
          ret = -1;
          break;
        }
        continue;
      }
    }
    if (NULL == TextEngine_LookupGlyph(&glyph_info, font, code_pt)) {
      ret = -1;
      break;
    }
    if (!ctx
        ->glyph_render_cb(&glyph_info,
                          &cursor,
                          ctx->pal,
                          ctx->margins,
                          ctx->userdata)) {
      ret = -1;
      break;
    }
    ++ret;
  }
  ctx->cursor_x = cursor.x;
  ctx->cursor_y = cursor.y;
  return ret;
}

BOOL TextEngine_RenderCallbacks_UseDefault_Tilemap(TextEngine_Ctx_t *ctx,
                                                   u32 bg_idx,
                                                   u32 bg_flags) {
  if (NULL == ctx)
    return FALSE; 
  if (NULL == ctx->current_font)
    return FALSE;
  // Normalize input fields to their accepted ranges of values
  bg_idx&=3;
  bg_flags &= REG_FIELD_MASK(BG_CNT, SCR_BLOCK_BASE)
            | REG_FIELD_MASK(BG_CNT, TILE_BLOCK_BASE)
            | REG_FLAG(BG_CNT, 8BPP);

  void *cbb;
  u16* scr_blocks;
  u32 scr_block_base  = REG_GET_FIELD(bg_flags, BG_CNT, SCR_BLOCK_BASE),
      tile_block_base = REG_GET_FIELD(bg_flags, BG_CNT, TILE_BLOCK_BASE),
      bitdepth = 4<<REG_GET_FLAG(bg_flags, BG_CNT, 8BPP),
      start_x = ctx->margins[TEXT_ENGINE_RENDER_MARGIN_LEFT] + ctx->cursor_x,
      start_y = ctx->margins[TEXT_ENGINE_RENDER_MARGIN_TOP] + ctx->cursor_y,
      cur_tile_idx, tile_x_ofs, tile_y_ofs;
  {
    u32 video_mode = REG_GET_FIELD(REG_DPY_CNT, DPY_CNT, MODE);
    switch (video_mode) {
    case 0:
      break;  // all bg indices are regular
    case 1:
      if (2 > bg_idx)  // In mode 1, if bg_idx is 0 or 1, then it's regular
        break;
      return FALSE;  // otherwise, it's either inactive or affine. Neither of
                     // which are supported by system.
    case 2:
      return FALSE;  // only unsupported affine bg's active in mode 2, 
                     // so return FALSE
    default:
      return FALSE;  // GBA is not in a tiling mode, so can't setup renderer
    }
  }
  if (ctx->current_font->bitdepth > bitdepth)
    return FALSE;
  if (NULL!=ctx->current_font->pal) {
    const u16 *pal = ctx->current_font->pal,
          PAL_CLR_CT = ctx->current_font->pal_color_count;
    Fast_Memcpy32(PAL_MEM_BG, pal, (PAL_CLR_CT*sizeof(u16))/sizeof(WORD));
    if (1&PAL_CLR_CT)
      PAL_MEM_BG[PAL_CLR_CT-1] = pal[PAL_CLR_CT-1];
    PAL_MEM_BG[0] = 0;  // To ensure transparency
  }
  bg_flags |= REG_VALUE(BG_CNT, BG_SIZE, BG_REGULAR_32x32_BLOCKS);


  REG_BG_OFS[bg_idx] = (Coord_t) { 256, 256 };
  switch (bg_idx) {
  case 0:
    REG_BG0_CNT = bg_flags;
    break;
  case 1:
    REG_BG1_CNT = bg_flags;
    break;
  case 2:
    REG_BG2_CNT = bg_flags;
    break;
  case 3:
    REG_BG3_CNT = bg_flags;
    break;
  default:
    return FALSE;  // This should never happen because we &=3'd bg_flags
  }
  
  scr_blocks = SCR_ENT_MEM[scr_block_base];
  for (u32 tid = 0,i,x,y=0; SCREEN_HEIGHT > y; y+=8, scr_blocks+=32)
    for (i = 0, x = 0; SCREEN_WIDTH > x; x+=8, ++i) scr_blocks[i] = (u16)tid++;

  if (4==bitdepth) {
    cbb = TILE4_MEM[tile_block_base];
  } else {
    cbb = TILE8_MEM[tile_block_base];
  }
  // clear tile mem
  {
    u32 tiles_for_screen = (SCREEN_WIDTH/8) * (SCREEN_HEIGHT/8); // 600
    u32 tile_bytes = (bitdepth == 4) ? 32 : 64;
    Fast_Memset32(cbb, 0, (tiles_for_screen * tile_bytes) / sizeof(WORD));
  }

  cur_tile_idx = (start_y/8)*(SCREEN_WIDTH/8) + start_x/8;
  tile_x_ofs = start_x&7;
  tile_y_ofs = start_y&7;
  
  __GBADEV_INTERNAL__txt_engine_def_udata = (TextEngine_TilemapState_t) {
    .surface = {
      .pitch = bitdepth,
      .pal = PAL_MEM_BG,
      .sdata = cbb,
      .width = 8,
      .height = 8,
      .type = 4==bitdepth ? TEXT_ENGINE_TEXT_SURFACE_TYPE_4BPP_TILEMAP
                          : TEXT_ENGINE_TEXT_SURFACE_TYPE_8BPP_TILEMAP,
    },
    .cur_tile_idx = cur_tile_idx,
    .tile_x_ofs = tile_x_ofs,
    .tile_y_ofs = tile_y_ofs,
  };

  ctx->userdata = (void*)&__GBADEV_INTERNAL__txt_engine_def_udata;
  ctx->glyph_render_cb = TextEngine_DefaultRenderCallback_Tilemap;
  ctx->clear_render_cb = TextEngine_DefaultClearCallback_Tilemap;
   
  REG_DPY_CNT |= REG_FLAG(DPY_CNT, BG0)<<bg_idx;
  return TRUE;
}


__attribute__ ((__format__ ( __printf__, 2, 3 ) ))
int TextEngine_Printf(TextEngine_Ctx_t *ctx, const char *__restrict fmt,
    ...) {
  if (NULL == ctx) return -1;
  if (NULL == fmt) return -1;
  char *s;
  int ret;
  {
    i32 len;
    va_list args;
    va_start(args, fmt);
    len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    if (0 > len)
      return -1;
    s = malloc(sizeof(char)*(len+1));
    if (NULL == s)
      return -1;
    va_start(args, fmt);
    vsnprintf(s, len+1, fmt, args);
    va_end(args);
    s[len] = '\0';
  }
  ret = TextEngine_ProcessString(ctx, s);
  free(s);
  return ret;
}
