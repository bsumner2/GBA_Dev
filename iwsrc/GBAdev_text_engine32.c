#include <GBAdev_types.h>
#include <GBAdev_functions.h>
#include <GBAdev_memdef.h>
#include <GBAdev_text_engine.h>
#include <GBAdev_math.h>
#include "GBAdev_text_engine_private.h"
#include <sys/cdefs.h>
typedef enum e_txt_engine_escape_seq_type {
  ESCAPE_SEQUENCE_X_COORD=0,
  ESCAPE_SEQUENCE_Y_COORD,
  ESCAPE_SEQUENCE_PAIR_COORD,
  ESCAPE_SEQUENCE_PAL_SET,
  ESCAPE_SEQUENCE_FG_SET,
  ESCAPE_SEQUENCE_BG_SET,
  ESCAPE_SEQUENCE_INVALID
} TextEngine_EscapeSequence_Type_e;



#ifdef __DEFAULT_CALLBACK_TILEMAP_ASM_VERSIONS_UNIMPLEMENTED__





IWRAM_CODE BOOL TextEngine_DefaultRenderCallback_Tilemap(
                                    const TextEngine_Font_Glyph_t *glyph_info,
                                    TextEngine_TextSurface_t *surface,
                                    __unused void *userdata) {
  Coord_t *cursor = &surface->cursor;
  const u32 
    LFT = surface->margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_LEFT],
    TOP = surface->margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_TOP],
    RGT = surface->margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_RIGHT],
    BTM = surface->margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_BOTTOM],
    TXTBOX_W = RGT - LFT,
    TXTBOX_H = BTM - TOP,
    GW = glyph_info->width, 
    GH = glyph_info->height,
    GBPP = glyph_info->bpp,
    GPITCH = glyph_info->cell_pitch;
  u32 start_x = cursor->x+LFT, start_y = cursor->y+TOP, cur_tile_idx,
      tile_x_ofs, tile_y_ofs;
  if ((cursor->x + GW) > TXTBOX_W) {
    cursor->x = 0;
    cursor->y += glyph_info->max_height;
    start_x = LFT;
    start_y = cursor->y + TOP;
    if ((cursor->y + GH) > TXTBOX_H)
      return FALSE;
    cur_tile_idx = TilemapState_CalculateCurTileIdx(LFT, start_y);
    tile_x_ofs = LFT&7;
    tile_y_ofs = start_y&7;
  } else if ((cursor->y + GH) > TXTBOX_H) {
    return FALSE;
  } else {
    cur_tile_idx = TilemapState_CalculateCurTileIdx(start_x, start_y);
    tile_x_ofs = start_x&7;
    tile_y_ofs = start_y&7;
  }
  const u8 *src = (const u8*)glyph_info->data;
  const u8 *idxmap = surface->idx_map;
  u8 *basetile, *dst = (u8*)surface->sdata;
  u16 *tmap_base = NULL, *tmap_p = NULL, tmap_ent;
  u32 src_y, src_x, dst_x_base = tile_x_ofs,
      dst_y = tile_y_ofs, dst_x;
  const u16 CUR_PALBANK = REG_VALUE(SCREEN_ENTRY,
                                   PALBANK_IDX,
                                   (u16)surface->cur_palbank);
  BOOL is_8bpp = 0!=(surface->type&TEXT_ENGINE_TEXT_SURFACE_INDEXED256),
       big_endian = glyph_info->bitpack_big_endian;
  if (is_8bpp) {
    dst += cur_tile_idx<<6;
  } else {
    dst += cur_tile_idx<<5;
    tmap_base = (u16*)surface->screen_ent_data
              + (start_x>>3) + (start_y>>3<<5);
    tmap_ent = *tmap_base;
    tmap_ent&=REG_FIELD_RESET_MASK(SCREEN_ENTRY, PALBANK_IDX);
    tmap_ent|=CUR_PALBANK;
    *tmap_base = tmap_ent;
  }
  basetile = dst;
