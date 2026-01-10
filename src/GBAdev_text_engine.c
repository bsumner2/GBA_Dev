#include <GBAdev_text_engine.h>
#include <GBAdev_types.h>
#include <GBAdev_memdef.h>
#include <GBAdev_memmap.h>
#include <GBAdev_functions.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ANSI_CURSOR_UP            'A'
#define ANSI_CURSOR_DOWN          'B'
#define ANSI_CURSOR_RIGHT         'C'
#define ANSI_CURSOR_LEFT          'D'
#define ANSI_CURSOR_DOWN_WITH_CR  'E'
#define ANSI_CURSOR_UP_WITH_CR    'F'
#define ANSI_CURSOR_TO_COLUMN     'G'
#define ANSI_CURSOR_TO_HOME       'H'


typedef enum e_txt_engine_gbadev_esc_seqs {
  TEXT_ENGINE_GBADEV_ESCAPE_INVALID                 =0x80000000UL,
  TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_PAL            =0x00000001UL,
  TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_CURSOR_X       =0x00000002UL,
  TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_CURSOR_Y       =0x00000004UL,
  TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_CURSOR_XY      =0x00000008UL,
  TEXT_ENGINE_GBADEV_ESCAPE_TYPE_CLEAR_RENDERBOX    =0x00000010UL,
  TEXT_ENGINE_GBADEV_ESCAPE_TYPE_RESET              =0x00000020UL,
  TEXT_ENGINE_GBADEV_ESCAPE_TYPE_MOVE_CURSOR        =0x00000040UL,
  TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_FG             =0x00000080UL,
  TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_BG             =0x00000100UL,
  TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_FONT           =0x00000200UL,
  TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_MARGINS        =0x00000400UL,
  TEXT_ENGINE_GBADEV_ESCAPE_TYPE_PRINT_PRESET_STR   =0x00000800UL,
  TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_PALBANK        =0x00001000UL,
  TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_IDXMAP         =0x00002000UL,
  TEXT_ENGINE_GBADEV_ESCAPE_TYPE_MASK               =0x00003FFFUL,
  TEXT_ENGINE_GBADEV_ESCAPE_PAL_ARGV_MASK           =0x003FC000UL,
  TEXT_ENGINE_GBADEV_ESCAPE_MARGIN_ARGV_LEFT        =0x04000000UL,
  TEXT_ENGINE_GBADEV_ESCAPE_MARGIN_ARGV_TOP         =0x08000000UL,
  TEXT_ENGINE_GBADEV_ESCAPE_MARGIN_ARGV_RIGHT       =0x10000000UL,
  TEXT_ENGINE_GBADEV_ESCAPE_MARGIN_ARGV_BOTTOM      =0x20000000UL,
  TEXT_ENGINE_GBADEV_ESCAPE_MARGIN_ARGV_MASK        =0x3C000000UL,
} TextEngine_GBADev_EscapeSequences_e;
#define TEXT_ENGINE_GBADEV_ESCAPE_PAL_ARGV_SHAMT 14

#define SET_X       0b01
#define SET_Y       0b10
#define SET_BOTH    0b11

extern IWRAM_CODE BOOL TextEngine_DefaultRenderCallback_BMP(
                                                const TextEngine_Font_Glyph_t*,
                                                TextEngine_TextSurface_t*,
                                                void*);


extern IWRAM_CODE BOOL TextEngine_DefaultRenderCallback_Tilemap(
                                                const TextEngine_Font_Glyph_t*,
                                                TextEngine_TextSurface_t*,
                                                void*);


extern IWRAM_CODE void TextEngine_DefaultClearCallback_Tilemap(
                                            const Rect_t*,
                                            TextEngine_TextSurface_t*,
                                            void*);
extern IWRAM_CODE void TextEngine_DefaultClearCallback_BMP(
                                            const Rect_t*,
                                            TextEngine_TextSurface_t*,
                                            void*);

BOOL __GBADEV_INTERNAL__TextEngine_ProcessEscapeChars_ANSI(
                                                         TextEngine_Ctx_t *ctx, 
                                                         const char **iostr);

BOOL __GBADEV_INTERNAL__TextEngine_ProcessEscapeChars_GBADev(
                                                         TextEngine_Ctx_t *ctx, 
                                                         const char **iostr);

TextEngine_Font_Glyph_t *__GBADEV_INTERNAL__TextEngine_LookupGlyph(
                                                TextEngine_Font_Glyph_t *dst,
                                                const TextEngine_Font_t *font,
                                                u32 code_pt);


static BOOL  __GBADEV_INTERNAL__TextEngine_ANSIEscapes_ApplySGR(
                                                    TextEngine_Ctx_t *ctx,
                                                    u32 args[16],
                                                    u32 argc);
static BOOL __GBADEV_INTERNAL__TextEngine_ANSIEscapes_ApplyCMV(
                                                   TextEngine_Ctx_t *ctx,
                                                   u32 args[16],
                                                   u32 argc,
                                                   u32 mvmt_argv);


static BOOL __GBADEV_INTERNAL__TextEngine_ANSIEscapes_ApplyEL(
                                                   TextEngine_Ctx_t *ctx,
                                                   u32 args[16],
                                                   u32 argc);
static BOOL __GBADEV_INTERNAL__TextEngine_ANSIEscapes_ApplyED(
                                                   TextEngine_Ctx_t *ctx,
                                                   u32 args[16],
                                                   u32 argc);
static BOOL
        __GBADEV_INTERNAL__TextEngine_GBADevEscapes_ParseULong(
                                                      u32 *dst,
                                                      const char **iostr,
                                                      u32 base);
static BOOL   
      __GBADEV_INTERNAL__TextEngine_GBADevEscapes_SetMargins(
                                      TextEngine_TextSurface_t *surface,
                                      TextEngine_GBADev_EscapeSequences_e seq,
                                      const char **iostr);

static BOOL  
      __GBADEV_INTERNAL__TextEngine_GBADevEscapes_PrintPresetString(
                                      TextEngine_Ctx_t *ctx,
                                      const char **iostr);
static BOOL  
      __GBADEV_INTERNAL__TextEngine_GBADevEscapes_SetFont(
                                      TextEngine_Ctx_t *ctx,
                                      const char **iostr);

static BOOL
        __GBADEV_INTERNAL__TextEngine_GBADevEscape_SetPal(
                                                      TextEngine_Ctx_t *ctx,
                                                      u32 pal_idx,
                                                      const char **iostr);

static BOOL
        __GBADEV_INTERNAL__TextEngine_GBADevEscape_SetIdxMap(
                                                      TextEngine_Ctx_t *ctx,
                                                      u32 pal_idx,
                                                      const char **iostr);


static BOOL
        __GBADEV_INTERNAL__TextEngine_GBADevEscape_SetCursor(
                                                      TextEngine_Ctx_t *ctx,
                                                      u32 which_flags,
                                                      const char **iostr);
static BOOL
        __GBADEV_INTERNAL__TextEngine_GBADevEscape_RenderClear(
                                                      TextEngine_Ctx_t *ctx);
static BOOL
        __GBADEV_INTERNAL__TextEngine_GBADevEscape_ResetCtxState(
                                                      TextEngine_Ctx_t *ctx);
static BOOL
        __GBADEV_INTERNAL__TextEngine_GBADevEscape_MoveCursor(
                                                      TextEngine_Ctx_t *ctx,
                                                      const char **iostr);

static BOOL 
        __GBADEV_INTERNAL__TextEngine_GBADevEscapes_SetFGBG(
                                            TextEngine_TextSurface_t *surface,
                                            u32 fgbg,
                                            const char **iostr);
static BOOL 
        __GBADEV_INTERNAL__TextEngine_GBADevEscape_SetPalBank(
                                            TextEngine_TextSurface_t *surface,
                                            const char **iostr);


static int __GBADEV_INTERNAL__TextEngine_ProcessString(TextEngine_Ctx_t *ctx,
                                                       const char *str);

