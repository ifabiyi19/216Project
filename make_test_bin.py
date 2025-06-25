#!/usr/bin/env python3
"""
make_test_bin.py

Creates test.bin containing a short ARMv7 “program”:
  MOV R1, #5
  ADD R2, R1, #3
  SUB R3, R2, #2
  CMP R3, #6
  B   .        (infinite loop)

Each instruction is encoded as a 32-bit little-endian word.
"""

# List of 32-bit ARM opcodes (in hex)
insts = [
    0xE3A01005,  # MOV R1,#5
    0xE2812003,  # ADD R2,R1,#3
    0xE2423002,  # SUB R3,R2,#2
    0xE3530006,  # CMP R3,#6
    0xE3A0100A,  # MOV R1, #10
    0xE2812004,  # ADD R2, R1, #4
    0xE2423003,  # SUB R3, R2, #3
    0xE353000A,  # CMP R3, #10
    0xE3A01014,  # MOV, R1, #20
    0xE2812005,  # ADD R2, R1 #5
    0xE2423004,  # SUB R3, R2, #4
    0xE3530014,  # CMP R3, #20
    0xE3A0101E,  # MOV R1, #30
    0xE2812006,  # ADD R2, R1, #6
    0xEAFFFFFE,  # B .   (loop back)

]

# Write them out in little-endian order
with open("test.bin", "wb") as f:
    for opcode in insts:
        f.write(opcode.to_bytes(4, byteorder="little"))

