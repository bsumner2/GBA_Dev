/** (C) 10 of September, 2025 Burt Sumner */
    .section .text
    .thumb_func
    .align 2
    .global SRAM_Write
    .type SRAM_Write %function
    /* r0: src::const void* r1: size::u32 r2: offset::u32 */
SRAM_Write:
    CMP r1, #0
    BLE .LSRAM_write_bad_return
    CMP r2, #0
    BLT .LSRAM_write_bad_return
    CMP r0, #0
    BEQ .LSRAM_write_bad_return

    MOV r12, r2  /* r12 = r2 = offset */

    ADD r2, r2, r1  /* r2 = offset+size */
    
    MOV r3, #1
    LSL r3, r3, #16  /* r3 = 1<<16 = 0x10000 := SIZEOF(SRAM) */
    CMP r2, r3
    BGT .LSRAM_write_bad_return

    MOV r3, #4  /* r3 = 0x0004 */
    MOV r2, #0x80
    LSL r2, r2, #2  /* r2 = 0x0080<<2 = 0x0200 */
    ORR r3, r3, r2  /* r3 = 0x0200 | 0x0004 = 0x0204 */
    LSL r2, r2, #17  /* r2 = 0x0200<<17 = 0x04000000 */
    ORR r3, r3, r2  /* r3 = 0x04000000 | 0x0204 = 0x04000204 = &REG_WAIT_CNT */
    LDRH r2, [r3]  /* r2 = *(volatile u16*)(&REG_WAIT_CNT) = REG_WAIT_CNT */
    PUSH { r2, r3 }  /* Save REG_WAIT_CNT and &REG_WAIT_CNT to stack for 
                        restoration after completing SRAM r/w ops. */
    PUSH { r4 }
    MOV r4, #3  /* r4 = 3 */
    ORR r2, r2, r4  /* r2 = REG_WAIT_CNT|3 
                       We need to set REG_WAIT_CNT bits 0-1 to 3, so we can just
                       ORR REG_WAIT_CNT's current value with the mask for this
                       register field, and ensure we've set the wait state field
                       to 3. */
    POP { r4 }
    STRH r2, [r3]  /* basically, REG_WAIT_CNT|=3 */
    MOV r2, r12  /* r2 = r12 = offset (restore r2 to being arg2) */
    MOV r3, #0xE0
    LSL r3, r3, #20  /*r3 = 0x0E000000 = SRAM */
    ADD r2, r2, r3  /* r2 = SRAM + offset = &SRAM[offset] */
.LSRAM_write_loop:
        LDRB r3, [r0]
        STRB r3, [r2]
        ADD r0, r0, #1
        ADD r2, r2, #1
        SUB r1, r1, #1
        CMP r1, #0
        BNE .LSRAM_write_loop

    POP { r2, r3 }
    STRH r2, [r3]
    MOV r0, #1
    BX lr 
.LSRAM_write_bad_return:
    MOV r0, #0
    BX lr
    .size SRAM_Write, .- SRAM_Write

    .thumb_func
    .align 2
    .global SRAM_Read
    .type SRAM_Read %function
    /* r0: src::void* r1: size::u32 r2: offset::u32 */