BOOL __GBADEV_INTERNAL__TextEngine_ANSIEscapes_ApplyED(TextEngine_Ctx_t *ctx,
                                           u32 args[16],
                                           u32 argc) {
  if (0 == argc) {
    argc = 1;
    args[0] = 0;
  }
  if (1!=argc)
    return FALSE;
  argc = args[0];
  Rect_t r = { .x=0, .y=0, .w=0, .h=0 };
  const Coord_t *SCURSOR = &ctx->surface.cursor;
  const u16 *MARGINS = ctx->surface.margins;
  u32 left_margin     = MARGINS[TEXT_ENGINE_TEXT_SURFACE_MARGIN_LEFT],
      right_margin    = MARGINS[TEXT_ENGINE_TEXT_SURFACE_MARGIN_RIGHT],
      top_margin      = MARGINS[TEXT_ENGINE_TEXT_SURFACE_MARGIN_TOP],
      bottom_margin   = MARGINS[TEXT_ENGINE_TEXT_SURFACE_MARGIN_BOTTOM],
      cursor_x        = SCURSOR->x + left_margin;
  r.y = top_margin;
  r.h = bottom_margin - top_margin;

  switch (argc) {
  case 0:
    r.x = cursor_x, r.w = right_margin - cursor_x;
    break;
  case 1:
    r.x = cursor_x, r.w = cursor_x - left_margin;
    break;
  case 2:
  r.x = left_margin, r.w = right_margin - left_margin;
    break;
  default:
    return FALSE;
  }
  ctx->clear_render_cb(&r, &ctx->surface, ctx->userdata);
  return TRUE;

}


BOOL __GBADEV_INTERNAL__TextEngine_ANSIEscapes_ApplyEL(TextEngine_Ctx_t *ctx,
                                           u32 args[16],
                                           u32 argc) {
  if (0 == argc) {
    argc = 1;
    args[0] = 0;
  }
  if (1!=argc)
    return FALSE;
  // now that weve established argc should only ever be one, we can recycle
  // effectively dead var argc for other uses: argc will now hold args[0]
  argc = args[0];
  Rect_t r = { .x=0, .y=0,.w=0, .h=ctx->current_font->glyph_height };
  const Coord_t *SCURSOR = &ctx->surface.cursor;
  const u16 *MARGINS = ctx->surface.margins;
  u32 left_margin   = MARGINS[TEXT_ENGINE_TEXT_SURFACE_MARGIN_LEFT],
      right_margin  = MARGINS[TEXT_ENGINE_TEXT_SURFACE_MARGIN_RIGHT],
      cursor_x      = SCURSOR->x + left_margin;
  r.y = SCURSOR->y + MARGINS[TEXT_ENGINE_TEXT_SURFACE_MARGIN_TOP];

  switch (argc) {
  case 0:
    r.x = cursor_x, r.w = right_margin - cursor_x;
    break;
  case 1:
    r.x = left_margin, r.w = cursor_x - left_margin;
    break;
  case 2:
    r.x = left_margin, r.w = right_margin - left_margin;
    break;
  default:
    return FALSE;
  }
  ctx->clear_render_cb(&r, &ctx->surface, ctx->userdata);
  return TRUE;
}




BOOL __GBADEV_INTERNAL__TextEngine_ANSIEscapes_ApplyCMV(TextEngine_Ctx_t *ctx,
                                           u32 args[16],
                                           u32 argc,
                                           u32 mvmt_argv) {
  if (0==argc) {
    if (ANSI_CURSOR_TO_HOME == mvmt_argv) {
      ctx->surface.cursor = (Coord_t) { 0, 0 };
      return TRUE;
    }
    return FALSE;
  }
  if (1!=argc)
    return FALSE;
  argc = args[0];
  if (0==argc)
    return TRUE;  // Move cursor 0 units=>successful without doing anything,
                  // so just return true!

  const u16 *MARGINS = ctx->surface.margins;
  Coord_t *cursor = &ctx->surface.cursor;

  const u32 WDTH  = (MARGINS[TEXT_ENGINE_TEXT_SURFACE_MARGIN_RIGHT]
                        - MARGINS[TEXT_ENGINE_TEXT_SURFACE_MARGIN_LEFT]),
            HGHT  = (MARGINS[TEXT_ENGINE_TEXT_SURFACE_MARGIN_BOTTOM]
                        - MARGINS[TEXT_ENGINE_TEXT_SURFACE_MARGIN_TOP]),
            CURSOR_ORIGIN_X = cursor->x,
            CURSOR_ORIGIN_Y = cursor->y;
  // Note: Remember we've recycled otherwise dead var, argc, to store args[0],
  // so whenever you see argc, just know it now houses args[0], not arg count.
  switch (mvmt_argv) {
  case ANSI_CURSOR_UP:
    argc*=ctx->current_font->glyph_height;
    if (argc > CURSOR_ORIGIN_Y)
      return FALSE;
    cursor->y -= argc;
    return TRUE;
  case ANSI_CURSOR_DOWN:
    argc*=ctx->current_font->glyph_height;
    if ((argc+CURSOR_ORIGIN_Y) > HGHT)
      return FALSE;
    cursor->y += argc;
    return TRUE;
  case ANSI_CURSOR_RIGHT:
    argc *= ctx->current_font->glyph_width;
    if ((argc + CURSOR_ORIGIN_X) > WDTH)
      return FALSE;
    cursor->x += argc;
    return TRUE;
  case ANSI_CURSOR_LEFT:
    argc*=ctx->current_font->glyph_width;
    if (argc > CURSOR_ORIGIN_X)
      return FALSE;
    cursor->x -= argc;
    return TRUE;
  case ANSI_CURSOR_DOWN_WITH_CR:
    argc*=ctx->current_font->glyph_height;
    if ((argc+CURSOR_ORIGIN_Y) > HGHT)
      return FALSE;
    cursor->x = 0;
    cursor->y += argc;
    return TRUE;
  case ANSI_CURSOR_UP_WITH_CR:
    argc*=ctx->current_font->glyph_height;
    if (argc > CURSOR_ORIGIN_Y)
      return FALSE;
    cursor->x = 0;
    cursor->y -= argc;
    return TRUE;
  case ANSI_CURSOR_TO_COLUMN:
    argc *= ctx->current_font->glyph_width;
    if (argc >= WDTH)
      return FALSE;
    cursor->x = argc;
    return TRUE;
  default:
    return FALSE;
  }
  
  return TRUE;

}

BOOL __GBADEV_INTERNAL__TextEngine_ANSIEscapes_ApplySGR(TextEngine_Ctx_t *ctx,
                                            u32 args[16],
                                            u32 argc) {
  if (NULL == ctx || 0 == args)
    return FALSE;
  if (38 == args[0] || 48 == args[0]) {
    if (5 == args[1]) {  // Even if args[1] was uninitialized, argc definitely
                         // was initialized, so if it happened to be 5 
                         // uninited, then argc would be 1 and would not pass
                         // the arg check that follows:
      if (3 != argc) {  // No more than 3 args: one for bg/fg, one for color
                        // assignment type (5 = xterm256pal idx), and one for
                        // the desired xterm256 pal idx
        return FALSE;
    }
    if (256 <= args[2])
      return FALSE;
    extern const unsigned short __GBADEV_INTERNAL__XTerm256Pal[256];
    if (38 == args[0])
      ctx->surface.pal[1] = __GBADEV_INTERNAL__XTerm256Pal[args[2]];
    else
      ctx->surface.pal[0] = __GBADEV_INTERNAL__XTerm256Pal[args[2]];
    return TRUE;
    } else if (2 == args[1]) {
      if (5 != argc) {
        return FALSE;
      }
      u16 color = 0;
      for (u32 shamt = 0, cur_channel, i = 0; 3>i; ++i, shamt+=5) {
        cur_channel = args[2+i];
        if (31 < cur_channel)
          return FALSE;
        color |= (cur_channel&31)<<shamt;
      }
      if (38 == args[0])
        ctx->surface.pal[1] = color;
      else
        ctx->surface.pal[0] = color;
      return TRUE;
    }
  }
  if (2 < argc)
    return FALSE;
  for (u32 i=0, arg=args[i]; argc>i; arg=args[++i]) {
      switch (arg) {
      case 49:
        ctx->surface.pal[0] = ctx->current_font->pal[0];
        continue;
        break;
      case 39:
        ctx->surface.pal[1] = ctx->current_font->pal[1];
        continue;
        break;
      case 0:
        ctx->surface.pal[0] = ctx->current_font->pal[0];
        ctx->surface.pal[1] = ctx->current_font->pal[1];
        continue;
        break;
      default:
        break;
      }
    if (38 > arg && 29 < arg) {
      extern const unsigned short __GBADEV_INTERNAL__XTerm256Pal[256];
      ctx->surface.pal[1] = __GBADEV_INTERNAL__XTerm256Pal[arg-30];
    } else if (48 > arg && 39 < arg) {
      extern const unsigned short __GBADEV_INTERNAL__XTerm256Pal[256];
      ctx->surface.pal[0] = __GBADEV_INTERNAL__XTerm256Pal[arg-40];
    }
  }
  return TRUE;
}


