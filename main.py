import sys
import os
import struct
import importlib

gpr = [
    "0",  "at", "v0", "v1",
    "a0", "a1", "a2", "a3",
    "t0", "t1", "t2", "t3",
    "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3",
    "s4", "s5", "s6", "s7",
    "t8", "t9", "k0", "k1",
    "gp", "sp", "fp", "ra",
]

gpr_i   = [gpr[i] + (".i[IX]"  if i != 0x00 else "") for i in range(len(gpr))]
gpr_iu  = [gpr[i] + (".iu[IX]" if i != 0x00 else "") for i in range(len(gpr))]
gpr_ll  = [gpr[i] + (".ll"     if i != 0x00 else "") for i in range(len(gpr))]
gpr_llu = [gpr[i] + (".llu"    if i != 0x00 else "") for i in range(len(gpr))]

cop1_fmt = {
    0x10: "f[IX]",
    0x11: "d",
    0x14: "i[IX]",
}

cpu_reg = [
    # (0x0000000000000001, "r0"),
    (0x0000000000000002, "at"),
    (0x0000000000000004, "v0"),
    (0x0000000000000008, "v1"),
    (0x0000000000000010, "a0"),
    (0x0000000000000020, "a1"),
    (0x0000000000000040, "a2"),
    (0x0000000000000080, "a3"),
    (0x0000000000000100, "t0"),
    (0x0000000000000200, "t1"),
    (0x0000000000000400, "t2"),
    (0x0000000000000800, "t3"),
    (0x0000000000001000, "t4"),
    (0x0000000000002000, "t5"),
    (0x0000000000004000, "t6"),
    (0x0000000000008000, "t7"),
    (0x0000000000010000, "s0"),
    (0x0000000000020000, "s1"),
    (0x0000000000040000, "s2"),
    (0x0000000000080000, "s3"),
    (0x0000000000100000, "s4"),
    (0x0000000000200000, "s5"),
    (0x0000000000400000, "s6"),
    (0x0000000000800000, "s7"),
    (0x0000000001000000, "t8"),
    (0x0000000002000000, "t9"),
    # (0x0000000004000000, "k0"),
    # (0x0000000008000000, "k1"),
    (0x0000000010000000, "gp"), # [C]
    (0x0000000020000000, "sp"),
    (0x0000000040000000, "fp"),
    (0x0000000080000000, "ra"),
    (0x0001000000000000, "lo"),
    (0x0001000000000000, "hi"),
    (0x0000000100000000, "f0"),
    (0x0000000200000000, "f2"),
    (0x0000000400000000, "f4"),
    (0x0000000800000000, "f6"),
    (0x0000001000000000, "f8"),
    (0x0000002000000000, "f10"),
    (0x0000004000000000, "f12"),
    (0x0000008000000000, "f14"),
    (0x0000010000000000, "f16"),
    (0x0000020000000000, "f18"),
    (0x0000040000000000, "f20"),
    (0x0000080000000000, "f22"),
    (0x0000100000000000, "f24"),
    (0x0000200000000000, "f26"),
    (0x0000400000000000, "f28"),
    (0x0000800000000000, "f30"),
]

stack_reg = [
    ["reg_t", cpu_reg],
    ["uint ", [
        (0x0002000000000000, "c1c"),
    ]],
]

app       = None
xpr       = None
btbl      = None
jtbl      = None
data      = None
offs      = None
addr_s    = None
addr      = None
inst      = None
inst_op   = None
inst_rs   = None
inst_rt   = None
inst_rd   = None
inst_sa   = None
inst_func = None
inst_immu = None
inst_imms = None
inst_fmt  = None
inst_ft   = None
inst_fs   = None
inst_fd   = None
inst_bdst = None
inst_jdst = None
inst_maxb = None
reg_flag  = 0
chk_flag  = 0
stack_min  = 0
# stack_max  = 0
# stack_used = False
wret      = False

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
    if inst == 0x1000FFFF:
        # print("    0x%08X," % addr)
        return [(addr, "")], False
    if inst == 0x10000001:
        return [(addr, "")], False
    return op_table[inst_op]()

