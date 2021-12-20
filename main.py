import sys
import os
import struct
import importlib

gpr = [
    "0",    "at",   "v0",   "v1",
    "a0",   "a1",   "a2",   "a3",
    "t0",   "t1",   "t2",   "t3",
    "t4",   "t5",   "t6",   "t7",
    "s0",   "s1",   "s2",   "s3",
    "s4",   "s5",   "s6",   "s7",
    "t8",   "t9",   "k0",   "k1",
    "gp",   "sp",   "r30",  "ra",
]

cop1_fmt = {
    0x10: "f[IX]",
    0x11: "d",
    0x14: "i[IX]",
}

reg_cpu = [
    # (1 <<  0, "r0"),
    (1 <<  1, "at"),
    (1 <<  2, "v0"),
    (1 <<  3, "v1"),
    (1 <<  4, "a0"),
    (1 <<  5, "a1"),
    (1 <<  6, "a2"),
    (1 <<  7, "a3"),
    (1 <<  8, "t0"),
    (1 <<  9, "t1"),
    (1 << 10, "t2"),
    (1 << 11, "t3"),
    (1 << 12, "t4"),
    (1 << 13, "t5"),
    (1 << 14, "t6"),
    (1 << 15, "t7"),
    (1 << 16, "s0"),
    (1 << 17, "s1"),
    (1 << 18, "s2"),
    (1 << 19, "s3"),
    (1 << 20, "s4"),
    (1 << 21, "s5"),
    (1 << 22, "s6"),
    (1 << 23, "s7"),
    (1 << 24, "t8"),
    (1 << 25, "t9"),
    # (1 << 26, "k0"),
    # (1 << 27, "k1"),
    (1 << 28, "gp"), # [C]
    (1 << 29, "sp"),
    (1 << 30, "r30"),
    (1 << 31, "ra"),
]

reg_fpu = [
    (1 << 32, "f0"),
    (1 << 33, "f2"),
    (1 << 34, "f4"),
    (1 << 35, "f6"),
    (1 << 36, "f8"),
    (1 << 37, "f10"),
    (1 << 38, "f12"),
    (1 << 39, "f14"),
    (1 << 40, "f16"),
    (1 << 41, "f18"),
    (1 << 42, "f20"),
    (1 << 43, "f22"),
    (1 << 44, "f24"),
    (1 << 45, "f26"),
    (1 << 46, "f28"),
    (1 << 47, "f30"),
]

reg_lohi = [
    (1 << 48, "lo"),
    (1 << 48, "hi"),
]

stack_reg = [
    ["s32", reg_cpu],
    ["REG", reg_fpu],
    ["s32", reg_lohi],
]

def op_unpack():
    global inst
    global inst_op
    global inst_rs
    global inst_rt
    global inst_rd
    global inst_sa
    global inst_func
    global inst_immu
    global inst_imms
    global inst_fmt
    global inst_ft
    global inst_fs
    global inst_fd
    global inst_bdst
    global inst_jdst
    global stack_min
    global wret
    inst, = struct.unpack(">I", data[addr-offs : addr-offs+4])
    inst_op   = inst >> 26 & 0x3F
    inst_rs   = inst >> 21 & 0x1F
    inst_rt   = inst >> 16 & 0x1F
    inst_rd   = inst >> 11 & 0x1F
    inst_sa   = inst >>  6 & 0x1F
    inst_func = inst >>  0 & 0x3F
    inst_immu = inst >>  0 & 0xFFFF
    inst_fmt  = inst_rs
    inst_ft   = inst_rt
    inst_fs   = inst_rd
    inst_fd   = inst_sa
    inst_imms = inst_immu - (inst_immu << 1 & 0x10000)
    inst_bdst = addr+4 + (inst_imms << 2)
    inst_jdst = (addr & 0xF0000000) | (inst & 0x03FFFFFF) << 2
    # lw
    if inst_op == 0x23:
        # ra
        if inst_rt == 0x1F:
            if inst_immu == stack_min:
                wret = True
    # sw
    if inst_op == 0x2B:
        # ra
        if inst_rt == 0x1F:
            stack_min = inst_immu
            wret = False

def op_process():
    op_unpack()
    if inst in {0x1000FFFF, 0x10000001}: return [(addr, "")], False
    return op_table[inst_op]()

def op_maxb(jt):
    global inst_maxb
    if jt & 2:
        if inst_maxb < inst_bdst:   inst_maxb = inst_bdst
    else:
        if inst_maxb < inst_jdst:   inst_maxb = inst_jdst
    return jt

def op_jt():
    if inst == 0x10000001: return 0
    # if inst_op == 0x00:
    #     if inst_func == 0x08:               return op_maxb(1)
    if inst_op == 0x01:
        if inst_rt in {0x00, 0x01}:         return op_maxb(2)
        if inst_rt in {0x02, 0x03}:         return op_maxb(3)
    if inst_op == 0x02:                     return op_maxb(1)
    if inst_op == 0x11:
        if inst_rs == 0x08:
            if inst_rt in {0x00, 0x01}:     return op_maxb(2)
            if inst_rt in {0x02, 0x03}:     return op_maxb(3)
    if inst_op in {0x04, 0x05, 0x06, 0x07}: return op_maxb(2)
    if inst_op in {0x14, 0x15, 0x16, 0x17}: return op_maxb(3)
    return 0

