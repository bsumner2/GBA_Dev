/** (C) 29 of December, 2025 Burt Sumner */
/** Free to use, but this copyright message must remain here */

#ifndef _GBADEV_STDIO_PORTABLE_H_
#define _GBADEV_STDIO_PORTABLE_H_

#include <GBAdev_text_engine.h>
#ifdef __cplusplus
extern "C" {
#endif  /* C++ Name mangler guard */

#include <sys/iosupport.h>

/**
 * @brief Pipe TextEngine into a data output stream.
 * @param fd Filedescriptor for either stdout or stderr. Accepts no other values.
 * @param ctx The Text Engine Ctx static or heap instance that you 
 * want hooked up to this data stream
 * */
BOOL TextEngine_StdioPortable_PipeOutstreamToTextCtx(int fd, 
                                                     TextEngine_Ctx_t *ctx);

#ifdef __cplusplus
}
#endif  /* C++ Name mangler guard */

#endif  /* _GBADEV_STDIO_PORTABLE_H_ */
