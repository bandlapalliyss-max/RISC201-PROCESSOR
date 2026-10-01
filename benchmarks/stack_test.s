; ========================================================
; Benchmark: Full-Descending Stack & Macro Preprocessor Test
; Tests:
;   - pushm and popm macro expansion
;   - Stack boundary tracking and preservation
; ========================================================
.text
    ; Initialize registers with test values
    mov r1, 111
    mov r2, 222
    mov r3, 333
    mov r4, 444

    ; Push multiple registers onto full-descending stack
    pushm {r1, r2, r3, r4}

    ; Overwrite registers with garbage
    mov r1, 0
    mov r2, 0
    mov r3, 0
    mov r4, 0

    ; Pop multiple registers back off stack
    popm {r1, r2, r3, r4}

    ; Values in r1..r4 should be restored to 111, 222, 333, 444
    nop