def op_end():
    global hret
    if inst_op == 0x00 and inst_func == 0x08:
        if inst_rs == 0x1F:
            if hret == False: hret = True
            return True
        else:
            hret = None
            return False
    if hret and inst >> 16 == 0x1000 and inst_imms < 0: return True
    # if wret and inst_op == 0x02: return True
    return False

def op_null():
    raise RuntimeError("null 0x%08X:0x%08X 0x%02X" % (addr, inst, inst_func))

def op_shift():
    global reg_flag
    global rll_flag
    flag = 1 << inst_rd
    reg_flag |= flag
    if inst == 0x00000000: return [(addr, "")], False
    if inst_func in {0x3C, 0x3E, 0x3F}:
        rll_flag |= flag
    if inst_func in {0x04, 0x06, 0x07}:
        mask = {
            0x04: 0x1F,
            0x06: 0x1F,
            0x07: 0x1F,
        }[inst_func]
        rs = "(%s & 0x%02X)" % (gpr[inst_rs], mask)
    else:
        sa = inst_sa
        if inst_func in {0x3C, 0x3E, 0x3F}:
            sa += 32
        rs = "%d" % sa
    op, td, t = {
        0x00: ("<<", "(s32)", "(u32)"),
        0x02: (">>", "(s32)", "(u32)"),
        0x03: (">>", "(s32)", "(s32)"),
        0x04: ("<<", "(s32)", "(u32)"),
        0x06: (">>", "(s32)", "(u32)"),
        0x07: (">>", "(s32)", "(s32)"),
        0x3C: ("<<", "(s64)", "(u64)"),
        0x3E: (">>", "(s64)", "(u64)"),
        0x3F: (">>", "(s64)", "(s64)"),
    }[inst_func]
    rt = gpr[inst_rt]
    rd = gpr[inst_rd]
    return [(addr, "    %s = %s(%s%s %s %s);\n" % (
        rd, td, t, rt, op, rs
    ))], False

def op_jr():
    global addr
    global wret
    end = op_end()
    rs = inst_rs
    addr += 4
    ln, e = op_process()
    l = ln[0][1] if len(ln) > 0 else ""
    if end:
        line = "    return;\n"
        end = addr-4 >= inst_maxb
    else:
        line = (
            "    switch ((PTR)%s)\n"
            "    {\n"
        ) % gpr[rs]
        a = addr
        addr = addr_s
        wret = False
        while True:
            line += "        case 0x%08XU: goto _%08X; break;\n" % (addr, addr)
            btbl.add(addr)
            op_unpack()
            addr += 4
            op_jt()
            if op_end() and addr-4 >= inst_maxb: break
        addr = a
        line += "    }\n"
    return [(addr-4, l+line)] + ln, end

def op_jalr():
    global addr
    rs = gpr[inst_rs]
    addr += 4
    ln, e = op_process()
    l = ln[0][1] if len(ln) > 0 else ""
    line = "    __call(%s);\n" % rs
    return [(addr-4, l), (addr, line)], False

def op_break():
    line = "    __break(%d);\n" % inst_rt
    return [(addr, line)], False

def op_mfhilo():
    global reg_flag
    reg_flag |= 1 << inst_rd
    rd = gpr[inst_rd]
    src = {
        0x10: "hi",
        0x12: "lo",
    }[inst_func]
    return [(addr, "    %s = %s;\n" % (rd, src))], False

def op_mult():
    global reg_flag
    reg_flag |= 1 << 48
    x, t = {
        0x18: ("s64", "(s32)"),
        0x19: ("u64", "(u32)"),
    }[inst_func]
    rs = gpr[inst_rs]
    rt = gpr[inst_rt]
    line = (
        "    {\n"
        "        %s x = %s%s * %s%s;\n"
        "        lo = x >>  0;\n"
        "        hi = x >> 32;\n"
        "    }\n"
    ) % (x, t, rs, t, rt)
    return [(addr, line)], False

def op_div():
    global reg_flag
    reg_flag |= 1 << 48
    t = {
        0x1A: "(s32)",
        0x1B: "(u32)",
        0x1E: "(s64)",
        0x1F: "(u64)",
    }[inst_func]
    rs = gpr[inst_rs]
    rt = gpr[inst_rt]
    line = (
        "    if (%s%s != 0)\n"
        "    {\n"
        "        lo = %s%s / %s%s;\n"
        "        hi = %s%s %% %s%s;\n"
        "    }\n"
    ) % (t, rt, t, rs, t, rt, t, rs, t, rt)
    return [(addr, line)], False

def op_arith():
    global reg_flag
    global rll_flag
    flag = 1 << inst_rd
    reg_flag |= flag
    if inst_func in {0x24, 0x25, 0x26, 0x27}:
        if rll_flag & (1 << inst_rs | 1 << inst_rt):
            rll_flag |= flag
    if inst_func in {0x2D}:
        rll_flag |= flag
    op, s, tt, e = {
        0x20: ("+", "(s32)((s32)", "(s32)", ")"),
        0x21: ("+", "(s32)((s32)", "(s32)", ")"),
        0x22: ("-", "(s32)((s32)", "(s32)", ")"),
        0x23: ("-", "(s32)((s32)", "(s32)", ")"),
        0x24: ("&", "",   "", ""),
        0x25: ("|", "",   "", ""),
        0x26: ("^", "",   "", ""),
        0x27: ("|", "~(", "", ")"),
        0x2A: ("<", "(s32)((s32)", "(s32)", ")"),
        0x2B: ("<", "(s32)((u32)", "(u32)", ")"),
        0x2D: ("+", "(s64)((s64)", "(s64)", ")"),
    }[inst_func]
    rs = gpr[inst_rs]
    rt = gpr[inst_rt]
    rd = gpr[inst_rd]
    if addr in xpr:
        line = "    %s = %s;\n" % (rd, xpr[addr])
    else:
        line = "    %s = %s%s %s %s%s%s;\n" % (rd, s, rs, op, tt, rt, e)
    return [(addr, line)], False

