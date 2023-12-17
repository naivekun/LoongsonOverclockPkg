# LoongArch iocsr commands

ASM_GLOBAL ASM_PFX(AsmIOCSRRead32)
ASM_GLOBAL ASM_PFX(AsmIOCSRRead64)
ASM_GLOBAL ASM_PFX(AsmIOCSRWrite32)
ASM_GLOBAL ASM_PFX(AsmIOCSRWrite64)
ASM_GLOBAL ASM_PFX(AsmIOCSRRead8)
ASM_GLOBAL ASM_PFX(AsmIOCSRWrite8)

ASM_PFX(AsmIOCSRRead8):
    ori $t0, $zero, 0x28
    csrwr $t0, 0
    ld.b $a0, $a0, 0
    csrwr $t0, 0
    bstrpick.d $a0, $a0, 7, 0 
    ret

ASM_PFX(AsmIOCSRRead32):
    ori $t0, $zero, 0x28
    csrwr $t0, 0
    ld.w $a0, $a0, 0
    csrwr $t0, 0
    bstrpick.d $a0, $a0, 31, 0
    ret

ASM_PFX(AsmIOCSRRead64):
    ori $t0, $zero, 0x28
    csrwr $t0, 0
    ld.d $a0, $a0, 0
    csrwr $t0, 0
    bstrpick.d $a0, $a0, 63, 0
    ret

ASM_PFX(AsmIOCSRWrite8):
    ori $t0, $zero, 0x28
    csrwr $t0, 0
    st.b $a1, $a0, 0
    csrwr $t0, 0
    move $a0, $a1
    ret

ASM_PFX(AsmIOCSRWrite32):
    ori $t0, $zero, 0x28
    csrwr $t0, 0
    st.w $a1, $a0, 0
    csrwr $t0, 0
    move $a0, $a1
    ret

ASM_PFX(AsmIOCSRWrite64):
    ori $t0, $zero, 0x28
    csrwr $t0, 0
    st.d $a1, $a0, 0
    csrwr $t0, 0
    move $a0, $a1
    ret



