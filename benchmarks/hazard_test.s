; ========================================================
; Benchmark: Hazard Stress Test (Sarangi Section 10.4 - 10.7)
; Tests:
;   1. RAW dependency forwarding (EX->EX, MA->EX)
;   2. Load-Use hazard 1-cycle stall interlock
;   3. Load-Store memory forwarding without stalling
;   4. Branch-Lock flush on taken conditional branch
; ========================================================
.text
    ; Initialize base registers
    mov r2, 10
    mov r3, 20
    mov r5, 5

    ; Test 1: Intense back-to-back ALU RAW forwarding
    add r1, r2, r3     ; r1 = 10 + 20 = 30
    sub r4, r1, r5     ; r4 = 30 - 5 = 25 (uses r1 via forwarding)
    mul r6, r4, r1     ; r6 = 25 * 30 = 750 (uses r4 and r1)

    ; Test 2: Store then Load-Use Hazard
    movh r8, 0x0000
    addu r8, r8, 0x2000
    st r6, 0[r8]       ; store 750 into mem[0x2000]

    ld r9, 0[r8]       ; load from mem[0x2000] into r9
    add r10, r9, 50    ; LOAD-USE HAZARD: r10 = 750 + 50 = 800 (stalls 1 cycle)

    ; Test 3: Load followed by Store (zero-stall forwarding)
    ld r11, 0[r8]      ; load 750
    st r11, 4[r8]      ; store forwarded from RW -> MA stage without stall!

    ; Test 4: Branch flush test
    cmp r10, 800
    beq .target        ; Branch is taken! Instructions below must be flushed
    add r12, r12, 999  ; Wrong path! Must be cancelled (bubble)
    add r13, r13, 999  ; Wrong path! Must be cancelled (bubble)

.target:
    mov r12, 42        ; Correct path!
    nop
