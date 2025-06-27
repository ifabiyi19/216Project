#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEM_SIZE (1024*1024)

//  memory & registers 
unsigned char memory[MEM_SIZE];
unsigned int R[16];  // R0–R15 (R15 is PC)
unsigned int N_flag, Z_flag;

// Load ARM binary into memory 
void load_binary(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) { perror("fopen"); exit(1); }
    int n = fread(memory, 1, MEM_SIZE, f);
    fclose(f);
    printf("Loaded %d bytes\n", n);
}

/* Fetch a 32-bit little-endian word from memory */
unsigned int fetch32(unsigned int addr) {
    return
      memory[addr]
    | memory[addr+1]<<8
    | memory[addr+2]<<16
    | memory[addr+3]<<24;
}

unsigned short fetch16(unsigned int addr) {
    return memory[addr] | (memory[addr+1] << 8);
}

int sign_extend(int val, int bits) {
    int m = 1 << (bits - 1);
    return (val ^ m) - m;
}

/* Execute one data-processing instruction */
void execDataProcessingARM(unsigned int inst) {
    unsigned int opc = (inst >> 21) & 0xF;   // opcode
    unsigned int S   = (inst >> 20) & 0x1;   // S-bit: 1 -> update flags
    unsigned int Rn  = (inst >> 16) & 0xF;
    unsigned int Rd  = (inst >> 12) & 0xF;
    unsigned int imm = (inst >> 25) & 0x1;
    unsigned int imm12 = inst & 0xFFF;
    unsigned int Rm  = inst & 0xF;
    unsigned int op2 = imm ? imm12 : R[Rm];
    unsigned int result;
    switch (opc) {
      case 0xD: // MOV / MOVS
        result = op2;
        R[Rd] = result;
        if (S) {
            printf("ARM MOVS R%u,#%u -> R%u=0x%X\n", Rd, op2, Rd, result);
            N_flag = (result >> 31) & 1;
            Z_flag = (result == 0);
        } else {
            printf("ARM MOV  R%u,#%u -> R%u=0x%X\n", Rd, op2, Rd, result);
        }
        break;

      case 0x4: // ADD / ADDS
        result = R[Rn] + op2;
        R[Rd] = result;
        if (S) {
            printf("ARM ADDS R%u,R%u,#%u -> R%u=0x%X\n", Rd, Rn, op2, Rd, result);
            N_flag = (result >> 31) & 1;
            Z_flag = (result == 0);
        } else {
            printf("ARM ADD  R%u,R%u,#%u -> R%u=0x%X\n", Rd, Rn, op2, Rd, result);
        }
        break;

      case 0x2: // SUB / SUBS
        result = R[Rn] - op2;
        R[Rd] = result;
        if (S) {
            printf("ARM SUBS R%u,R%u,#%u -> R%u=0x%X\n", Rd, Rn, op2, Rd, result);
            N_flag = (result >> 31) & 1;
            Z_flag = (result == 0);
        } else {
            printf("ARM SUB  R%u,R%u,#%u -> R%u=0x%X\n", Rd, Rn, op2, Rd, result);
        }
        break;

      case 0xA: { // CMP
        result = (int)R[Rn] - (int)op2;
        N_flag = (result < 0);
        Z_flag = (result == 0);
        printf("ARM CMP  R%u,#%u -> N=%u Z=%u\n", Rn, op2, N_flag, Z_flag);
      } break;

      default:
        printf("Unimplemented ARM Data processing opcode 0x%X\n", opc);
        exit(1);
    }
    R[15] += 4;

}

void execLoadStoreARM(unsigned int inst) {
    unsigned int L  = (inst >> 20) & 1;   // 1 = LDR, 0 = STR
    unsigned int Rn = (inst >> 16) & 0xF;
    unsigned int Rd = (inst >> 12) & 0xF;
    unsigned int imm= inst & 0xFFF;
    unsigned int addr = R[Rn] + imm;

    if (L) {
        unsigned int val = fetch32(addr);
        R[Rd] = val;
        printf("ARM LDR R%u,[R%u,#%u] -> R%u=0x%X\n", Rd, Rn, imm, Rd, R[Rd]);
    } else {
        unsigned int val = R[Rd];
        memory[addr]=val&0xFF; memory[addr+1]=(val>>8)&0xFF;
        memory[addr+2]=(val>>16)&0xFF; memory[addr+3]=(val>>24)&0xFF;
        printf("ARM STR R%u,[R%u,#%u] -> MEM[0x%X]=0x%X\n", Rd, Rn, imm, addr, val);
    }
    R[15] += 4;
}

