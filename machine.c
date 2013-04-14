#include <stdio.h>
#include <assert.h>
#include <string.h>

typedef unsigned int   word_t;
typedef signed   int   sword_t;
typedef unsigned char  byte_t;

enum InstrKind {
    INSTR_CTRL   = 0,
    INSTR_MEM    = 1,
    INSTR_TEST   = 2,
    INSTR_ARITH  = 3
};

enum ArithType {
    ADD = 0,
    SUB = 1,
    MUL = 2,
    DIV = 3
};

enum MemType {
    LOAD  = 1,
    STORE = 2,
    LOADI = 3    // load immediate
};

enum TestType {
    CMP = 0
};

enum CtrlType {
    HALT   = 0,
    BREQ   = 1,
    BRNE   = 2,
    GOTO   = 3
};

#pragma pack(push, 1)
union instruction {
    struct {
        word_t    kind  : 2;
        word_t    rest  : 30;
    };
    struct {
        word_t   kind: 2;
        word_t   type: 6;     // 2^6 = 64 values
    }  ctrl;
    struct {
        word_t    kind  : 2;
        word_t    type  : 6;
        word_t    dest  : 8;
        word_t    src1  : 8;
        word_t    src2  : 8;
    }  arith;
    struct {
        word_t   kind: 2;
        word_t   type: 6;
        word_t   reg : 8;
    }  mem;
    struct {
        word_t   kind: 2;
        word_t   type: 6;     // 2^6 = 64 values
        word_t   src1: 8;
        word_t   src2: 8;
    }  test ;
    word_t word;
};
#pragma pack(pop)

typedef union instruction instruction_t;

#define REG_COUNT 32

struct flags {
    word_t  running : 1;
    word_t  zero : 1;
};
typedef struct flags flags_t;

/*
 * context captures the state
 */
struct context {
    // local resources
    word_t  pc;      // pc   register
    flags_t flags;   // flag register
    word_t  regs[REG_COUNT];

    // shared resources
    word_t        * mem;
    instruction_t * instrs;
};
typedef struct context context_t;

/*
   graphical IR: every instruction is also a value (as it computes something)
 */

void run(context_t *ctx) {
    ctx->flags.running = 1;
    while (ctx->flags.running) {
        // IF -- instruction fetch
        word_t pc = ctx->pc++;
        instruction_t ins = ctx->instrs[pc];
        printf("Instruction kind %d\n", ins.kind);

        // ID -- instruction decode
        switch(ins.kind) {
        case INSTR_CTRL:
            switch (ins.ctrl.type) {
            case HALT:
                printf("Halting now.\n");
                ctx->flags.running  = 0;
                break;
	    case BREQ:
	    case BRNE:
		if (ctx->flags.zero == (ins.ctrl.type&1)) {
		    // TODO relative addressing ...
		    ctx->pc = ctx->instrs[pc];
		}
                break;
	    case BRNE:
                printf("Halting now.\n");
                ctx->flags.running  = 0;
                break;
            };
            break;

        case INSTR_ARITH:
            switch(ins.arith.type) {
            case ADD:
                // EX, WB
                ctx->regs[ins.arith.dest] =
                    (sword_t)ctx->regs[ins.arith.src1] +
                    (sword_t)ctx->regs[ins.arith.src2];
                break;
            case SUB:
                // EX, WB
                ctx->regs[ins.arith.dest] =
                    (sword_t)ctx->regs[ins.arith.src1] -
                    (sword_t)ctx->regs[ins.arith.src2];
                break;
            case MUL:
                // EX, WB
                ctx->regs[ins.arith.dest] =
                    (sword_t)ctx->regs[ins.arith.src1] *
                    (sword_t)ctx->regs[ins.arith.src2];
                break;
            case DIV:
                // EX, WB
                ctx->regs[ins.arith.dest] =
                    (sword_t)ctx->regs[ins.arith.src1] /
                    (sword_t)ctx->regs[ins.arith.src2];
                break;
            };
            break;
        case INSTR_MEM:
            switch(ins.mem.type) {
            case STORE:
                {
                    word_t *addr = ((word_t*)(ctx->instrs[ctx->pc].word));
                    ctx->pc++;
                    *addr = ctx->regs[ins.mem.reg];
                    //printf("STOREed reg %d to address %08x\n", ins.mem.reg, addr);
                }
                break;
            case LOAD:
            case LOADI:
                {
                    word_t *addr = ((word_t*)(ctx->instrs[ctx->pc].word));
                    ctx->pc++;
                    ctx->regs[ins.mem.reg] = (ins.mem.type == LOADI) ? (word_t)addr : *addr;
                    printf("LOADed reg %d to value %d\n", ins.mem.reg, ctx->regs[ins.mem.reg]);
                }
                break;
            }
            break;
        case INSTR_TEST:
            switch(ins.test.type) {
            case CMP:
                {
		    register sword_t a = (sword_t)ctx->regs[ins.test.src1];
		    register sword_t b = (sword_t)ctx->regs[ins.test.src2];
		    register sword_t r = a - b;
		    ctx->flags.zero = (r == 0);
                }
                break;
		// default:
                // assert("")
            }
	    break;
        default:
            printf("*** Unknown instruction %d ***\n", ins);
        }
    }
}

int main() {

    const int bytes_word  = sizeof(word_t);
    const int bytes_instr = sizeof(instruction_t);
    printf("sizeof word %d, sizeof instruction  %d\n", bytes_word, bytes_instr);
    assert(bytes_word == bytes_instr);

    word_t mem[512];
    instruction_t instrs[256];
    memset(instrs, 0, sizeof(instrs));

    /*
      for (i = 0; i != 20; i++) {
         sum += i
      }

      loadi i, 0
      loop:
      cmp  i, 20
      add  sum, sum, i 
      brne loop
      
     */

    // load i, 0
    instrs[0].kind = INSTR_MEM;
    instrs[0].mem.type = LOADI;
    instrs[0].mem.reg  = 0;
    instrs[1].word = 0;

    // loop: 2
    instrs[2].kind = INSTR_TEST;
    instrs[2].arith.type = CMP;
    instrs[2].arith.src1 = 0;
    instrs[2].arith.src2 = 0;
    instrs[2].arith.dest = 1;
    
    
    instrs[3].kind = INSTR_ARITH;
    instrs[3].arith.type = ADD;
    instrs[3].arith.src1 = 0;
    instrs[3].arith.src2 = 1;
    instrs[3].arith.dest = 2;
    instrs[4].kind = INSTR_ARITH;
    instrs[4].arith.type = SUB;
    instrs[4].arith.src1 = 0;
    instrs[4].arith.src2 = 2;
    instrs[4].arith.dest = 3;

    // instrs[2].kind =
    // instrs[2].kind = INSTR_CTRL;
    // instrs[2].ctrl.type = HALT;

    context_t ctx;
    memset(ctx.regs, 0, sizeof(ctx.regs));
    ctx.pc  = 0;
    ctx.mem = (word_t*)mem;
    ctx.instrs =  (instruction_t*)instrs;
    ctx.flags.running = 1;
    run(&ctx);

    int i = 0;
    for (i = 0; i < REG_COUNT; ++i) {
        printf("reg[%02d]: %d\n", i, ctx.regs[i]);
    }
}