def op_maxb(jt):
    global inst_maxb
    if jt & 2:
        if inst_maxb < inst_bdst:
            inst_maxb = inst_bdst
    else:
        if inst_maxb < inst_jdst:
            inst_maxb = inst_jdst
    return jt

def op_jt():
    if inst == 0x10000001:
        return 0
    # if inst_op == 0x00:
    #     if inst_func == 0x08:
    #         return op_maxb(1)
    if inst_op == 0x01:
        if inst_rt in {0x00, 0x01}:
            return op_maxb(2)
        if inst_rt in {0x02, 0x03}:
            return op_maxb(3)
    if inst_op == 0x11:
        if inst_rs == 0x08:
            if inst_rt in {0x00, 0x01}:
                return op_maxb(2)
            if inst_rt in {0x02, 0x03}:
                return op_maxb(3)
    if inst_op == 0x02:
        return op_maxb(1)
    if inst_op in {0x04, 0x05, 0x06, 0x07}:
        return op_maxb(2)
    if inst_op in {0x14, 0x15, 0x16, 0x17}:
        return op_maxb(3)
    return 0

def op_end():
    return inst == 0x03E00008 # or (inst_op == 0x02 and wret)

def op_null():
    raise RuntimeError("null 0x%08X:0x%08X 0x%02X" % (addr, inst, inst_func))

def op_shift():
    global reg_flag
    reg_flag |= 0x0000000000000001 << inst_rd
    if inst == 0x00000000:
        return [(addr, "")], False
    if inst_func in {0x04, 0x06, 0x07}:
        mask = {
            0x04: 0x1F,
            0x06: 0x1F,
            0x07: 0x1F,
        }[inst_func]
        rs = "(%s & 0x%02X)" % (gpr_i[inst_rs], mask)
    else:
        sa = inst_sa
        if inst_func in {0x3C, 0x3E, 0x3F}:
            sa += 32
        rs = "%d" % sa
    op, gpr_rd, gpr_rt = {
        0x00: ("<<", gpr_i, gpr_iu),
        0x02: (">>", gpr_i, gpr_iu),
        0x03: (">>", gpr_i, gpr_i),
        0x04: ("<<", gpr_i, gpr_iu),
        0x06: (">>", gpr_i, gpr_iu),
        0x07: (">>", gpr_i, gpr_i),
        0x3C: ("<<", gpr_ll, gpr_llu),
        0x3E: (">>", gpr_ll, gpr_llu),
        0x3F: (">>", gpr_ll, gpr_ll),
    }[inst_func]
    rt = gpr_rt[inst_rt]
    rd = gpr_rd[inst_rd]
    return [(addr, "    %s = %s %s %s;\n" % (rd, rt, op, rs))], False

def op_jr():
    global addr
    global wret
    rs = inst_rs
    addr += 4
    next, end = op_process()
    l = next[0][1] if len(next) > 0 else ""
    if rs == 0x1F:
        line = "    return;\n"
        end = addr-4 >= inst_maxb
    else:
        line = (
            "    switch (%s)\n"
            "    {\n"
        ) % gpr_iu[rs]
        a = addr
        addr = addr_s
        wret = False
        while True:
            line += "        case 0x%08XU: goto _%08X; break;\n" % (addr, addr)
            btbl.add(addr)
            op_unpack()
            addr += 4
            op_jt()
            if op_end() and addr-4 >= inst_maxb:
                break
        addr = a
        line += "    }\n"
        end = False
    return [(addr-4, l+line)] + next, end

def op_jalr():
    global addr
    rs = gpr_iu[inst_rs]
    addr += 4
    next, end = op_process()
    l = next[0][1] if len(next) > 0 else ""
    line = "    __call(%s);\n" % rs
    return [(addr-4, l), (addr, line)], False

def op_break():
    return [(addr, "    __break(0x%05XU);\n" % (inst >> 6 & 0xFFFFF))], False

def op_mfhilo():
    global reg_flag
    reg_flag |= 0x0000000000000001 << inst_rd
    rd = gpr_ll[inst_rd]
    src = {
        0x10: "hi.ll",
        0x12: "lo.ll",
    }[inst_func]
    return [(addr, "    %s = %s;\n" % (rd, src))], False

