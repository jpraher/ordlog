
#include <stdio.h>

typedef unsigned char  byte_t;
typedef unsigned short hword_t;
typedef unsigned int   word_t;
typedef unsigned long long dword_t;

#pragma push(pack,1)
struct instrr {   // R-type (register)
    word_t opcode  : 6;
    word_t rs      : 5;
    word_t rt      : 5;
    word_t rd      : 5;
    word_t shamt   : 5;
    word_t funct   : 6;
};

struct instri {    // I-type (immediate)
    word_t opcode : 6;
    word_t rs     : 5;
    word_t rt     : 5;
    word_t imm    : 16;
};

struct instrj {    // J-type (jump)
    word_t opcode  : 6;
    word_t address : 26;
};

typedef struct instrr instrr_t;
typedef struct instri instri_t;
typedef struct instrj instrj_t;

union  instr {
    instrr_t  r;
    instri_t  i;
    instrj_t  j;
};
typedef union instr instr_t;

#pragma pop(pack)



#define B8  0xff
#define B7  0x7f   
#define B6  0x3f
#define B5  0x1f
#define B4  0xf
#define B16 0xffff
#define B26 0x3ffffff

#define R(opcode,s,t,d,sh,f)    \
    (((((word_t)opcode) & B6) << 26)		\
    | ((s & B5)  << (32-11)) \
    | ((t & B5)  << (32-16)) \
    | ((d & B5)  << (32-21)) \
    | ((sh & B5) << (32-26)) \
    | (f & B6))

#define I(opcode,s,t,imm)   ((opcode & B6) << (32-6)) | ((s & B5) << (32-11)) | ((t & B5) << (32-16)) | (imm & B16)
#define J(opcode,target)    ((opcode & B6) << (32-7)) | (target & B26)

inline word_t opcode(word_t w) { return w >> 26; }
inline word_t r_rs(word_t w) { return (w >> 21) & B5; }
inline word_t r_rt(word_t w) { return (w >> 16) & B5; }
inline word_t r_rd(word_t w) { return (w >> 11) & B5; }
inline word_t r_shamt(word_t w) { return (w >> 6) & B5; }
inline word_t r_funct(word_t w) { return w & B6; }

/*
  define and match on the types...
 */
#define ADD(s,t,d)      R(0x0u,s,t,d,0x0u,0x20u)
#define ADDI(s,t,imm)   I(0x8u,s,t,imm)
#define ADDIU(s,t,imm)  I(0x9u,s,t,imm)
#define ADDU(s,t,d)     R(0x0u,s,t,d,0x0u,0x21u)
#define AND(s,t,d)      R(0x0u,s,t,d,0x0u,0x24u)
#define ANDI(s,t,imm)   I(0xcu,s,t,imm)
#define BEQ(s,t,imm)    I(0x4u,s,t,imm)
#define BNEQ(s,t,imm)   I(0x5u,s,t,imm)
#define JMP(target)     J(0x2u,target)
#define JAL(target)     J(0x3u,target)
#define JR(s)           R(0x0u,s,0x0u,0x0u,0x0u,0x8u)
#define LBU(s,t,imm)    I(0x24u,s,t,imm)

#define M_ADD           ADD(0,0,0)
#define M_ADDI          ADDI(0,0,0)
#define M_ADDIU         ADDIU(0,0,0)
#define M_ADDU          ADDU(0,0,0)

#define R_MASK          0xfc00003fu // (((word_t)B6) << 26 | B6)
#define J_MASK          0xfc000000u // (B6 << 26)
#define I_MASK          0xfc000000u // (B6 << 26)


void r_print(word_t w2) {
    printf("INSTR: %d %d %d %d %d %d\n", 
	   opcode(w2),
	   r_rs(w2),
	   r_rt(w2),
	   r_rd(w2),
	   r_shamt(w2),
	   r_funct(w2));
}

// #define IS_R(opcode) 

int main(int argc, const char ** argv) {

    word_t instrs[] = {
	ADD(0,1,2)
    };

    word_t t = instrs[0] & R_MASK;
    instrr_t r = *((instrr_t*)&t);
    word_t m = M_ADD;
    instrr_t r1 = *((instrr_t*)&m);
    word_t w2 = R(1,2,3,4,5,6);
    word_t w3 = R(1,2,3,4,5,0x20);
    word_t w4 = ADD(1,2,3);
    r_print(w2);
    r_print(w3);
    r_print(w4);
        
    printf("instr %08x\n", t&B6);
    printf("mask  %08x\n", M_ADD);
    printf("test  %08x\n", (w4 & R_MASK));
    printf("mask  %08x\n", R_MASK);
    printf("instr %08x\n", r_funct(t));
    printf("instr %08x\n", r_funct(m));
    printf("instr %08x\n", w2>>26);
    printf("xx %08x\n", 0x20 & 0x3f);
    printf("instr %08x\n", w4);
    printf("instr %08x\n", r_funct(w3));
    switch (instrs[0] & R_MASK) {
    case M_ADD:
	// add instruction
	printf("add\n");
	break;
    case M_ADDU:
 	// add instruction
	printf("add\n");
	break;
    default:
 	// add instruction
	printf("**unknown**\n");
	break;
    }

    return 0;
}