#if 1
  const u8 *src_scanln;
  u8 curpx, cur_scanline_byte=0;
  for (src_y=0; GH>src_y; ++src_y, src+=GPITCH) {
    if (8 <= dst_y) {
    dst_y = 0;
      if (is_8bpp) {
        basetile += 30<<6;
      } else {
        basetile += 30<<5;
        tmap_base += 32;
        tmap_ent = *tmap_base;
        tmap_ent &= REG_FIELD_RESET_MASK(SCREEN_ENTRY, PALBANK_IDX);
        tmap_ent |= CUR_PALBANK;
        *tmap_base = tmap_ent;
      }
    }
    dst = basetile+(dst_y<<(is_8bpp+2));
    tmap_p = tmap_base;
    for (dst_x = dst_x_base, src_x = 0, src_scanln=src; GW > src_x; ++src_x) {
      switch (GBPP) {
      case 1:
        if (0==(7&src_x))
          cur_scanline_byte = *src_scanln++;
        curpx = 0!=((big_endian
                  ? 0x80>>(src_x&7)
                  : 1<<(src_x&7))&cur_scanline_byte);
        break;
      case 4:
        if (0==(1&src_x))
          cur_scanline_byte = *src_scanln++;
        curpx = cur_scanline_byte;
        if (big_endian^(src_x&1))
          curpx>>=4;
        curpx&=0x0F;
        break;
      case 8:
        curpx = *src_scanln++;
        break;
      default:
        return FALSE;
      }
      if (NULL!=idxmap) {
        curpx = idxmap[curpx];
      }
      if (is_8bpp) {
        curpx&=0xFF;
        if (dst_x&1) {
          ((u16*)dst)[dst_x>>1] &= 0x00FF;
          ((u16*)dst)[dst_x++>>1] |= curpx<<8;
        } else {
          ((u16*)dst)[dst_x>>1] &= 0xFF00;
          ((u16*)dst)[dst_x++>>1] |= curpx;
        }
      } else {
        curpx&=0x0F;
        if (dst_x&2) {
          if (dst_x&1) {
            ((u16*)dst)[dst_x>>2] &= 0x0FFF;
            ((u16*)dst)[dst_x++>>2] |= curpx<<12;
          } else {
            ((u16*)dst)[dst_x>>2] &= 0xF0FF;
            ((u16*)dst)[dst_x++>>2] |= curpx<<8;
          }
        } else if (dst_x&1) {
          ((u16*)dst)[dst_x>>2] &= 0xFF0F;
          ((u16*)dst)[dst_x++>>2] |= curpx<<4;
        } else {
          ((u16*)dst)[dst_x>>2] &= 0xFFF0;
          ((u16*)dst)[dst_x++>>2] |= curpx;
        }
      }
      if (8>dst_x)
        continue;
      if ((src_x+1)>=GW)
        break;
      dst_x = 0;
      if (is_8bpp) {
        dst += 64;
      } else {
        dst += 32;
        tmap_ent = *++tmap_p;
        tmap_ent&=REG_FIELD_RESET_MASK(SCREEN_ENTRY, PALBANK_IDX);
        tmap_ent|=CUR_PALBANK;
        *tmap_p = tmap_ent;
      }
    }
    ++dst_y;
  }
  cursor->x += GW;
  return TRUE;
#else
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
  tile_x_ofs = (LFT+cursor->x)&7;
  cur_tile_idx = TilemapState_CalculateCurTileIdx(LFT+cursor->x,
                                                         TOP+cursor->y);

  return TRUE;
#endif
}


IWRAM_CODE void TextEngine_DefaultClearCallback_Tilemap(
                                            const Rect_t *area,
                                            TextEngine_TextSurface_t *surface,
                                            __unused void *userdata) {
  u8 *basetile = surface->sdata, *curtile;
  u16 *tmap_base=NULL, *tmap_p=NULL, tmap_ent;
  const u16 CUR_PALBANK = REG_VALUE(SCREEN_ENTRY, 
                                   PALBANK_IDX,
                                   (u16)surface->cur_palbank);
  BOOL is_8bpp;
  is_8bpp = 0!=(surface->type&TEXT_ENGINE_TEXT_SURFACE_INDEXED256);
  if (is_8bpp) {
    basetile += TilemapState_CalculateCurTileIdx(area->x, area->y)<<6;
  } else {
    basetile += TilemapState_CalculateCurTileIdx(area->x, area->y)<<5;
    tmap_base = (u16*)surface->screen_ent_data + (area->x>>3) + (area->y>>3<<5);
    tmap_ent = *tmap_base;
    tmap_ent &= REG_FIELD_RESET_MASK(SCREEN_ENTRY, PALBANK_IDX);
    tmap_ent |= CUR_PALBANK;
    *tmap_base = tmap_ent;
  }
  const u32 W = area->w, H = area->h;
  u32 dst_x_base = area->x&7, dst_y = area->y&7, dst_x, rx=0, ry=0;
  for (ry = 0; H > ry; ++ry) {
    curtile = basetile+(dst_y<<(2+is_8bpp));
    if (8 <= dst_y) {
      dst_y = 0;
      if (is_8bpp) {
        basetile += 30<<6;
      } else {
        basetile += 30<<5;
        tmap_base += 32;
        tmap_ent = *tmap_base;
        tmap_ent &= REG_FIELD_RESET_MASK(SCREEN_ENTRY, PALBANK_IDX);
        tmap_ent |= CUR_PALBANK;
        *tmap_base = tmap_ent;
        tmap_p = tmap_base;
      }
    } else if (!is_8bpp) {
      tmap_p = tmap_base;
    }
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
      if ((rx+1)>=W)
        break;
      dst_x = 0;
      if (is_8bpp) {
        curtile += 64;
      } else {
        curtile += 32;
        tmap_ent = *++tmap_p;
        tmap_ent &= REG_FIELD_RESET_MASK(SCREEN_ENTRY, PALBANK_IDX);
        tmap_ent |= CUR_PALBANK;
        *tmap_p = tmap_ent;
      }
    }
    ++dst_y;
  }
}


#endif  /* Until we've created viable ASM versions of the default render and 
         * clear callbacks for tilemap, we use their C-written counterparts. */


