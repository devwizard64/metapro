from app.UNSME00 import *

reg = (
	1 <<  2 | # v0
	1 <<  3 | # v1
	1 <<  4 | # a0
	1 <<  5 | # a1
	1 <<  6 | # a2
	1 <<  7 | # a3
	1 <<  8 | # t0
	1 <<  9 | # t1
	1 << 10 | # t2
	1 << 11 | # t3
	1 << 12 | # t4
	1 << 13 | # t5
	1 << 14 | # t6
	1 << 15 | # t7
	1 << 16 | # s0
	1 << 17 | # s1
	1 << 18 | # s2
	1 << 19 | # s3
	1 << 20 | # s4
	1 << 21 | # s5
	1 << 22 | # s6
	1 << 23 | # s7
	1 << 24 | # t8
	1 << 25 | # t9
	1 << 29 | # sp
	1 << 30 | # s8
	1 << 31 | # ra
	1 << 32 | # f0
	1 << 33 | # f2
	1 << 34 | # f4
	1 << 35 | # f6
	1 << 36 | # f8
	1 << 37 | # f10
	1 << 38 | # f12
	1 << 39 | # f14
	1 << 40 | # f16
	1 << 41 | # f18
	1 << 42 | # f20
	1 << 43 | # f22
	1 << 44 | # f24
	1 << 45 | # f26
	1 << 46 | # f28
	1 << 47 | # f30
	1 << 48   # lo/hi
)

a02_dst = [
	0x8029BF00,
	0x802C3460,
]

a02_pat.update({
	# ???
	0x802C3458: [0x03E00008],
})

a07_pat = {
	# ???
	0x80384E24: [0x00000000],
}

c03_pat = {
	# ???
	0x80402F78: [0x00000000],
}

segment = [
	[0x00001050, 0x80246050, 0x8027F590, [], a00_pat, a00_xpr, a00_ins],
	[0x0003A590, 0x8027F590, 0x8029C770, [], {}, {}, {}],
	[0x00057770, 0x8029C770, 0x802CB5C0, a02_dst, a02_pat, {}, {}],
	[0x000865C0, 0x802CB5C0, 0x802D5E00, [], a03_pat, a03_xpr, a03_ins],
	[0x00090E00, 0x802D5E00, 0x802F9730, [], {}, a04_xpr, a04_ins],
	[0x000B4730, 0x802F9730, 0x80314A30, [], {}, {}, {}],
	# 0x8032B260
	[0x000CFA30, 0x80314A30, 0x803223B0, [], {}, {}, a06_ins],
	[0x000F5580, 0x80378800, 0x80385F90, [], a07_pat, {}, {}],
	[0x0021F4C0, 0x8016F000, 0x801A7830, [], {}, {}, {}],
	[0x01202000, 0x80402000, 0x80402234, [], {}, {}, {}],
	[0x01202700, 0x80402700, 0x80402728, [], {}, {}, {}],
	[0x01202F50, 0x80402F50, 0x80402F98, [], c03_pat, {}, {}],
	[0x01203000, 0x80403000, 0x8040347C, [], {}, {}, {}],
	[0x01203500, 0x80403500, 0x80403648, [], {}, {}, {}],
]
