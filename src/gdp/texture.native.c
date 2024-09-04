static void *gdp_texture_rgba16(TXTARG *arg, const u8 *src, uint w, uint h)
{
	arg->internalFormat = GL_RGBA;
	arg->format         = GL_RGBA;
	arg->type           = GL_UNSIGNED_SHORT_5_5_5_1;
	return __halfswap(malloc(2*w*h), src, 2*w*h);
}

#ifndef APP_UNSM
static void *gdp_texture_rgba32(TXTARG *arg, const u8 *src, uint w, uint h)
{
	arg->internalFormat = GL_RGBA;
	arg->format         = GL_RGBA;
	arg->type           = GL_UNSIGNED_BYTE;
	return memcpy(malloc(4*w*h), src, 4*w*h);
}
#else
#define gdp_texture_rgba32 NULL
#endif

#define gdp_texture_yuv16  NULL

static void *gdp_texture_ia4(TXTARG *arg, const u8 *src, uint w, uint h)
{
	void *buf;
	u8 *dst;
	uint len;
	arg->internalFormat = GL_LUMINANCE_ALPHA;
	arg->format         = GL_LUMINANCE_ALPHA;
	arg->type           = GL_UNSIGNED_BYTE;
	buf = dst = malloc(2*w*h);
	len = w*h/2;
	do
	{
		dst[0] = IA4_IH(src[0]);
		dst[1] = IA4_AH(src[0]);
		dst[2] = IA4_IL(src[0]);
		dst[3] = IA4_AL(src[0]);
		dst += 4;
		src += 1;
	}
	while (--len > 0);
	return buf;
}

static void *gdp_texture_ia8(TXTARG *arg, const u8 *src, uint w, uint h)
{
	void *buf;
	u8 *dst;
	uint len;
	arg->internalFormat = GL_LUMINANCE_ALPHA;
	arg->format         = GL_LUMINANCE_ALPHA;
	arg->type           = GL_UNSIGNED_BYTE;
	buf = dst = malloc(2*w*h);
	len = w*h;
	do
	{
		dst[0] = IA8_I(src[0]);
		dst[1] = IA8_A(src[0]);
		dst += 2;
		src += 1;
	}
	while (--len > 0);
	return buf;
}

#ifdef APP_UNSM
static void *gdp_texture_ia8_face(TXTARG *arg, const u8 *src, uint w, uint h)
{
	void *buf;
	u8 *dst;
	uint len;
	arg->internalFormat = GL_RGBA;
	arg->format         = GL_LUMINANCE_ALPHA;
	arg->type           = GL_UNSIGNED_BYTE;
	buf = dst = malloc(2*w*h);
	len = w*h;
	do
	{
		dst[0] = 0xFF;
		dst[1] = IA8_I(src[0]);
		dst += 2;
		src += 1;
	}
	while (--len > 0);
	return buf;
}
#endif

static void *gdp_texture_ia16(TXTARG *arg, const u8 *src, uint w, uint h)
{
	arg->internalFormat = GL_LUMINANCE_ALPHA;
	arg->format         = GL_LUMINANCE_ALPHA;
	arg->type           = GL_UNSIGNED_BYTE;
	return memcpy(malloc(2*w*h), src, 2*w*h);
}

#ifndef APP_UNSM
static void *gdp_texture_i4(TXTARG *arg, const u8 *src, uint w, uint h)
{
	void *buf;
	u8 *dst;
	uint len;
	arg->internalFormat = GL_LUMINANCE_ALPHA;
	arg->format         = GL_LUMINANCE_ALPHA;
	arg->type           = GL_UNSIGNED_BYTE;
	buf = dst = malloc(2*w*h);
	len = w*h/2;
	do
	{
		dst[0] = dst[1] = I4_IH(src[0]);
		dst[2] = dst[3] = I4_IL(src[0]);
		dst += 4;
		src += 1;
	}
	while (--len > 0);
	return buf;
}

static void *gdp_texture_i8(TXTARG *arg, const u8 *src, uint w, uint h)
{
	void *buf;
	u8 *dst;
	uint len;
	arg->internalFormat = GL_LUMINANCE_ALPHA;
	arg->format         = GL_LUMINANCE_ALPHA;
	arg->type           = GL_UNSIGNED_BYTE;
	buf = dst = malloc(2*w*h);
	len = w*h;
	do
	{
		dst[0] = dst[1] = src[0];
		dst += 2;
		src += 1;
	}
	while (--len > 0);
	return buf;
}
#else
#define gdp_texture_i4     NULL
#define gdp_texture_i8     NULL
#endif