def op_multdiv():
    global reg_flag
    reg_flag |= 0x0001000000000000
    line = ""
    if inst_func in {0x18, 0x19}:
        x, gpr_t = {
            0x18: ("s64", gpr_i),
            0x19: ("u64", gpr_iu),
        }[inst_func]
        rs = gpr_t[inst_rs]
        rt = gpr_t[inst_rt]
        line += (
            "    {\n"
            "        %s x = %s * %s;\n"
            "        lo.ll = (s32)(x >>  0);\n"
            "        hi.ll = (s32)(x >> 32);\n"
            "    }\n"
        ) % (x, rs, rt)
    if inst_func in {0x1A, 0x1B, 0x1E, 0x1F}:
        gpr_t = {
            0x1A: gpr_i,
            0x1B: gpr_iu,
            0x1E: gpr_ll,
            0x1F: gpr_llu,
        }[inst_func]
        rs = gpr_t[inst_rs]
        rt = gpr_t[inst_rt]
        line += (
            "    if (%s != 0)\n"
            "    {\n"
            "        lo.ll = (s32)(%s / %s);\n"
            "        hi.ll = (s32)(%s %% %s);\n"
            "    }\n"
        ) % (rt, rs, rt, rs, rt)
    return [(addr, line)], False

def op_arith():
    global reg_flag
    reg_flag |= 0x0000000000000001 << inst_rd
    op, gpr_rd, gpr_rs, gpr_rt, start, end = {
        0x20: ("+", gpr_i,  gpr_i,  gpr_i,  "", ""),
        0x21: ("+", gpr_i,  gpr_i,  gpr_i,  "", ""),
        0x22: ("-", gpr_i,  gpr_i,  gpr_i,  "", ""),
        0x23: ("-", gpr_i,  gpr_i,  gpr_i,  "", ""),
        0x24: ("&", gpr_ll, gpr_ll, gpr_ll, "", ""),
        0x25: ("|", gpr_ll, gpr_ll, gpr_ll, "", ""),
        0x26: ("^", gpr_ll, gpr_ll, gpr_ll, "", ""),
        0x27: ("|", gpr_ll, gpr_ll, gpr_ll, "~(", ")"),
        0x2A: ("<", gpr_i,  gpr_i,  gpr_i,  "", ""),
        0x2B: ("<", gpr_i,  gpr_iu, gpr_iu, "", ""),
        0x2D: ("+", gpr_ll, gpr_ll, gpr_ll, "", ""),
    }[inst_func]
    rs = gpr_rs[inst_rs]
    rt = gpr_rt[inst_rt]
    rd = gpr_rd[inst_rd]
    if addr in xpr:
        line = "    %s = %s;\n" % (rd, xpr[addr])
    else:
        line = "    %s = %s%s %s %s%s;\n" % (
            rd, start, rs, op, rt, end
        )
    return [(addr, line)], False

def op_special():
    return op_special_table[inst_func]()

def op_regimm():
    return op_regimm_table[inst_rt]()

def op_j():
    global wret
    # if wret == None:
    #     print("    0x%08X, # %06X" % (addr, addr-offs))
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
    next, end = op_process()
    l = next[0][1] if len(next) > 0 else ""
    if jdst in app.lib:
        line = "    lib_%s();\n" % app.lib[jdst]
    else:
        line = "    app_%08X();\n" % jdst
    return [(a, l), (addr, line)], end