def op_special():
    return op_special_table[inst_func]()

def op_regimm():
    return op_regimm_table[inst_rt]()

def op_j():
    global wret
    # if wret == None: print("    0x%08X, # %06X" % (addr, addr-offs))
    x = op_jal() if wret and inst_jdst not in btbl else op_b()
    wret = False
    return x

def op_jal():
    global addr
    a = addr
    end = {
        0x02: a >= inst_maxb,
        0x03: False,
    }[inst_op]
    jdst = inst_jdst
    jtbl.add(jdst)
    addr += 4
    ln, e = op_process()
    l = ln[0][1] if len(ln) > 0 else ""
    if jdst in app.lib:
        line = "    lib_%s();\n" % app.lib[jdst]
    else:
        line = "    app_%08X();\n" % jdst
    return [(a, l), (addr, line)], end

def op_b():
    global addr
    end = False
    jt = op_jt()
    if jt == 1:
        bc = "1"
        bdst = inst_jdst
    else:
        rs = "(s32)" + gpr[inst_rs]
        rt = "(s32)" + gpr[inst_rt]
        if inst_op in {0x01, 0x06, 0x07, 0x16, 0x17}:
            rt = "0"
        if inst_op == 0x11:
            bc = {
                0x00: cop1_cmp_f,
                0x01: cop1_cmp_t,
                0x02: cop1_cmp_f,
                0x03: cop1_cmp_t,
            }[inst_rt]
        else:
            if inst_op == 0x01:
                op = {
                    0x00: "<",
                    0x01: ">=",
                    0x02: "<",
                    0x03: ">=",
                    0x10: "<",
                    0x11: ">=",
                    0x12: "<",
                    0x13: ">=",
                }[inst_rt]
            else:
                op = {
                    0x04: "==",
                    0x05: "!=",
                    0x06: "<=",
                    0x07: ">",
                    0x14: "==",
                    0x15: "!=",
                    0x16: "<=",
                    0x17: ">",
                }[inst_op]
                if op_end(): end = addr >= inst_maxb
            bc = "%s %s %s" % (rs, op, rt)
        bdst = inst_bdst
    btbl.add(bdst)
    addr += 4
    ln, e = op_process()
    l = ln[0][1] if len(ln) > 0 else ""
    line = []
    ln = (
        "    if (%s)\n"
        "    {\n"
    ) % bc
    if l != "":
        ln += "    " + l
    ln += (
        "        goto _%08X;\n"
        "    }\n"
    ) % bdst
    if l != "" and (jt & 2):
        ln += (
            "    else\n"
            "    {\n"
        )
        # b
        if jt == 2:
            line.append((addr-4, ln))
            ln = (
                "    " + l +
                "    }\n"
            )
            line.append((addr, ln))
        # bl
        if jt == 3:
            ln += (
                "        goto b_%08X;\n"
                "    }\n"
            ) % (addr+4)
            line.append((addr-4, ln))
            ln = l + ("b_%08X:;\n" % (addr+4))
            line.append((addr, ln))
    else:
        line.append((addr-4, ln))
        line.append((addr, ""))
    return line, end

def op_bal():
    global addr
    a = addr
    if inst_rs != 0x00:
        raise RuntimeError("bgezal $%s" % gpr[inst_rs])
    bdst = inst_bdst
    jtbl.add(bdst)
    addr += 4
    ln, end = op_process()
    l = ln[0][1] if len(ln) > 0 else ""
    if bdst in app.lib:
        line = "    lib_%s();\n" % app.lib[bdst]
    else:
        line = "    app_%08X();\n" % bdst
    return [(a, l), (addr, line)], False

def op_arithi():
    global reg_flag
    global rll_flag
    flag = 1 << inst_rt
    reg_flag |= flag
    if inst_op in {0x0C, 0x0D, 0x0E}:
        if rll_flag & (1 << inst_rs):
            rll_flag |= flag
    if inst_op in {0x19}:
        rll_flag |= flag
    # sltiu
    if inst_op == 0x0B:
        imm = "0x%04XU" % (inst_imms & 0xFFFFFFFF)
    # andi, ori, xori
    elif inst_op in {0x0C, 0x0D, 0x0E}:
        imm = "0x%04XU" % inst_immu
    else:
        imm = "(s16)0x%04X" % inst_immu
    op, s, e = {
        0x08: ("+", "(s32)((s32)", ")"),
        0x09: ("+", "(s32)((s32)", ")"),
        0x0A: ("<", "(s32)((s32)", ")"),
        0x0B: ("<", "(s32)((u32)", ")"),
        0x0C: ("&", "", ""),
        0x0D: ("|", "", ""),
        0x0E: ("^", "", ""),
        0x19: ("+", "(s64)((s64)", ")"),
    }[inst_op]
    rs = gpr[inst_rs]
    rt = gpr[inst_rt]
    if addr in xpr:
        line = "    %s = %s;\n" % (rt, xpr[addr])
    else:
        line = "    %s = %s%s %s %s%s;\n" % (rt, s, rs, op, imm, e)
    return [(addr, line)], False