BOOL __GBADEV_INTERNAL__TextEngine_ProcessEscapeChars_ANSI(
                                                         TextEngine_Ctx_t *ctx,
                                                         const char **iostr) {
  const char *str = *iostr;
  if ('[' != *str++)
    return FALSE;
  u32 args[16],
      argc, curarg,
      c, finalizer_char = '\0', pc = '[';
  BOOL loop_exited_abnormally = TRUE;
  for (curarg = argc = 0, c=*str++; ';'==c || isalnum(c); pc = c, c = *str++) {
    if (isdigit(c)) {
      curarg *= 10;
      curarg += c&15;
      continue;
    }
    if (16 <= argc || !isdigit(pc))
      break;  // Max arg count reached or delim in invalid position
    args[argc++] = curarg;
    curarg = 0;
    if (';' == c)
      continue;
    loop_exited_abnormally = FALSE;
    break;
  }
  *iostr = str;
  if (loop_exited_abnormally)
    return FALSE;
  finalizer_char = c;

  switch (finalizer_char) {
  case 'A':
  case 'B':
  case 'C':
  case 'D':
  case 'E':
  case 'F':
  case 'G':
  case 'H':
    return __GBADEV_INTERNAL__TextEngine_ANSIEscapes_ApplyCMV(ctx, 
                                                  args,
                                                  argc,
                                                  finalizer_char);
  case 'J':
    return __GBADEV_INTERNAL__TextEngine_ANSIEscapes_ApplyED(ctx, args, argc);
  case 'K':
    return __GBADEV_INTERNAL__TextEngine_ANSIEscapes_ApplyEL(ctx, args, argc);
  case 'm':
    return __GBADEV_INTERNAL__TextEngine_ANSIEscapes_ApplySGR(ctx, args, argc);
  default:
    return FALSE;
  }  
}

static int __GBADEV_INTERNAL__TextEngine_Strncmp_CaseInsensitive(
                                                      const char *comparand0,
                                                      const char *comparand1,
                                                      size_t max_cmp_len) {
  u32 c0, c1;
  for (size_t i = 0; max_cmp_len > i; ++i) {
    c0 = tolower(*comparand0++);
    c1 = tolower(*comparand1++);
    if (c0!=c1)
      return (i32)(c0-c1);
    else if (c0)
      continue;
    // If we're here, then strings both matched and terminated before reaching 
    // max_cmp_len, which is only a max len, not a min, so we can return 0 here
    return 0;
  }
  return 0;
}


static const u8 STATE_TRANSITION_LIST_COUNTS[6] = {
  8, 2, 2, 2, 2, 2
};

static const struct s_gbadev_escape_dfa_transition {
  u8 input;
  u8 dst_state;
} __attribute__((packed)) *TRANSITION_LISTS[] = {
  [0x00] = (struct s_gbadev_escape_dfa_transition[]) {
    {'b', 0x04},
    {'c', 0x06},
    {'f', 0x03},
    {'i', 0x18},
    {'m', 0x02},
    {'p', 0x05},
    {'r', 0x07},
    {'x', 0x01},
    {'y', 0x13}
  },
  [0x01] = (struct s_gbadev_escape_dfa_transition[]) {
    {'=', 0x12},
    {'y', 0x08}
  },
  [0x02] = (struct s_gbadev_escape_dfa_transition[]) {
    {'a', 0x0A},
    {'v', 0x09}
  },
  [0x03] = (struct s_gbadev_escape_dfa_transition[]) {
    {'g', 0x0B},
    {'o', 0x0C}
  },
  [0x04] = (struct s_gbadev_escape_dfa_transition[]) {
    {'a', 0x21},
    {'g', 0x0D}
  },
  [0x05] = (struct s_gbadev_escape_dfa_transition[]) {
    {'a', 0x0F},
    {'s', 0x0E}
  },
  [0x06] = (struct s_gbadev_escape_dfa_transition[]) {
    {'l', 0x10}
  },
  [0x07] = (struct s_gbadev_escape_dfa_transition[]) {
    {'e', 0x11}
  },
  [0x08] = (struct s_gbadev_escape_dfa_transition[]) {
    {'=', 0x12}
  },
  [0x09] = (struct s_gbadev_escape_dfa_transition[]) {
    {'=', 0x12}
  },
  [0x0A] = (struct s_gbadev_escape_dfa_transition[]) {
    {'r', 0x14}
  },
  [0x0B] = (struct s_gbadev_escape_dfa_transition[]) {
    {'=', 0x12}
  },
  [0x0C] = (struct s_gbadev_escape_dfa_transition[]) {
    {'n', 0x15}
  },
  [0x0D] = (struct s_gbadev_escape_dfa_transition[]) {
    {'=', 0x12}
  },
  [0x0E] = (struct s_gbadev_escape_dfa_transition[]) {
    {'t', 0x16}
  },
  [0x0F] = (struct s_gbadev_escape_dfa_transition[]) {
    {'l', 0x17}
  },
  [0x10] = (struct s_gbadev_escape_dfa_transition[]) {
    {'r', 0x12}
  },
  [0x11] = (struct s_gbadev_escape_dfa_transition[]) {
    {'s', 0x19}
  },
  [0x12] = (struct s_gbadev_escape_dfa_transition*)NULL,  // transition 
                                                          // list for state 
                                                          // no.18, which is 
                                                          // the universal 
                                                          // accepting state
                                                          // (i.e.: the only 
                                                          // acceptor state).
  [0x13] = (struct s_gbadev_escape_dfa_transition[]) {
    {'=', 0x12}
  },
  [0x14] = (struct s_gbadev_escape_dfa_transition[]) {
    {'g', 0x1A}
  },
  [0x15] = (struct s_gbadev_escape_dfa_transition[]) {
    {'t', 0x1B}
  },
  [0x16] = (struct s_gbadev_escape_dfa_transition[]) {
    {'r', 0x1C}
  },
  [0x17] = (struct s_gbadev_escape_dfa_transition[]) {
    {'.', 0x12}
  },
  [0x18] = (struct s_gbadev_escape_dfa_transition[]) {
    {'m', 0x24}
  },
  [0x19] = (struct s_gbadev_escape_dfa_transition[]) {
    {'e', 0x1D}
  },
  [0x1A] = (struct s_gbadev_escape_dfa_transition[]) {
    {'i', 0x1E}
  },
  [0x1B] = (struct s_gbadev_escape_dfa_transition[]) {
    {'=', 0x12}
  },
  [0x1C] = (struct s_gbadev_escape_dfa_transition[]) {
    {'#', 18}
  },
  [0x1D] = (struct s_gbadev_escape_dfa_transition[]) {
    {'t', 0x12}
  },
  [0x1E] = (struct s_gbadev_escape_dfa_transition[]) {
    {'n', 0x20}
  },
  [0x1F] = (struct s_gbadev_escape_dfa_transition*)NULL,  // Another lazy 
                                                          // delete due to 
                                                          // fixes to DFA 
                                                          // design
  [0x20] = (struct s_gbadev_escape_dfa_transition[]) {
    {'.', 0x12},
    {'=', 0x12}
  },
  [0x21] = (struct s_gbadev_escape_dfa_transition[]) {
    {'n', 0x22}
  },
  [0x22] = (struct s_gbadev_escape_dfa_transition[]) {
    {'k', 0x23}
  },
  [0x23] = (struct s_gbadev_escape_dfa_transition[]) {
    {'=', 0x12}
  },
  [0x24] = (struct s_gbadev_escape_dfa_transition[]) {
    {'a', 0x25}
  },
  [0x25] = (struct s_gbadev_escape_dfa_transition[]) {
    {'p', 0x26}
  },
  [0x26] = (struct s_gbadev_escape_dfa_transition[]) {
    {'.', 0x12}
  },

};

