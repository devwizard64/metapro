static void *gdp_texture_rgba16(TXTARG *arg, const u8 *src, uint w, uint h)
{
	void *buf;
	u8 *dst;
	uint len;
	arg->internalFormat = GPU_RGBA5551;
	buf = dst = malloc(2*w*h);
	len = w*h;
	do
	{
		dst[0] = src[1];
		dst[1] = src[0];
		dst += 2;
		src += 2;
	}
	while (--len > 0);
	return buf;
}

#ifndef APP_UNSM
static void *gdp_texture_rgba32(TXTARG *arg, const u8 *src, uint w, uint h)
{
	arg->internalFormat = GPU_RGBA8;
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
	arg->internalFormat = GPU_LA4;
	buf = dst = malloc(w*h);
	len = w*h/2;
	do
	{
		dst[0] = IA4_IAH(src[0]);
		dst[1] = IA4_IAL(src[0]);
		dst += 2;
		src += 1;
	}
	while (--len > 0);
	return buf;
}

static void *gdp_texture_ia8(TXTARG *arg, const u8 *src, uint w, uint h)
{
	arg->internalFormat = GPU_LA4;
	return memcpy(malloc(w*h), src, w*h);
}

#ifdef APP_UNSM
static void *gdp_texture_ia8_face(TXTARG *arg, const u8 *src, uint w, uint h)
{
	void *buf;
	u8 *dst;
	uint len;
	arg->internalFormat = GPU_LA4;
	buf = dst = malloc(w*h);
	len = w*h;
	do
	{
		dst[0] = 0xF0 | (src[0] >> 4);
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
	u8 *dst;
	uint len;
	arg->internalFormat = GPU_LA8;
	buf = dst = malloc(2*w*h);
	len = w*h;
	do
	{
		dst[0] = src[1];
		dst[1] = src[0];
		dst += 2;
		src += 2;
	}
	while (--len > 0);
	return buf;
}

#ifndef APP_UNSM
static void *gdp_texture_i4(TXTARG *arg, const u8 *src, uint w, uint h)
{
	void *buf;
	u8 *dst;
	uint len;
	arg->internalFormat = GPU_LA4;
	buf = dst = malloc(w*h);
	len = w*h/2;
	do
	{
		dst[0] = I4_IH(src[0]);
		dst[1] = I4_IL(src[0]);
		dst += 2;
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
	arg->internalFormat = GPU_LA8;
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
