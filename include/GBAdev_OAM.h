/** (C) 25 of December, 2025 Burt Sumner */
/** Free to use, but this copyright message must remain here */

#ifndef _GBADEV_OAM_H_
#define _GBADEV_OAM_H_

#include <GBAdev_functions.h>
#include <GBAdev_types.h>
#ifdef __cplusplus
extern "C" {
#endif  /* C++ Name mangler guard */


BOOL OAM_Init(Obj_Attr_t *obj_attrs, u32 count);

BOOL Obj_Affine_Transform_Copy(Obj_Affine_Transform_t *dst, 
    const Obj_Affine_Transform_t *src, 
    u32 count);



INLINE void OAM_Copy(Obj_Attr_t *dst, const Obj_Attr_t *src, u32 count);


INLINE void OAM_Copy(Obj_Attr_t *dst, const Obj_Attr_t *src, u32 count) {
  Fast_Memcpy32(dst, src, count<<1);
}

#ifdef __cplusplus
}
#endif  /* C++ Name mangler guard */

#endif  /* _GBADEV_OAM_H_ */