static const u32 STATE_TRANSITION_LIST_COUNTS_COUNT = 6,
                   STATE_COUNT = sizeof(TRANSITION_LISTS)/sizeof(void*);



static TextEngine_GBADev_EscapeSequences_e
          __GBADEV_INTERNAL__TextEngine_ParseEscapeID(const char **iostr) {
  if (NULL==iostr)
    return TEXT_ENGINE_GBADEV_ESCAPE_INVALID;
  const char *pstr=*iostr;
  const struct s_gbadev_escape_dfa_transition *translist;
  u32 c, state = 0, newstate, trans_count, midpt;
  int cmpval;
  TextEngine_GBADev_EscapeSequences_e seq = TEXT_ENGINE_GBADEV_ESCAPE_INVALID;
  for (c=tolower(*pstr++); c; c = tolower(*pstr++)) {
    trans_count = STATE_TRANSITION_LIST_COUNTS_COUNT > state 
                          ? STATE_TRANSITION_LIST_COUNTS[state]
                          : 1;
    translist = TRANSITION_LISTS[state];
    if (NULL==translist) {
      return TEXT_ENGINE_GBADEV_ESCAPE_INVALID; 
    }
    newstate = 0xFFFFFFFFUL;
    if (8==trans_count) {
      for (midpt = 4; trans_count; midpt = trans_count>>1) {
        if (3 > trans_count) {
          if (1==trans_count) {
            if (translist[0].input == c) {
              newstate = translist[0].dst_state;
            }
            break;
          }
          cmpval = c-translist[1].input;
          if (0 > cmpval) {
            if (translist[0].input==c)
              newstate = translist[0].dst_state;
          } else if (0 == cmpval) {
            newstate = translist[1].dst_state;
          }
          break;
        }
        cmpval = c-translist[midpt].input;
        if (0==cmpval) {
          newstate = translist[midpt].dst_state;
          break;
        } else if (0 > cmpval) {
          trans_count = midpt;
          continue;
        } else {
          ++midpt;
          trans_count -= midpt;
          translist += midpt;
          continue;
        }
      }
    } else if (2==trans_count) {
      cmpval = c-translist[1].input;
      if (0 > cmpval) {
        if (translist[0].input==c)
          newstate = translist[0].dst_state;
      } else if (0 == cmpval) {
        newstate = translist[1].dst_state;
      }
    } else {
      if (c==translist[0].input)
        newstate = translist[0].dst_state;
    }
    if (STATE_COUNT <= newstate)
      return FALSE;
    
    if (0xFFFFFFFFUL == newstate)
      return TEXT_ENGINE_GBADEV_ESCAPE_INVALID;
    if (18 == newstate) {
      *iostr = pstr;
      switch (state) {
      case 0x01:
        return TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_CURSOR_X;
      case 0x08:
        return TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_CURSOR_XY;
      case 0x09:
        return TEXT_ENGINE_GBADEV_ESCAPE_TYPE_MOVE_CURSOR;
      case 0x0B:
        return TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_FG;
      case 0x0D:
        return TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_BG;
      case 0x10:
        return TEXT_ENGINE_GBADEV_ESCAPE_TYPE_CLEAR_RENDERBOX;
      case 0x13:
        return TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_CURSOR_Y;
      case 0x17:
        seq = TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_PAL;
        break;
      case 0x1B:
        return TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_FONT;
      case 0x1C:
        return TEXT_ENGINE_GBADEV_ESCAPE_TYPE_PRINT_PRESET_STR;
      case 0x1D:
        return TEXT_ENGINE_GBADEV_ESCAPE_TYPE_RESET;
      case 0x20:
        if ('.' == c) {
          seq = TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_MARGINS;
          break;
        };
        return TEXT_ENGINE_GBADEV_ESCAPE_MARGIN_ARGV_MASK
              | TEXT_ENGINE_GBADEV_ESCAPE_TYPE_CLEAR_RENDERBOX;
      case 0x23:
        return TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_PALBANK;
      case 0x26:
        seq = TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_IDXMAP;
        break;
      default:
        return TEXT_ENGINE_GBADEV_ESCAPE_INVALID;
      }
      break;
    }
    state = newstate;
    continue;
  }
  if (TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_MARGINS == seq) {
    int len;
    for (c=pstr[len=0]; c && '='!=c; c=pstr[++len])
      if (6 < len)
        return TEXT_ENGINE_GBADEV_ESCAPE_INVALID;
    if (0==c)
      return TEXT_ENGINE_GBADEV_ESCAPE_INVALID;
    char buf[7] = {0};
    for (int i = 0; len > i; ++i)
      buf[i] = tolower(pstr[i]);
    *iostr = pstr+=len;
    switch (buf[0]) {
    case 'l':
      if (0!=__GBADEV_INTERNAL__TextEngine_Strncmp_CaseInsensitive(
                                                            buf,
                                                            "left",
                                                            LSTRLEN("left")))
        return TEXT_ENGINE_GBADEV_ESCAPE_INVALID;
      seq |= TEXT_ENGINE_GBADEV_ESCAPE_MARGIN_ARGV_LEFT;
      break;
    case 't':
      if (0!=__GBADEV_INTERNAL__TextEngine_Strncmp_CaseInsensitive(
                                                            buf,
                                                            "top",
                                                            LSTRLEN("top")))
        return TEXT_ENGINE_GBADEV_ESCAPE_INVALID;
      seq |= TEXT_ENGINE_GBADEV_ESCAPE_MARGIN_ARGV_TOP;
      break;
    case 'r':
      if (0!=__GBADEV_INTERNAL__TextEngine_Strncmp_CaseInsensitive(
                                                            buf,
                                                            "right",
                                                            LSTRLEN("right")))
        return TEXT_ENGINE_GBADEV_ESCAPE_INVALID;
      seq |= TEXT_ENGINE_GBADEV_ESCAPE_MARGIN_ARGV_RIGHT;
      break;
    case 'b':
      if (0!=__GBADEV_INTERNAL__TextEngine_Strncmp_CaseInsensitive(
                                                            buf,
                                                            "bottom",
                                                            LSTRLEN("bottom")))
        return TEXT_ENGINE_GBADEV_ESCAPE_INVALID;
      seq |= TEXT_ENGINE_GBADEV_ESCAPE_MARGIN_ARGV_BOTTOM;
      break;
    default:
      return TEXT_ENGINE_GBADEV_ESCAPE_INVALID;
    }
    if ('='!=*pstr)
      return TEXT_ENGINE_GBADEV_ESCAPE_INVALID;
    *iostr = ++pstr;
    return seq;
  }
  if ((TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_PAL==seq) ||
      (TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_IDXMAP == seq)) {
    char *endp = NULL;
    u32 idx = strtoull(pstr, &endp, 10);
    if (NULL==endp || pstr==endp)
      return TEXT_ENGINE_GBADEV_ESCAPE_INVALID;
    pstr = endp;
    if ('=' != *pstr)
      return TEXT_ENGINE_GBADEV_ESCAPE_INVALID;
    *iostr = ++pstr;
    idx <<= TEXT_ENGINE_GBADEV_ESCAPE_PAL_ARGV_SHAMT;
    idx &= TEXT_ENGINE_GBADEV_ESCAPE_PAL_ARGV_MASK;
    seq |= idx;
    return seq;
  }
  return seq;

}