def op_b():
    global addr
    jt = op_jt()
    if jt == 1:
        bc = "1"
        bdst = inst_jdst
    else:
        rs = gpr_i[inst_rs]
        rt = gpr_i[inst_rt]
        if inst_op in {0x01, 0x06, 0x07, 0x16, 0x17}:
            rt = "0"
        if inst_op == 0x11:
            bc = {
                0x00: "!c1c",
                0x01: "c1c",
                0x02: "!c1c",
                0x03: "c1c",
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
            bc = "%s %s %s" % (rs, op, rt)
        bdst = inst_bdst
    btbl.add(bdst)
    addr += 4
    next, end = op_process()
    lines = []
    line = (
        "    if (%s)\n"
        "    {\n"
    ) % bc
    l = next[0][1] if len(next) > 0 else ""
    if l != "":
        line += "    " + l
    line += (
        "        goto _%08X;\n"
        "    }\n"
    ) % bdst
    if l != "" and (jt & 2):
        line += (
            "    else\n"
            "    {\n"
        )
        # b
        if jt == 2:
            lines.append((addr-4, line))
            line = (
                "    " + l +
                "    }\n"
            )
            lines.append((addr, line))
        # bl
        if jt == 3:
            line += (
                "        goto b_%08X;\n"
                "    }\n"
            ) % (addr+4)
            lines.append((addr-4, line))
            line = l + ("b_%08X:;\n" % (addr+4))
            lines.append((addr, line))
    else:
        lines.append((addr-4, line))
        lines.append((addr, ""))
    return lines, False

def op_arithi():
    global reg_flag
    # global stack_max
    reg_flag |= 0x0000000000000001 << inst_rt
    # if inst_rt == 0x1D:
    #     s = 0x10 - inst_imms
    #     if stack_max < s:
    #         stack_max = s
    # sltiu
    if inst_op == 0x0B:
        imm = "0x%04XU" % (inst_imms & 0xFFFFFFFF)
    # andi, ori, xori
    elif inst_op in {0x0C, 0x0D, 0x0E}:
        imm = "0x%04XU" % inst_immu
    else:
        if inst_imms < 0:
            imm = "-0x%04X" % -inst_imms
        else:
            imm = "0x%04X" % inst_imms
    op, gpr_rt, gpr_rs = {
        0x08: ("+", gpr_i, gpr_i),
        0x09: ("+", gpr_i, gpr_i),
        0x0A: ("<", gpr_i, gpr_i),
        0x0B: ("<", gpr_i, gpr_iu),
        0x0C: ("&", gpr_ll, gpr_ll),
        0x0D: ("|", gpr_ll, gpr_ll),
        0x0E: ("^", gpr_ll, gpr_ll),
        0x19: ("+", gpr_ll, gpr_ll),
    }[inst_op]
    rs = gpr_rs[inst_rs]
    rt = gpr_rt[inst_rt]
    if addr in xpr:
        line = "    %s = %s;\n" % (rt, xpr[addr])
    else:
        line = "    %s = %s %s %s;\n" % (
            rt, rs, op, imm
        )
    return [(addr, line)], False

def op_lui():
    global reg_flag
    reg_flag |= 0x0000000000000001 << inst_rt
    rt = gpr_i[inst_rt]
    return [(addr, "    %s = 0x%04X0000;\n" % (rt, inst_immu))], False

def op_mfc1():
    global reg_flag
    reg_flag |= 0x0000000000000001 << inst_rt
    line = "    %s = f%d.iu[%d^IX];\n" % (
        gpr_iu[inst_rt], inst_rd & ~1, inst_rd & 1
    )
    return [(addr, line)], False

def op_mtc1():
    global reg_flag
    reg_flag |= 0x0000000100000000 << (inst_rd >> 1)
    rd = {
        0x04: "iu[%d^IX]" % (inst_rd & 1), # mtc1
        0x05: "llu",                       # dmtc1
    }[inst_rs]
    line = "    f%d.%s = %s;\n" % (
        inst_rd & ~1, rd, gpr_iu[inst_rt]
    )
    return [(addr, line)], False

def op_cfc1():
    global reg_flag
    reg_flag |= 0x0000000000000001 << inst_rt
    return [(addr, "    %s = 0;\n" % gpr_iu[inst_rt])], False

def op_ctc1():
    return [(addr, "    /* CTC1 */\n")], False

def op_arithf():
    global reg_flag
    reg_flag |= 0x0000000100000000 << (inst_fd >> 1)
    fmt = cop1_fmt[inst_fmt]
    ft = "f%d.%s" % (inst_ft & ~1, fmt)
    fs = "f%d.%s" % (inst_fs & ~1, fmt)
    fd = "f%d.%s" % (inst_fd & ~1, fmt)
    if inst_func in {0x04, 0x05, 0x06, 0x07}:
        start, end = {
            0x04: ("sqrtf(", ")"),
            0x05: ("fabsf(", ")"),
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

def op_cvt():
    global reg_flag
    reg_flag |= 0x0000000100000000 << (inst_fd >> 1)
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
    global reg_flag
    reg_flag |= 0x0002000000000000
    op = {
        0x32: "==",
        0x3C: "<",
        0x3E: "<=",
    }[inst_func]
    fmt = cop1_fmt[inst_fmt]
    ft = "f%d.%s" % (inst_ft & ~1, fmt)
    fs = "f%d.%s" % (inst_fs & ~1, fmt)
    return [(addr, "    c1c = %s %s %s;\n" % (fs, op, ft))], False

def op_cop1_f():
    return op_cop1_func_table[inst_func]()

def op_cop1():
    return op_cop1_table[inst_rs]()

def op_load():
    global reg_flag
    # global stack_used
    global wret
    rs = gpr_i[inst_rs]
    if inst_op == 0x31:
        reg_flag |= 0x0000000100000000 << (inst_rt >> 1)
        rt = "f%d.iu[%d^IX] = " % (inst_rt & ~1, inst_rt & 1)
    elif inst_op == 0x35:
        reg_flag |= 0x0000000100000000 << (inst_rt >> 1)
        rt = "f%d.llu = " % (inst_rt & ~1)
    else:
        if inst_rt == 0x00:
            return [(addr, "")], False
        reg_flag |= 0x0000000000000001 << inst_rt
        if inst_op in {0x22, 0x26}:
            rs += ", &%s" % gpr_iu[inst_rt]
            rt = ""
        else:
            rt = "%s = " % gpr_i[inst_rt]
    t = {
        0x20: "s8",
        0x21: "s16",
        0x22: "u32_l",
        0x23: "s32",
        0x24: "u8",
        0x25: "u16",
        0x26: "u32_r",
        0x27: "u32",
        0x31: "u32",
        0x35: "u64",
        0x37: "s64",
    }[inst_op]
    # if inst_rs == 0x1D and inst_immu >= stack_min and inst_immu < stack_max:
    #     stack_used = True
    #     k = {
    #         0x20: 7,
    #         0x21: 6,
    #         0x23: 4,
    #         0x24: 7,
    #         0x25: 6,
    #         0x27: 4,
    #         0x31: 4,
    #         0x35: 0,
    #         0x37: 0,
    #     }[inst_op]
    #     line = "    %s*(%s *)&stack[0x%04X];\n" % (rt, t, inst_immu ^ k)
    # else:
    line = "    %s__read_%s((s16)0x%04X + %s);\n" % (rt, t, inst_immu, rs)
    return [(addr, line)], False

def op_store():
    global chk_flag
    # global stack_used
    rs = gpr_i[inst_rs]
    if inst_op == 0x39 or inst_op == 0x3D:
        flag = 0x000100000000 << (inst_rt >> 1)
        # f20-f30
        if (flag & 0xFC0000000000) and not ((reg_flag | chk_flag) & flag):
            chk_flag |= flag
            return [(addr, "")], False
        if inst_op == 0x39:
            rt = "f%d.iu[%d^IX]" % (inst_rt & ~1, inst_rt & 1)
        else:
            rt = "f%d.llu" % (inst_rt & ~1)
    else:
        flag = 0x000000000001 << inst_rt
        # s0-s7, gp, s8, ra
        if (flag & 0x0000D0FF0000) and not ((reg_flag | chk_flag) & flag):
            chk_flag |= flag
            return [(addr, "")], False
        rt = gpr_i[inst_rt]
    t = {
        0x28: "u8",
        0x29: "u16",
        0x2A: "u32_l",
        0x2B: "u32",
        0x2E: "u32_r",
        0x39: "u32",
        0x3D: "u64",
    }[inst_op]
    # if inst_rs == 0x1D and inst_immu >= stack_min and inst_immu < stack_max:
    #     stack_used = True
    #     k = {
    #         0x28: 7,
    #         0x29: 6,
    #         0x2B: 4,
    #         0x39: 4,
    #         0x3D: 0,
    #     }[inst_op]
    #     line = "    *(%s *)&stack[0x%04X] = %s;\n" % (t, inst_immu ^ k, rt)
    # else:
    line = "    __write_%s((s16)0x%04X + %s, %s);\n" % (t, inst_immu, rs, rt)
    return [(addr, line)], False

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
    op_multdiv, # 0x18 mult [C]
    op_multdiv, # 0x19 multu
    op_multdiv, # 0x1A div
    op_multdiv, # 0x1B divu
    op_null,    # 0x1C dmult
    op_null,    # 0x1D dmultu
    op_multdiv, # 0x1E ddiv
    op_multdiv, # 0x1F ddivu
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
    op_null,    # 0x11 bgezal
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
    op_null,    # 0x0C round.w
    op_cvt,     # 0x0D trunc.w
    op_null,    # 0x0E ceil.w
    op_null,    # 0x0F floor.w
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
    op_load,    # 0x22 lwl
    op_load,    # 0x23 lw
    op_load,    # 0x24 lbu
    op_load,    # 0x25 lhu
    op_load,    # 0x26 lwr
    op_null,    # 0x27 lwu
    op_store,   # 0x28 sb
    op_store,   # 0x29 sh
    op_store,   # 0x2A swl
    op_store,   # 0x2B sw
    op_null,    # 0x2C sdl
    op_null,    # 0x2D sdr
    op_store,   # 0x2E swr
    op_null,    # 0x2F cache
    op_null,    # 0x30 ll
    op_load,    # 0x31 lwc1
    op_null,    # 0x32 lwc2
    op_null,    # 0x33
    op_null,    # 0x34 lld
    op_load,    # 0x35 ldc1
    op_null,    # 0x36 ldc2
    op_load,    # 0x37 ld
    op_null,    # 0x38 sc
    op_store,   # 0x39 swc1
    op_null,    # 0x3A swc2
    op_null,    # 0x3B
    op_null,    # 0x3C scd
    op_store,   # 0x3D sdc1
    op_null,    # 0x3E sdc2
    op_null,    # 0x3F sd
]

def main(argc, argv):
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
    global stack_min
    # global stack_max
    # global stack_used
    global wret
    if argc < 2:
        print("usage: %s <app>" % argv[0])
        return 1
    app = importlib.import_module("app.%s.app" % argv[1])
    if argc > 2:
        app_src = ["app"] + [
            "%08X" % src for src, start, end, dst, pat, xpr, ins in app.segment
        ]
        print(" ".join(["%s%s.o" % (argv[2], app) for app in app_src]))
        return 0
    path_app   = os.path.join("app", argv[1])
    path_build = os.path.join("build", argv[1])
    with open(os.path.join(path_app, "app.bin"), "rb") as f:
        data = f.read()
    data = app.patch(data)
    app_h = (
        "#ifndef _APP_H_\n"
        "#define _APP_H_\n"
        "\n"
        "#include \"types.h\"\n"
        "\n"
        "#define APP_U%c%c%c\n"
        "#define APP_%c%d\n"
    ) % struct.unpack(">59xBBBBB", data[:0x40])
    if len(app.dcall) > 0:
        app_h += "#define APP_DCALL\n"
    if len(app.cache) > 0:
        app_h += "#define APP_CACHE\n"
    a = ""
    b = ""
    i = 0
    for flag, reg in cpu_reg:
        if app.reg & flag:
            m = "R_" + reg.upper()
            a += "#define %s 0x%02X\n" % (m.ljust(15), i)
            b += "#define %s cpu_reg[%s]\n" % (reg, m)
            i += 1
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
        "%s"
        "#define CPU_REG_LEN     0x%02X\n"
        "\n"
        "struct app_call_t\n"
        "{\n"
        "    u32    addr;\n"
        "    void (*call)(void);\n"
        "};\n"
        "\n"
    ) % (
        app.header,
        argv[1], app.entry, app.bss[0], app.bss[1], app.sp, app.main,
        a, b, i
    )
    if len(app.cache) > 0:
        app_h += (
            "struct app_cache_t\n"
            "{\n"
            "    u32 addr;\n"
            "    u32 size;\n"
            "};\n"
            "\n"
        )
    g_addr = set()
    d_addr = set()
    for src, start, end, dst, pat, xpr, ins in app.segment:
        offs = start - src
        addr = start
        while addr < end:
            f = False
            while True:
                op_unpack()
                if inst != 0x00000000:
                    break
                f = True
                addr += 4
            if addr >= end:
                break
            # if f:
            #     print("    0x%08X," % addr)
            dst.append(addr)
            inst_maxb = 0
            stack_min  = 0x00
            # stack_max  = 0x10
            # stack_used = False
            wret = None
            while True:
                op_unpack()
                op_jt()
                if op_end() and addr >= inst_maxb:
                    break
                addr += 4
            addr += 8
        dst.sort()
        dst = set(dst)
        d_addr |= dst & g_addr
        g_addr |= dst
    g_addr = sorted(g_addr)
    d_addr = sorted(d_addr)
    app_h += "extern const struct app_call_t app_call_table[%d];\n" % \
        len(g_addr)
    if len(app.dcall) > 0:
        app_h += "extern const u32 app_dcall_table[%d];\n" % len(app.dcall)
    if len(app.cache) > 0:
        app_h += "extern const struct app_cache_t app_cache_table[%d];\n" % \
            len(app.cache)
    app_h += "\n"
    app_c = (
        "#include \"types.h\"\n"
        "#include \"app.h\"\n"
        "#include \"cpu.h\"\n"
        "#include \"lib.h\"\n"
        "\n"
        "const struct app_call_t app_call_table[%d] =\n"
        "{\n"
    ) % len(g_addr)
    for addr in g_addr:
        app_c += "    {0x%08XU, app_%08X},\n" % (addr, addr)
    app_c += "};\n"
    if len(app.dcall) > 0:
        app_c += (
            "\n"
            "const u32 app_dcall_table[%d] =\n"
            "{\n"
        ) % len(app.dcall)
        for addr in app.dcall:
            app_c += "    0x%08XU,\n" % (addr & 0x1FFFFFFF)
        app_c += "};\n"
    app_c += "\n"
    if len(app.cache) > 0:
        app_c += (
            "const struct app_cache_t app_cache_table[%d] =\n"
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
    with open(os.path.join(path_build, "app.c"), "w") as f:
        f.write(app_c)
    jtbl = set()
    for src, start, end, dst, pat, xpr, ins in app.segment:
        offs = start - src
        for addr in pat:
            patch = B"".join([struct.pack(">I", x) for x in pat[addr]])
            data = data[:addr-offs] + patch + data[addr-offs + len(patch):]
        app_c = (
            "#include <math.h>\n"
            "\n"
            "#define _%s_%08X_C_\n"
            "#include \"types.h\"\n"
            "#include \"app.h\"\n"
            "#include \"cpu.h\"\n"
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
            chk_flag = 0
            stack_min  = 0x00
            # stack_max  = 0x10
            # stack_used = False
            wret = None
            btbl = set()
            lines = []
            while True:
                line, end = op_process()
                lines += line
                addr += 4
                if end:
                    break
            # if stack_used:
            #     app_c += "    u8 stack[0x%04X];\n" % (
            #         (stack_max+0x07) & ~0x07
            #     )
            for t, reg in stack_reg:
                for flag, r in reg:
                    if app.reg & flag:
                        continue
                    if (reg_flag | chk_flag) & flag:
                        app_c += "    unused %s %s;\n" % (t, r)
            for addr, line in lines:
                if addr in btbl:
                    app_c += "_%08X:;\n" % addr
                if addr in ins:
                    app_c += ins[addr]
                app_c += line
            app_c += "}\n"
        with open(os.path.join(path_build, "%08X.c" % src), "w") as f:
            f.write(app_c)
    app_h += (
        "\n"
        "#endif /* _APP_H_ */\n"
    )
    with open(os.path.join(path_build, "app.h"), "w") as f:
        f.write(app_h)
    for addr in sorted(jtbl):
        for src, start, end, dst, pat, xpr, ins in app.segment:
            if addr in dst:
                break
        else:
            if addr not in app.lib:
                print("    0x%08X: \"%08X\"," % (addr, addr))
    return 0

if __name__ == "__main__":
    sys.exit(main(len(sys.argv), sys.argv))
