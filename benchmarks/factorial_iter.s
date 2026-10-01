; ========================================================
; Benchmark: Iterative Factorial (Sarangi Example 29)
; Computes 6! = 720 (0x2D0) and stores in r1
; ========================================================
.text
    mov r0, 6          ; Input number n = 6
    mov r1, 1          ; prod = 1
    mov r2, r0         ; idx = n

.loop:
    mul r1, r1, r2     ; prod = prod * idx
    sub r2, r2, 1      ; idx = idx - 1
    cmp r2, 1          ; compare (idx, 1)
    bgt .loop          ; if idx > 1 continue loop

.exit:
    nop
