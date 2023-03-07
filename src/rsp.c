#include "types.h"
#include "sys.h"
#include "cpu.h"
#include "rsp.h"

#ifdef LLE

/* #define RSP_DEBUG */

#ifdef RSP_DEBUG
#define debugi(...) \
    {printf("rsp: 1%03X ", (rsp.pc-4) & 0xFFF); printf(__VA_ARGS__);}
#else
#define debugi(...) {}
#endif

#define pause() {if (getchar() < 0) exit(1);}

#define SP_MEM_ADDR     0
#define SP_DRAM_ADDR    1
#define SP_RD_LEN       2
#define SP_WR_LEN       3
#define SP_STATUS       4
#define SP_DMA_FULL     5
#define SP_DMA_BUSY     6
#define SP_SEMAPHORE    7
#define DPC_START       8
#define DPC_END         9
#define DPC_CURRENT     10
#define DPC_STATUS      11
#define DPC_CLOCK       12
#define DPC_BUFBUSY     13
#define DPC_PIPEBUSY    14
#define DPC_TMEM        15

#define VX  (AX_H >> 1)

#define INST_F0     (inst >> 26       )
#define INST_F1     (inst >>  0 & 0x3F)
#define INST_R0     (inst >> 21 & 0x1F)
#define INST_R1     (inst >> 16 & 0x1F)
#define INST_R2     (inst >> 11 & 0x1F)
#define INST_R3     (inst >>  6 & 0x1F)
#define INST_E0     (inst >> 21 & 0x0F)
#define INST_E1     (inst >> 11 & 0x0F)
#define INST_E2     (inst >>  7 & 0x0F)

#define INST_OP     INST_F0
#define INST_FUNC   INST_F1
#define INST_RS     INST_R0
#define INST_RT     INST_R1
#define INST_RD     INST_R2
#define INST_SA     INST_R3
#define INST_VT     INST_R1
#define INST_VS     INST_R2
#define INST_VD     INST_R3
#define INST_EV     INST_E0
#define INST_DE     INST_E1
#define INST_E      INST_E2
#define INST_OFFB   ((s8)(inst << 1) >> 1)
#define INST_OFFS   ((s8)(inst << 1)     )
#define INST_OFFL   ((s8)(inst << 1) << 1)
#define INST_OFFD   ((s8)(inst << 1) << 2)
#define INST_OFFQ   ((s8)(inst << 1) << 3)
#define INST_IMMS   ((s16)inst)
#define INST_IMMU   ((u16)inst)
#define INST_IMMH   (inst << 16)
#define INST_BDST   (rsp.pc + ((s16)inst << 2))
#define INST_JDST   ((rsp.pc & 0xF0000000) | (inst << 2 & 0x0FFFFFFC))

#define INST_EVS    str_element[INST_EV]
#define INST_DES    str_element[INST_DE]

#define rs  rsp.reg[INST_RS]
#define rt  rsp.reg[INST_RT]
#define rd  rsp.reg[INST_RD]
#define vt  rsp.vreg[INST_VT]
#define vs  rsp.vreg[INST_VS]
#define vd  rsp.vreg[INST_VD]
#define ev  rsp_element[INST_EV]

typedef void RSPCALL(u32 inst);

static u8 rsp_mem[0x2000];
static RSP rsp;

static const u8 rsp_element[][8] =
{
    {0, 1, 2, 3, 4, 5, 6, 7},
    {0, 1, 2, 3, 4, 5, 6, 7},
    {0, 0, 2, 2, 4, 4, 6, 6},
    {1, 1, 3, 3, 5, 5, 7, 7},
    {0, 0, 0, 0, 4, 4, 4, 4},
    {1, 1, 1, 1, 5, 5, 5, 5},
    {2, 2, 2, 2, 6, 6, 6, 6},
    {3, 3, 3, 3, 7, 7, 7, 7},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {2, 2, 2, 2, 2, 2, 2, 2},
    {3, 3, 3, 3, 3, 3, 3, 3},
    {4, 4, 4, 4, 4, 4, 4, 4},
    {5, 5, 5, 5, 5, 5, 5, 5},
    {6, 6, 6, 6, 6, 6, 6, 6},
    {7, 7, 7, 7, 7, 7, 7, 7},
};

#ifdef DEBUG
static const char *const str_element[] =
{
    "",
    "",
    "[0q]",
    "[1q]",
    "[0h]",
    "[1h]",
    "[2h]",
    "[3h]",
    "[0]",
    "[1]",
    "[2]",
    "[3]",
    "[4]",
    "[5]",
    "[6]",
    "[7]",
};
#endif

static u32 rsp_dma_rd(u32 len)
{
    uint bank = rsp.mem_addr & 0x1000;
    uint mem  = rsp.mem_addr & 0xFF8;
    PTR  dram = rsp.dram_addr & 0xFFFFF8;
    uint skip   = (len >> 20 & 0xFF8);
    uint count  = (len >> 12 &  0xFF);
    uint length = (len >>  0 & 0xFF8) + 8;
    uint i;
    for (i = 0; i <= count; i++)
    {
        __byteswap(&rsp_mem[bank | (mem & 0xFFF)], &cpu_dram[dram], length);
        mem  += length;
        dram += length;
        dram += skip;
    }
    rsp.mem_addr  = bank | mem;
    rsp.dram_addr = dram;
    return (len & 0xFF800000) | 0xFF8;
}

static u32 rsp_dma_wr(u32 len)
{
    uint bank = rsp.mem_addr & 0x1000;
    uint mem  = rsp.mem_addr & 0xFF8;
    PTR  dram = rsp.dram_addr & 0xFFFFF8;
    uint skip   = (len >> 20 & 0xFF8);
    uint count  = (len >> 12 &  0xFF);
    uint length = (len >>  0 & 0xFF8) + 8;
    uint i;
    for (i = 0; i <= count; i++)
    {
        __byteswap(&cpu_dram[dram], &rsp_mem[bank | (mem & 0xFFF)], length);
        mem  += length;
        dram += length;
        dram += skip;
    }
    rsp.mem_addr  = bank | mem;
    rsp.dram_addr = dram;
    return (len & 0xFF800000) | 0xFF8;
}

static void rsp_branch(uint addr)
{
    if (rsp.bcode == 0)
    {
        rsp.baddr = addr;
        rsp.bcode = 2;
    }
}

static s16 rsp_clamp_i(s64 x)
{
    s16 h = x >> 32;
    s16 m = x >> 16;
    if (h >  0 || (h == 0 && m <  0)) return -0x8000;
    if (h < -1 || (h <  0 && m >= 0)) return +0x7FFF;
    return m;
}

static u16 rsp_clamp_f(s64 x)
{
    s16 h = x >> 32;
    s16 m = x >> 16;
    if (h >  0 || (h == 0 && m <  0)) return 0x0000;
    if (h < -1 || (h <  0 && m >= 0)) return 0xFFFF;
    return x;
}

static u16 rsp_clamp_u(s64 x)
{
    s16 h = x >> 32;
    s16 m = x >> 16;
    if (h >= 0 && m < 0x0000) return 0x0000;
    if (h <  0 && m < 0x7FFF) return 0xFFFF;
    return m;
}

static s16 rsp_clamp_s(int x)
{
    if (x < -0x7FFF) return -0x7FFF;
    if (x > +0x7FFF) return +0x7FFF;
    return x;
}

static void rsp_null(UNUSED u32 inst)
{
    rsp.bcode = -1;
    wdebug("illegal instruction 0x%08" FMT_X "\n", inst);
}

static void rsp_sll(u32 inst)
{
    rd = (u32)rt << INST_SA;
    if (inst == 0x00000000U) {debugi("nop\n");}
    else {debugi("sll $%d, $%d, %d\n", INST_RD, INST_RT, INST_SA);}
}

