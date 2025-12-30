#include <GBAdev_types.h>
#include <GBAdev_functions.h>
#include <GBAdev_memdef.h>
#include <GBAdev_text_engine.h>
#include <GBAdev_math.h>
#include "GBAdev_text_engine_private.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
typedef enum e_txt_engine_escape_seq_type {
  ESCAPE_SEQUENCE_X_COORD=0,
  ESCAPE_SEQUENCE_Y_COORD,
  ESCAPE_SEQUENCE_PAIR_COORD,
  ESCAPE_SEQUENCE_PAL_SET,
  ESCAPE_SEQUENCE_FG_SET,
  ESCAPE_SEQUENCE_BG_SET,
  ESCAPE_SEQUENCE_INVALID
} TextEngine_EscapeSequence_Type_e;


IWRAM_CODE BOOL __GBADEV_INTERNAL__TextEngine_ProcessEscapeChars(
                                                      TextEngine_Ctx_t *ctx,
                                                      Coord_t *cursor,
                                                      const char **iostr) {
  const char *str = *iostr, *tmp_strp;
  char *endp;
  u32 cur, arg;
  i32 bracket_pda = 0;
  BOOL brackets_encountered;
  if ('[' != *str++)
    return FALSE;

  cur = *str++;
  do {
    if ('$'!=cur)
      return FALSE;
    bracket_pda = 0;
    brackets_encountered = FALSE;
    for (tmp_strp=str, cur=*tmp_strp++; '\0'!=cur && '='!=cur; cur=*tmp_strp++)
      switch (cur) {
      case '[':
        ++bracket_pda;
        brackets_encountered = TRUE;
        continue;
      case ']':
        if (0 == bracket_pda)
          return FALSE;
        --bracket_pda;
        continue;
      default:
        continue;
      }
    if ('=' != cur)
      return FALSE;
    if (0!=bracket_pda)
      return FALSE;
    if (brackets_encountered) {
      if (!isdigit(*tmp_strp))
        return FALSE;
      if (0!=strncmp(str, "pal[", LSTRLEN("pal[")))
        return FALSE;
      str+=LSTRLEN("pal[");
      if ('['!=*str++)
        return FALSE;
      cur = *str;
      if (!isdigit(cur))
        return FALSE;
      if ('0' == cur) {
        cur = *++str;
        if (']' == cur) {
          cur = 0;
        } else if ('x' == cur) {
          cur = 16;
          ++str;
        } else {
          while ('0'==cur)
            cur = *++str;
          if (!isdigit(cur))
            return FALSE;
          if ('8' == cur || '9' == cur)
            return FALSE;
          cur = 8;
        }
      } else {
        cur = 10;
      }
      if (0!=cur) {
        cur = strtoul(str, &endp, cur);
        if (NULL == endp || endp == str) {
          return FALSE;
        }
        str = endp;
        if (*str != ']')
          return FALSE;
      }
      if ('0' == *tmp_strp) {
        if ('x' == *++tmp_strp) {
          ++tmp_strp;
          arg = strtoul(tmp_strp, &endp, 16);
        } else {
          for (arg = *tmp_strp; '0'==arg; arg = *++tmp_strp) continue;
          if (!isdigit(arg))
            return FALSE;
          if ('8' == arg || '9' == arg)
            return FALSE;
          arg = strtoul(tmp_strp, &endp, 8);
        }
      } else {
        arg = strtoul(tmp_strp, &endp, 10);
      }
      if (NULL == endp || endp == tmp_strp)
        return FALSE;
      ctx->pal[cur] = arg;
      str = endp;
      cur = *str++;
      continue;
    }
    cur = *str++;
    if ('x' <= cur) {
      if ('=' == *str) {
        switch (cur) {
        case 'x':
          arg = ESCAPE_SEQUENCE_X_COORD;
          break;
        case 'y':
          arg = ESCAPE_SEQUENCE_Y_COORD;
          break;
        default:
          return FALSE;
        }
      } else {
        if ('x' != cur)
          return FALSE;
        if (0!=strncmp(str, "y=", LSTRLEN("y=")))
          return FALSE;
        arg = ESCAPE_SEQUENCE_PAIR_COORD;
      }
    } else if ('g'==*str) {
      switch (cur) {
      case 'f':
        arg = ESCAPE_SEQUENCE_FG_SET;
        break;
      case 'b':
        arg = ESCAPE_SEQUENCE_BG_SET;
        break;
      default:
        return FALSE;
      }
    } else {
      return FALSE;
    }
    if ('0' == *tmp_strp) {
      if ('x' == *++tmp_strp) {
        ++tmp_strp;
        cur = strtoul(tmp_strp, &endp, 16);
      } else {
        for (cur = *tmp_strp; '0'==cur; cur = *++tmp_strp) continue;
        if (!isdigit(cur))
          return FALSE;
        if ('8' == cur || '9' == cur)
          return FALSE;
        cur = strtoul(tmp_strp, &endp, 8);
      }
    } else {
      cur = strtoul(tmp_strp, &endp, 10);
    }
    if (NULL == endp || endp == tmp_strp)
      return FALSE;
    str = endp;
    switch (arg) {
    case ESCAPE_SEQUENCE_X_COORD:
      ctx->cursor_x = cur;
      cur = *str++;
      continue;
    case ESCAPE_SEQUENCE_Y_COORD:
      ctx->cursor_y = cur;
      cur = *str++;
      continue;
    case ESCAPE_SEQUENCE_FG_SET:
      ctx->pal[1] = cur;
      cur = *str++;
      continue;
    case ESCAPE_SEQUENCE_BG_SET:
      ctx->pal[0] = cur;
      cur = *str++;
      continue;
    case ESCAPE_SEQUENCE_PAIR_COORD:
      ctx->cursor_x = cur;
      break;
    }
    tmp_strp = endp;
    if ('0' == *tmp_strp) {
      if ('x' == *++tmp_strp) {
        ++tmp_strp;
        cur = strtoul(tmp_strp, &endp, 16);
      } else {
        for (cur = *tmp_strp; '0'==cur; cur = *++tmp_strp) continue;
        if (!isdigit(cur))
          return FALSE;
        if ('8' == cur || '9' == cur)
          return FALSE;
        cur = strtoul(tmp_strp, &endp, 8);
      }
    } else {
      cur = strtoul(tmp_strp, &endp, 10);
    }
    if (NULL == endp || endp == tmp_strp)
      return FALSE;
    ctx->cursor_y = cur;
    str = endp;
    cur = *str++;

  } while ('\0' != cur && ']' != cur);
  
  return ']' == cur;
}


