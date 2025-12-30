.if 0
    .section .iwram, "ax", %progbits
    
    .arm
    .align 2
    .global TextEngine_DefaultRenderCallback_BMP
    .type TextEngine_DefaultRenderCallback_BMP %function
TextEngine_DefaultRenderCallback_BMP:
    .size TextEngine_DefaultRenderCallback_BMP, .-TextEngine_DefaultRenderCallback_BMP

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@    

    .arm
    .align 2
    .global TextEngine_DefaultRenderCallback_Tilemap
    .type TextEngine_DefaultRenderCallback_Tilemap %function
TextEngine_DefaultRenderCallback_Tilemap:
    
    .size TextEngine_DefaultRenderCallback_Tilemap, .-TextEngine_DefaultRenderCallback_Tilemap

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@    

    .arm
    .align 2
    .global TextEngine_DefaultClearCallback_BMP
    .type TextEngine_DefaultClearCallback_BMP %function
TextEngine_DefaultClearCallback_BMP:
    .size TextEngine_DefaultClearCallback_BMP, .-TextEngine_DefaultClearCallback_BMP

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@    

    .arm
    .align 2
    .global TextEngine_DefaultClearCallback_Tilemap
    .type TextEngine_DefaultClearCallback_Tilemap %function
TextEngine_DefaultClearCallback_Tilemap:
    
    .size TextEngine_DefaultClearCallback_Tilemap, .-TextEngine_DefaultClearCallback_Tilemap

    
    
.endif
