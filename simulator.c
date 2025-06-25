#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEM_SIZE (1024*1024)

//  memory & registers 
unsigned char memory[MEM_SIZE];
unsigned int R[16];  // R0–R15 (R15 is PC)

// Load ARM binary into memory 
void load_binary(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) { perror("fopen"); exit(1); }
    int n = fread(memory, 1, MEM_SIZE, f);
    fclose(f);
    printf("Loaded %d bytes\n", n);
}

/* Fetch a 32-bit little-endian word from memory */
unsigned int fetch(unsigned int addr) {
    return
      memory[addr]
    | memory[addr+1]<<8
    | memory[addr+2]<<16
    | memory[addr+3]<<24;
}

/* Execute one data-processing instruction */
void execDataProcessing(unsigned int inst) {
    unsigned int opcode = (inst >> 21) & 0xF;
    unsigned int isImm  = (inst >> 25) & 0x1;
    unsigned int Rn     = (inst >> 16) & 0xF;
    unsigned int Rd     = (inst >> 12) & 0xF;
    unsigned int imm12  = inst & 0xFFF;
    unsigned int Rm     = inst & 0xF;
    unsigned int op2    = isImm ? imm12 : R[Rm];

    switch (opcode) {
      case 0xD:  // MOV
        if (isImm) {
          R[Rd] = op2;
          printf("MOV R%u,#%u\n", Rd, op2);
        } else {
          R[Rd] = op2;
          printf("MOV R%u,R%u\n", Rd, Rm);
        }
        break;

      case 0x4:  // ADD
        if (isImm) {
          R[Rd] = R[Rn] + op2;
          printf("ADD R%u,R%u,#%u\n", Rd, Rn, op2);
        } else {
          R[Rd] = R[Rn] + op2;
          printf("ADD R%u,R%u,R%u\n", Rd, Rn, Rm);
        }
        break;

      case 0x2:  // SUB
        if (isImm) {
          R[Rd] = R[Rn] - op2;
          printf("SUB R%u,R%u,#%u\n", Rd, Rn, op2);
        } else {
          R[Rd] = R[Rn] - op2;
          printf("SUB R%u,R%u,R%u\n", Rd, Rn, Rm);
        }
        break;

      case 0xA:  // CMP
        if (isImm) {
          printf("CMP R%u,#%u\n", Rn, op2);
        } else {
          printf("CMP R%u,R%u\n", Rn, Rm);
        }
        break;

      default:
        printf("Unimplemented DP opcode 0x%X\n", opcode);
        exit(1);
    }

    /* advance PC */
    R[15] += 4;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <arm_binary.bin>\n", argv[0]);
        return 1;
    }

    /* zero regs & load program */
    memset(R, 0, sizeof(R));
    load_binary(argv[1]);
    R[15] = 0;  // start at address 0

    while (1) {
        unsigned int pc = R[15];
        if (pc + 4 > MEM_SIZE) break;

        unsigned int inst = fetch(pc);

        // data-processing if bits[27:26]==00 
        if ((inst & 0x0C000000) == 0) {
            unsigned int before[16];
            memcpy(before, R, sizeof(R));

            execDataProcessing(inst);

            //print any register changes 
            for (int i = 0; i < 15; i++) {
                if (before[i] != R[i]) {
                    printf("  R%d: 0x%X → 0x%X\n", i, before[i], R[i]);
                }
            }
            printf("  PC: 0x%X → 0x%X\n", before[15], R[15]);
        } else {
            printf("Not a data processing instruction @0x%X: 0x%X\n", pc, inst);
            break;
        }
    }

    printf("Simulation complete.\n");
    return 0;
}