def op_lui():
    global reg_flag
    reg_flag |= 1 << inst_rt
    rt = gpr[inst_rt]
    return [(addr, "    %s = (s32)0x%04X0000;\n" % (rt, inst_immu))], False

def op_mfc1():
    global reg_flag
    reg_flag |= 1 << inst_rt
    line = "    %s = f%d.iu[%d^IX];\n" % (
        gpr[inst_rt], inst_rd & ~1, inst_rd & 1
    )
    return [(addr, line)], False

def op_mtc1():
    global reg_flag
    reg_flag |= (1 << 32) << (inst_rd >> 1)
    rd = {
        0x04: "iu[%d^IX]" % (inst_rd & 1), # mtc1
        0x05: "llu",                       # dmtc1
    }[inst_rs]
    line = "    f%d.%s = %s;\n" % (inst_rd & ~1, rd, gpr[inst_rt])
    return [(addr, line)], False

def op_cfc1():
    global reg_flag
    reg_flag |= 1 << inst_rt
    line = "    %s = 0;\n" % gpr[inst_rt]
    return [(addr, line)], False

def op_ctc1():
    return [(addr, "")], False

def op_arithf():
    global reg_flag
    reg_flag |= (1 << 32) << (inst_fd >> 1)
    fmt = cop1_fmt[inst_fmt]
    ft = "f%d.%s" % (inst_ft & ~1, fmt)
    fs = "f%d.%s" % (inst_fs & ~1, fmt)
    fd = "f%d.%s" % (inst_fd & ~1, fmt)
    if inst_func in {0x04, 0x05, 0x06, 0x07}:
        sqrt, fabs = {
            0x10: ("sqrtf(", "fabsf("),
            0x11: ("sqrt(",  "fabs("),
        }[inst_fmt]
        start, end = {
            0x04: (sqrt, ")"),
            0x05: (fabs, ")"),
            0x06: ("", ""),
            0x07: ("-", ""),
        }[inst_func]
        return [(addr, "    %s = %s%s%s;\n" % (fd, start, fs, end))], False
    else:
        op = {
            0x00: "+",
            0x01: "-",
            0x02: "*",
            0x03: "/",
        }[inst_func]
        return [(addr, "    %s = %s %s %s;\n" % (fd, fs, op, ft))], False
    return [(addr, "")], False

def op_round():
    global reg_flag
    reg_flag |= (1 << 32) << (inst_fd >> 1)
    src, round_, ceil, floor = {
        0x10: ("f[IX]", "roundf", "ceilf", "floorf"),
        0x11: ("d",     "round",  "ceil",  "floor"),
    }[inst_fmt]
    dst, fnc = {
        0x08: ("ll",    round_), # round.l
        0x0A: ("ll",    ceil),   # ceil.l
        0x0B: ("ll",    floor),  # floor.l
        0x0C: ("i[IX]", round_), # round.w
        0x0E: ("i[IX]", ceil),   # ceil.w
        0x0F: ("i[IX]", floor),  # floor.w
    }[inst_func]
    fs = "f%d.%s" % (inst_fs & ~1, src)
    fd = "f%d.%s" % (inst_fd & ~1, dst)
    return [(addr, "    %s = %s(%s);\n" % (fd, fnc, fs))], False

def op_cvt():
    global reg_flag
    reg_flag |= (1 << 32) << (inst_fd >> 1)
    dst = {
        0x0D: "i[IX]", # trunc.w
        0x20: "f[IX]", # cvt.s
        0x21: "d",     # cvt.d
        0x24: "i[IX]", # cvt.w
    }[inst_func]
    fmt = cop1_fmt[inst_fmt]
    fs = "f%d.%s" % (inst_fs & ~1, fmt)
    fd = "f%d.%s" % (inst_fd & ~1, dst)
    return [(addr, "    %s = %s;\n" % (fd, fs))], False

def op_cmp():
    global cop1_cmp_f
    global cop1_cmp_t
    f, t = {
        0x32: ("!=", "=="),
        0x3C: (">=", "<"),
        0x3E: (">",  "<="),
    }[inst_func]
    fmt = cop1_fmt[inst_fmt]
    ft = "f%d.%s" % (inst_ft & ~1, fmt)
    fs = "f%d.%s" % (inst_fs & ~1, fmt)
    cop1_cmp_f = "%s %s %s" % (fs, f, ft)
    cop1_cmp_t = "%s %s %s" % (fs, t, ft)
    return [(addr, "")], False

def op_cop1_f():
    return op_cop1_func_table[inst_func]()

def op_cop1():
    return op_cop1_table[inst_rs]()

