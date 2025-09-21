    .section .rodata
    .align 2 
    .global __GBADEV_IRQ_ENABLE_BITS
    .type __GBADEV__IRQ_ENABLE_BITS %object
__GBADEV_IRQ_ENABLE_BITS:
    .short 0x0004, 0x0008, 0x0004, 0x0010, 0x0004, 0x0020, 0x0102, 0x0040
    .short 0x0106, 0x0040, 0x010A, 0x0040, 0x010E, 0x0040, 0x0128, 0x4000
    .short 0x00BA, 0x4000, 0x00C6, 0x4000, 0x00D2, 0x4000, 0x00DE, 0x4000
    .short 0x0132, 0x4000, 0x0000, 0x0000
    .size __GBADEV__IRQ_ENABLE_BITS, .-__GBADEV_IRQ_ENABLE_BITS
    
    .section .bss
    .align 2
    .global __GBADEV__ISR_CALLBACK_TABLE
    .type __GBADEV__ISR_CALLBACK_TABLE %object
    /* __GBADEV__ISR_CALLBACK_TABLE::struct {u32 flag; void (*cb)(void)}[15] */
__GBADEV__ISR_CALLBACK_TABLE:
    .space 120, 0
    .size __GBADEV__ISR_CALLBACK_TABLE, 120

    .section .iwram, "ax", %progbits
    .arm
    .align 2
    .global ISR_Handler_Basic
    .type ISR_Handler_Basic %function
ISR_Handler_Basic:
    MOV r0, #0x04000000
    /* *(r0+=0x0200):=*((volatile u32*)0x04000200 = REG_IF<<16|REG_IE */
    LDR r12, [r0, #0x0200]!
    AND r2, r12, r12, LSR #16  /* r2 = REG_IE&REG_IF */
    STRH r2, [r0, #2]  /* REG_IF = REG_IE&REG_IF := REG_IF&=REG_IE */
    LDR r3, [r0, #-0x0208]  /* r3 = REG_IFBIOS */
    ORR r3, r3, r2  /* r3 |= REG_IF&REG_IE := r3 = REG_IFBIOS|(REG_IE&REG_IF) */
    STR r3, [r0, #-0x0208]  /* REG_IFBIOS = REG_IFBIOS|(REG_IE&REG_IF) */
    BX lr
    .size ISR_Handler_Basic, .-ISR_Handler_Basic

    .arm
    .align 2
    .global ISR_Handler_Switchboard
    .type ISR_Handler_Switchboard %function
ISR_Handler_Switchboard:
    STMFD sp!, { lr }
    BL ISR_Handler_Basic
    LDR r1, =__GBADEV__ISR_CALLBACK_TABLE
.LISR_handler_switchboard_callback_search:
        LDR r3, [r1], #8
        TST r3, r2
        BNE .LISR_handler_switchboard_matchfound
        CMP r3, #0
        BNE .LISR_handler_switchboard_callback_search
.LISR_handler_switchboard_matchfound:
    LDRNE r1, [r1, #-4]
    CMPNE r1, #0
    LDMEQFD sp!, { r1 }  /* if CB_TABLE[i].callback==NULL: pop retaddr to r1,
                            and return to caller via r1 */
    LDMNEFD sp!, { lr }  /* if CB_TABLE[i].callback!=NULL: pop retaddr to lr, 
                            and just tail call the callback with BX r1 */
    BX r1
    .size ISR_Handler_Switchboard, .-ISR_Handler_Switchboard

    .arm
    .align 2
    .global ISR_Handler_Switchboard_Nested
    .type ISR_Handler_Switchboard_Nested %function
ISR_Handler_Switchboard_Nested:
    STMFD sp!, { lr }
    BL ISR_Handler_Basic
    LDMFD sp!, { lr }
    LDR r1, =__GBADEV__ISR_CALLBACK_TABLE
.LISR_handler_nested_callback_search:
        LDR r3, [r1], #8
        TST r3, r2
        BNE .LISR_handler_nested_matchfound
        CMP r3, #0
        BNE .LISR_handler_nested_callback_search
.LISR_handler_nested_matchfound:
    LDRNE r1, [r1, #-4]
    CMPNE r1, #0
    BXEQ lr

    LDR r3, [r0, #8]
    STRB r0, [r0, #8]
    BIC r2, r12, r2
    STRH r2, [r0]
    
    /* Save current IRQ mode context */
    MRS r2, spsr
    STMFD sp!, { r2-r3, r12, lr }
    /* Set current mode to SYS mode */
    MRS r3, cpsr
    BIC r3, r3, #0xDF
    ORR r3, r3, #0x1F
    MSR cpsr, r3
    /* Call ISR */
    STMFD sp!, { r0, lr }
    MOV lr, pc
    BX r1
    LDMFD sp!, { r0, lr }
    
    STRB r0, [ r0, #8]
    MRS r3, cpsr
    BIC r3, r3, #0xDF
    ORR r3, r3, #0x92
    MSR cpsr, r3

    LDMFD sp!, { r2-r3, r12, lr }
    MSR spsr, r2
    STRH r12, [r0]
    STR r3, [r0, #8]
    BX lr
    .size ISR_Handler_Switchboard_Nested, .-ISR_Handler_Switchboard_Nested
