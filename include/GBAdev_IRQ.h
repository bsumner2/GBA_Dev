/** (C) 25 of December, 2025 Burt Sumner */
/** Free to use, but this copyright message must remain here */

#ifndef _GBADEV_IRQ_H_
#define _GBADEV_IRQ_H_

#include <GBAdev_types.h>
#include <GBAdev_util_macros.h>
#ifdef __cplusplus
extern "C" {
#endif  /* C++ Name mangler guard */

/**
 * @brief Offers a basic ISR callback function, that simply acknowledges 
 * incoming interrupts before exiting.
 * */
IWRAM_CODE void ISR_Handler_Basic(void);


/**
 * @brief Offers the most complex of the three options. This ISR callback will
 * acknowledge the interrupt, and will call whichever callback function has been
 * registered to the respective interrupt sender. In addition to calling 
 * callbacks, this ISR allows for nesting of interrupts (e.g.: vblank interrupt
 * can be triggered during another IRQ's callback function execution).
 * */
IWRAM_CODE void ISR_Handler_Switchboard_Nested(void);

/**
 * @brief Acks interrupts AND calls a callback function registered to a given
 * IRQ source callback if one is registered; however, this version of the
 * ISR Handler function collection does not allow for nested interrupts.
 * */
IWRAM_CODE void ISR_Handler_Switchboard(void);

/**
 * @brief add callback for given IRQ type flag, with given priority level
 * ----------------------------------------------------------------------------
 * @param irq_type Type of IRQ to register this callback with.
 * ----------------------------------------------------------------------------
 * @param priority_lvl Priority level of the ISR. Higher priority => IRQ
 * takes precedence over others.
 * ----------------------------------------------------------------------------
 * @param cb The callback to be called when given IRQ type is triggered and 
 * detected as highest priority ISR callback in function registry.
 * ----------------------------------------------------------------------------
 * */
void ISR_Add_Callback(IRQ_Index_t irq_type, u32 priority_lvl, void (*cb)(void));

void ISR_Remove_Callback(IRQ_Index_t irq_type);
#ifdef __cplusplus
}
#endif  /* C++ Name mangler guard */

#endif  /* _GBADEV_IRQ_H_ */