def op_load():
    global reg_flag
    t = {
        0x20: "s8",
        0x21: "s16",
        0x23: "s32",
        0x24: "u8",
        0x25: "u16",
        0x27: "u32",
        0x31: "u32",
    }[inst_op]
    rs = gpr[inst_rs]
    if inst_op == 0x31:
        reg_flag |= (1 << 32) << (inst_rt >> 1)
        rt = "f%d.i[%d^IX]" % (inst_rt & ~1, inst_rt & 1)
    else:
        if inst_rt == 0x00: return [(addr, "")], False
        reg_flag |= 1 << inst_rt
        rt = gpr[inst_rt]
    line = "    %s = *__%s(%s + (s16)0x%04X);\n" % (rt, t, rs, inst_immu)
    return [(addr, line)], False

def op_store():
    global chk_flag
    t = {
        0x28: "s8",
        0x29: "s16",
        0x2B: "s32",
        0x39: "s32",
    }[inst_op]
    rs = gpr[inst_rs]
    if inst_op == 0x39:
        flag = (1 << 32) << (inst_rt >> 1)
        # f20-f30
        if (flag & 0xFC0000000000) and not ((reg_flag | chk_flag) & flag):
            chk_flag |= flag
            return [(addr, "")], False
        rt = "f%d.i[%d^IX]" % (inst_rt & ~1, inst_rt & 1)
    else:
        flag = 1 << inst_rt
        # s0-s7, gp, s8, ra
        if (flag & 0x0000D0FF0000) and not ((reg_flag | chk_flag) & flag):
            chk_flag |= flag
            return [(addr, "")], False
        rt = gpr[inst_rt]
    line = "    *__%s(%s + (s16)0x%04X) = %s;\n" % (t, rs, inst_immu, rt)
    return [(addr, line)], False

def op_ls(rt):
    t = {
        0x22: "lwl",
        0x26: "lwr",
        0x2A: "swl",
        0x2E: "swr",
        0x35: "ldc1",
        0x37: "ld",
        0x3D: "sdc1",
    }[inst_op]
    rs = gpr[inst_rs]
    line = "    __%s(%s + (s16)0x%04X, %s);\n" % (t, rs, inst_immu, rt)
    return [(addr, line)], False

def op_lw():
    global reg_flag
    if inst_rt == 0x00: return [(addr, "")], False
    reg_flag |= 1 << inst_rt
    return op_ls(gpr[inst_rt])

def op_ld():
    global reg_flag
    global rll_flag
    if inst_rt == 0x00: return [(addr, "")], False
    reg_flag |= 1 << inst_rt
    rll_flag |= 1 << inst_rt
    return op_ls(gpr[inst_rt])

def op_ldc1():
    global reg_flag
    reg_flag |= (1 << 32) << (inst_rt >> 1)
    return op_ls("f%d" % (inst_rt & ~1))

def op_sw():
    global chk_flag
    flag = 1 << inst_rt
    # s0-s7, gp, s8, ra
    if (flag & 0x0000D0FF0000) and not ((reg_flag | chk_flag) & flag):
        chk_flag |= flag
        return [(addr, "")], False
    return op_ls(gpr[inst_rt])

def op_sdc1():
    global chk_flag
    flag = (1 << 32) << (inst_rt >> 1)
    # f20-f30
    if (flag & 0xFC0000000000) and not ((reg_flag | chk_flag) & flag):
        chk_flag |= flag
        return [(addr, "")], False
    return op_ls("f%d" % (inst_rt & ~1))

op_special_table = [
    op_shift,   # 0x00 sll
    op_null,    # 0x01
    op_shift,   # 0x02 srl
    op_shift,   # 0x03 sra
    op_shift,   # 0x04 sllv
    op_null,    # 0x05
    op_shift,   # 0x06 srlv
    op_shift,   # 0x07 srav
    op_jr,      # 0x08 jr
    op_jalr,    # 0x09 jalr
    op_null,    # 0x0A
    op_null,    # 0x0B
    op_null,    # 0x0C syscall
    op_break,   # 0x0D break
    op_null,    # 0x0E
    op_null,    # 0x0F sync
    op_mfhilo,  # 0x10 mfhi
    op_null,    # 0x11 mthi
    op_mfhilo,  # 0x12 mflo
    op_null,    # 0x13 mtlo
    op_null,    # 0x14 dsllv
    op_null,    # 0x15
    op_null,    # 0x16 dsrlv
    op_null,    # 0x17 dsrav
    op_mult,    # 0x18 mult [C]
    op_mult,    # 0x19 multu
    op_div,     # 0x1A div
    op_div,     # 0x1B divu
    op_null,    # 0x1C dmult
    op_null,    # 0x1D dmultu
    op_div,     # 0x1E ddiv
    op_div,     # 0x1F ddivu
    op_arith,   # 0x20 add
    op_arith,   # 0x21 addu
    op_arith,   # 0x22 sub
    op_arith,   # 0x23 subu
    op_arith,   # 0x24 and
    op_arith,   # 0x25 or
    op_arith,   # 0x26 xor
    op_arith,   # 0x27 nor
    op_null,    # 0x28
    op_null,    # 0x29
    op_arith,   # 0x2A slt
    op_arith,   # 0x2B sltu
    op_null,    # 0x2C dadd
    op_arith,   # 0x2D daddu [C]
    op_null,    # 0x2E dsub
    op_null,    # 0x2F dsubu
    op_null,    # 0x30 tge
    op_null,    # 0x31 tgeu
    op_null,    # 0x32 tlt
    op_null,    # 0x33 tltu
    op_null,    # 0x34 teq
    op_null,    # 0x35
    op_null,    # 0x36 tne
    op_null,    # 0x37
    op_null,    # 0x38 dsll
    op_null,    # 0x39
    op_null,    # 0x3A dsrl
    op_null,    # 0x3B dsra
    op_shift,   # 0x3C dsll32
    op_null,    # 0x3D
    op_null,    # 0x3E dsrl32
    op_shift,   # 0x3F dsra32
]

