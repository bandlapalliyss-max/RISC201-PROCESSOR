; ========================================================
; Benchmark: Array Summation (Sarangi Example 48 / 112)
; Tests memory access (st, ld), base-offset addressing,
; loop index increment, and pointer arithmetic.
; Initializes an array of 5 elements, then computes their sum.
; Sum = 10 + 20 + 30 + 40 + 50 = 150 (0x96) stored in r0.
; ========================================================
.text
    ; Base address of array at 0x1000
    movh r1, 0x0000
    addu r1, r1, 0x1000

    ; Store 5 elements into memory
    mov r3, 10
    st r3, 0[r1]
    mov r3, 20
    st r3, 4[r1]
    mov r3, 30
    st r3, 8[r1]
    mov r3, 40
    st r3, 12[r1]
    mov r3, 50
    st r3, 16[r1]

    ; Sum loop
    mov r0, 0          ; sum = 0
    mov r2, 0          ; idx = 0
    mov r4, 5          ; limit = 5

.loop:
    cmp r2, r4
    beq .done
    lsl r5, r2, 2      ; byte offset = idx * 4
    add r6, r1, r5     ; effective address = base + offset
    ld r7, 0[r6]       ; load array[idx]
    add r0, r0, r7     ; sum += array[idx]
    add r2, r2, 1      ; idx++
    b .loop

.done:
    nop