static void rsp_srl(u32 inst)
{
    rd = (u32)rt >> INST_SA;
    debugi("srl $%d, $%d, %d\n", INST_RD, INST_RT, INST_SA);
}

static void rsp_sra(u32 inst)
{
    rd = (s32)rt >> INST_SA;
    debugi("sra $%d, $%d, %d\n", INST_RD, INST_RT, INST_SA);
}

static void rsp_sllv(u32 inst)
{
    rd = (u32)rt << (rs & 0x1F);
    debugi("sllv $%d, $%d, $%d\n", INST_RD, INST_RT, INST_RS);
}

static void rsp_srlv(u32 inst)
{
    rd = (u32)rt >> (rs & 0x1F);
    debugi("srlv $%d, $%d, $%d\n", INST_RD, INST_RT, INST_RS);
}

static void rsp_srav(u32 inst)
{
    rd = (s32)rt >> (rs & 0x1F);
    debugi("srav $%d, $%d, $%d\n", INST_RD, INST_RT, INST_RS);
}

static void rsp_jr(u32 inst)
{
    rsp_branch(rs);
    debugi("jr $%d\n", INST_RS);
}

static void rsp_jalr(u32 inst)
{
    rd = rsp.pc + 4;
    rsp_branch(rs);
    debugi("jalr $%d, $%d\n", INST_RD, INST_RS);
}

static void rsp_break(UNUSED u32 inst)
{
    rsp.bcode = -1;
    debugi("break\n");
}

static void rsp_add(u32 inst)
{
    rd = rs + rt;
    debugi("add $%d, $%d, $%d\n", INST_RD, INST_RS, INST_RT);
}

static void rsp_sub(u32 inst)
{
    rd = rs - rt;
    debugi("sub $%d, $%d, $%d\n", INST_RD, INST_RS, INST_RT);
}

static void rsp_and(u32 inst)
{
    rd = rs & rt;
    debugi("and $%d, $%d, $%d\n", INST_RD, INST_RS, INST_RT);
}

static void rsp_or(u32 inst)
{
    rd = rs | rt;
    debugi("or $%d, $%d, $%d\n", INST_RD, INST_RS, INST_RT);
}

static void rsp_xor(u32 inst)
{
    rd = rs ^ rt;
    debugi("xor $%d, $%d, $%d\n", INST_RD, INST_RS, INST_RT);
}

static void rsp_nor(u32 inst)
{
    rd = ~(rs | rt);
    debugi("nor $%d, $%d, $%d\n", INST_RD, INST_RS, INST_RT);
}

static void rsp_slt(u32 inst)
{
    rd = rs < rt;
    debugi("slt $%d, $%d, $%d\n", INST_RD, INST_RS, INST_RT);
}

static void rsp_sltu(u32 inst)
{
    rd = (u32)rs < (u32)rt;
    debugi("sltu $%d, $%d, $%d\n", INST_RD, INST_RS, INST_RT);
}

static RSPCALL *const rsp_special_table[] =
{
    /* 0x00 */  rsp_sll,
    /* 0x01 */  rsp_null,
    /* 0x02 */  rsp_srl,
    /* 0x03 */  rsp_sra,
    /* 0x04 */  rsp_sllv,
    /* 0x05 */  rsp_null,
    /* 0x06 */  rsp_srlv,
    /* 0x07 */  rsp_srav,
    /* 0x08 */  rsp_jr,
    /* 0x09 */  rsp_jalr,
    /* 0x0A */  rsp_null,
    /* 0x0B */  rsp_null,
    /* 0x0C */  rsp_null,
    /* 0x0D */  rsp_break,
    /* 0x0E */  rsp_null,
    /* 0x0F */  rsp_null,
    /* 0x10 */  rsp_null,
    /* 0x11 */  rsp_null,
    /* 0x12 */  rsp_null,
    /* 0x13 */  rsp_null,
    /* 0x14 */  rsp_null,
    /* 0x15 */  rsp_null,
    /* 0x16 */  rsp_null,
    /* 0x17 */  rsp_null,
    /* 0x18 */  rsp_null,
    /* 0x19 */  rsp_null,
    /* 0x1A */  rsp_null,
    /* 0x1B */  rsp_null,
    /* 0x1C */  rsp_null,
    /* 0x1D */  rsp_null,
    /* 0x1E */  rsp_null,
    /* 0x1F */  rsp_null,
    /* 0x20 */  rsp_add,
    /* 0x21 */  rsp_add,
    /* 0x22 */  rsp_sub,
    /* 0x23 */  rsp_sub,
    /* 0x24 */  rsp_and,
    /* 0x25 */  rsp_or,
    /* 0x26 */  rsp_xor,
    /* 0x27 */  rsp_nor,
    /* 0x28 */  rsp_null,
    /* 0x29 */  rsp_null,
    /* 0x2A */  rsp_slt,
    /* 0x2B */  rsp_sltu,
    /* 0x2C */  rsp_null,
    /* 0x2D */  rsp_null,
    /* 0x2E */  rsp_null,
    /* 0x2F */  rsp_null,
    /* 0x30 */  rsp_null, /* tge */
    /* 0x31 */  rsp_null, /* tgeu */
    /* 0x32 */  rsp_null, /* tlt */
    /* 0x33 */  rsp_null, /* tltu */
    /* 0x34 */  rsp_null, /* teq */
    /* 0x35 */  rsp_null,
    /* 0x36 */  rsp_null, /* tne */
    /* 0x37 */  rsp_null,
    /* 0x38 */  rsp_null,
    /* 0x39 */  rsp_null,
    /* 0x3A */  rsp_null,
    /* 0x3B */  rsp_null,
    /* 0x3C */  rsp_null,
    /* 0x3D */  rsp_null,
    /* 0x3E */  rsp_null,
    /* 0x3F */  rsp_null,
};

static void rsp_special(u32 inst)
{
    rsp_special_table[INST_FUNC](inst);
}

static void rsp_bltz(u32 inst)
{
    if (rs <  0) rsp_branch(INST_BDST);
    debugi("bltz $%d, 0x%08X\n", INST_RS, INST_BDST);
}

static void rsp_bgez(u32 inst)
{
    if (rs >= 0) rsp_branch(INST_BDST);
    debugi("bgez $%d, 0x%08X\n", INST_RS, INST_BDST);
}

static void rsp_bltzal(u32 inst)
{
    if (rs <  0)
    {
        rsp.reg[31] = rsp.pc + 4;
        rsp_branch(INST_BDST);
    }
    debugi("bltzal $%d, 0x%08X\n", INST_RS, INST_BDST);
}

static void rsp_bgezal(u32 inst)
{
    if (rs >= 0)
    {
        rsp.reg[31] = rsp.pc + 4;
        rsp_branch(INST_BDST);
    }
    debugi("bgezal $%d, 0x%08X\n", INST_RS, INST_BDST);
}

static RSPCALL *const rsp_regimm_table[] =
{
    /* 0x00 */  rsp_bltz,
    /* 0x01 */  rsp_bgez,
    /* 0x02 */  rsp_null,
    /* 0x03 */  rsp_null,
    /* 0x04 */  rsp_null,
    /* 0x05 */  rsp_null,
    /* 0x06 */  rsp_null,
    /* 0x07 */  rsp_null,
    /* 0x08 */  rsp_null, /* tgei */
    /* 0x09 */  rsp_null, /* tgeiu */
    /* 0x0A */  rsp_null, /* tlti */
    /* 0x0B */  rsp_null, /* tltiu */
    /* 0x0C */  rsp_null, /* teqi */
    /* 0x0D */  rsp_null,
    /* 0x0E */  rsp_null, /* tnei */
    /* 0x0F */  rsp_null,
    /* 0x10 */  rsp_bltzal,
    /* 0x11 */  rsp_bgezal,
    /* 0x12 */  rsp_null,
    /* 0x13 */  rsp_null,
    /* 0x14 */  rsp_null,
    /* 0x15 */  rsp_null,
    /* 0x16 */  rsp_null,
    /* 0x17 */  rsp_null,
    /* 0x18 */  rsp_null,
    /* 0x19 */  rsp_null,
    /* 0x1A */  rsp_null,
    /* 0x1B */  rsp_null,
    /* 0x1C */  rsp_null,
    /* 0x1D */  rsp_null,
    /* 0x1E */  rsp_null,
    /* 0x1F */  rsp_null,
};