BOOL __GBADEV_INTERNAL__TextEngine_GBADevEscapes_ParseULong(u32 *dst,
                                              const char **iostr,
                                              u32 base) {
  u32 dstlocal=0, c;
  const char *str = *iostr;
  if (8==base) {
    if ('0'!=*str)
      return FALSE;
    for (c=*++str; '0'==c; c = *++str)
      continue;
    if ('7' < c || '0' > c) {
      *dst = 0;
      *iostr = str;
      return TRUE;
    }
    do {
      dstlocal <<= 3;
      dstlocal |= c&7;
    } while (c=*++str, 0!=c && 8 > (c&7));
    *dst = dstlocal;
    *iostr = str;
    return TRUE;
  }
  if (10==base) {
    c = *str++;
    if ('0'==c || !isdigit(c))
      return FALSE;
    dstlocal = c&15;
    for (c=*str; isdigit(c); c=*++str) {
      dstlocal *= 10;
      dstlocal |= c&15;
    }
    *dst=dstlocal;
    *iostr = str;
    return TRUE;
  }
  if (16==base) {
    c=*str++;
    if ('0'!=c) {
      if ('#'!=c)
        return FALSE;
    } else if (c=tolower(*str++), 'x' != c) {
      return FALSE;
    }
    for (c = *str; c && isxdigit(c); c = *++str) {
      dstlocal <<= 4;
      if (isdigit(c)) {
        dstlocal |= c&15;
      } else {
        dstlocal |= 9 + (c&7);
      }
    }
    *dst = dstlocal;
    *iostr = str;
    return TRUE;
  }
  return FALSE;
}

#define ADVANCE_PAST_WHITESPACE(str)  \
  while (isspace((u32)*str)) ++str


BOOL __GBADEV_INTERNAL__TextEngine_GBADevEscapes_SetMargins(
                                      TextEngine_TextSurface_t *surface,
                                      TextEngine_GBADev_EscapeSequences_e seq,
                                      const char **iostr) {
  if (0 == (seq&TEXT_ENGINE_GBADEV_ESCAPE_MARGIN_ARGV_MASK)) {
    const char *pstr = *iostr;
    u32 cur_margin,c;
    // Margin change atomic, so store new margins in a tmp array, and only 
    // transfer them over if for loop succeeds 
    u32 new_margins[4];
    for (u32 i=0; ;) {
      ADVANCE_PAST_WHITESPACE(pstr);
      if (!__GBADEV_INTERNAL__TextEngine_GBADevEscapes_ParseULong(&cur_margin,
                                                                  &pstr,
                                                                  10))
        return FALSE;
      ADVANCE_PAST_WHITESPACE(pstr);
      c=*pstr;
      new_margins[i++] = cur_margin;
      if (TEXT_ENGINE_TEXT_SURFACE_MARGIN_CT > i) {
        if (','!=c) {
          return FALSE;
        }
        ++pstr;
        continue;
      } else {
        break;
      }  
    }
    if (new_margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_LEFT]
        >= new_margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_RIGHT])
      return FALSE;
    if (new_margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_TOP]
        >= new_margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_BOTTOM])
      return FALSE;
    if (new_margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_RIGHT] > surface->width
        || new_margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_LEFT] >= surface->width)
      return FALSE;
    if (new_margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_BOTTOM] > surface->height
        || new_margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_TOP] >= surface->height)
      return FALSE;

    // Commit margin changes
    for (u32 i=0; TEXT_ENGINE_TEXT_SURFACE_MARGIN_CT>i; ++i) {
      surface->margins[i] = new_margins[i];
    }
    *iostr = pstr;
    return TRUE;
  }
  const char *pstr = *iostr;
  u32 margin, idx;
  ADVANCE_PAST_WHITESPACE(pstr);
  if (!__GBADEV_INTERNAL__TextEngine_GBADevEscapes_ParseULong(&margin, 
                                                              &pstr,
                                                              10))
    return FALSE;
  ADVANCE_PAST_WHITESPACE(pstr);
  switch (seq&TEXT_ENGINE_GBADEV_ESCAPE_MARGIN_ARGV_MASK) {
  case TEXT_ENGINE_GBADEV_ESCAPE_MARGIN_ARGV_LEFT:
    if (margin >= surface->width)
      return FALSE;
    if (margin >= surface->margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_RIGHT])
      return FALSE;
    idx = TEXT_ENGINE_TEXT_SURFACE_MARGIN_LEFT;
    break;
  case TEXT_ENGINE_GBADEV_ESCAPE_MARGIN_ARGV_TOP:
    if (margin >= surface->height)
      return FALSE;
    if (margin >= surface->margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_BOTTOM])
      return FALSE;
    idx = TEXT_ENGINE_TEXT_SURFACE_MARGIN_TOP;
    break;
  case TEXT_ENGINE_GBADEV_ESCAPE_MARGIN_ARGV_RIGHT:
    if (margin > surface->width)
      return FALSE;
    if (margin <= surface->margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_LEFT])
      return FALSE;
    idx = TEXT_ENGINE_TEXT_SURFACE_MARGIN_RIGHT;
    break;
  case TEXT_ENGINE_GBADEV_ESCAPE_MARGIN_ARGV_BOTTOM:
    if (margin > surface->height)
      return FALSE;
    if (margin <= surface->margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_TOP])
      return FALSE;
    idx = TEXT_ENGINE_TEXT_SURFACE_MARGIN_BOTTOM;
    break;
  default:
    return FALSE;
  }
  if (4 <= idx)
    return FALSE;
  

  
  surface->margins[idx] = (u16)(margin&0xFFFF);
  *iostr = pstr;
  return TRUE;
}

BOOL __GBADEV_INTERNAL__TextEngine_GBADevEscapes_PrintPresetString(
                                                         TextEngine_Ctx_t *ctx,
                                                         const char **iostr) {
  const char *str = *iostr;
  u32 idx;
  if (NULL==ctx->predef_string_table || 0UL == ctx->prdef_string_table_size)
    return FALSE;
  ADVANCE_PAST_WHITESPACE(str);
  if (!__GBADEV_INTERNAL__TextEngine_GBADevEscapes_ParseULong(&idx, &str, 10))
    return FALSE;
  ADVANCE_PAST_WHITESPACE(str);
  if (ctx->prdef_string_table_size <= idx) {
    return FALSE;
  }
  if (NULL == ctx->predef_string_table[idx])
    return FALSE;
  __GBADEV_INTERNAL__TextEngine_ProcessString(ctx,
                                              ctx->predef_string_table[idx]);
  *iostr = str;
  return TRUE;
}

BOOL __GBADEV_INTERNAL__TextEngine_GBADevEscapes_SetFont(TextEngine_Ctx_t *ctx,
                                                          const char **iostr) {
  const char *str = *iostr;
  u32 idx;
  ADVANCE_PAST_WHITESPACE(str);
  if (NULL==ctx->font_registry || 0UL == ctx->font_registry_size)
    return FALSE;
  if (!__GBADEV_INTERNAL__TextEngine_GBADevEscapes_ParseULong(&idx, &str, 10))
    return FALSE;
  ADVANCE_PAST_WHITESPACE(str);
  if (ctx->font_registry_size <= idx) {
    return FALSE;
  }
  if (NULL == ctx->font_registry[idx])
    return FALSE;
  
  ctx->current_font = ctx->font_registry[idx];
  *iostr = str;
  return TRUE;
}