#ifdef __DEFAULT_CALLBACK_MODE3_ASM_VERSIONS_UNIMPLEMENTED__


// Stub
IWRAM_CODE BOOL TextEngine_DefaultRenderCallback_BMP(
                                    const TextEngine_Font_Glyph_t *glyph_info,
                                    TextEngine_TextSurface_t *surface,
                                    __unused void *userdata) {
  u16 *cur_scanline = surface->sdata, *writep;
  u16 *surf_pal = surface->pal;
  Coord_t *cursor = &surface->cursor;
  const u8 *gdata = glyph_info->data;
  u32 LFT = surface->margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_LEFT],
      TOP = surface->margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_TOP],
      RGT = surface->margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_RIGHT],
      BTM = surface->margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_BOTTOM],
      TBOX_W = RGT-LFT,
      TBOX_H = BTM-TOP,
      GW = glyph_info->width,
      GH = glyph_info->height,
      GPITCH = glyph_info->cell_pitch,
      PITCH = surface->pitch,
      GBPP = glyph_info->bpp;

  u32 start_x=cursor->x, start_y=cursor->y, gpixel;
  BOOL is_16bpp = surface->type & TEXT_ENGINE_TEXT_SURFACE_RAWXBGR1555,
       big_endian = glyph_info->bitpack_big_endian;
  {
    BOOL write_box_ovf = FALSE;
  
    if ((start_x + GW) > TBOX_W) {
      cursor->x = 0;
      cursor->y += glyph_info->max_height;
      if ((cursor->y + GH) > TBOX_H) {
        write_box_ovf = TRUE;
      } else {
        start_x = LFT, start_y = cursor->y + TOP;
      }
    } else if ((cursor->y + GH) > TBOX_H) {
      write_box_ovf = TRUE;
    } else {
      start_x = LFT+cursor->x, start_y = cursor->y + TOP;
    }
    if (write_box_ovf) {
      if (surface->type&TEXT_ENGINE_TEXT_SURFACE_DOUBLE_BUFFERED &&
          surface->flags&TEXT_ENGINE_TEXT_SURFACE_PAGE_FLIP_ON_TEXTBOX_OVERFLOW) {
        UIPTR_T newpage = (UIPTR_T)surface->sdata;
        if (TEXT_ENGINE_TEXT_SURFACE_TYPE_BMP_MODE5==surface->type) {
          surface->sdata = (void*)(newpage^M5_PAGE_SIZE);
        } else {
          surface->sdata = (void*)(newpage^M4_PAGE_SIZE);
        }
      }
      return FALSE;
    }
  }
  if (is_16bpp)
    cur_scanline += start_x + start_y*PITCH/2;
  else
    cur_scanline += (start_x>>1) + start_y*PITCH/2;

  for (u32 wx,x,y = 0; GH > y; ++y, cur_scanline+=PITCH/2, gdata +=GPITCH) {
    writep = cur_scanline;
    for (wx=start_x,x = 0; GW > x; ++x, ++wx) {
      switch (GBPP) {
      case 1:
        if (big_endian)
          gpixel = 0!=(gdata[x/8]&(0x80>>(x&7)));
        else
          gpixel = 0!=(gdata[x/8]&(1<<(x&7)));
        break;
      case 4:
        if (big_endian^(x&1))
          gpixel = 0x0F&(gdata[x/2]>>4);
        else
          gpixel = 0x0F&gdata[x/2];
        break;
      case 8:
        gpixel = gdata[x];
        break;
      default:
        return FALSE;
      }
      if (is_16bpp) {
        *writep++ = surf_pal[gpixel];
      } else {
        if (wx&1) {
          *writep &= 0x00FF;
          *writep++ |= 0xFF00&(gpixel<<8);
        } else {
          *writep &= 0xFF00;
          *writep |= gpixel&0x00FF;
        }
      }
    }
  }
  cursor->x += GW;
  return TRUE;
}

// Stub
IWRAM_CODE void TextEngine_DefaultClearCallback_BMP(
                                            const Rect_t *bounds,
                                            TextEngine_TextSurface_t *surface,
                                            __unused void *userdata) {
  const u32 PITCH = surface->pitch, BW=bounds->w, BH = bounds->h,
        START_X = bounds->x;
  u16 *cur_scanline = surface->sdata, *writep;
  BOOL is_16bpp = surface->type&TEXT_ENGINE_TEXT_SURFACE_RAWXBGR1555;
  if (is_16bpp)
    cur_scanline += bounds->x + bounds->y*PITCH;
  else
    cur_scanline += (bounds->x>>1) + bounds->y*PITCH/2;
  for (u32 wx, x,y=0; BH > y; ++y, cur_scanline += PITCH) {
    writep = cur_scanline;
    for (wx = START_X, x = 0; BW > x; ++x, ++wx) {
      if (is_16bpp)
        *writep++ = 0;
      else if (wx&1)
        *writep++ &= 0x00FF;
      else
        *writep &= 0xFF00;
    }
  }
}


#endif  /* Until we've created viable ASM versions of the default render and 
         * clear callbacks for BMP, we use their C-written counterparts. */