static void rsp_regimm(u32 inst)
{
    rsp_regimm_table[INST_RT](inst);
}

static void rsp_j(u32 inst)
{
    rsp_branch(INST_JDST);
    debugi("j 0x%08X\n", INST_JDST);
}

static void rsp_jal(u32 inst)
{
    rsp.reg[31] = rsp.pc + 4;
    rsp_branch(INST_JDST);
    debugi("jal 0x%08X\n", INST_JDST);
}

static void rsp_beq(u32 inst)
{
    if (rs == rt) rsp_branch(INST_BDST);
    debugi("beq $%d, $%d, 0x%08X\n", INST_RS, INST_RT, INST_BDST);
}

static void rsp_bne(u32 inst)
{
    if (rs != rt) rsp_branch(INST_BDST);
    debugi("bne $%d, $%d, 0x%08X\n", INST_RS, INST_RT, INST_BDST);
}

static void rsp_blez(u32 inst)
{
    if (rs <= 0) rsp_branch(INST_BDST);
    debugi("blez $%d, 0x%08X\n", INST_RS, INST_BDST);
}

static void rsp_bgtz(u32 inst)
{
    if (rs >  0) rsp_branch(INST_BDST);
    debugi("bgtz $%d, 0x%08X\n", INST_RS, INST_BDST);
}

static void rsp_addi(u32 inst)
{
    rt = rs + INST_IMMS;
    debugi("addi $%d, $%d, 0x%04X\n", INST_RT, INST_RS, INST_IMMU);
}

static void rsp_slti(u32 inst)
{
    rt = rs < INST_IMMS;
    debugi("slti $%d, $%d, 0x%04X\n", INST_RT, INST_RS, INST_IMMU);
}

static void rsp_sltiu(u32 inst)
{
    rt = (u32)rs < (u32)INST_IMMS;
    debugi("sltiu $%d, $%d, 0x%04X\n", INST_RT, INST_RS, INST_IMMU);
}

static void rsp_andi(u32 inst)
{
    rt = rs & INST_IMMU;
    debugi("andi $%d, $%d, 0x%04X\n", INST_RT, INST_RS, INST_IMMU);
}

static void rsp_ori(u32 inst)
{
    rt = rs | INST_IMMU;
    debugi("ori $%d, $%d, 0x%04X\n", INST_RT, INST_RS, INST_IMMU);
}

static void rsp_xori(u32 inst)
{
    rt = rs ^ INST_IMMU;
    debugi("xori $%d, $%d, 0x%04X\n", INST_RT, INST_RS, INST_IMMU);
}

static void rsp_lui(u32 inst)
{
    rt = INST_IMMH;
    debugi("lui $%d, $%d, 0x%04X\n", INST_RT, INST_RS, INST_IMMU);
}

static void rsp_mfc0(u32 inst)
{
    switch (INST_RD)
    {
        case SP_MEM_ADDR:   rt = rsp.mem_addr;  break;
        case SP_DRAM_ADDR:  rt = rsp.dram_addr; break;
        case SP_RD_LEN:     rt = rsp.rd_len;    break;
        case SP_WR_LEN:     rt = rsp.wr_len;    break;
        case SP_STATUS:     rt = 0;             break;
        case SP_DMA_FULL:   rt = 0;             break;
        case SP_DMA_BUSY:   rt = 0;             break;
        case SP_SEMAPHORE:  rt = 0;             break;
        case DPC_START:     rt = 0;             break;
        case DPC_END:       rt = 0;             break;
        case DPC_CURRENT:   rt = 0;             break;
        case DPC_STATUS:    rt = 0;             break;
        case DPC_CLOCK:     rt = 0;             break;
        case DPC_BUFBUSY:   rt = 0;             break;
        case DPC_PIPEBUSY:  rt = 0;             break;
        case DPC_TMEM:      rt = 0;             break;
        default:
            wdebug("rsp: mfc0 $%d, $c%d\n", INST_RT, INST_RD);
            break;
    }
    debugi("mfc0 $%d, $c%d\n", INST_RT, INST_RD);
}

static void rsp_mtc0(u32 inst)
{
    if ((INST_RD == SP_RD_LEN || INST_RD == SP_WR_LEN) && rt == -1)
    {
        wdebug("rsp: bad dma\n");
        rsp.bcode = -1;
        return;
    }
    switch (INST_RD)
    {
        case SP_MEM_ADDR:   rsp.mem_addr  = rt; break;
        case SP_DRAM_ADDR:  rsp.dram_addr = rt; break;
        case SP_RD_LEN:     rsp.rd_len = rsp_dma_rd(rt); break;
        case SP_WR_LEN:     rsp.wr_len = rsp_dma_wr(rt); break;
        case SP_STATUS:     break;
        case SP_DMA_FULL:   break;
        case SP_DMA_BUSY:   break;
        case SP_SEMAPHORE:  break;
        case DPC_START:     break;
        case DPC_END:       break;
        case DPC_CURRENT:   break;
        case DPC_STATUS:    break;
        case DPC_CLOCK:     break;
        case DPC_BUFBUSY:   break;
        case DPC_PIPEBUSY:  break;
        case DPC_TMEM:      break;
        default:
            wdebug("rsp: mtc0 $%d, $c%d\n", INST_RT, INST_RD);
            break;
    }
    debugi("mtc0 $%d, $c%d\n", INST_RT, INST_RD);
}

static RSPCALL *const rsp_cop0_table[] =
{
    /* 0x00 */  rsp_mfc0,
    /* 0x01 */  rsp_null,
    /* 0x02 */  rsp_null,
    /* 0x03 */  rsp_null,
    /* 0x04 */  rsp_mtc0,
    /* 0x05 */  rsp_null,
    /* 0x06 */  rsp_null,
    /* 0x07 */  rsp_null,
    /* 0x08 */  rsp_null,
    /* 0x09 */  rsp_null,
    /* 0x0A */  rsp_null,
    /* 0x0B */  rsp_null,
    /* 0x0C */  rsp_null,
    /* 0x0D */  rsp_null,
    /* 0x0E */  rsp_null,
    /* 0x0F */  rsp_null,
    /* 0x10 */  rsp_null,
    /* 0x11 */  rsp_null,
    /* 0x12 */  rsp_null,
    /* 0x13 */  rsp_null,
    /* 0x14 */  rsp_null,
    /* 0x15 */  rsp_null,
    /* 0x16 */  rsp_null,
    /* 0x17 */  rsp_null,
    /* 0x18 */  rsp_null,
    /* 0x19 */  rsp_null,
    /* 0x1A */  rsp_null,
    /* 0x1B */  rsp_null,
    /* 0x1C */  rsp_null,
    /* 0x1D */  rsp_null,
    /* 0x1E */  rsp_null,
    /* 0x1F */  rsp_null,
};

static void rsp_cop0(u32 inst)
{
    rsp_cop0_table[INST_RS](inst);
}

static void rsp_mfc2(u32 inst)
{
    rt = vs.s[INST_E >> 1];
    debugi("mfc2 $%d, $v%d[%d]\n", INST_RT, INST_RD, INST_E);
}

