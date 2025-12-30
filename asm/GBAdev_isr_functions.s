    .section .rodata
    .align 2 
    .type __GBADEV__IRQ_ENABLE_BITS %object
__GBADEV_IRQ_ENABLE_BITS:
    .short 0x0004, 0x0008, 0x0004, 0x0010, 0x0004, 0x0020, 0x0102, 0x0040
    .short 0x0106, 0x0040, 0x010A, 0x0040, 0x010E, 0x0040, 0x0128, 0x4000
    .short 0x00BA, 0x4000, 0x00C6, 0x4000, 0x00D2, 0x4000, 0x00DE, 0x4000
    .short 0x0132, 0x4000, 0x0000, 0x0000
    .size __GBADEV__IRQ_ENABLE_BITS, .-__GBADEV_IRQ_ENABLE_BITS
    
    .section .bss
    .align 2
    .type __GBADEV__ISR_CALLBACK_TABLE %object
    /* __GBADEV__ISR_CALLBACK_TABLE::struct {
     *      u32 flag; 
     *      u32 priority;
     *      void (*cb)(void);
     * }[15]
     * size: sizeof(struct {u32; u32; void(*)(void);}[15]) = 12*15 = 180 */
__GBADEV__ISR_CALLBACK_TABLE:
    .space 120, 0
    .size __GBADEV__ISR_CALLBACK_TABLE, 120

    .align 2
    .type __GBADEV__ISR_CB_TABLE_NEXT_IDX %object
__GBADEV__ISR_CB_TABLE_NEXT_IDX:
    .space 4, 0
    .size __GBADEV__ISR_CB_TABLE_NEXT_IDX, 4

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
        LDR r3, [r1], #12
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
        LDR r3, [r1], #12
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




    .section .text
    .thumb_func
    .align 2
    .global ISR_Remove_Callback
    .type ISR_Remove_Callback %function
ISR_Remove_Callback:
    // r0: irq type
    // return: was_removed:boolean
    CMP r0, #14
    BGE .LISR_Remove_Callback_Return_NoMatchFound
    MOV r1, #1
    LSL r1, r1, r0  // r1 = 1<<irq_type
    LDR r0, =__GBADEV__ISR_CALLBACK_TABLE
    LDR r3, =__GBADEV__ISR_CB_TABLE_NEXT_IDX
    LDR r2, [r3]
    CMP r2, #0
    BLT .LISR_Remove_Callback_SetBackToZero
    BEQ .LISR_Remove_Callback_Return_NoMatchFound
    MOV r12, r3

    PUSH { r4 }
    MOV r4, #0
.LISR_Remove_Callback_SearchLoop:
        LDR r3, [r0]
        // r0 = &__GBADEV__ISR_CALLBACK_TABLE[i], r1 = irq_flag, 
        // r2 = __GBADEV__ISR_CB_TABLE_NEXT_IDX, r3 = *(u32*)r0, r4 = i
        CMP r3, r1
        BEQ .LISR_Remove_Callback_FoundMatch

        // r0 = &__GBADEV__ISR_CALLBACK_TABLE[r4 = ++i]
        ADD r0, r0, #12
        ADD r4, r4, #1

        // if r4 < r2 <=> i < __GBADEV__ISR_CB_TABLE_NEXT_IDX
        CMP r4, r2
        BLT .LISR_Remove_Callback_SearchLoop
    // If we reach here, no match was found
    POP { r4 }
    MOV r0, #0
    BX lr

.LISR_Remove_Callback_FoundMatch:
    PUSH { r5-r7 }
    // r0 = &callback_table[i], r1 = <Unneeded old data>, 
    // r2 = __GBADEV__ISR_CB_TABLE_NEXT_IDX, r3 = <Unneeded old data>, r4 = i
    ADD r4, r4, #1  // r4 = i+1, and r0 thus becomes &callback_table[i-1]
    MOV r5, #12
    ADD r1, r0, r5  // r1 = &callback_table[i]
    // for r4 (aka i) = match_idx + 1 ; i < NXT_IDX (aka r2); ++i:
    //      callback_table[i-1] = callback_table[i]
    CMP r4, r2
    BGE .LISR_Remove_Callback_Return_MatchFound
.LISR_RemoveCallback_MemMoveLoop:
        LDMIA r1!, { r5-r7 }  // r5-r7 = callback_table[i]
        STMIA r0!, { r5-r7 }  // callback_table[i-1] = r5-r7
        ADD r4, r4, #1  // r4 = ++i
        CMP r4, r2  // if i < NXT_IDX then continue else break
        BLT .LISR_RemoveCallback_MemMoveLoop
.LISR_Remove_Callback_Return_MatchFound:
    MOV r5, #0
    MOV r6, r5
    MOV r7, r5
    STMIA r0!, { r5-r7 }
    POP { r5-r7 }
    SUB r2, r2, #1  // r2 = NXT_IDX-1
    MOV r3, r12  // r3 = &NXT_IDX
    STR r2, [r3]  // *(&NXT_IDX) = NXT_IDX-1 <=> --NXT_IDX
    POP { r4 }
    MOV r0, #1
    BX lr
    
