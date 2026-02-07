/** (C) 25 of December, 2025 Burt Sumner */
/** Free to use, but this copyright message must remain here */

#ifndef _GBADEV_OAM_H_
#define _GBADEV_OAM_H_

#include <GBAdev_functions.h>
#include <GBAdev_types.h>
#include <GBAdev_memdef.h>
#ifdef __cplusplus
extern "C" {
#endif  /* C++ Name mangler guard */

/**
 * @brief OAM must be a multiple of 4, for sake of also setting up the matrices
 * that 
 **/
BOOL OAM_Init(Obj_Attr_u *obj_attrs, u32 count);

BOOL Obj_Affine_Transform_Copy(Obj_Affine_Transform_t *dst, 
    const Obj_Affine_Transform_t *src, 
    u32 count);

INLINE BOOL Obj_Attr_Copy(Obj_Attr_u *dst, const Obj_Attr_u *src, u32 count);



INLINE void OAM_Copy(Obj_Attr_u *dst, const Obj_Attr_u *src, u32 count);

/**
 * @brief Copy OAM attribute data without affecting affine transform matrices 
 * that are interleaved and may be active.
 **/
BOOL Obj_Attr_Copy(Obj_Attr_u *dst, const Obj_Attr_u *src, u32 count) {
  for (u32 i=0; count>i; ++i) {
		dst[i].attrs.attr0 = src[i].attrs.attr0;
		dst[i].attrs.attr1 = src[i].attrs.attr1;
		dst[i].attrs.attr2 = src[i].attrs.attr2;
  }
  return TRUE;
}

void OAM_Copy(Obj_Attr_u *dst, const Obj_Attr_u *src, u32 count) {
  Fast_Memcpy32(dst, src, count<<1);
}

#ifdef __cplusplus
}
#endif  /* C++ Name mangler guard */

#endif  /* _GBADEV_OAM_H_ */