static void rsp_cfc2(UNUSED u32 inst)
{
    edebug("rsp: cfc2 $%d, $c%d\n", rt, rd);
}

static void rsp_mtc2(u32 inst)
{
    vs.s[INST_E >> 1] = rt;
    debugi("mtc2 $%d, $v%d[%d]\n", INST_RT, INST_RD, INST_E);
}

static void rsp_ctc2(UNUSED u32 inst)
{
    edebug("rsp: ctc2 $%d, $c%d\n", rt, rd);
}

static void rsp_vmulf(u32 inst)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        s64 x = (int)vs.s[i] * (int)vt.s[ev[i]];
        rsp.acc[i] = (x << 1) + 0x8000;
        vd.s[i] = rsp_clamp_i(rsp.acc[i]);
    }
    debugi("vmulf $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vmulu(u32 inst)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        s64 x = (int)vs.s[i] * (int)vt.s[ev[i]];
        rsp.acc[i] = (x << 1) + 0x8000;
        vd.u[i] = rsp_clamp_u(rsp.acc[i]);
    }
    debugi("vmulu $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vrndp(UNUSED u32 inst)
{
    edebug(
        "rsp: vrndp $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS
    );
}

static void rsp_vmulq(UNUSED u32 inst)
{
    edebug(
        "rsp: vmulq $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS
    );
}

static void rsp_vmudl(u32 inst)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        s64 x = (int)vs.u[i] * (int)vt.u[ev[i]];
        rsp.acc[i] = x >> 16;
        vd.s[i] = rsp.acc[i];
    }
    debugi("vmudl $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vmudm(u32 inst)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        s64 x = (int)vs.s[i] * (int)vt.u[ev[i]];
        rsp.acc[i] = x;
        vd.s[i] = rsp.acc[i] >> 16;
    }
    debugi("vmudm $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vmudn(u32 inst)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        s64 x = (int)vs.u[i] * (int)vt.s[ev[i]];
        rsp.acc[i] = x;
        vd.s[i] = rsp.acc[i];
    }
    debugi("vmudn $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vmudh(u32 inst)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        s64 x = (int)vs.s[i] * (int)vt.s[ev[i]];
        rsp.acc[i] = x << 16;
        vd.s[i] = rsp_clamp_i(rsp.acc[i]);
    }
    debugi("vmudh $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vmacf(u32 inst)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        s64 x = (int)vs.s[i] * (int)vt.s[ev[i]];
        rsp.acc[i] += x << 1;
        vd.s[i] = rsp_clamp_i(rsp.acc[i]);
    }
    debugi("vmacf $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vmacu(u32 inst)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        s64 x = (int)vs.s[i] * (int)vt.s[ev[i]];
        rsp.acc[i] += x << 1;
        vd.u[i] = rsp_clamp_u(rsp.acc[i]);

    }
    debugi("vmacu $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vrndn(UNUSED u32 inst)
{
    edebug(
        "rsp: vrndn $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS
    );
}

static void rsp_vmacq(UNUSED u32 inst)
{
    edebug(
        "rsp: vmacq $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS
    );
}

static void rsp_vmadl(u32 inst)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        s64 x = (int)vs.u[i] * (int)vt.u[ev[i]];
        rsp.acc[i] += x >> 16;
        vd.s[i] = rsp_clamp_f(rsp.acc[i]);
    }
    debugi("vmadl $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vmadm(u32 inst)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        s64 x = (int)vs.s[i] * (int)vt.u[ev[i]];
        rsp.acc[i] += x;
        vd.s[i] = rsp_clamp_i(rsp.acc[i]);
    }
    debugi("vmadm $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vmadn(u32 inst)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        s64 x = (int)vs.u[i] * (int)vt.s[ev[i]];
        rsp.acc[i] += x;
        vd.s[i] = rsp_clamp_f(rsp.acc[i]);
    }
    debugi("vmadn $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vmadh(u32 inst)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        s64 x = (int)vs.s[i] * (int)vt.s[ev[i]];
        rsp.acc[i] += x << 16;
        vd.s[i] = rsp_clamp_i(rsp.acc[i]);
    }
    debugi("vmadh $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vadd(u32 inst)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        int x = (int)vs.s[i] + (int)vt.s[ev[i]] + (rsp.vco >> i & 1);
        vd.s[i] = rsp.acc[i] = rsp_clamp_s(x);
    }
    rsp.vco &= ~0xFF;
    debugi("vadd $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vsub(u32 inst)
{
    int i;
    for (i = 0; i < 8; i++)
    {
        int x = (int)vs.s[i] - (int)vt.s[ev[i]] - (rsp.vco >> i & 1);
        vd.s[i] = rsp.acc[i] = rsp_clamp_s(x);
    }
    rsp.vco &= ~0xFF;
    debugi("vsub $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vabs(UNUSED u32 inst)
{
    edebug(
        "rsp: vabs $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS
    );
}

static void rsp_vaddc(u32 inst)
{
    int i;
    rsp.vco = 0;
    for (i = 0; i < 8; i++)
    {
        int x = (int)vs.s[i] + (int)vt.s[ev[i]];
        vd.s[i] = rsp.acc[i] = x;
        rsp.vco |= (x >> 16 & 1) << i;
    }
    debugi("vaddc $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vsubc(u32 inst)
{
    int i;
    rsp.vco = 0;
    for (i = 0; i < 8; i++)
    {
        int x = (int)vs.s[i] - (int)vt.s[ev[i]];
        vd.s[i] = rsp.acc[i] = x;
        rsp.vco |= (x >> 16 != 0) << i << 8 | (x >> 16 < 0) << i;
    }
    debugi("vsubc $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vsar(u32 inst)
{
    int i;
    switch (INST_EV)
    {
        case 8|0:
            for (i = 0; i < 8; i++) vd.s[i] = rsp.acc[i] >> 32;
            break;
        case 8|1:
            for (i = 0; i < 8; i++) vd.s[i] = rsp.acc[i] >> 16;
            break;
        case 8|2:
            for (i = 0; i < 8; i++) vd.s[i] = rsp.acc[i] >> 0;
            break;
        default:
            for (i = 0; i < 8; i++) vd.s[i] = 0;
            break;
    }
    debugi("vsar $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vlt(u32 inst)
{
    int i;
    uint vco = rsp.vco;
    uint vce = rsp.vce;
    rsp.vcc = 0;
    rsp.vco = 0;
    rsp.vce = 0;
    for (i = 0; i < 8; i++)
    {
        int s = vs.s[i];
        int t = vt.s[ev[i]];
        if (s < t || (s == t && ((vco >> i & 1) && !(vce >> i & 1))))
        {
            vd.s[i] = rsp.acc[i] = s;
            rsp.vcc |= 1 << i;
        }
        else
        {
            vd.s[i] = rsp.acc[i] = t;
        }
    }
    debugi("vlt $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_veq(UNUSED u32 inst)
{
    edebug(
        "rsp: veq $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS
    );
}

static void rsp_vne(UNUSED u32 inst)
{
    edebug(
        "rsp: vne $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS
    );
}

static void rsp_vge(u32 inst)
{
    int i;
    uint vco = rsp.vco;
    uint vce = rsp.vce;
    rsp.vcc = 0;
    rsp.vco = 0;
    rsp.vce = 0;
    for (i = 0; i < 8; i++)
    {
        int s = vs.s[i];
        int t = vt.s[ev[i]];
        if (s > t || (s == t && (!(vco >> i & 1) || (vce >> i & 1))))
        {
            vd.s[i] = rsp.acc[i] = s;
            rsp.vcc |= 1 << i;
        }
        else
        {
            vd.s[i] = rsp.acc[i] = t;
        }
    }
    debugi("vge $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vcl(u32 inst)
{
    int i;
    uint vcc = rsp.vcc;
    uint vco = rsp.vco;
    uint vce = rsp.vce;
    rsp.vcc = 0;
    rsp.vco = 0;
    rsp.vce = 0;
    for (i = 0; i < 8; i++)
    {
        uint ge   = vcc >> 8 >> i & 1;
        uint le   = vcc >> 0 >> i & 1;
        uint neq  = vco >> 8 >> i & 1;
        uint sign = vco >> 0 >> i & 1;
        uint e    = vce      >> i & 1;
        int di;
        if (sign)
        {
            di = vs.s[i] + vt.s[ev[i]];
            if (!neq)
            {
                le = e ?
                    ((di & (1 << 16)) == 0 || di <= (1 << 16)) :
                    ((di & (1 << 16)) == 0 && di <= (1 << 16));
            }
            di = le ? -vt.s[ev[i]] : vs.s[i];
        }
        else
        {
            di = vs.s[i] - vt.s[ev[i]];
            if (!neq) ge = di >= 0;
            di = ge ? vt.s[ev[i]] : vs.s[i];
        }
        vd.s[i] = rsp.acc[i] = di;
        rsp.vcc |= ge << i << 8 | le << i;
    }
    debugi("vcl $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vch(UNUSED u32 inst)
{
    edebug(
        "rsp: vch $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS
    );
}

static void rsp_vcr(UNUSED u32 inst)
{
    edebug(
        "rsp: vcr $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS
    );
}

static void rsp_vmrg(UNUSED u32 inst)
{
    edebug(
        "rsp: vmrg $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS
    );
}

static void rsp_vand(u32 inst)
{
    int i;
    for (i = 0; i < 8; i++) vd.s[i] = rsp.acc[i] = vs.s[i] & vt.s[ev[i]];
    debugi("vand $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vnand(u32 inst)
{
    int i;
    for (i = 0; i < 8; i++) vd.s[i] = rsp.acc[i] = ~(vs.s[i] & vt.s[ev[i]]);
    debugi("vnand $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vor(u32 inst)
{
    int i;
    for (i = 0; i < 8; i++) vd.s[i] = rsp.acc[i] = vs.s[i] | vt.s[ev[i]];
    debugi("vor $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vnor(u32 inst)
{
    int i;
    for (i = 0; i < 8; i++) vd.s[i] = rsp.acc[i] = ~(vs.s[i] | vt.s[ev[i]]);
    debugi("vnor $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vxor(u32 inst)
{
    int i;
    for (i = 0; i < 8; i++) vd.s[i] = rsp.acc[i] = vs.s[i] ^ vt.s[ev[i]];
    debugi("vxor $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vnxor(u32 inst)
{
    int i;
    for (i = 0; i < 8; i++) vd.s[i] = rsp.acc[i] = ~(vs.s[i] ^ vt.s[ev[i]]);
    debugi("vnxor $v%d, $v%d, $v%d%s\n", INST_VD, INST_VS, INST_VT, INST_EVS);
}

static void rsp_vrcp(UNUSED u32 inst)
{
    edebug("rsp: vrcp $v%d%s, $v%d%s\n", INST_VD, INST_DES, INST_VT, INST_EVS);
}

static void rsp_vrcpl(UNUSED u32 inst)
{
    edebug("rsp: vrcpl $v%d%s, $v%d%s\n", INST_VD, INST_DES, INST_VT, INST_EVS);
}

static void rsp_vrcph(UNUSED u32 inst)
{
    edebug("rsp: vrcph $v%d%s, $v%d%s\n", INST_VD, INST_DES, INST_VT, INST_EVS);
}

static void rsp_vmov(UNUSED u32 inst)
{
    edebug("rsp: vmov $v%d%s, $v%d%s\n", INST_VD, INST_DES, INST_VT, INST_EVS);
}

static void rsp_vrsq(UNUSED u32 inst)
{
    edebug("rsp: vrsq $v%d%s, $v%d%s\n", INST_VD, INST_DES, INST_VT, INST_EVS);
}

static void rsp_vrsql(UNUSED u32 inst)
{
    edebug("rsp: vrsql $v%d%s, $v%d%s\n", INST_VD, INST_DES, INST_VT, INST_EVS);
}

static void rsp_vrsqh(UNUSED u32 inst)
{
    edebug("rsp: vrsqh $v%d%s, $v%d%s\n", INST_VD, INST_DES, INST_VT, INST_EVS);
}

static void rsp_vnop(UNUSED u32 inst)
{
    debugi("vnop\n");
}

static RSPCALL *const rsp_cop2_func_table[] =
{
    /* 0x00 */  rsp_vmulf,
    /* 0x01 */  rsp_vmulu,
    /* 0x02 */  rsp_vrndp,
    /* 0x03 */  rsp_vmulq,
    /* 0x04 */  rsp_vmudl,
    /* 0x05 */  rsp_vmudm,
    /* 0x06 */  rsp_vmudn,
    /* 0x07 */  rsp_vmudh,
    /* 0x08 */  rsp_vmacf,
    /* 0x09 */  rsp_vmacu,
    /* 0x0A */  rsp_vrndn,
    /* 0x0B */  rsp_vmacq,
    /* 0x0C */  rsp_vmadl,
    /* 0x0D */  rsp_vmadm,
    /* 0x0E */  rsp_vmadn,
    /* 0x0F */  rsp_vmadh,
    /* 0x10 */  rsp_vadd,
    /* 0x11 */  rsp_vsub,
    /* 0x12 */  rsp_null,
    /* 0x13 */  rsp_vabs,
    /* 0x14 */  rsp_vaddc,
    /* 0x15 */  rsp_vsubc,
    /* 0x16 */  rsp_null,
    /* 0x17 */  rsp_null,
    /* 0x18 */  rsp_null,
    /* 0x19 */  rsp_null,
    /* 0x1A */  rsp_null,
    /* 0x1B */  rsp_null,
    /* 0x1C */  rsp_null,
    /* 0x1D */  rsp_vsar,
    /* 0x1E */  rsp_null,
    /* 0x1F */  rsp_null,
    /* 0x20 */  rsp_vlt,
    /* 0x21 */  rsp_veq,
    /* 0x22 */  rsp_vne,
    /* 0x23 */  rsp_vge,
    /* 0x24 */  rsp_vcl,
    /* 0x25 */  rsp_vch,
    /* 0x26 */  rsp_vcr,
    /* 0x27 */  rsp_vmrg,
    /* 0x28 */  rsp_vand,
    /* 0x29 */  rsp_vnand,
    /* 0x2A */  rsp_vor,
    /* 0x2B */  rsp_vnor,
    /* 0x2C */  rsp_vxor,
    /* 0x2D */  rsp_vnxor,
    /* 0x2E */  rsp_null,
    /* 0x2F */  rsp_null,
    /* 0x30 */  rsp_vrcp,
    /* 0x31 */  rsp_vrcpl,
    /* 0x32 */  rsp_vrcph,
    /* 0x33 */  rsp_vmov,
    /* 0x34 */  rsp_vrsq,
    /* 0x35 */  rsp_vrsql,
    /* 0x36 */  rsp_vrsqh,
    /* 0x37 */  rsp_vnop,
    /* 0x38 */  rsp_null,
    /* 0x39 */  rsp_null,
    /* 0x3A */  rsp_null,
    /* 0x3B */  rsp_null,
    /* 0x3C */  rsp_null,
    /* 0x3D */  rsp_null,
    /* 0x3E */  rsp_null,
    /* 0x3F */  rsp_null,
};

static void rsp_cop2_func(u32 inst)
{
    rsp_cop2_func_table[INST_FUNC](inst);
}

static RSPCALL *const rsp_cop2_table[] =
{
    /* 0x00 */  rsp_mfc2,
    /* 0x01 */  rsp_null,
    /* 0x02 */  rsp_cfc2,
    /* 0x03 */  rsp_null,
    /* 0x04 */  rsp_mtc2,
    /* 0x05 */  rsp_null,
    /* 0x06 */  rsp_ctc2,
    /* 0x07 */  rsp_null,
    /* 0x08 */  rsp_null,
    /* 0x09 */  rsp_null,
    /* 0x0A */  rsp_null,
    /* 0x0B */  rsp_null,
    /* 0x0C */  rsp_null,
    /* 0x0D */  rsp_null,
    /* 0x0E */  rsp_null,
    /* 0x0F */  rsp_null,
    /* 0x10 */  rsp_cop2_func,
    /* 0x11 */  rsp_cop2_func,
    /* 0x12 */  rsp_cop2_func,
    /* 0x13 */  rsp_cop2_func,
    /* 0x14 */  rsp_cop2_func,
    /* 0x15 */  rsp_cop2_func,
    /* 0x16 */  rsp_cop2_func,
    /* 0x17 */  rsp_cop2_func,
    /* 0x18 */  rsp_cop2_func,
    /* 0x19 */  rsp_cop2_func,
    /* 0x1A */  rsp_cop2_func,
    /* 0x1B */  rsp_cop2_func,
    /* 0x1C */  rsp_cop2_func,
    /* 0x1D */  rsp_cop2_func,
    /* 0x1E */  rsp_cop2_func,
    /* 0x1F */  rsp_cop2_func,
};

static void rsp_cop2(u32 inst)
{
    rsp_cop2_table[INST_RS](inst);
}

static void rsp_lb(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_IMMS) & 0xFFF];
    rt = (s8)(x[0]);
    debugi("lb $%d, 0x%04X($%d)\n", INST_RT, INST_IMMU, INST_RS);
}

static void rsp_lh(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_IMMS) & 0xFFF];
    rt = (s16)(x[0] << 8 | x[1]);
    debugi("lh $%d, 0x%04X($%d)\n", INST_RT, INST_IMMU, INST_RS);
}

static void rsp_lw(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_IMMS) & 0xFFF];
    rt = (s32)(x[0] << 24 | x[1] << 16 | x[2] << 8 | x[3]);
    debugi("lw $%d, 0x%04X($%d)\n", INST_RT, INST_IMMU, INST_RS);
}

static void rsp_lbu(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_IMMS) & 0xFFF];
    rt = x[0];
    debugi("lbu $%d, 0x%04X($%d)\n", INST_RT, INST_IMMU, INST_RS);
}

static void rsp_lhu(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_IMMS) & 0xFFF];
    rt = x[0] << 8 | x[1];
    debugi("lhu $%d, 0x%04X($%d)\n", INST_RT, INST_IMMU, INST_RS);
}

static void rsp_sb(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_IMMS) & 0xFFF];
    x[0] = rt >> 0;
    debugi("sb $%d, 0x%04X($%d)\n", INST_RT, INST_IMMU, INST_RS);
}

static void rsp_sh(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_IMMS) & 0xFFF];
    x[0] = rt >> 8;
    x[1] = rt >> 0;
    debugi("sh $%d, 0x%04X($%d)\n", INST_RT, INST_IMMU, INST_RS);
}

static void rsp_sw(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_IMMS) & 0xFFF];
    x[0] = rt >> 24;
    x[1] = rt >> 16;
    x[2] = rt >>  8;
    x[3] = rt >>  0;
    debugi("sw $%d, 0x%04X($%d)\n", INST_RT, INST_IMMU, INST_RS);
}

static void rsp_lbv(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_OFFB) & 0xFFF];
    vt.b[(INST_E+0)^VX] = x[0];
    debugi("lbv $v%d[%d], 0x%04X($%d)\n", INST_VT, INST_E, INST_OFFB, INST_RS);
}

static void rsp_lsv(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_OFFS) & 0xFFF];
    vt.b[(INST_E+0)^VX] = x[0];
    vt.b[(INST_E+1)^VX] = x[1];
    debugi("lsv $v%d[%d], 0x%04X($%d)\n", INST_VT, INST_E, INST_OFFS, INST_RS);
}

static void rsp_llv(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_OFFL) & 0xFFF];
    vt.b[(INST_E+0)^VX] = x[0];
    vt.b[(INST_E+1)^VX] = x[1];
    vt.b[(INST_E+2)^VX] = x[2];
    vt.b[(INST_E+3)^VX] = x[3];
    debugi("llv $v%d[%d], 0x%04X($%d)\n", INST_VT, INST_E, INST_OFFL, INST_RS);
}

static void rsp_ldv(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_OFFD) & 0xFFF];
    vt.b[(INST_E+0)^VX] = x[0];
    vt.b[(INST_E+1)^VX] = x[1];
    vt.b[(INST_E+2)^VX] = x[2];
    vt.b[(INST_E+3)^VX] = x[3];
    vt.b[(INST_E+4)^VX] = x[4];
    vt.b[(INST_E+5)^VX] = x[5];
    vt.b[(INST_E+6)^VX] = x[6];
    vt.b[(INST_E+7)^VX] = x[7];
    debugi("ldv $v%d[%d], 0x%04X($%d)\n", INST_VT, INST_E, INST_OFFD, INST_RS);
}

static void rsp_lqv(u32 inst)
{
    uint x = (rs+INST_OFFQ) & 0xFFF;
    uint e = 0;
    do {vt.b[e++^VX] = rsp_mem[x++];} while (x & 15);
    debugi("lqv $v%d[%d], 0x%04X($%d)\n", INST_VT, INST_E, INST_OFFQ, INST_RS);
}

static void rsp_lrv(u32 inst)
{
    uint x = (rs+INST_OFFQ) & 0xFFF;
    uint e = 16 - (x & 15);
    x &= ~15;
    while (e < 16) vt.b[e++^VX] = rsp_mem[x++];
    debugi("lrv $v%d[%d], 0x%04X($%d)\n", INST_VT, INST_E, INST_OFFQ, INST_RS);
}

static void rsp_lpv(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_OFFB) & 0xFFF];
    vt.s[0] = x[0] << 8;
    vt.s[1] = x[1] << 8;
    vt.s[2] = x[2] << 8;
    vt.s[3] = x[3] << 8;
    vt.s[4] = x[4] << 8;
    vt.s[5] = x[5] << 8;
    vt.s[6] = x[6] << 8;
    vt.s[7] = x[7] << 8;
    debugi("lpv $v%d[%d], 0x%04X($%d)\n", INST_VT, INST_E, INST_OFFB, INST_RS);
}