.LISR_Remove_Callback_SetBackToZero:
    MOV r3, r12
    MOV r2, #0
    STR r2, [r3]
.LISR_Remove_Callback_Return_NoMatchFound:
    MOV r0, #0
    BX lr
    .size ISR_Remove_Callback, .-ISR_Remove_Callback





    .thumb_func
    .align 2
    .global ISR_Add_Callback
    .type ISR_Add_Callback %function
ISR_Add_Callback:
    // r0: irq type r1: priority lvl r2: cb fn addr
    PUSH { r0-r2, lr }
    BL ISR_Remove_Callback
    POP { r0-r2 }
    PUSH { r4-r7 }
    
    MOV r4, #1
    LSL r4, r4, r0
    MOV r0, r4  // r0 = 1<<irq_type = irq_flag
    LDR r4, =__GBADEV__ISR_CB_TABLE_NEXT_IDX
    LDR r5, [r4]
    CMP r5, #14
    BGE .LISR_Add_Callback_TableFull
    LDR r3, =__GBADEV__ISR_CALLBACK_TABLE
    LDR r4, [r3]
    CMP r4, #0
    BEQ .LISR_Add_Callback_InsertAtEnd
.LISR_Add_Callback_FindSlotLoop:
        LDR r4, [r3, #4]  // r4 = table[i].priority
        CMP r1, r4  // COMPARE(insert_priority, table[i].priority)
        BLT .LISR_Add_Callback_FoundSlot  // if prio < table[i].prio found slot
        BEQ .LISR_Add_Callback_ResolvePriorityCollusion  // if eq, use cmp flag
        MOV r4, #12
        ADD r3, r3, r4  // r4 = &table[++i]
.LISR_Add_Callback_ResolvePriorityCollusion:
    LDR r4, [r3]  // r4 = table[i].irq_flag
    CMP r0, r4  // CMP(flag, table[i].flag)
    BLT .LISR_Add_Callback_FoundSlot
    MOV r4, #12
    ADD r3, r3, r4  // r3 = &table[++i]
    LDR r4, [r3]  // r4 = table[i].flag
    CMP r4, #0  // if CMP(table[i].flag, 0)
    BNE .LISR_Add_Callback_FoundSlot  // if not zero, then it's not at end
    // otherwise, continue, as it's inserted at the end of the table
.LISR_Add_Callback_InsertAtEnd:
    STR r0, [r3]
    STR r1, [r3, #4]
    STR r2, [r3, #8]
    LDR r0, =__GBADEV__ISR_CB_TABLE_NEXT_IDX
    LDR r1, [r0]
    ADD r1, r1, #1
    STR r1, [r0]  // ++__GBADEV__ISR_CB_TABLE_NEXT_IDX
    MOV r0, #1
    POP { r4-r7 }
    POP { r1 }
    BX r1
.LISR_Add_Callback_FoundSlot:
    PUSH { r0-r2 }
    LDR r0, =__GBADEV__ISR_CALLBACK_TABLE
    LDR r4, =__GBADEV__ISR_CB_TABLE_NEXT_IDX
    LDR r1, [r4]  // r1 = nxt_idx
    MOV r2, r1  // r2 = nxt_idx
    LSL r1, r1, #1  // r1 = nxt_idx*2
    ADD r1, r1, r2  // r1 = nxt_idx*2 + nxt_idx = nxt_idx*3
    LSL r1, r1, #2  // r1 = (nxt_idx*3)<<2 = nxt_idx*3*4 = nxt_idx*12
    ADD r0, r0, r1  // r0 = &table[nxt_idx]
    ADD r2, r2, #1  // r2 = nxt_idx+1
    STR r2, [r4]  // *r4 = *&nxt_idx = nxt_idx+1 <=> ++nxt_idx
    MOV r2, #12
    SUB r1, r0, r2  // r1 = &table[nxt_idx-2]
    LSL r2, r2, #1  // r2 = r2<<1 = r2*2 = 12*2 = 24
    // r0: &table[nxt_idx-1] r1: &table[nxt_idx-2], r2 = 24
.LISR_Add_Callback_AdvanceDataForwardLoop:
        LDMIA r1!, { r5-r7 }
        STMIA r0!, { r5-r7 }
        SUB r1, r1, r2
        SUB r0, r0, r2
        CMP r0, r3
        BNE .LISR_Add_Callback_AdvanceDataForwardLoop
    POP { r0-r2 }
    STMIA r3!, { r0-r2 }
    POP { r5-r7 }
    POP { r1 }
    MOV r0, #1
    BX r1
.LISR_Add_Callback_TableFull:
    POP { r4-r7 }
    POP { r1 }
    MOV r0, #0
    BX r1
    .size ISR_Add_Callback, .-ISR_Add_Callback