SRAM_Read:
    CMP r1, #0
    BLE .LSRAM_read_bad_return
    CMP r2, #0
    BLE .LSRAM_read_bad_return
    CMP r0, #0
    BEQ .LSRAM_read_bad_return

    MOV r12, r2  /* r12 = r2 = offset */

    ADD r2, r2, r1  /* r2 = offset+size */
    
    MOV r3, #1
    LSL r3, r3, #16  /* r3 = 1<<16 = 0x10000 := SIZEOF(SRAM) */
    CMP r2, r3
    BGT .LSRAM_read_bad_return

    MOV r3, #4  /* r3 = 0x0004 */
    MOV r2, #0x80
    LSL r2, r2, #2  /* r2 = 0x0080<<2 = 0x0200 */
    ORR r3, r3, r2  /* r3 = 0x0200 | 0x0004 = 0x0204 */
    LSL r2, r2, #17  /* r2 = 0x0200<<17 = 0x04000000 */
    ORR r3, r3, r2  /* r3 = 0x04000000 | 0x0204 = 0x04000204 = &REG_WAIT_CNT */
    LDRH r2, [r3]  /* r2 = *(volatile u16*)(&REG_WAIT_CNT) = REG_WAIT_CNT */
    PUSH { r2, r3 }  /* Save REG_WAIT_CNT and &REG_WAIT_CNT to stack for 
                        restoration after completing SRAM r/w ops. */
    PUSH { r4 }
    MOV r4, #3  /* r4 = 3 */
    ORR r2, r2, r4  /* r2 = REG_WAIT_CNT|3 
                       We need to set REG_WAIT_CNT bits 0-1 to 3, so we can just
                       ORR REG_WAIT_CNT's current value with the mask for this
                       register field, and ensure we've set the wait state field
                       to 3. */
    POP { r4 }
    STRH r2, [r3]  /* basically, REG_WAIT_CNT|=3 */
    MOV r2, r12  /* r2 = r12 = offset (restore r2 to being arg2) */
    MOV r3, #0xE0
    LSL r3, r3, #20  /*r3 = 0x0E000000 = SRAM */
    ADD r2, r2, r3  /* r2 = SRAM + offset = &SRAM[offset] */
.LSRAM_read_loop:
        LDRB r3, [r2]
        STRB r3, [r0]
        ADD r2, r2, #1
        ADD r0, r0, #1
        SUB r1, r1, #1
        CMP r1, #0
        BNE .LSRAM_read_loop

    POP { r2, r3 }
    STRH r2, [r3]
    MOV r0, #1
    BX lr
.LSRAM_read_bad_return:
    MOV r0, #0
    BX lr
    .size SRAM_Read, .- SRAM_Read

/* Credit for Fast_Memcpy32 implementation goes to J Vijn
 * https://www.coranac.com/tonc/text/asm.htm */
    .section .iwram,"ax", %progbits
    .arm
    .align 2
    .global Fast_Memcpy32
    .type Fast_Memcpy32 %function
    /* r0: dst r1: src r2: word_ct */
Fast_Memcpy32:
    AND r12, r2, #7
    MOVS r2, r2, LSR #3  /* r3:= word_ct/8 */
    BEQ .Lfast_memcpy32_remainder
    STMFD sp!, {r4-r10}
.Lfast_memcpy32_blocks:
        LDMIA r1!, {r3-r10}
        STMIA r0!, {r3-r10}
        SUBS r2, r2, #1
        BNE .Lfast_memcpy32_blocks
    LDMFD sp!, {r4-r10}
 .Lfast_memcpy32_remainder:
        SUBS r12, r12, #1
        LDRCS r3, [r1], #4
        STRCS r3, [r0], #4
        BCS .Lfast_memcpy32_remainder
    BX lr
    .size Fast_Memcpy32, .-Fast_Memcpy32

    .arm
    .align 2
    .global Fast_Memset32
    .type Fast_Memset32 %function
    /* r0: dst r1: fill_word r2: word_ct */
Fast_Memset32:
    AND r12, r2, #7
    MOVS r2, r2, LSR #3
    BEQ .Lfast_memset32_remainder
    STMFD sp!, {r4-r9}
    MOV r3, r1
    MOV r4, r1
    MOV r5, r1
    MOV r6, r1
    MOV r7, r1
    MOV r8, r1
    MOV r9, r1
.Lfast_memset32_blocks:
        STMIA r0!, {r1, r3-r9}
        SUBS r2, r2, #1
        BNE .Lfast_memset32_blocks
    LDMFD sp!, {r4-r9}