#ifdef __DEFAULT_CALLBACK_TILEMAP_ASM_VERSIONS_UNIMPLEMENTED__


#if 0  // Block off old implementation that failed as I forgot about how
       // MEM_VRAM databus doesn't allow byte-level write ops. Trying to write
       // ((u8*)MEM_VRAM)[0] = (u8)foo;
       // results in the following effectively happening to the memory:
       // ((u16*)MEM_VRAM)[0] = (u16)foo|((u16)foo<<8)
       // Therefore, the solution is to cast dst to u16 and adjust all pointer
       // indexes to accommodate new ptr type, which is now 2 bytes per index
       // Fixed implementation is in the #else preprocessor block
IWRAM_CODE BOOL TextEngine_DefaultRenderCallback_Tilemap(
                                    const TextEngine_Font_Glyph_t *glyph_info,
                                    Coord_t *cursor,
                                    u16 *pal,
                                    const u16 *margins,
                                    void *userdata) {
  TextEngine_TilemapState_t *state = (TextEngine_TilemapState_t*)userdata;
  const u32 
    LFT = margins[TEXT_ENGINE_RENDER_MARGIN_LEFT],
    TOP = margins[TEXT_ENGINE_RENDER_MARGIN_TOP],
    RGT = margins[TEXT_ENGINE_RENDER_MARGIN_RIGHT],
    BTM = margins[TEXT_ENGINE_RENDER_MARGIN_BOTTOM],
    TXTBOX_W = RGT - LFT,
    TXTBOX_H = BTM - TOP,
    GW = glyph_info->width, 
    GH = glyph_info->height,
    GPITCH = glyph_info->cell_pitch;
  
  if ((cursor->x + GW) > TXTBOX_W) {
    u32 start_y;
    cursor->x = 0;
    cursor->y += glyph_info->max_height;
    start_y = cursor->y + TOP;
    if ((cursor->y + GH) > TXTBOX_H)
      return FALSE;
    state->cur_tile_idx = TilemapState_CalculateCurTileIdx(LFT, start_y);
    state->tile_x_ofs = LFT&7, state->tile_y_ofs = start_y&7;
  } else if ((cursor->y + GH) > TXTBOX_H) {
    return FALSE;
  }
  const u8 *src = (const u8*)glyph_info->data;
  u8 *basetile, *dst = (u8*)state->surface.sdata;
  u32 src_y, src_x, dst_x = state->tile_x_ofs, dst_x_base = state->tile_x_ofs,
      dst_y = state->tile_y_ofs;
  u32 tile_sz, cur, tile_pitch;
  BOOL is_8bpp = 0!=(state->surface.type&TEXT_ENGINE_TEXT_SURFACE_INDEXED256),
       big_endian = glyph_info->bitpack_big_endian;
  tile_sz = is_8bpp ? 64 : 32;
  if (is_8bpp)
    dst += state->cur_tile_idx<<6, tile_pitch = 8;
  else
    dst += state->cur_tile_idx<<5, tile_pitch = 4;
  basetile = dst;
  for (src_y = 0; GH > src_y; ++src_y, src+=GPITCH) {
    dst = basetile + dst_y*tile_pitch;
    for (dst_x = dst_x_base, src_x = 0; GW > src_x; ++src_x) {
      if (is_8bpp) {
        if (8 == glyph_info->bpp) {
          dst[dst_x++] = src[src_x];
        } else if (4 == glyph_info->bpp) {
          if ((src_x&1)^big_endian)
              dst[dst_x++] = (src[src_x>>1]>>4)&0x0F;
          else
            dst[dst_x++] = src[src_x>>1]&0x0F;
        } else {
          if (big_endian)
            dst[dst_x++] = src[src_x/8]&(0x80>>(src_x&7)) ? 1 : 0;
          else
            dst[dst_x++] = src[src_x/8]&(1<<(src_x&7)) ? 1 : 0;
        }
      } else if (dst_x&1) {
        if (4==glyph_info->bpp) {
          if (big_endian^(src_x&1))
            dst[dst_x++>>1] |= src[src_x>>1]&0xF0;
          else
            dst[dst_x++>>1] |= (src[src_x>>1]<<4)&0xF0;
        } else {
          dst[dst_x++>>1] |= (src[src_x>>3]&(big_endian
                                    ?0x80>>(src_x&7)
                                    :1<<(src_x&7))) ? 0x10 : 0;
        }
      } else {
        if (4==glyph_info->bpp) {
          if (big_endian^(src_x&1))
            dst[dst_x++>>1] = (src[src_x>>1]>>4)&0x0F;
          else
            dst[dst_x++>>1] = src[src_x>>1]&0x0F;
        } else {
          dst[dst_x++>>1] = (src[src_x>>3]&(big_endian
                                    ?0x80>>(src_x&7)
                                    :1<<(src_x&7))) ? 1 : 0;
        }
      }
      if (8>dst_x)
        continue;
      dst_x = 0;
      dst = basetile+tile_sz+dst_y*tile_pitch;
    }
    ++dst_y;
    if (8>dst_y)
      continue;
    dst_y = 0;
    basetile += is_8bpp ? 30*64 : 30*32;
  }
  cursor->x += GW;
  state->tile_x_ofs = (LFT+cursor->x)&7;
  state->cur_tile_idx = TilemapState_CalculateCurTileIdx(LFT+cursor->x,
                                                         TOP+cursor->y);

  return TRUE;
}
#else  // Fixed impl below:

