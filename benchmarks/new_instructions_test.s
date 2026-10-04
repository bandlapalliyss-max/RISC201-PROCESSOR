; ==============================================================================
; RISC201 Demonstration of 5 New Instructions:
;   - min  : Signed minimum of rs1 and rs2/imm
;   - max  : Signed maximum of rs1 and rs2/imm
;   - rots : Rotate right by specified shift amount
;   - cbeq : Fused compare-and-branch-if-equal
;   - cbgt : Fused compare-and-branch-if-greater-than
; ==============================================================================
.text
    ; 1. Test MIN instruction (both register and immediate forms)
    mov r0, 42
    mov r1, 15
    min r2, r0, r1       ; r2 = min(42, 15) = 15
    min r12, r0, 8       ; r12 = min(42, 8) = 8

    ; 2. Test MAX instruction (both register and immediate forms)
    max r3, r0, r1       ; r3 = max(42, 15) = 42
    max r13, r1, 100     ; r13 = max(15, 100) = 100

    ; 3. Test ROTS (rotate right) instruction
    mov r4, 0x12
    rots r5, r4, 4       ; r5 = 0x12 rot_right 4 = 0x20000001

    ; 4. Test CBEQ (compare and branch if equal)
    mov r6, 10
    mov r7, 10
    cbeq r6, r7, .equal_pass
    mov r8, 0            ; Fail flag if fall-through taken
    b .test_cbgt

.equal_pass:
    mov r8, 1            ; Success flag for CBEQ

.test_cbgt:
    ; 5. Test CBGT (compare and branch if greater than)
    mov r9, 25
    mov r10, 20
    cbgt r9, r10, .gt_pass
    mov r11, 0           ; Fail flag if fall-through taken
    b .done

.gt_pass:
    mov r11, 1           ; Success flag for CBGT

.done:
    nop