static void rsp_luv(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_OFFB) & 0xFFF];
    vt.s[0] = x[0] << 7;
    vt.s[1] = x[1] << 7;
    vt.s[2] = x[2] << 7;
    vt.s[3] = x[3] << 7;
    vt.s[4] = x[4] << 7;
    vt.s[5] = x[5] << 7;
    vt.s[6] = x[6] << 7;
    vt.s[7] = x[7] << 7;
    debugi("luv $v%d[%d], 0x%04X($%d)\n", INST_VT, INST_E, INST_OFFB, INST_RS);
}

static void rsp_lhv(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_OFFB) & 0xFFF];
    vt.s[0] = x[2*0] << 7;
    vt.s[1] = x[2*1] << 7;
    vt.s[2] = x[2*2] << 7;
    vt.s[3] = x[2*3] << 7;
    vt.s[4] = x[2*4] << 7;
    vt.s[5] = x[2*5] << 7;
    vt.s[6] = x[2*6] << 7;
    vt.s[7] = x[2*7] << 7;
    debugi("lhv $v%d[%d], 0x%04X($%d)\n", INST_VT, INST_E, INST_OFFB, INST_RS);
}

static void rsp_lfv(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_OFFB) & 0xFFF];
    vt.s[INST_E+0] = x[4*0] << 7;
    vt.s[INST_E+1] = x[4*1] << 7;
    vt.s[INST_E+2] = x[4*2] << 7;
    vt.s[INST_E+3] = x[4*3] << 7;
    debugi("lfv $v%d[%d], 0x%04X($%d)\n", INST_VT, INST_E, INST_OFFB, INST_RS);
}