op_regimm_table = [
    op_b,       # 0x00 bltz
    op_b,       # 0x01 bgez
    op_b,       # 0x02 bltzl
    op_b,       # 0x03 bgezl
    op_null,    # 0x04
    op_null,    # 0x05
    op_null,    # 0x06
    op_null,    # 0x07
    op_null,    # 0x08 tgei
    op_null,    # 0x09 tgeiu
    op_null,    # 0x0A tlti
    op_null,    # 0x0B tltiu
    op_null,    # 0x0C teqi
    op_null,    # 0x0D
    op_null,    # 0x0E tnei
    op_null,    # 0x0F
    op_null,    # 0x10 bltzal
    op_bal,     # 0x11 bgezal
    op_null,    # 0x12 bltzall
    op_null,    # 0x13 bgezall
    op_null,    # 0x14
    op_null,    # 0x15
    op_null,    # 0x16
    op_null,    # 0x17
    op_null,    # 0x18
    op_null,    # 0x19
    op_null,    # 0x1A
    op_null,    # 0x1B
    op_null,    # 0x1C
    op_null,    # 0x1D
    op_null,    # 0x1E
    op_null,    # 0x1F
]

op_cop1_func_table = [
    op_arithf,  # 0x00 add
    op_arithf,  # 0x01 sub
    op_arithf,  # 0x02 mul
    op_arithf,  # 0x03 div
    op_arithf,  # 0x04 sqrt
    op_arithf,  # 0x05 abs
    op_arithf,  # 0x06 mov
    op_arithf,  # 0x07 neg
    op_null,    # 0x08 round.l
    op_null,    # 0x09 trunc.l
    op_null,    # 0x0A ceil.l
    op_null,    # 0x0B floor.l
    op_round,   # 0x0C round.w
    op_cvt,     # 0x0D trunc.w
    op_round,   # 0x0E ceil.w
    op_round,   # 0x0F floor.w
    op_null,    # 0x10
    op_null,    # 0x11
    op_null,    # 0x12
    op_null,    # 0x13
    op_null,    # 0x14
    op_null,    # 0x15
    op_null,    # 0x16
    op_null,    # 0x17
    op_null,    # 0x18
    op_null,    # 0x19
    op_null,    # 0x1A
    op_null,    # 0x1B
    op_null,    # 0x1C
    op_null,    # 0x1D
    op_null,    # 0x1E
    op_null,    # 0x1F
    op_cvt,     # 0x20 cvt.s
    op_cvt,     # 0x21 cvt.d
    op_null,    # 0x22
    op_null,    # 0x23
    op_cvt,     # 0x24 cvt.w
    op_null,    # 0x25 cvt.l
    op_null,    # 0x26
    op_null,    # 0x27
    op_null,    # 0x28
    op_null,    # 0x29
    op_null,    # 0x2A
    op_null,    # 0x2B
    op_null,    # 0x2C
    op_null,    # 0x2D
    op_null,    # 0x2E
    op_null,    # 0x2F
    op_null,    # 0x30 c.f
    op_null,    # 0x31 c.un
    op_cmp,     # 0x32 c.eq
    op_null,    # 0x33 c.ueq
    op_null,    # 0x34 c.olt
    op_null,    # 0x35 c.ult
    op_null,    # 0x36 c.ole
    op_null,    # 0x37 c.ule
    op_null,    # 0x38 c.sf
    op_null,    # 0x39 c.ngle
    op_null,    # 0x3A c.seq
    op_null,    # 0x3B c.ngl
    op_cmp,     # 0x3C c.lt
    op_null,    # 0x3D c.nge
    op_cmp,     # 0x3E c.le
    op_null,    # 0x3F c.ngt
]

op_cop1_table = [
    op_mfc1,    # 0x00 mfc1
    op_null,    # 0x01 dmfc1
    op_cfc1,    # 0x02 cfc1
    op_null,    # 0x03
    op_mtc1,    # 0x04 mtc1
    op_mtc1,    # 0x05 dmtc1 [C]
    op_ctc1,    # 0x06 ctc1
    op_null,    # 0x07
    op_b,       # 0x08 bc1
    op_null,    # 0x09
    op_null,    # 0x0A
    op_null,    # 0x0B
    op_null,    # 0x0C
    op_null,    # 0x0D
    op_null,    # 0x0E
    op_null,    # 0x0F
    op_cop1_f,  # 0x10
    op_cop1_f,  # 0x11
    op_null,    # 0x12
    op_null,    # 0x13
    op_cop1_f,  # 0x14
    op_null,    # 0x15
    op_null,    # 0x16
    op_null,    # 0x17
    op_null,    # 0x18
    op_null,    # 0x19
    op_null,    # 0x1A
    op_null,    # 0x1B
    op_null,    # 0x1C
    op_null,    # 0x1D
    op_null,    # 0x1E
    op_null,    # 0x1F
]

