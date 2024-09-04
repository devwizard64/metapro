static void *gdp_texture_rgba16(TXTARG *arg, const u8 *src, uint w, uint h)
{
	void *buf;
	u16 *dst;
	uint len;
	arg->type = GL_RGBA;
	buf = dst = malloc(2*w*h);
	len = w*h;
	do
	{
		uint x = src[0] << 8 | src[1];
		dst[0] =
			(x >> 11         ) |
			(x >>  1 & 0x03E0) |
			(x <<  9 & 0x7C00) |
			(x << 15         );
		dst += 1;
		src += 2;
	}
	while (--len > 0);
	return buf;
}

#ifndef APP_UNSM
static void *gdp_texture_rgba32(TXTARG *arg, const u8 *src, uint w, uint h)
{
	void *buf;
	u16 *dst;
	uint len;
	arg->type = GL_RGBA;
	buf = dst = malloc(2*w*h);
	len = w*h;
	do
	{
		dst[0] =
			(src[0] >> 3 & 0x001F) |
			(src[1] << 2 & 0x03E0) |
			(src[2] << 7 & 0x7C00) |
			(src[3] << 8 & 0x8000);
		dst += 1;
		src += 4;
	}
	while (--len > 0);
	return buf;
}
#else
#define gdp_texture_rgba32 NULL
#endif

#define gdp_texture_yuv16  NULL

static void *gdp_texture_ia4(TXTARG *arg, const u8 *src, uint w, uint h)
{
	void *buf;
	u16 *dst;
	uint len;
	arg->type = GL_RGBA;
	buf = dst = malloc(2*w*h);
	len = w*h/2;
	do
	{
		uint h = src[0] >> 5;
		uint l = src[0] >> 1 & 7;
		dst[0] = 0x421*(h << 2 | h >> 1) | (src[0] << 11 & 0x8000);
		dst[1] = 0x421*(l << 2 | l >> 1) | (src[0] << 15 & 0x8000);
		dst += 2;
		src += 1;
	}
	while (--len > 0);
	return buf;
}

static void *gdp_texture_ia8(TXTARG *arg, const u8 *src, uint w, uint h)
{
	void *buf;
	u16 *dst;
	uint len;
	arg->type = GL_RGBA;
	buf = dst = malloc(2*w*h);
	len = w*h;
	do
	{
		uint i = src[0] >> 4;
		dst[0] = 0x421*(i << 1 | i >> 3) | (src[0] << 12 & 0x8000);
		dst += 1;
		src += 1;
	}
	while (--len > 0);
	return buf;
}

#ifdef APP_UNSM
static void *gdp_texture_ia8_face(TXTARG *arg, const u8 *src, uint w, uint h)
{
	void *buf;
	u16 *dst;
	uint len;
	arg->type = GL_RGBA;
	buf = dst = malloc(2*w*h);
	len = w*h;
	do
	{
		dst[0] = 0x7FFF | (src[0] << 8 & 0x8000);
		dst += 1;
		src += 1;
	}
	while (--len > 0);
	return buf;
}
#endif

static void *gdp_texture_ia16(TXTARG *arg, const u8 *src, uint w, uint h)
{
	void *buf;
	u16 *dst;
	uint len;
	arg->type = GL_RGBA;
	buf = dst = malloc(2*w*h);
	len = w*h;
	do
	{
		dst[0] = 0x421*(src[0] >> 3) | (src[1] << 8 & 0x8000);
		dst += 1;
		src += 2;
	}
	while (--len > 0);
	return buf;
}

#ifndef APP_UNSM
static void *gdp_texture_i4(TXTARG *arg, const u8 *src, uint w, uint h)
{
	void *buf;
	u16 *dst;
	uint len;
	arg->type = GL_RGBA;
	buf = dst = malloc(2*w*h);
	len = w*h/2;
	do
	{
		uint h = src[0] >> 4;
		uint l = src[0] & 15;
		dst[0] = 0x421*(h << 1 | h >> 3) | (h << 12 & 0x8000);
		dst[0] = 0x421*(l << 1 | l >> 3) | (l << 12 & 0x8000);
		dst += 2;
		src += 1;
	}
	while (--len > 0);
	return buf;
}

static void *gdp_texture_i8(TXTARG *arg, const u8 *src, uint w, uint h)
{
	void *buf;
	u16 *dst;
	uint len;
	arg->type = GL_RGBA;
	buf = dst = malloc(2*w*h);
	len = w*h;
	do
	{
		dst[0] = 0x421*(src[0] >> 3) | (src[0] << 8 & 0x8000);
		dst += 1;
		src += 1;
	}
	while (--len > 0);
	return buf;
}
#else
#define gdp_texture_i4     NULL
#define gdp_texture_i8     NULL
#endif