static void rsp_ltv(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_OFFB) & 0xFFF];
    uint e = INST_E >> 1;
    int i;
    for (i = 0; i < 8; i++)
    {
        rsp.vreg[INST_VT+e].b[2*i+(0^VX)] = *x++;
        rsp.vreg[INST_VT+e].b[2*i+(1^VX)] = *x++;
        e = (e+1) & 7;
    }
    debugi("ltv $v%d[%d], 0x%02X($%d)\n",INST_VT, INST_E, INST_OFFB, INST_RS);
}

static RSPCALL *const rsp_lwc2_table[] =
{
    /* 0x00 */  rsp_lbv,
    /* 0x01 */  rsp_lsv,
    /* 0x02 */  rsp_llv,
    /* 0x03 */  rsp_ldv,
    /* 0x04 */  rsp_lqv,
    /* 0x05 */  rsp_lrv,
    /* 0x06 */  rsp_lpv,
    /* 0x07 */  rsp_luv,
    /* 0x08 */  rsp_lhv,
    /* 0x09 */  rsp_lfv,
    /* 0x0A */  rsp_null, /* lwv */
    /* 0x0B */  rsp_ltv,
    /* 0x0C */  rsp_null,
    /* 0x0D */  rsp_null,
    /* 0x0E */  rsp_null,
    /* 0x0F */  rsp_null,
    /* 0x10 */  rsp_null,
    /* 0x11 */  rsp_null,
    /* 0x12 */  rsp_null,
    /* 0x13 */  rsp_null,
    /* 0x14 */  rsp_null,
    /* 0x15 */  rsp_null,
    /* 0x16 */  rsp_null,
    /* 0x17 */  rsp_null,
    /* 0x18 */  rsp_null,
    /* 0x19 */  rsp_null,
    /* 0x1A */  rsp_null,
    /* 0x1B */  rsp_null,
    /* 0x1C */  rsp_null,
    /* 0x1D */  rsp_null,
    /* 0x1E */  rsp_null,
    /* 0x1F */  rsp_null,
};