op_table = [
    op_special, # 0x00 special
    op_regimm,  # 0x01 regimm
    op_j,       # 0x02 j
    op_jal,     # 0x03 jal
    op_b,       # 0x04 beq
    op_b,       # 0x05 bne
    op_b,       # 0x06 blez
    op_b,       # 0x07 bgtz
    op_arithi,  # 0x08 addi
    op_arithi,  # 0x09 addiu
    op_arithi,  # 0x0A slti
    op_arithi,  # 0x0B sltiu
    op_arithi,  # 0x0C andi
    op_arithi,  # 0x0D ori
    op_arithi,  # 0x0E xori
    op_lui,     # 0x0F lui
    op_null,    # 0x10 cop0
    op_cop1,    # 0x11 cop1
    op_null,    # 0x12 cop2
    op_null,    # 0x13
    op_b,       # 0x14 beql
    op_b,       # 0x15 bnel
    op_b,       # 0x16 blezl
    op_b,       # 0x17 bgtzl
    op_null,    # 0x18 daddi
    op_arithi,  # 0x19 daddiu
    op_null,    # 0x1A ldl
    op_null,    # 0x1B ldr
    op_null,    # 0x1C
    op_null,    # 0x1D
    op_null,    # 0x1E
    op_null,    # 0x1F
    op_load,    # 0x20 lb
    op_load,    # 0x21 lh
    op_lw,      # 0x22 lwl
    op_load,    # 0x23 lw
    op_load,    # 0x24 lbu
    op_load,    # 0x25 lhu
    op_lw,      # 0x26 lwr
    op_null,    # 0x27 lwu
    op_store,   # 0x28 sb
    op_store,   # 0x29 sh
    op_sw,      # 0x2A swl
    op_store,   # 0x2B sw
    op_null,    # 0x2C sdl
    op_null,    # 0x2D sdr
    op_sw,      # 0x2E swr
    op_null,    # 0x2F cache
    op_null,    # 0x30 ll
    op_load,    # 0x31 lwc1
    op_null,    # 0x32 lwc2
    op_null,    # 0x33
    op_null,    # 0x34 lld
    op_ldc1,    # 0x35 ldc1
    op_null,    # 0x36 ldc2
    op_ld,      # 0x37 ld
    op_null,    # 0x38 sc
    op_store,   # 0x39 swc1
    op_null,    # 0x3A swc2
    op_null,    # 0x3B
    op_null,    # 0x3C scd
    op_sdc1,    # 0x3D sdc1
    op_null,    # 0x3E sdc2
    op_null,    # 0x3F sd
]