BOOL __GBADEV_INTERNAL__TextEngine_GBADevEscape_SetPal(
                                                      TextEngine_Ctx_t *ctx,
                                                      u32 pal_idx,
                                                      const char **iostr) {
  if (NULL == ctx->surface.pal)
    return FALSE;

  if (ctx->surface.type&TEXT_ENGINE_TEXT_SURFACE_INDEXED16) {
    if (16 <= pal_idx)
      return FALSE;
  } else if (ctx->surface.type&TEXT_ENGINE_TEXT_SURFACE_RAWXBGR1555) {
    if (ctx->surface.pal_clr_ct <= pal_idx)
      return FALSE;
  }
  const char *str = *iostr;
  u32 clr;
  ADVANCE_PAST_WHITESPACE(str);
  if (!__GBADEV_INTERNAL__TextEngine_GBADevEscapes_ParseULong(&clr, &str, 16))
    return FALSE;
  ADVANCE_PAST_WHITESPACE(str);
  ctx->surface.pal[pal_idx] = clr&0xFFFF;
  *iostr = str;
  return TRUE;
}

BOOL __GBADEV_INTERNAL__TextEngine_GBADevEscape_SetIdxMap(
                                                      TextEngine_Ctx_t *ctx,
                                                      u32 pal_idx,
                                                      const char **iostr) {
  if (NULL == ctx->surface.pal)
    return FALSE;

  if (ctx->current_font->pal_color_count <= pal_idx)
    return FALSE;
  
  const char *str = *iostr;
  u32 mapped_idx;
  ADVANCE_PAST_WHITESPACE(str);
  if (!__GBADEV_INTERNAL__TextEngine_GBADevEscapes_ParseULong(&mapped_idx,
                                                              &str,
                                                              10))
    return FALSE;
  ADVANCE_PAST_WHITESPACE(str);
  ctx->surface.idx_map[pal_idx] = mapped_idx&0xFF;
  *iostr = str;
  return TRUE;

  
}

BOOL __GBADEV_INTERNAL__TextEngine_GBADevEscape_SetCursor(
                                                      TextEngine_Ctx_t *ctx,
                                                      u32 set_which_flags,
                                                      const char **iostr) {
  const char *str = *iostr;
  u32 loc;
  ADVANCE_PAST_WHITESPACE(str);
  if (!__GBADEV_INTERNAL__TextEngine_GBADevEscapes_ParseULong(&loc, &str, 10))
    return FALSE;
  ADVANCE_PAST_WHITESPACE(str);
  if (set_which_flags&SET_X) {
    if (loc >=
        (u32)(ctx->surface.margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_RIGHT]
         - ctx->surface.margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_LEFT]))
      return FALSE;
    ctx->surface.cursor.x = (i16)loc;
  } else if (set_which_flags&SET_Y) {
    if (loc >= 
        (u32)(ctx->surface.margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_BOTTOM]
         -ctx->surface.margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_TOP]))
      return FALSE;
    ctx->surface.cursor.y = (i16)loc;
    *iostr = str;
    return TRUE;
  } else {
    return FALSE;
  }
  if (0==(set_which_flags&SET_Y)) {
    *iostr = str;
    return TRUE;
  }
  if (','!=*str)
    return FALSE;


  ADVANCE_PAST_WHITESPACE(str);
  if (!__GBADEV_INTERNAL__TextEngine_GBADevEscapes_ParseULong(&loc, &str, 10))
    return FALSE;
  ADVANCE_PAST_WHITESPACE(str);

  if (loc >= 
      (u32)(ctx->surface.margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_BOTTOM]
       -ctx->surface.margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_TOP]))
    return FALSE;
  ctx->surface.cursor.y = (i16)loc;
  *iostr = str;
  return TRUE;
  
}

BOOL __GBADEV_INTERNAL__TextEngine_GBADevEscape_RenderClear(
                                                      TextEngine_Ctx_t *ctx) {
  const Rect_t r = {
    .x = ctx->surface.margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_LEFT],
    .y = ctx->surface.margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_TOP],
    .w = ctx->surface.margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_RIGHT]
          - ctx->surface.margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_LEFT],
    .h = ctx->surface.margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_BOTTOM]
          - ctx->surface.margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_TOP]
  };
  ctx->clear_render_cb(&r,&ctx->surface, ctx->userdata);
  return TRUE;
}

BOOL __GBADEV_INTERNAL__TextEngine_GBADevEscapes_SetFGBG(
                                             TextEngine_TextSurface_t *surface,
                                             u32 fgbg,
                                             const char **iostr) {
  
  const char *str = *iostr;
  u32 clr;
  ADVANCE_PAST_WHITESPACE(str);
  if (!__GBADEV_INTERNAL__TextEngine_GBADevEscapes_ParseULong(&clr, &str, 10))
    return FALSE;
  ADVANCE_PAST_WHITESPACE(str);
  if (clr!=(0xFFFF&clr))
    return FALSE;
  if (NULL == surface->pal)
    return FALSE;
  surface->pal[fgbg] = clr&0xFFFF;
  *iostr = str;
  return TRUE;
}

BOOL __GBADEV_INTERNAL__TextEngine_GBADevEscape_SetPalBank(
                                     TextEngine_TextSurface_t *surface,
                                     const char **iostr) {
  const char *str = *iostr;
  u32 pb;
  ADVANCE_PAST_WHITESPACE(str);
  if (!__GBADEV_INTERNAL__TextEngine_GBADevEscapes_ParseULong(&pb, &str, 10))
    return FALSE;
  ADVANCE_PAST_WHITESPACE(str);
  surface->cur_palbank = pb&15;
  *iostr = str;
  return TRUE;
}

BOOL __GBADEV_INTERNAL__TextEngine_GBADevEscape_ResetCtxState(
                                                      TextEngine_Ctx_t *ctx) {
  const u16 *fontpal = ctx->current_font->pal;
  u16 *surfpal = ctx->surface.pal;
  u32 pal_clr_ct = ctx->current_font->pal_color_count;
  const u32 
    ENCODING_FMT = (ctx->surface.type&TEXT_ENGINE_TEXT_SURFACE_ENCODING_MASK);
  ctx->surface.margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_LEFT] = 0;
  ctx->surface.margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_TOP] = 0;
  ctx->surface.margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_RIGHT] 
    = ctx->surface.width;
  ctx->surface.margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_BOTTOM]
    = ctx->surface.height;
  if (NULL!=ctx->font_registry && 0<ctx->font_registry_size) {
    const TextEngine_Font_t *def_font = ctx->font_registry[0];
    if (ENCODING_FMT == TEXT_ENGINE_TEXT_SURFACE_INDEXED16) {
      if (def_font->bitdepth > 4)
        return FALSE;
    }
    
    ctx->current_font = ctx->font_registry[0];
  }
  ctx->surface.cursor = (Coord_t) { 0, 0 };
  Fast_Memcpy32(surfpal, fontpal, sizeof(u16)*pal_clr_ct/sizeof(WORD));
  if (pal_clr_ct&1)
    surfpal[pal_clr_ct-1] = fontpal[pal_clr_ct-1];
  if (TEXT_ENGINE_TEXT_SURFACE_INDEXED16 == ENCODING_FMT)
    ctx->surface.cur_palbank = 0;
  if (NULL == ctx->surface.idx_map)
    return TRUE;
  u16 *imap2 = NULL, cur = 0;
  if (1UL&((UIPTR_T)ctx->surface.idx_map)) {
    --pal_clr_ct;
    ctx->surface.idx_map[0] = 0;
    cur = 0x0201;
    imap2 = (u16*)(&ctx->surface.idx_map[1]);
  } else {
    imap2 = (u16*)ctx->surface.idx_map;
    cur = 0x0100;
  }
  const u32 IMAP_CT2 = pal_clr_ct>>1;
  for (u32 i = 0; IMAP_CT2 > i; ++i, cur+=0x0101)
    *imap2++ = cur;
  if (pal_clr_ct&1) {
    *(u8*)imap2 = pal_clr_ct-1;
  }
  return TRUE;
}