.Lfast_memset32_remainder:
        SUBS r12, r12, #1
        STRCS r1, [r0], #4
        BCS .Lfast_memset32_remainder
    BX lr
    .size Fast_Memset32, .-Fast_Memset32

    .arm
    .align 2
    .type OAM_Init32_Local %function
OAM_Init32_Local:
    STMFD sp!, { r4-r9 }
    MOV r2, #0x0200
    MOV r3, r2, LSL #15
    MOV r4, r2
    MOV r5, #0
    MOV r6, r2
    MOV r7, r5
    MOV r8, r2
    MOV r9, r3
.LOAM_init32_local_loop:
        SUBS r1, r1, #1
        STMCSIA r0!, { r2-r9 }
        BCS .LOAM_init32_local_loop
    LDMFD sp!, { r4-r9 }
    BX lr
    .size OAM_Init32_Local, .-OAM_Init32_Local


    .section .text
    .thumb_func
    .align 2
    .global OAM_Init
    .type OAM_Init %function
    /* r0: obj_attrs::Obj_Attr_t* r1: count::u32 */
    /* Sets all Obj_Attr_t's to HIDE while simultanelously setting all obj 
       affine transforms, interlaced within, between, these Obj_Attr_t 
       instances, to the Identity matrix: { 0x0100, 0x0000, 0x0000, 0x0100 },
       where matrix components are 8.8 fixed point numbers
       */
OAM_Init:
    MOV r2, #3
    TST r1, r2
    BNE .LOAM_init_bad_return
    CMP r1, #0
    BLE .LOAM_init_bad_return
    CMP r1, #128
    BGT .LOAM_init_bad_return
    PUSH { lr }
    PUSH { r0, r1 }
    LSR r1, r1, #2  /* r1 = count/4 */
    LDR r3, =OAM_Init32_Local
    BL .LOAM_init_r3lcall
    POP { r1, r2 }
    LSL r2, r2, #1  /* r2 = count Obj_Attr_t's * 2words/Obj_Attr_t 
                          = count*2 words */
    MOV r0, #0x70
    LSL r0, r0, #20  /* r0 = 0x70<<20 = 0x07000000 */
    /* so now r0 = OAM_BUF, r1 = obj_attrs:arg0, r2 = count*2:arg1*2 */
    LDR r3, =Fast_Memcpy32
    BL .LOAM_init_r3lcall
    MOV r0, #1
    POP { r3 }
.LOAM_init_r3lcall:
    BX r3
.LOAM_init_bad_return:
    MOV r0, #0
    BX lr
    .size OAM_Init, .-OAM_Init

    .thumb_func
    .align 2
    .global OAM_Affine_Transform_Copy
    .type OAM_Affine_Transform_Copy %function
    /* r0: obj_attr_dst r1: obj_attr_src r2: count */
OAM_Affine_Transform_Copy:
    CMP r2, #0
    BLE .LOAM_affine_transform_cpy_bad_return
    CMP r2, #32
    BGT .LOAM_affine_transform_cpy_bad_return
    ADD r0, r0, #6
    ADD r1, r1, #6
.LOAM_affine_transform_cpy_loop:
        LDRH r3, [r1]
        STRH r3, [r0]
        ADD r0, r0, #8
        ADD r1, r1, #8
        LDRH r3, [r1]
        STRH r3, [r0]
        ADD r0, r0, #8
        ADD r1, r1, #8
        LDRH r3, [r1]
        STRH r3, [r0]
        ADD r0, r0, #8
        ADD r1, r1, #8
        LDRH r3, [r1]
        STRH r3, [r0]
        ADD r0, r0, #8
        ADD r1, r1, #8
        SUB r2, r2, #1
        CMP r2, #0
        BNE .LOAM_affine_transform_cpy_loop

    MOV r0, #1
    BX lr
.LOAM_affine_transform_cpy_bad_return:
    MOV r0, #0
    BX lr
    .size OAM_Affine_Transform_Copy, .-OAM_Affine_Transform_Copy

