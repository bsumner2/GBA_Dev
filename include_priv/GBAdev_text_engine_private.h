/** (C) 30 of December, 2025 Burt Sumner */
/** Free to use, but this copyright message must remain here */

#ifndef _GBADEV_TEXT_ENGINE_PRIVATE_H_
#define _GBADEV_TEXT_ENGINE_PRIVATE_H_

#ifdef __cplusplus
extern "C" {
#endif  /* C++ Name mangler guard */

#define TilemapState_CalculateCurTileIdx(start_x, start_y)  \
  ((start_y/8)*(SCREEN_WIDTH/8) + start_x/8)

#ifdef __cplusplus
}
#endif  /* C++ Name mangler guard */

#endif  /* _GBADEV_TEXT_ENGINE_PRIVATE_H_ */
