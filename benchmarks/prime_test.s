; ========================================================
; Benchmark: Prime Number Verification (Sarangi Example 30)
; Checks if r1 is prime. Stores boolean result in r0 (1 = prime, 0 = composite)
; Tests mod, cmp, conditional branches (beq, bgt)
; Test with 29 (Prime) -> r0 should be 1
; ========================================================
.text
    mov r1, 29         ; test number = 29
    mov r2, 2          ; divisor = 2

.loop:
    mod r3, r1, r2     ; r3 = r1 % r2
    cmp r3, 0          ; check remainder == 0
    beq .notprime      ; if remainder == 0, composite!
    add r2, r2, 1      ; divisor++
    cmp r1, r2         ; compare r1 with divisor
    bgt .loop          ; while divisor < r1, continue

    mov r0, 1          ; number is prime
    b .exit

.notprime:
    mov r0, 0          ; number is composite

.exit:
    nop
