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
  TEXT_ENGINE_TEXT_SURFACE_BMP         =          0x0001,
  TEXT_ENGINE_TEXT_SURFACE_TILEMAP     =          0x0002,
  TEXT_ENGINE_TEXT_SURFACE_INDEXED16   =          0x0004,
  TEXT_ENGINE_TEXT_SURFACE_INDEXED256  =          0x0008,
  TEXT_ENGINE_TEXT_SURFACE_RAWXBGR1555 =          0x0010,
} TextEngine_TextSurface_TypeFlag_e;

#define TEXT_ENGINE_TEXT_SURFACE_TYPE_4BPP_TILEMAP  \
  (TEXT_ENGINE_TEXT_SURFACE_TILEMAP|TEXT_ENGINE_TEXT_SURFACE_INDEXED16)
#define TEXT_ENGINE_TEXT_SURFACE_TYPE_8BPP_TILEMAP  \
  (TEXT_ENGINE_TEXT_SURFACE_TILEMAP|TEXT_ENGINE_TEXT_SURFACE_INDEXED256)

typedef enum e_txt_engine_margin_idx {
  TEXT_ENGINE_RENDER_MARGIN_LEFT=0,
  TEXT_ENGINE_RENDER_MARGIN_TOP,
  TEXT_ENGINE_RENDER_MARGIN_RIGHT,
  TEXT_ENGINE_RENDER_MARGIN_BOTTOM,
  TEXT_ENGINE_RENDER_MARGINS_CT
} TextEngine_MarginIndex_e;

typedef struct s_txt_engine_font TextEngine_Font_t;
typedef struct s_txt_engine_txt_surface TextEngine_TextSurface_t;
typedef struct s_txt_engine_tilemap_state TextEngine_TilemapState_t;
typedef struct s_txt_engine_font_glyph TextEngine_Font_Glyph_t;
typedef struct s_txt_engine_ctx TextEngine_Ctx_t;

typedef BOOL (*TextEngine_GlyphRender_cb)(
                                    const TextEngine_Font_Glyph_t *glyph_info,
                                    Coord_t *cursor_in_out,
                                    u16 *pal,
                                    const u16 *margins,
                                    void *userdata);
typedef void (*TextEngine_ClearRender_cb)(const Rect_t *bounds, void *userdata);

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
  u16 *pal;
  u32 pitch;
  u16 width;
  u16 height;
  u16 type;
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
  TextEngine_TextSurface_t text_window;
  i16 cursor_x;
  i16 cursor_y;
  TextEngine_Font_t *current_font;
  u8 *char_lookup_table;
  u16 *pal;
  u32 flags;
  u16 margins[4];
  TextEngine_GlyphRender_cb glyph_render_cb;
  TextEngine_ClearRender_cb clear_render_cb;
  void *userdata;
  const char **predef_string_table;
};


#define __DEFAULT_CALLBACK_MODE3_ASM_VERSIONS_UNIMPLEMENTED__
#define __DEFAULT_CALLBACK_TILEMAP_ASM_VERSIONS_UNIMPLEMENTED__


BOOL TextEngine_RenderCallbacks_UseDefault_Tilemap(TextEngine_Ctx_t *ctx,
                                                   u32 bg_idx,
                                                   u32 bg_flags);

void TextEngine_RenderCallbacks_UseCustom(TextEngine_Ctx_t *ctx,
                                          TextEngine_GlyphRender_cb render_cb,
                                          TextEngine_ClearRender_cb clear_cb,
                                          void *userdata);

__attribute__ ((__format__ ( __printf__, 2, 3 ) ))
int TextEngine_Printf(TextEngine_Ctx_t *ctx, const char *__restrict fmt,
    ...);

IWRAM_CODE BOOL TextEngine_DefaultRenderCallback_Tilemap(
                                                const TextEngine_Font_Glyph_t*,
                                                Coord_t*,
                                                u16*,
                                                const u16*,
                                                void*);
IWRAM_CODE BOOL TextEngine_DefaultRenderCallback_Mode3(
                                                const TextEngine_Font_Glyph_t*,
                                                Coord_t*,
                                                u16*,
                                                const u16*,
                                                void*);

IWRAM_CODE void TextEngine_DefaultClearCallback_Tilemap(const Rect_t*,
                                                        void*);
IWRAM_CODE void TextEngine_DefaultClearCallback_Mode3(const Rect_t*,
                                                      void*);

#ifdef __cplusplus
}
#endif  /* C++ Name mangler guard */

#endif  /* _GBADEV_TEXT_ENGINE_H_ */
