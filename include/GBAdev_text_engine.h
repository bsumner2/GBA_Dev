/** (C) 18 of November, 2025 Burt Sumner */
/** Free to use, but this copyright message must remain here */

#ifndef _GBADEV_TEXT_ENGINE_H_
#define _GBADEV_TEXT_ENGINE_H_

#ifdef __cplusplus
extern "C" {
#endif  /* C++ Name mangler guard */

#include <GBAdev_types.h>

#define TEXT_ENGINE_TEXT_SURFACE_MODE_MASK        0x0003
#define TEXT_ENGINE_TEXT_SURFACE_ENCODING_MASK    0x001C

typedef enum e_txt_surface_type_flags {
  TEXT_ENGINE_TEXT_SURFACE_BMP             =      0x0001,
  TEXT_ENGINE_TEXT_SURFACE_TILEMAP         =      0x0002,
  TEXT_ENGINE_TEXT_SURFACE_INDEXED16       =      0x0004,
  TEXT_ENGINE_TEXT_SURFACE_INDEXED256      =      0x0008,
  TEXT_ENGINE_TEXT_SURFACE_RAWXBGR1555     =      0x0010,
  TEXT_ENGINE_TEXT_SURFACE_DOUBLE_BUFFERED =      0x0020
} TextEngine_TextSurface_TypeFlag_e;

typedef enum e_txt_engine_txt_surface_flags {
  TEXT_ENGINE_TEXT_SURFACE_PAGE_FLIP_ON_TEXTBOX_OVERFLOW =              0x0001,
} TextEngine_TextSurface_Flags_e;

#define TEXT_ENGINE_TEXT_SURFACE_TYPE_4BPP_TILEMAP  \
  (TEXT_ENGINE_TEXT_SURFACE_TILEMAP|TEXT_ENGINE_TEXT_SURFACE_INDEXED16)
#define TEXT_ENGINE_TEXT_SURFACE_TYPE_8BPP_TILEMAP  \
  (TEXT_ENGINE_TEXT_SURFACE_TILEMAP|TEXT_ENGINE_TEXT_SURFACE_INDEXED256)
#define TEXT_ENGINE_TEXT_SURFACE_TYPE_BMP_MODE3  \
  (TEXT_ENGINE_TEXT_SURFACE_BMP|TEXT_ENGINE_TEXT_SURFACE_RAWXBGR1555)
#define TEXT_ENGINE_TEXT_SURFACE_TYPE_BMP_MODE4  \
  (TEXT_ENGINE_TEXT_SURFACE_BMP |  \
   TEXT_ENGINE_TEXT_SURFACE_INDEXED256 |  \
   TEXT_ENGINE_TEXT_SURFACE_DOUBLE_BUFFERED)
#define TEXT_ENGINE_TEXT_SURFACE_TYPE_BMP_MODE5  \
  (TEXT_ENGINE_TEXT_SURFACE_BMP |  \
   TEXT_ENGINE_TEXT_SURFACE_RAWXBGR1555 |  \
   TEXT_ENGINE_TEXT_SURFACE_DOUBLE_BUFFERED)
typedef enum e_txt_engine_margin_idx {
  TEXT_ENGINE_TEXT_SURFACE_MARGIN_LEFT=0,
  TEXT_ENGINE_TEXT_SURFACE_MARGIN_TOP,
  TEXT_ENGINE_TEXT_SURFACE_MARGIN_RIGHT,
  TEXT_ENGINE_TEXT_SURFACE_MARGIN_BOTTOM,
  TEXT_ENGINE_TEXT_SURFACE_MARGIN_CT
} TextEngine_MarginIndex_e;

typedef struct s_txt_engine_font TextEngine_Font_t;
typedef struct s_txt_engine_txt_surface TextEngine_TextSurface_t;
typedef struct s_txt_engine_tilemap_state TextEngine_TilemapState_t;
typedef struct s_txt_engine_font_glyph TextEngine_Font_Glyph_t;
typedef struct s_txt_engine_ctx TextEngine_Ctx_t;

typedef BOOL (*TextEngine_GlyphRender_cb)(
                                    const TextEngine_Font_Glyph_t *glyph_info,
                                    TextEngine_TextSurface_t *render_surface,
                                    void *userdata);
typedef void (*TextEngine_ClearRender_cb)(
                                  const Rect_t *bounds,
                                  TextEngine_TextSurface_t *render_surface,
                                  void *userdata);

struct s_txt_engine_font {
  const void *glyph_data;
  const u16 *pal;
  const u8 *glyph_widths;
  const u8 *glyph_heights;
  u16 glyph0_char_code, errglyph_idx;
  u16 glyph_count;
  u16 pal_color_count;
  /// Stores width for fixed-width fonts, but also stores max width for
  /// variable width fonts.
  u8 glyph_width;
  /// Stores height for fixed-height fonts, but also stores max height for
  /// variable height fonts.
  u8 glyph_height;
  u8 cell_width;
  u8 cell_height;
  u16 cell_size;
  u16 cell_pitch;
  u8 bitdepth;
  BOOL bitpack_big_endian;
};

struct s_txt_engine_txt_surface {
  void *sdata;
  void *screen_ent_data;
  u16 *pal;
  u8 *idx_map;
  u16 margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_CT];
  Coord_t cursor;
  u32 pitch;
  u16 width;
  u16 height;
  u16 type;
  u16 pal_clr_ct;
  u16 flags;
  u8 cur_palbank;
};