static void rsp_lwc2(u32 inst)
{
    rsp_lwc2_table[INST_RD](inst);
}

static void rsp_sbv(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_OFFB) & 0xFFF];
    x[0] = vt.b[(INST_E+0)^VX];
    debugi("sbv $v%d[%d], 0x%04X($%d)\n", INST_VT, INST_E, INST_OFFB, INST_RS);
}

static void rsp_ssv(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_OFFS) & 0xFFF];
    x[0] = vt.b[(INST_E+0)^VX];
    x[1] = vt.b[(INST_E+1)^VX];
    debugi("ssv $v%d[%d], 0x%04X($%d)\n", INST_VT, INST_E, INST_OFFS, INST_RS);
}

static void rsp_slv(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_OFFL) & 0xFFF];
    x[0] = vt.b[(INST_E+0)^VX];
    x[1] = vt.b[(INST_E+1)^VX];
    x[2] = vt.b[(INST_E+2)^VX];
    x[3] = vt.b[(INST_E+3)^VX];
    debugi("slv $v%d[%d], 0x%04X($%d)\n", INST_VT, INST_E, INST_OFFL, INST_RS);
}

static void rsp_sdv(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_OFFD) & 0xFFF];
    x[0] = vt.b[(INST_E+0)^VX];
    x[1] = vt.b[(INST_E+1)^VX];
    x[2] = vt.b[(INST_E+2)^VX];
    x[3] = vt.b[(INST_E+3)^VX];
    x[4] = vt.b[(INST_E+4)^VX];
    x[5] = vt.b[(INST_E+5)^VX];
    x[6] = vt.b[(INST_E+6)^VX];
    x[7] = vt.b[(INST_E+7)^VX];
    debugi("sdv $v%d[%d], 0x%04X($%d)\n", INST_VT, INST_E, INST_OFFD, INST_RS);
}

static void rsp_sqv(u32 inst)
{
    uint x = (rs+INST_OFFQ) & 0xFFF;
    uint e = 0;
    do {rsp_mem[x++] = vt.b[e++^VX];} while (x & 15);
    debugi("sqv $v%d[%d], 0x%04X($%d)\n", INST_VT, INST_E, INST_OFFQ, INST_RS);
}

static void rsp_srv(u32 inst)
{
    uint x = (rs+INST_OFFQ) & 0xFFF;
    uint e = 16 - (x & 15);
    x &= ~15;
    while (e < 16) rsp_mem[x++] = vt.b[e++^VX];
    debugi("srv $v%d[%d], 0x%04X($%d)\n", INST_VT, INST_E, INST_OFFQ, INST_RS);
}

static void rsp_spv(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_OFFB) & 0xFFF];
    x[0] = vt.s[0] >> 8;
    x[1] = vt.s[1] >> 8;
    x[2] = vt.s[2] >> 8;
    x[3] = vt.s[3] >> 8;
    x[4] = vt.s[4] >> 8;
    x[5] = vt.s[5] >> 8;
    x[6] = vt.s[6] >> 8;
    x[7] = vt.s[7] >> 8;
    debugi("spv $v%d[%d], 0x%04X($%d)\n", INST_VT, INST_E, INST_OFFB, INST_RS);
}

static void rsp_suv(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_OFFB) & 0xFFF];
    x[0] = vt.s[0] >> 7;
    x[1] = vt.s[1] >> 7;
    x[2] = vt.s[2] >> 7;
    x[3] = vt.s[3] >> 7;
    x[4] = vt.s[4] >> 7;
    x[5] = vt.s[5] >> 7;
    x[6] = vt.s[6] >> 7;
    x[7] = vt.s[7] >> 7;
    debugi("suv $v%d[%d], 0x%04X($%d)\n", INST_VT, INST_E, INST_OFFB, INST_RS);
}

static void rsp_shv(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_OFFB) & 0xFFF];
    x[2*0] = vt.s[0] >> 7;
    x[2*1] = vt.s[1] >> 7;
    x[2*2] = vt.s[2] >> 7;
    x[2*3] = vt.s[3] >> 7;
    x[2*4] = vt.s[4] >> 7;
    x[2*5] = vt.s[5] >> 7;
    x[2*6] = vt.s[6] >> 7;
    x[2*7] = vt.s[7] >> 7;
    debugi("shv $v%d[%d], 0x%04X($%d)\n", INST_VT, INST_E, INST_OFFB, INST_RS);
}

static void rsp_sfv(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_OFFB) & 0xFFF];
    x[4*0] = vt.s[INST_E+0] >> 7;
    x[4*1] = vt.s[INST_E+1] >> 7;
    x[4*2] = vt.s[INST_E+2] >> 7;
    x[4*3] = vt.s[INST_E+3] >> 7;
    debugi("sfv $v%d[%d], 0x%04X($%d)\n", INST_VT, INST_E, INST_OFFB, INST_RS);
}

static void rsp_swv(UNUSED u32 inst)
{
    edebug(
        "rsp: swv $v%d[%d], 0x%02X($%d)\n", INST_VT, INST_E, INST_OFFB, INST_RS
    );
}