IWRAM_CODE BOOL TextEngine_DefaultRenderCallback_Tilemap(
                                    const TextEngine_Font_Glyph_t *glyph_info,
                                    Coord_t *cursor,
                                    u16 *pal,
                                    const u16 *margins,
                                    void *userdata) {
  TextEngine_TilemapState_t *state = (TextEngine_TilemapState_t*)userdata;
  const u32 
    LFT = margins[TEXT_ENGINE_RENDER_MARGIN_LEFT],
    TOP = margins[TEXT_ENGINE_RENDER_MARGIN_TOP],
    RGT = margins[TEXT_ENGINE_RENDER_MARGIN_RIGHT],
    BTM = margins[TEXT_ENGINE_RENDER_MARGIN_BOTTOM],
    TXTBOX_W = RGT - LFT,
    TXTBOX_H = BTM - TOP,
    GW = glyph_info->width, 
    GH = glyph_info->height,
    GPITCH = glyph_info->cell_pitch;
  u32 start_x = cursor->x+LFT, start_y = cursor->y+TOP;
  if ((cursor->x + GW) > TXTBOX_W) {
    cursor->x = 0;
    cursor->y += glyph_info->max_height;
    start_x = LFT;
    start_y = cursor->y + TOP;
    if ((cursor->y + GH) > TXTBOX_H)
      return FALSE;
    state->cur_tile_idx = TilemapState_CalculateCurTileIdx(LFT, start_y);
    state->tile_x_ofs = LFT&7;
    state->tile_y_ofs = start_y&7;
  } else if ((cursor->y + GH) > TXTBOX_H) {
    return FALSE;
  } else {
    state->cur_tile_idx = TilemapState_CalculateCurTileIdx(start_x, start_y);
    state->tile_x_ofs = start_x&7;
    state->tile_y_ofs = start_y&7;
  }
  const u8 *src = (const u8*)glyph_info->data;
  u8 *basetile, *dst = (u8*)state->surface.sdata;
  u32 src_y, src_x, dst_x_base = state->tile_x_ofs,
      dst_y = state->tile_y_ofs, dst_x;
  u32 tile_sz, tile_pitch;
  BOOL is_8bpp = 0!=(state->surface.type&TEXT_ENGINE_TEXT_SURFACE_INDEXED256),
       big_endian = glyph_info->bitpack_big_endian;
  tile_sz = is_8bpp ? 64 : 32;
  if (is_8bpp)
    dst += state->cur_tile_idx<<6, tile_pitch = 8;
  else
    dst += state->cur_tile_idx<<5, tile_pitch = 4;
  basetile = dst;
  for (src_y = 0; GH > src_y; ++src_y, src+=GPITCH) {
    dst = basetile + dst_y*tile_pitch;
    for (dst_x = dst_x_base, src_x = 0; GW > src_x; ++src_x) {
      if (is_8bpp) {
        if (8 == glyph_info->bpp) {
//          dst[dst_x++] = src[src_x];
          if (dst_x&1) {
            ((u16*)dst)[dst_x>>1] &= 0x00FF;
            ((u16*)dst)[dst_x++>>1] |= src[src_x]<<8;
          } else {
            ((u16*)dst)[dst_x>>1] &= 0xFF00;
            ((u16*)dst)[dst_x++>>1] |= src[src_x];
          }
        } else if (4 == glyph_info->bpp) {
          if ((src_x&1)^big_endian) {
            if (dst_x&1) {
              ((u16*)dst)[dst_x>>1] &= 0x00FF;
              ((u16*)dst)[dst_x++>>1] |= (src[src_x>>1]<<4)&0x0F00;
            } else {
              ((u16*)dst)[dst_x>>1] &= 0xFF00;
              ((u16*)dst)[dst_x++>>1] |= (src[src_x>>1]>>4)&0x000F;
            }
          } else {
            if (dst_x&1) {
              ((u16*)dst)[dst_x>>1] &= 0x00FF;
              ((u16*)dst)[dst_x++>>1] |= (src[src_x>>1]<<8)&0x0F00;
            } else {
              ((u16*)dst)[dst_x>>1] &= 0xFF00;
              ((u16*)dst)[dst_x++>>1] |= src[src_x>>1]&0x000F;
            }
          }
        } else {
          if (big_endian) {
            if (dst_x&1) {
              ((u16*)dst)[dst_x>>1] &= 0x00FF;
              ((u16*)dst)[dst_x++>>1] |= src[src_x/8]&(0x80>>(src_x&7)) 
                                                ? 0x0100 : 0;
            } else {
              ((u16*)dst)[dst_x>>1] &= 0xFF00;
              ((u16*)dst)[dst_x++>>1] |= src[src_x/8]&(0x80>>(src_x&7)) 
                                                ? 0x0001 : 0;
            }
          } else {
            if (dst_x&1) {
              ((u16*)dst)[dst_x>>1] &= 0x00FF;
              ((u16*)dst)[dst_x++>>1] |= src[src_x/8]&(1<<(src_x&7)) 
                                                ? 0x0100 : 0;
            } else {
              ((u16*)dst)[dst_x>>1] &= 0xFF00;
              ((u16*)dst)[dst_x++>>1] |= src[src_x/8]&(1<<(src_x&7)) 
                                                ? 0x0001 : 0;
            }
          }
        }
      } else if (dst_x&1) {
        if (4==glyph_info->bpp) {
          if (big_endian^(src_x&1)) {
            if (dst_x&2) {
              ((u16*)dst)[dst_x>>2] &= 0x0FFF;
              ((u16*)dst)[dst_x++>>2] |= (src[src_x>>1]<<8)&0xF000;
            } else {
              ((u16*)dst)[dst_x>>2] &= 0xFF0F;
              ((u16*)dst)[dst_x++>>2] |= src[src_x>>1]&0x00F0;

            }
          } else {
            if (dst_x&2) {
              ((u16*)dst)[dst_x>>2] &= 0x0FFF;
              ((u16*)dst)[dst_x++>>2] |= (src[src_x>>1]<<12)&0xF000;
            } else {
              ((u16*)dst)[dst_x>>2] &= 0xFF0F;
              ((u16*)dst)[dst_x++>>2] |= (src[src_x>>1]<<4)&0x00F0;
            }
          }
        } else {
          if (dst_x&2) {
            ((u16*)dst)[dst_x>>2] &= 0x0FFF;
            ((u16*)dst)[dst_x++>>2] |= (src[src_x>>3]&(big_endian
                                            ?0x80>>(src_x&7)
                                            :1<<(src_x&7))) ? 0x1000 : 0;
          } else {
            ((u16*)dst)[dst_x>>2] &= 0xFF0F;
            ((u16*)dst)[dst_x++>>2] |= (src[src_x>>3]&(big_endian
                                            ?0x80>>(src_x&7)
                                            :1<<(src_x&7))) ? 0x0010 : 0;
          }

        }
      } else {
        if (4==glyph_info->bpp) {
          if (big_endian^(src_x&1)) {
            if (dst_x&2) {
              ((u16*)dst)[dst_x>>2] &= 0xF0FF;
              ((u16*)dst)[dst_x++>>2] |= (src[src_x>>1]<<4)&0x0F00;
            } else {
              ((u16*)dst)[dst_x>>2] &= 0xFFF0;
              ((u16*)dst)[dst_x++>>2] |= (src[src_x>>1]>>4)&0x000F;
            }
          } else {
            if (dst_x&2) {
              ((u16*)dst)[dst_x>>2] &= 0xF0FF;
              ((u16*)dst)[dst_x++>>2] |= (src[src_x>>1]<<8)&0x0F00;
            } else {
              ((u16*)dst)[dst_x>>2] &= 0xFFF0;
              ((u16*)dst)[dst_x++>>2] |= src[src_x>>1]&0x000F;
            }
          }
        } else {
          if (dst_x&2) {
            ((u16*)dst)[dst_x>>2] &= 0xF0FF;
            ((u16*)dst)[dst_x++>>2] |= (src[src_x>>3]&(big_endian
                                                ?0x80>>(src_x&7)
                                                :1<<(src_x&7))) ? 0x0100 : 0;
          } else {
            ((u16*)dst)[dst_x>>2] &= 0xFFF0;
            ((u16*)dst)[dst_x++>>2] |= (src[src_x>>3]&(big_endian
                                              ?0x80>>(src_x&7)
                                              :1<<(src_x&7))) ? 0x0001 : 0;
          }
        }
      }
      if (8>dst_x)
        continue;
      dst_x = 0;
      dst = basetile+tile_sz+dst_y*tile_pitch;
    }
    ++dst_y;
    if (8>dst_y)
      continue;
    dst_y = 0;
    basetile += is_8bpp ? 30*64 : 30*32;
  }
  cursor->x += GW;
  state->tile_x_ofs = (LFT+cursor->x)&7;
  state->cur_tile_idx = TilemapState_CalculateCurTileIdx(LFT+cursor->x,
                                                         TOP+cursor->y);

  return TRUE;
}

