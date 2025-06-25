    .syntax unified
    .cpu cortex-m3      @ or cortex-a8, etc.
    .thumb              @ omit if you want pure ARM mode
    .text
    .global _start
_start:
    MOV   R1, #5        @ R1 ← 5
    ADD   R2, R1, #3    @ R2 ← R1 + 3
    SUB   R3, R2, #2    @ R3 ← R2 – 2
    CMP   R3, #6        @ compare R3 vs. 6
    MOV   R0, #0        @ just so we have something in R0
    B     .             @ infinite loop to stop