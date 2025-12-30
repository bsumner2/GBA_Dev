/** (C) 7 of September, 2025 Burt Sumner */
/** Free to use, but this copyright message must remain here */

#ifndef _GBADEV_FUNCTIONS_H_
#define _GBADEV_FUNCTIONS_H_

#include <GBAdev_types.h>
#ifdef __cplusplus
extern "C" {
#endif  /* C++ Name mangler guard */

#define LSTRLEN(lit) (sizeof(lit)-1)

#ifdef __thumb__
#define SUPERVISOR_CALL(idx) __asm volatile ("SVC\t" #idx ::: "r0", "r1", "r2", "r3")
#else
#define SUPERVISOR_CALL(idx) __asm volatile ("SVC\t" #idx "<<16" ::: "r0", "r1", "r2", "r3")
#endif
BOOL SRAM_Write(const void *src, u32 size, u32 offset);
BOOL SRAM_Read(void *dst, u32 size, u32 offset);

IWRAM_CODE void Fast_Memcpy32(void *dst, const void *src, u32 word_ct);
IWRAM_CODE void Fast_Memset32(void *dst, u32 fill_word, u32 word_ct);






#ifdef __cplusplus
}
#endif  /* C++ Name mangler guard */

#endif  /* _GBADEV_FUNCTIONS_H_ */