static void rsp_stv(u32 inst)
{
    u8 *x = &rsp_mem[(rs+INST_OFFB) & 0xFFF];
    uint e = INST_E >> 1;
    int i;
    for (i = 0; i < 8; i++)
    {
        *x++ = rsp.vreg[INST_VT+e].b[2*i+(0^VX)];
        *x++ = rsp.vreg[INST_VT+e].b[2*i+(1^VX)];
        e = (e+1) & 7;
    }
    debugi("stv $v%d[%d], 0x%02X($%d)\n",INST_VT, INST_E, INST_OFFB, INST_RS);
}

static RSPCALL *const rsp_swc2_table[] =
{
    /* 0x00 */  rsp_sbv,
    /* 0x01 */  rsp_ssv,
    /* 0x02 */  rsp_slv,
    /* 0x03 */  rsp_sdv,
    /* 0x04 */  rsp_sqv,
    /* 0x05 */  rsp_srv,
    /* 0x06 */  rsp_spv,
    /* 0x07 */  rsp_suv,
    /* 0x08 */  rsp_shv,
    /* 0x09 */  rsp_sfv,
    /* 0x0A */  rsp_swv,
    /* 0x0B */  rsp_stv,
    /* 0x0C */  rsp_null,
    /* 0x0D */  rsp_null,
    /* 0x0E */  rsp_null,
    /* 0x0F */  rsp_null,
    /* 0x10 */  rsp_null,
    /* 0x11 */  rsp_null,
    /* 0x12 */  rsp_null,
    /* 0x13 */  rsp_null,
    /* 0x14 */  rsp_null,
    /* 0x15 */  rsp_null,
    /* 0x16 */  rsp_null,
    /* 0x17 */  rsp_null,
    /* 0x18 */  rsp_null,
    /* 0x19 */  rsp_null,
    /* 0x1A */  rsp_null,
    /* 0x1B */  rsp_null,
    /* 0x1C */  rsp_null,
    /* 0x1D */  rsp_null,
    /* 0x1E */  rsp_null,
    /* 0x1F */  rsp_null,
};

static void rsp_swc2(u32 inst)
{
    rsp_swc2_table[INST_RD](inst);
}

static RSPCALL *const rsp_op_table[] =
{
    /* 0x00 */  rsp_special,
    /* 0x01 */  rsp_regimm,
    /* 0x02 */  rsp_j,
    /* 0x03 */  rsp_jal,
    /* 0x04 */  rsp_beq,
    /* 0x05 */  rsp_bne,
    /* 0x06 */  rsp_blez,
    /* 0x07 */  rsp_bgtz,
    /* 0x08 */  rsp_addi,
    /* 0x09 */  rsp_addi,
    /* 0x0A */  rsp_slti,
    /* 0x0B */  rsp_sltiu,
    /* 0x0C */  rsp_andi,
    /* 0x0D */  rsp_ori,
    /* 0x0E */  rsp_xori,
    /* 0x0F */  rsp_lui,
    /* 0x10 */  rsp_cop0,
    /* 0x11 */  rsp_null,
    /* 0x12 */  rsp_cop2,
    /* 0x13 */  rsp_null,
    /* 0x14 */  rsp_null,
    /* 0x15 */  rsp_null,
    /* 0x16 */  rsp_null,
    /* 0x17 */  rsp_null,
    /* 0x18 */  rsp_null,
    /* 0x19 */  rsp_null,
    /* 0x1A */  rsp_null,
    /* 0x1B */  rsp_null,
    /* 0x1C */  rsp_null,
    /* 0x1D */  rsp_null,
    /* 0x1E */  rsp_null,
    /* 0x1F */  rsp_null,
    /* 0x20 */  rsp_lb,
    /* 0x21 */  rsp_lh,
    /* 0x22 */  rsp_null,
    /* 0x23 */  rsp_lw,
    /* 0x24 */  rsp_lbu,
    /* 0x25 */  rsp_lhu,
    /* 0x26 */  rsp_null,
    /* 0x27 */  rsp_null,
    /* 0x28 */  rsp_sb,
    /* 0x29 */  rsp_sh,
    /* 0x2A */  rsp_null,
    /* 0x2B */  rsp_sw,
    /* 0x2C */  rsp_null,
    /* 0x2D */  rsp_null,
    /* 0x2E */  rsp_null,
    /* 0x2F */  rsp_null,
    /* 0x30 */  rsp_null,
    /* 0x31 */  rsp_null,
    /* 0x32 */  rsp_lwc2,
    /* 0x33 */  rsp_null,
    /* 0x34 */  rsp_null,
    /* 0x35 */  rsp_null,
    /* 0x36 */  rsp_null,
    /* 0x37 */  rsp_null,
    /* 0x38 */  rsp_null,
    /* 0x39 */  rsp_null,
    /* 0x3A */  rsp_swc2,
    /* 0x3B */  rsp_null,
    /* 0x3C */  rsp_null,
    /* 0x3D */  rsp_null,
    /* 0x3E */  rsp_null,
    /* 0x3F */  rsp_null,
};

static void rsp_op(u32 inst)
{
    rsp_op_table[INST_OP](inst);
}

void rsp_main(OSTask *task)
{
#ifdef RSP_DEBUG
    int step = FALSE;
#endif
    byteswap(&rsp_mem[0x0FC0], task, sizeof(OSTask));
    byteswap(
        &rsp_mem[0x1000],
        &cpu_dram[task->ucode_boot],
        task->ucode_boot_size
    );
    rsp.bcode = 0;
    rsp.pc = 0;
    for (;;)
    {
        u8 *x = &rsp_mem[0x1000 | (rsp.pc & 0xFFF)];
        u32 inst = x[0] << 24 | x[1] << 16 | x[2] << 8 | x[3];
        rsp.pc += 4;
#if 0
        if ((rsp.pc & 0xFFF) == 0x000) edebug("rsp: pc over 0x%08X\n", rsp.pc);
#else
        if ((rsp.pc & 0xFFF) == 0x000) return;
#endif
        rsp.reg[0] = 0;
        rsp_op(inst);
#ifdef RSP_DEBUG
#if 0
        if ((rsp.pc & 0xFFF) == 0x5C8 /*0x644*/) step = TRUE;
#endif
        if (step)
        {
            int i;
            for (i = 0; i < 32; i += 4)
            {
                printf(
                    "R%02d:%08X  R%02d:%08X  R%02d:%08X  R%02d:%08X\n",
                    i+0, rsp.reg[i+0],
                    i+1, rsp.reg[i+1],
                    i+2, rsp.reg[i+2],
                    i+3, rsp.reg[i+3]
                );
            }
            for (i = 0; i < 32; i++)
            {
                printf(
                    "V%02d: %04X %04X %04X %04X %04X %04X %04X %04X\n",
                    i,
                    rsp.vreg[i].u[0],
                    rsp.vreg[i].u[1],
                    rsp.vreg[i].u[2],
                    rsp.vreg[i].u[3],
                    rsp.vreg[i].u[4],
                    rsp.vreg[i].u[5],
                    rsp.vreg[i].u[6],
                    rsp.vreg[i].u[7]
                );
            }
            for (i = 0; i < 8; i++)
            {
                printf(
                    "ACC%d: %04X %04X %04X %04X\n",
                    i,
                    (u16)(rsp.acc[i] >> 48),
                    (u16)(rsp.acc[i] >> 32),
                    (u16)(rsp.acc[i] >> 16),
                    (u16)(rsp.acc[i] >>  0)
                );
            }
            printf(
                "VCC:%04X  VCO:%04X  VCE:%02X  MEM_ADDR:%04X  DRAM_ADDR:%08X\n",
                rsp.vcc, rsp.vco, rsp.vce, rsp.mem_addr, rsp.dram_addr
            );
            pause();
        }
#endif
        if (rsp.bcode < 0) break;
        if (rsp.bcode > 0 && --rsp.bcode == 0) rsp.pc = rsp.baddr;
    }
}

#endif