struct s_txt_engine_tilemap_state {
  TextEngine_TextSurface_t surface;
  u32 cur_tile_idx;
  u32 tile_x_ofs, tile_y_ofs;
};

struct s_txt_engine_font_glyph {
  const void *data;
  u16 width;
  u16 height;
  /// Maximum width for any glyph in font. Unused
  u16 max_width;
  /// Maximum height of any glyph in the font. Use for CRLF's
  u16 max_height;
  u16 cell_pitch;
  u16 bpp;
  BOOL bitpack_big_endian;
};

struct s_txt_engine_ctx {
  TextEngine_TextSurface_t surface;
  const TextEngine_Font_t *current_font;
  TextEngine_Font_t **font_registry;
  const char **predef_string_table;
  u32 font_registry_size;
  u32 prdef_string_table_size;
  u32 flags;
  TextEngine_GlyphRender_cb glyph_render_cb;
  TextEngine_ClearRender_cb clear_render_cb;
  void *userdata;
};


#define __DEFAULT_CALLBACK_MODE3_ASM_VERSIONS_UNIMPLEMENTED__
#define __DEFAULT_CALLBACK_TILEMAP_ASM_VERSIONS_UNIMPLEMENTED__


BOOL TextEngine_RenderCallbacks_UseDefault_Tilemap(TextEngine_Ctx_t *ctx,
                                                   u32 bg_idx,
                                                   u32 bg_flags);
BOOL TextEngine_RenderCallbacks_UseDefault_BMP(TextEngine_Ctx_t *ctx,
                                               u16 *pal_buffer,
                                               u16 bmp_mode,
                                               u16 pal_buffer_slot_count,
                                               u16 flags);

void TextEngine_RenderCallbacks_UseCustom(TextEngine_Ctx_t *ctx,
                                          TextEngine_GlyphRender_cb render_cb,
                                          TextEngine_ClearRender_cb clear_cb,
                                          void *userdata);

__attribute__ ((__format__ ( __printf__, 2, 3 ) ))
int TextEngine_Printf(TextEngine_Ctx_t *ctx, const char *__restrict fmt,
    ...);

int TextEngine_Puts(const char *__restrict s, TextEngine_Ctx_t *ctx);
int TextEngine_Putchar(u32 c, TextEngine_Ctx_t *ctx);


#ifdef __cplusplus
}
#endif  /* C++ Name mangler guard */

#endif  /* _GBADEV_TEXT_ENGINE_H_ */