BOOL __GBADEV_INTERNAL__TextEngine_GBADevEscape_MoveCursor(
                                                      TextEngine_Ctx_t *ctx,
                                                      const char **iostr) {
  const char *str = *iostr;
  u32 arg;
  i16 x, y;


  ADVANCE_PAST_WHITESPACE(str);
  if ('-' == *str) {
    ++str;
    x = 0x8000;
  } else {
    x = 0;
  }
  if (!__GBADEV_INTERNAL__TextEngine_GBADevEscapes_ParseULong(&arg, &str, 10))
    return FALSE;
  ADVANCE_PAST_WHITESPACE(str);
  x |= (arg&0x7FFF);
  if (','!=*str)
    return FALSE;
  ADVANCE_PAST_WHITESPACE(str);
  if ('-' == *str) {
    ++str;
    y = 0x8000;
  } else {
    y = 0;
  }

  if (!__GBADEV_INTERNAL__TextEngine_GBADevEscapes_ParseULong(&arg, &str, 10))
    return FALSE;
  ADVANCE_PAST_WHITESPACE(str);
  y |= (arg&0x7FFF);
  x *= ctx->current_font->glyph_width, y *= ctx->current_font->glyph_height;
  Coord_t coord = ctx->surface.cursor;
  coord.x += x, coord.y += y;
  if (0 > coord.x || 0 > coord.y)
    return FALSE;
  const u32 WDTH = ctx->surface.margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_RIGHT]
                  - ctx->surface.margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_LEFT],
            HGHT = ctx->surface.margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_BOTTOM]
                  - ctx->surface.margins[TEXT_ENGINE_TEXT_SURFACE_MARGIN_TOP];

  if (WDTH <= (u32)coord.x)
    return FALSE;
  if (HGHT <= (u32)coord.y)
    return FALSE;

  ctx->surface.cursor = coord;
  *iostr = str;
  return TRUE;
}


BOOL __GBADEV_INTERNAL__TextEngine_ProcessEscapeChars_GBADev(
                                                         TextEngine_Ctx_t *ctx,
                                                         const char **iostr) {
  const char *str = *iostr;
  u32 c = *str++;
  TextEngine_GBADev_EscapeSequences_e seq;
  if ('['!= c)
    return FALSE;
  for (c=*str++; ']'!=c && '\0' != c; c=*str++) {
    if (isspace(c))
      continue;
    if ('$'!=c)
      return FALSE;
    seq = __GBADEV_INTERNAL__TextEngine_ParseEscapeID(&str);
    if (TEXT_ENGINE_GBADEV_ESCAPE_INVALID&seq)
      return FALSE;
    switch ((TextEngine_GBADev_EscapeSequences_e)
            (seq&TEXT_ENGINE_GBADEV_ESCAPE_TYPE_MASK)) {
    case TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_PAL:
      if (!__GBADEV_INTERNAL__TextEngine_GBADevEscape_SetPal(
                                ctx,
                                (seq&TEXT_ENGINE_GBADEV_ESCAPE_PAL_ARGV_MASK)
                                  >>TEXT_ENGINE_GBADEV_ESCAPE_PAL_ARGV_SHAMT,
                                &str))
        return FALSE;
      continue; 
    case TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_CURSOR_X:
      if (!__GBADEV_INTERNAL__TextEngine_GBADevEscape_SetCursor(ctx, 
                                                                SET_X,
                                                                &str))
        return FALSE;
      continue;
    case TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_CURSOR_Y:
      if (!__GBADEV_INTERNAL__TextEngine_GBADevEscape_SetCursor(ctx, 
                                                                SET_Y,
                                                                &str))
        return FALSE;
      continue;
    case TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_CURSOR_XY:
      if (!__GBADEV_INTERNAL__TextEngine_GBADevEscape_SetCursor(ctx, 
                                                                SET_BOTH,
                                                                &str))
        return FALSE;
      continue;
    case TEXT_ENGINE_GBADEV_ESCAPE_TYPE_CLEAR_RENDERBOX:
      if (!__GBADEV_INTERNAL__TextEngine_GBADevEscape_RenderClear(ctx))
        return FALSE;
      continue;
    case TEXT_ENGINE_GBADEV_ESCAPE_TYPE_RESET:
      if (!__GBADEV_INTERNAL__TextEngine_GBADevEscape_ResetCtxState(ctx))
        return FALSE;
      continue;
    case TEXT_ENGINE_GBADEV_ESCAPE_TYPE_MOVE_CURSOR:
      if (!__GBADEV_INTERNAL__TextEngine_GBADevEscape_MoveCursor(ctx, &str))
        return FALSE;
      continue;
    case TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_FG:
      if (!__GBADEV_INTERNAL__TextEngine_GBADevEscapes_SetFGBG(&ctx->surface,
                                                               1,
                                                               &str))
        return FALSE;
      continue;
    case TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_BG:
      if (!__GBADEV_INTERNAL__TextEngine_GBADevEscapes_SetFGBG(&ctx->surface,
                                                               0,
                                                               &str))
        return FALSE;
      continue;
    case TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_PALBANK:
      if (!__GBADEV_INTERNAL__TextEngine_GBADevEscape_SetPalBank(&ctx->surface, 
                                                                 &str))
        return FALSE;
      continue;
    case TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_IDXMAP:
      if (!__GBADEV_INTERNAL__TextEngine_GBADevEscape_SetIdxMap(
                                ctx,
                                (seq&TEXT_ENGINE_GBADEV_ESCAPE_PAL_ARGV_MASK)
                                  >>TEXT_ENGINE_GBADEV_ESCAPE_PAL_ARGV_SHAMT,
                                &str))
        return FALSE;
      continue; 
    case TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_FONT:
      if (!__GBADEV_INTERNAL__TextEngine_GBADevEscapes_SetFont(ctx, &str))
        return FALSE;
      continue;
    case TEXT_ENGINE_GBADEV_ESCAPE_TYPE_SET_MARGINS:
      if (!__GBADEV_INTERNAL__TextEngine_GBADevEscapes_SetMargins(&ctx->surface,
                                                                  seq,
                                                                  &str))
        return FALSE;
      continue;
    case TEXT_ENGINE_GBADEV_ESCAPE_TYPE_PRINT_PRESET_STR:

      if (!__GBADEV_INTERNAL__TextEngine_GBADevEscapes_PrintPresetString(ctx,
                                                                          &str))
        return FALSE;
      continue;
    default:
      return FALSE;
    }
  }
  *iostr = str;
  return ']' == c;
}






TextEngine_Font_Glyph_t *__GBADEV_INTERNAL__TextEngine_LookupGlyph(
                                                TextEngine_Font_Glyph_t *dst,
                                                const TextEngine_Font_t *font,
                                                u32 code_pt) {
  u32 idx = code_pt - font->glyph0_char_code;
  if (font->glyph_count <= idx) {
    idx = font->errglyph_idx;
    if (font->glyph_count <= idx)
      return NULL;
  }
  // All static fields already filled, so only need the fields that depend on
  // glyph, rather than font.
  dst->data = (const void*)
    (((const uint8_t*)font->glyph_data) + idx*font->cell_size);
  if (NULL != font->glyph_widths)
    dst->width = font->glyph_widths[idx];
  if (NULL != font->glyph_heights)
    dst->height = font->glyph_heights[idx];
  return dst;
}

