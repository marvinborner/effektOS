#ifndef EFFEKT_TRUETYPE_C
#define EFFEKT_TRUETYPE_C

#define TTF_GLYPH_DATA 16

// recovering from the loss of libm

static int c_ttf_ifloor(double x)
{
	int i = (int)x;
	return x < (double)i ? i - 1 : i;
}

static int c_ttf_iceil(double x)
{
	int i = (int)x;
	return x > (double)i ? i + 1 : i;
}

static double c_ttf_sqrt(double x)
{
	double root;
	__asm__("sqrtsd %1, %0" : "=x"(root) : "x"(x));
	return root;
}

static unsigned long c_ttf_strlen(const char *string)
{
	unsigned long length = 0;

	while (string[length])
		length++;
	return length;
}

static double c_ttf_unsupported(void)
{
	hole("truetype: unsupported math");
	return 0.0;
}

#define STBTT_ifloor(x) c_ttf_ifloor(x)
#define STBTT_iceil(x) c_ttf_iceil(x)
#define STBTT_sqrt(x) c_ttf_sqrt(x)
#define STBTT_pow(x, y) ((void)(x), (void)(y), c_ttf_unsupported())
#define STBTT_fmod(x, y) ((void)(x), (void)(y), c_ttf_unsupported())
#define STBTT_cos(x) ((void)(x), c_ttf_unsupported())
#define STBTT_acos(x) ((void)(x), c_ttf_unsupported())
#define STBTT_fabs(x) __builtin_fabs(x)
#define STBTT_malloc(x, u) ((void)(u), malloc(x))
#define STBTT_free(x, u) ((void)(u), free(x))
#define STBTT_assert(x) ((void)0)
#define STBTT_strlen(x) c_ttf_strlen(x)
#define STBTT_memcpy memcpy
#define STBTT_memset memset

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

static stbtt_fontinfo *c_ttf_font(struct Pos info)
{
	return (stbtt_fontinfo *)c_bytearray_data(info);
}

// Effekt-side shall never know "font units"
static float c_ttf_scale(const stbtt_fontinfo *font, Int pixels)
{
	return stbtt_ScaleForPixelHeight(font, (float)pixels);
}

struct Pos c_ttf_init(struct Pos data);
struct Pos c_ttf_init(struct Pos data)
{
	const unsigned char *bytes = c_bytearray_data(data);
	int offset = stbtt_GetFontOffsetForIndex(bytes, 0);
	struct Pos info = c_bytearray_new(sizeof(stbtt_fontinfo));

	if (offset < 0 || !stbtt_InitFont(c_ttf_font(info), bytes, offset)) {
		erasePositive(info);
		erasePositive(data);
		return c_bytearray_new(0);
	}

	erasePositive(data);
	return info;
}

Int c_ttf_baseline(struct Pos info, Int pixels);
Int c_ttf_baseline(struct Pos info, Int pixels)
{
	const stbtt_fontinfo *font = c_ttf_font(info);
	float scale = c_ttf_scale(font, pixels);
	int ascent = 0;

	stbtt_GetFontVMetrics(font, &ascent, 0, 0);
	erasePositive(info);
	return (Int)((float)ascent * scale + 0.5f);
}

Double c_ttf_step(struct Pos info, Int pixels, Int codepoint);
Double c_ttf_step(struct Pos info, Int pixels, Int codepoint)
{
	const stbtt_fontinfo *font = c_ttf_font(info);
	float scale = c_ttf_scale(font, pixels);
	int advance = 0;

	stbtt_GetCodepointHMetrics(font, (int)codepoint, &advance, 0);
	erasePositive(info);
	return (Double)((float)advance * scale);
}

struct Pos c_ttf_glyph(struct Pos info, Int pixels, Int codepoint);
struct Pos c_ttf_glyph(struct Pos info, Int pixels, Int codepoint)
{
	const stbtt_fontinfo *font = c_ttf_font(info);
	float scale = c_ttf_scale(font, pixels);
	int width = 0, height = 0, x = 0, y = 0;
	unsigned char *coverage = stbtt_GetCodepointBitmap(
		font, 0, scale, (int)codepoint, &width, &height, &x, &y);
	struct Pos glyph = c_bytearray_new(TTF_GLYPH_DATA + width * height);
	int32_t *header = (int32_t *)c_bytearray_data(glyph);

	header[0] = width;
	header[1] = height;
	header[2] = x;
	header[3] = y;

	if (coverage) {
		memcpy(c_bytearray_data(glyph) + TTF_GLYPH_DATA, coverage,
		       (size_t)(width * height));
		stbtt_FreeBitmap(coverage, 0);
	}

	erasePositive(info);
	return glyph;
}

#endif
