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
arm_insts = [
    0xE3A01005,  # MOV R1,#5
    0xE2812003,  # ADD R2,R1,#3
    0xE2423002,  # SUB R3,R2,#2
    0xE3530006,  # CMP R3,#6
    0xE3A0100A,  # MOV R1,#10
    0xE2812004,  # ADD R2,R1,#4
    0xE2423003,  # SUB R3,R2,#3
    0xE353000A,  # CMP R3,#10
    0xE3A01014,  # MOV R1,#20
    0xE2812005,  # ADD R2,R1,#5
    0xE2423004,  # SUB R3,R2,#4
    0xE3530014,  # CMP R3,#20
    0xE3A0101E,  # MOV R1,#30
    0xE2812006,  # ADD R2,R1,#6
    0xE3A00010,  # MOV R0,#16
    0xE3A0102A,  # MOV R1,#42
    0xE5801000,  # STR R1,[R0,#0]
    0xE3A02000,  # MOV R2,#0
    0xE5902000,  # LDR R2,[R0,#0]
]
thumb_insts = [
    0x2001,  # MOV R0,#1
    0x2102,  # MOV R1,#2
    0x3205,  # ADD R2,#5
    0x3903,  # SUB R3,#3
    0x2A04,  # CMP R4,#4
    0xE000,  # B . (infinite loop)
]

# Write them out in little-endian order
with open("test_arm.bin", "wb") as f:
    for w in arm_insts:
        f.write(w.to_bytes(4, byteorder="little"))
print(f"Wrote {len(arm_insts)} ARM instructions to test_arm.bin ({len(arm_insts)*4} bytes)")

# Write Thumb binary
with open("test_thumb.bin", "wb") as f:
    for w in thumb_insts:
        f.write(w.to_bytes(2, byteorder="little"))
print(f"Wrote {len(thumb_insts)} THUMB instructions to test_thumb.bin ({len(thumb_insts)*2} bytes)")