int __GBADEV_INTERNAL__TextEngine_ProcessString(TextEngine_Ctx_t *ctx,
                                                const char *str) {
  if (NULL == ctx || NULL == str)
    return -1;
  const TextEngine_Font_t *font = ctx->current_font;
  if (NULL == font)
    return -1;
  TextEngine_GlyphRender_cb render_cb = ctx->glyph_render_cb;
  if (NULL == render_cb)
    return -1;
  TextEngine_Font_Glyph_t glyph_info = {
    .data = NULL,
    .width = font->glyph_width,
    .height = font->glyph_height,
    .max_width = font->glyph_width,
    .max_height = font->glyph_height,
    .cell_pitch = font->cell_pitch,
    .bpp = font->bitdepth,
    .bitpack_big_endian = font->bitpack_big_endian
  };
  Coord_t *cursor = &ctx->surface.cursor;
  u32 code_pt;
  int ret = 0;
  while (code_pt = *str++, 0!=code_pt) {
    if (code_pt < font->glyph0_char_code) {
      if ((unsigned)(code_pt - '\x1b') <= 1) {
        switch (code_pt) {
        case '\x1b':
          if (!__GBADEV_INTERNAL__TextEngine_ProcessEscapeChars_ANSI(ctx,
                                                                     &str)) {
            break;
          }
          continue;
        case '\x1c':
          if (!__GBADEV_INTERNAL__TextEngine_ProcessEscapeChars_GBADev(ctx,
                                                                       &str)) {
            break;
          }
          continue;
        default:
          break;
        }
        // If we broke and didn't continue, then failure scenario:
        ret = -1;
        break;
      } else if ('\n' == code_pt) {
        cursor->y += font->glyph_height;  // De facto max height field
        cursor->x = 0;
        continue;
      } else if ('\t' == code_pt) {
        if (NULL == __GBADEV_INTERNAL__TextEngine_LookupGlyph(&glyph_info,
                                                              font,
                                                              ' ')) {
          ret = -1;
          break;
        }
        for (code_pt=0; 4>code_pt; ++code_pt) {
          if (!render_cb(&glyph_info,
                         &ctx->surface,
                         ctx->userdata)) {
            ret = -1;
            break;
          }
        }
        if (-1 == ret)
          break;
        else
          continue;
      } else {
        ret = -1;
        break;
      }
    }
    if (NULL == __GBADEV_INTERNAL__TextEngine_LookupGlyph(&glyph_info,
                                                          font,
                                                          code_pt)) {
      ret = -1;
      break;
    }
    if (!render_cb(&glyph_info,
                          &ctx->surface,
                          ctx->userdata)) {
      ret = -1;
      break;
    }
    ++ret;
  }
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
      bitdepth = 4<<REG_GET_FLAG(bg_flags, BG_CNT, 8BPP);
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

  
  ctx->surface = (TextEngine_TextSurface_t) {
    .sdata = cbb,
    .screen_ent_data = SCR_ENT_MEM[scr_block_base],
    .pal = PAL_MEM_BG,
    .idx_map = NULL,
    .margins = {
      0,0,SCREEN_WIDTH, SCREEN_HEIGHT
    },
    .pitch = bitdepth,
    .width = SCREEN_WIDTH,
    .height = SCREEN_HEIGHT,
    .type = 4==bitdepth ? TEXT_ENGINE_TEXT_SURFACE_TYPE_4BPP_TILEMAP
                        : TEXT_ENGINE_TEXT_SURFACE_TYPE_8BPP_TILEMAP,
    .pal_clr_ct = 1<<bitdepth,
    .cur_palbank = 0,
    .flags = 0
  };

  ctx->glyph_render_cb = TextEngine_DefaultRenderCallback_Tilemap;
  ctx->clear_render_cb = TextEngine_DefaultClearCallback_Tilemap;
   
  REG_DPY_CNT |= REG_FLAG(DPY_CNT, BG0)<<bg_idx;
  return TRUE;
}

BOOL TextEngine_RenderCallbacks_UseDefault_BMP(TextEngine_Ctx_t *ctx,
                                               u16 *pal_buffer,
                                               u16 bmp_mode,
                                               u16 pal_buffer_slot_count,
                                               u16 flags) {
  if (NULL == ctx)
    return FALSE;
  if (NULL == ctx->current_font)
    return FALSE;
  void *startp;
  u32 width, height, pitch, type;
  if (5==bmp_mode) {
    width = 160;
    height = 128;
    pitch = M5_SCREEN_WIDTH*2;
    type = TEXT_ENGINE_TEXT_SURFACE_TYPE_BMP_MODE5;
    startp = VRAM_M5_P1;
  } else {
    width = 240;
    height = 160; 
    if (4==bmp_mode) {
      pal_buffer = PAL_MEM_BG;
      pal_buffer_slot_count = 128;
      pitch = M4_SCREEN_WIDTH;
      type = TEXT_ENGINE_TEXT_SURFACE_TYPE_BMP_MODE4;

      startp = VRAM_M4_P1;
    } else if (3 == bmp_mode) {
      pitch = M3_SCREEN_WIDTH*2;
      type = TEXT_ENGINE_TEXT_SURFACE_TYPE_BMP_MODE3;
      startp = VRAM_M3;
    } else {
      return FALSE;  // If not 3,4, or 5 then not a valid BMP mode!
    }
  }
  if (0!=(TEXT_ENGINE_TEXT_SURFACE_RAWXBGR1555&type)) {
    if (3UL&(UIPTR_T)pal_buffer)
      return FALSE;  // Ensure that user-provided pal buffer is word-aligned.
  }
  if (NULL == pal_buffer) {
    return FALSE;  // pal_buffer can be passed NULL iff bmp_mode == 4, as it 
                   // will just use pal memory anyway, and pal will be set to
                   // that mem ptr in the above if branching logic, but 
                   // otherwise, if it was passed NULL *and* bmp_mode is a 
                   // 16bpp mode, then a pal buffer is needed, and so it'll
                   // be NULL here only in these scenarios and thus is a
                   // failure case.
  }
  {
    const u16 *fpal = ctx->current_font->pal;
    u32 fpal_clr_ct = ctx->current_font->pal_color_count;
    if (pal_buffer_slot_count < fpal_clr_ct)
      return FALSE;  // Need a pal buffer large enough to fully accommodate full
                   // font palette.

    Fast_Memcpy32(pal_buffer,
                  fpal,
                  fpal_clr_ct*sizeof(u16)/sizeof(WORD));
    if (fpal_clr_ct&1)
      pal_buffer[fpal_clr_ct-1] = fpal[fpal_clr_ct-1];
  }
  ctx->surface = (TextEngine_TextSurface_t) {
    .sdata = startp,
    .pal = pal_buffer,
    .idx_map = NULL,
    .margins = { 0,0,width,height },
    .cursor = { 0,0 },
    .pitch = pitch,
    .width = width,
    .height = height,
    .type = type,
    .pal_clr_ct = pal_buffer_slot_count,
    .flags = flags
  };
  ctx->userdata = NULL;
  ctx->glyph_render_cb = TextEngine_DefaultRenderCallback_BMP;
  ctx->clear_render_cb = TextEngine_DefaultClearCallback_BMP;
  REG_DPY_CNT &= REG_FIELD_RESET_MASK(DPY_CNT, MODE)
                  & REG_FLAG_RESET_MASK(DPY_CNT, BG0)
                  & REG_FLAG_RESET_MASK(DPY_CNT, BG1)
                  & REG_FLAG_RESET_MASK(DPY_CNT, BG2)
                  & REG_FLAG_RESET_MASK(DPY_CNT, BG3)
                  & REG_FLAG_RESET_MASK(DPY_CNT, BLANK);
  REG_DPY_CNT |= REG_VALUE(DPY_CNT, MODE, bmp_mode)|REG_FLAG(DPY_CNT, BG2);
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
  ret = __GBADEV_INTERNAL__TextEngine_ProcessString(ctx, s);
  free(s);
  return ret;
}

int TextEngine_Puts(const char *__restrict s, TextEngine_Ctx_t *ctx) {
  if (NULL == ctx) return -1;
  if (NULL == s) return -1;

  return __GBADEV_INTERNAL__TextEngine_ProcessString(ctx, s);
}

int TextEngine_Putchar(u32 c, TextEngine_Ctx_t *ctx) {
  char buf[2] = { c, '\0' };
  return __GBADEV_INTERNAL__TextEngine_ProcessString(ctx, buf);
}