#endif  /* tmp block for fixed Tilemap Render CB's fixed impl */

IWRAM_CODE void TextEngine_DefaultClearCallback_Tilemap(const Rect_t *area,
                                                        void *userdata) {
  TextEngine_TilemapState_t *state = (TextEngine_TilemapState_t*)userdata;
  u8 *basetile = state->surface.sdata, *curtile;
  BOOL is_8bpp;
  is_8bpp = 0!=(state->surface.type&TEXT_ENGINE_TEXT_SURFACE_INDEXED256);
  if (is_8bpp)
    basetile += TilemapState_CalculateCurTileIdx(area->x, area->y)<<6;
  else
   basetile += TilemapState_CalculateCurTileIdx(area->x, area->y)<<5;
  const u32 W = area->w, H = area->h, TSZ = is_8bpp ? 64 : 32,
        TPITCH = is_8bpp ? 8 : 4;
  u32 dst_x_base = area->x&7, dst_y = area->y&7, dst_x, rx=0, ry=0;
  for (ry = 0; H > ry; ++ry) {
    curtile = basetile+dst_y*TPITCH;
    for (dst_x = dst_x_base, rx = 0; W > rx; ++rx) {
      if (is_8bpp) {
        if (dst_x&1) {
          ((u16*)curtile)[dst_x++>>1] &= 0x00FF;
        } else {
          ((u16*)curtile)[dst_x++>>1] &= 0xFF00;
        }
      } else if (dst_x&2) {
        if (dst_x&1) {
          ((u16*)curtile)[dst_x++>>2] &= 0x0FFF;
        } else {
          ((u16*)curtile)[dst_x++>>2] &= 0xF0FF;
        }
      } else {
        if (dst_x&1) {
          ((u16*)curtile)[dst_x++>>2] &= 0xFF0F;
        } else {
          ((u16*)curtile)[dst_x++>>2] &= 0xFFF0;
        }
      }
      if (8 > dst_x)
        continue;
      dst_x = 0;
      curtile = basetile+TSZ+dst_y*TPITCH;
    }
    if (8>++dst_y)
      continue;
    dst_y = 0;
    basetile += is_8bpp ? 30*64 : 30*32;
  }
}


#endif  /* Until we've created viable ASM versions of the default render and 
         * clear callbacks for tilemap, we use their C-written counterparts. */


#ifdef __DEFAULT_CALLBACK_MODE3_ASM_VERSIONS_UNIMPLEMENTED__


// Stub
IWRAM_CODE BOOL TextEngine_DefaultRenderCallback_Mode3(const TextEngine_Font_Glyph_t*,
                                                       Coord_t*,
                                                       u16*,
                                                       const u16*,
                                                       void*) {
  return TRUE;
}

// Stub
IWRAM_CODE void TextEngine_DefaultClearCallback_Mode3(const Rect_t*,
                                                      void*) {
  return;
}


#endif  /* Until we've created viable ASM versions of the default render and 
         * clear callbacks for Mode3, we use their C-written counterparts. */