def main(argv):
    global app
    global xpr
    global btbl
    global jtbl
    global data
    global offs
    global addr_s
    global addr
    global inst
    global inst_maxb
    global reg_flag
    global chk_flag
    global rll_flag
    global stack_min
    global hret
    global wret
    if len(argv) < 2:
        print("usage: %s <app> [build]" % argv[0])
        return 1
    app = importlib.import_module("app." + argv[1])
    if len(argv) == 3:
        obj = ["app"] + [
            "%08X" % src
            for src, start, end, dst, pat, xpr, ins in app.segment
        ]
        print(" ".join(["%s%s.o" % (argv[2], x) for x in obj]))
        return 0
    path_app   = os.path.join("app", argv[1])
    path_build = os.path.join("build", argv[1])
    with open(os.path.join(path_app, "app.bin"), "rb") as f: data = f.read()
    if app.patch != None: data = app.patch(data)
    app_h = (
        "#ifndef __APP_H__\n"
        "#define __APP_H__\n"
        "\n"
        "#include \"types.h\"\n"
        "\n"
        "#define APP_U%c%c%c\n"
        "#define APP_%c%d\n"
    ) % struct.unpack(">59xBBBBB", data[:0x40])
    if len(app.dcall) > 0: app_h += "#define APP_DCALL\n"
    if len(app.cache) > 0: app_h += "#define APP_CACHE\n"
    s = ""
    r = 3*[0]
    for flag, reg in reg_cpu+reg_fpu+reg_lohi:
        if app.reg & flag:
            if   flag & 0x00010000200000FC: i = 0
            elif flag & 0x00020000DFFFFF03: i = 1
            elif flag & 0x0000FFFF00000000: i = 2
            s += "#define %s cpu.%s[%s]\n" % (
                reg.ljust(15), ("arg", "ext", "reg")[i], r[i]
            )
            r[i] += 1
    app_h += (
        "%s"
        "\n"
        "#define APP_PATH        \"%s\"\n"
        "#define APP_ENTRY       0x%08X\n"
        "#define APP_BSS_ADDR    0x%08X\n"
        "#define APP_BSS_SIZE    0x%08X\n"
        "#define APP_STACK       0x%08X\n"
        "#define app_main        app_%08X\n"
        "\n"
        "%s"
        "#define CPU_ARG_LEN     %d\n"
        "#define CPU_EXT_LEN     %d\n"
        "#define CPU_REG_LEN     %d\n"
        "\n"
        "struct app_call\n"
        "{\n"
        "    PTR    addr;\n"
        "    void (*call)(void);\n"
        "};\n"
        "\n"
    ) % (
        app.header,
        argv[1], app.entry, app.bss[0], app.bss[1], app.sp, app.main,
        s, r[0], r[1], r[2],
    )
    if len(app.cache) > 0:
        app_h += (
            "struct app_cache\n"
            "{\n"
            "    PTR addr;\n"
            "    u32 size;\n"
            "};\n"
            "\n"
        )
    g_addr = set()
    d_addr = set()
    for src, start, end, dst, pat, xpr, ins in app.segment:
        offs = start - src
        for addr in pat:
            patch = B"".join([struct.pack(">I", x) for x in pat[addr]])
            data = data[:addr-offs] + patch + data[addr-offs + len(patch):]
        addr = start
        while addr < end:
            # f = False
            while True:
                op_unpack()
                if inst != 0x00000000: break
            #     f = True
                addr += 4
            if addr >= end:
                break
            # if f: print("    0x%08X," % addr)
            dst.append(addr)
            inst_maxb = 0
            stack_min = 0
            hret = False
            wret = None
            while True:
                op_unpack()
                op_jt()
                addr += 4
                if op_end() and addr-4 >= inst_maxb: break
            addr += 4
        dst.sort()
        dst = set(dst)
        d_addr |= dst & g_addr
        g_addr |= dst
    g_addr = sorted(g_addr)
    d_addr = sorted(d_addr)
    app_h += "extern const struct app_call app_call_table[%d+1];\n" % \
        len(g_addr)
    if len(app.dcall) > 0:
        app_h += "extern const PTR app_dcall_table[%d+1];\n" % len(app.dcall)
    if len(app.cache) > 0:
        app_h += "extern const struct app_cache app_cache_table[%d];\n" % \
            len(app.cache)
    app_h += "\n"
    app_c = (
        "#include \"types.h\"\n"
        "#include \"app.h\"\n"
        "#include \"cpu.h\"\n"
        "\n"
        "const struct app_call app_call_table[%d+1] =\n"
        "{\n"
    ) % len(g_addr)
    for addr in g_addr:
        app_c += "    {0x%08XU, app_%08X},\n" % (addr, addr)
    app_c += (
        "    {0xFFFFFFFFU, NULL},\n"
        "};\n"
    )
    if len(app.dcall) > 0:
        app_c += (
            "\n"
            "const PTR app_dcall_table[%d+1] =\n"
            "{\n"
        ) % len(app.dcall)
        for addr in app.dcall:
            app_c += "    0x%08XU,\n" % (addr & 0x1FFFFFFF)
        app_c += (
            "    0xFFFFFFFFU,\n"
            "};\n"
        )
    app_c += "\n"
    if len(app.cache) > 0:
        app_c += (
            "const struct app_cache app_cache_table[%d] =\n"
            "{\n"
        ) % len(app.cache)
        for start, end in app.cache:
            app_c += "    {0x%08XU, 0x%08XU},\n" % (start, end-start)
        app_c += "};\n"
    for addr in d_addr:
        name = "void app_%08X(void)" % addr
        app_h += "extern %s;\n" % name
        app_c += (
            "\n"
            "%s\n"
            "{\n"
            "    switch (__dcall(0x%08XU))\n"
            "    {\n"
        ) % (name, addr)
        for src, start, end, dst, pat, xpr, ins in app.segment:
            if addr in dst:
                app_c += "        case 0x%08XU: app_%08X_%08X(); break;\n" % (
                    src, addr, src
                )
        app_c += (
            "    }\n"
            "}\n"
        )
    with open(os.path.join(path_build, "app.c"), "w") as f: f.write(app_c)
    jtbl = set()
    for src, start, end, dst, pat, xpr, ins in app.segment:
        offs = start - src
        app_c = (
            "#define __%s_%08X_C__\n"
            "#include \"types.h\"\n"
            "#include \"app.h\"\n"
            "#include \"cpu.h\"\n"
            "#include \"sys.h\"\n"
            "#include \"lib.h\"\n"
        ) % (argv[1], src)
        for addr in dst:
            addr_s = addr
            if addr in d_addr:
                name = "void app_%08X_%08X(void)" % (addr, src)
            else:
                name = "void app_%08X(void)" % addr
            app_h += "extern %s;\n" % name
            app_c += (
                "\n"
                "%s\n"
                "{\n"
            ) % name
            inst_maxb = 0
            reg_flag = app.reg
            chk_flag = app.reg
            rll_flag = 0
            stack_min = 0
            hret = False
            wret = None
            btbl = set()
            line = []
            while True:
                ln, end = op_process()
                line += ln
                addr += 4
                if end: break
            for t, reg in stack_reg:
                for flag, r in reg:
                    if app.reg & flag: continue
                    if (reg_flag | chk_flag) & flag:
                        app_c += "    unused %s %s;\n" % (
                            "s64" if rll_flag & flag else t, r
                        )
            for addr, ln in line:
                if addr in btbl:    app_c += "_%08X:;\n" % addr
                if addr in ins:     app_c += ins[addr]
                app_c += ln
            app_c += "}\n"
        with open(os.path.join(path_build, "%08X.c" % src), "w") as f:
            f.write(app_c)
    app_h += (
        "\n"
        "#endif /* __APP_H__ */\n"
    )
    with open(os.path.join(path_build, "app.h"), "w") as f: f.write(app_h)
    code = 0
    for addr in sorted(jtbl):
        for src, start, end, dst, pat, xpr, ins in app.segment:
            if addr in dst: break
        else:
            if addr not in app.lib:
                print("    0x%08X: \"%s_%08X\"," % (addr, argv[1], addr))
                code = 1
    return code

if __name__ == "__main__":
    sys.exit(main(sys.argv))