void execBranchARM(unsigned int inst) {
    int imm24 = inst & 0x00FFFFFF;
    imm24 = sign_extend(imm24, 24) << 2;
    unsigned int newPC = R[15] + 8 + imm24;
    printf("ARM B to 0x%X -> PC=0x%X\n", newPC, newPC);
    R[15] = newPC;
}

void execThumb(unsigned short inst) {
    unsigned short op5 = inst >> 11;
    if ((inst & 0xF800) == 0x2000) { // MOV Rd,#imm8
        unsigned int Rd = (inst >> 8) & 0x7;
        unsigned int imm8 = inst & 0xFF;
        R[Rd] = imm8;
        printf("THUMB MOV R%u,#%u -> R%u=0x%X\n", Rd, imm8, Rd, R[Rd]);
        R[15] += 2;
    }
    else if ((inst & 0xF800) == 0x3000) { // ADD Rd,#imm8
        unsigned int Rd = (inst >> 8) & 0x7;
        unsigned int imm8 = inst & 0xFF;
        R[Rd] += imm8;
        printf("THUMB ADD R%u,#%u -> R%u=0x%X\n", Rd, imm8, Rd, R[Rd]);
        R[15] += 2;
    }
    else if ((inst & 0xF800) == 0x3800) { // SUB Rd,#imm8
        unsigned int Rd = (inst >> 8) & 0x7;
        unsigned int imm8 = inst & 0xFF;
        R[Rd] -= imm8;
        printf("THUMB SUB R%u,#%u -> R%u=0x%X\n", Rd, imm8, Rd, R[Rd]);
        R[15] += 2;
    }
    else if ((inst & 0xF800) == 0x2800) { // CMP Rd,#imm8
        unsigned int Rd = (inst >> 8) & 0x7;
        unsigned int imm8 = inst & 0xFF;
        int res = (int)R[Rd] - (int)imm8;
        N_flag = res < 0;
        Z_flag = res == 0;
        printf("THUMB CMP R%u,#%u -> N=%u Z=%u\n", Rd, imm8, N_flag, Z_flag);
        R[15] += 2;
    }
    else if (op5 == 0x1C) { // B<cond> unconditional
        int imm11 = inst & 0x7FF;
        int offset = sign_extend(imm11, 11) << 1;
        unsigned int newPC = R[15] + 4 + offset;
        printf("THUMB B to 0x%X -> PC=0x%X\n", newPC, newPC);
        R[15] = newPC;
    }
    else {
        printf("Unimplemented THUMB inst 0x%04X at PC=0x%X\n", inst, R[15]);
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <binary.bin> <ARM|THUMB>\n", argv[0]);
        return 1;
    }
    int thumb = (strcmp(argv[2], "THUMB") == 0);
    memset(R, 0, sizeof(R)); N_flag = Z_flag = 0;
    load_binary(argv[1]);
    R[15] = 0;

    while (1) {
        unsigned int pc = R[15];
        if (thumb) {
            if (pc + 2 > MEM_SIZE) break;
            unsigned short inst16 = fetch16(pc);
            execThumb(inst16);
        } else {
            if (pc + 4 > MEM_SIZE) break;
            unsigned int inst32 = fetch32(pc);
            unsigned int top3 = (inst32 >> 25) & 0x7;
            if ((inst32 & 0x0E000000) == 0x0A000000) execBranchARM(inst32);
            else if ((inst32 >> 26) == 1) execLoadStoreARM(inst32);
            else execDataProcessingARM(inst32);
        }
    }
    printf("Simulation complete.\n");
    return 0;
}
