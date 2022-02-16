/*
 * WebP encoding support via libwebp
 * Copyright (c) 2013 Justin Ruggles <justin.ruggles@gmail.com>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * WebP encoder using libwebp: common structs and methods.
 */

#include "libavutil/opt.h"
#include "libwebpenc_common.h"

const AVCodecDefault ff_libwebp_defaults[] = {
    { "compression_level",  "4"  },
    { "global_quality",     "-1" },
    { NULL },
};

#define OFFSET(x) offsetof(LibWebPContextCommon, x)
#define VE AV_OPT_FLAG_VIDEO_PARAM | AV_OPT_FLAG_ENCODING_PARAM
static const AVOption options[] = {
    { "lossless",   "Use lossless mode",       OFFSET(lossless), AV_OPT_TYPE_INT,   { .i64 =  0 },  0, 1,                           VE           },
    { "preset",     "Configuration preset",    OFFSET(preset),   AV_OPT_TYPE_INT,   { .i64 = -1 }, -1, WEBP_PRESET_TEXT,            VE, "preset" },
    { "none",       "do not use a preset",                              0, AV_OPT_TYPE_CONST, { .i64 = -1                  }, 0, 0, VE, "preset" },
    { "default",    "default preset",                                   0, AV_OPT_TYPE_CONST, { .i64 = WEBP_PRESET_DEFAULT }, 0, 0, VE, "preset" },
    { "picture",    "digital picture, like portrait, inner shot",       0, AV_OPT_TYPE_CONST, { .i64 = WEBP_PRESET_PICTURE }, 0, 0, VE, "preset" },
    { "photo",      "outdoor photograph, with natural lighting",        0, AV_OPT_TYPE_CONST, { .i64 = WEBP_PRESET_PHOTO   }, 0, 0, VE, "preset" },
    { "drawing",    "hand or line drawing, with high-contrast details", 0, AV_OPT_TYPE_CONST, { .i64 = WEBP_PRESET_DRAWING }, 0, 0, VE, "preset" },
    { "icon",       "small-sized colorful images",                      0, AV_OPT_TYPE_CONST, { .i64 = WEBP_PRESET_ICON    }, 0, 0, VE, "preset" },
    { "text",       "text-like",                                        0, AV_OPT_TYPE_CONST, { .i64 = WEBP_PRESET_TEXT    }, 0, 0, VE, "preset" },
    { "cr_threshold","Conditional replenishment threshold",     OFFSET(cr_threshold), AV_OPT_TYPE_INT, { .i64 =  0  },  0, INT_MAX, VE           },
    { "cr_size"     ,"Conditional replenishment block size",    OFFSET(cr_size)     , AV_OPT_TYPE_INT, { .i64 =  16 },  0, 256,     VE           },
    { "quality"     ,"Quality",                OFFSET(quality),  AV_OPT_TYPE_FLOAT, { .dbl =  75 }, 0, 100,                         VE           },

	{ "webp_lossless"     ,"Lossless encoding (0=lossy(default), 1=lossless)",                OFFSET(user_config.lossless),  AV_OPT_TYPE_INT, { .i64 =  0 }, 0, 1,                         VE           },
	{ "webp_quality"     ,"Quality between 0 and 100", OFFSET(user_config.quality),  AV_OPT_TYPE_FLOAT, { .dbl =  75 }, 0, 100,                         VE           },
	{ "webp_method"     ,"quality/speed trade-off (0=fast, 6=slower-better)",                OFFSET(user_config.method),  AV_OPT_TYPE_INT, { .i64 =  4 }, 0, 6,                         VE           },
	{ "webp_target_size"     ,"If non-zero, set the desired target size in bytes.",                OFFSET(user_config.target_size),  AV_OPT_TYPE_INT, { .i64 =  0 }, 0, INT_MAX,                         VE           },
	{ "webp_target_PSNR"     ,"if non-zero, specifies the minimal distortion to try to achieve.", OFFSET(user_config.target_PSNR),  AV_OPT_TYPE_FLOAT, { .dbl =  0 }, 0, INT_MAX,                         VE           },
	{ "webp_segments"     ,"maximum number of segments to use, in [1..4].",                OFFSET(user_config.segments),  AV_OPT_TYPE_INT, { .i64 =  4 }, 0, 4,                         VE           },
	{ "webp_sns_strength"     ,"Spatial Noise Shaping. 0=off, 100=maximum.",                OFFSET(user_config.sns_strength),  AV_OPT_TYPE_INT, { .i64 =  50 }, 0, 100,                         VE           },
	{ "webp_filter_strength"     ,"range: [0 = off .. 100 = strongest]",                OFFSET(user_config.filter_strength),  AV_OPT_TYPE_INT, { .i64 =  60 }, 1, 100,                         VE           },
	{ "webp_filter_sharpness"     ,"range: [0 = off .. 7 = least sharp].",                OFFSET(user_config.filter_sharpness),  AV_OPT_TYPE_INT, { .i64 =  0 }, 0, 7,                         VE           },
	{ "webp_filter_type"     ,"filtering type: 0 = simple, 1 = strong (only used if filter_strength > 0 or autofilter > 0)",                OFFSET(user_config.filter_type),  AV_OPT_TYPE_INT, { .i64 =  1 }, 0, 1,                         VE           },
	{ "webp_autofilter"     ,"Auto adjust filter's strength [0 = off, 1 = on]",                OFFSET(user_config.autofilter),  AV_OPT_TYPE_INT, { .i64 =  0 }, 0, 1,                         VE           },
	{ "webp_alpha_compression"     ,"Algorithm for encoding the alpha plane (0 = none, 1 = compressed with WebP lossless). Default is 1.",                OFFSET(user_config.alpha_compression),  AV_OPT_TYPE_INT, { .i64 =  1 }, 0, 1,                         VE           },
	{ "webp_alpha_filtering"     ,"Predictive filtering method for alpha plane. 0: none, 1: fast, 2: best. Default if 1.",                OFFSET(user_config.alpha_filtering),  AV_OPT_TYPE_INT, { .i64 =  1 }, 0, 2,                         VE           },
	{ "webp_alpha_quality"     ,"Between 0 (smallest size) and 100 (lossless). Default is 100.",                OFFSET(user_config.alpha_quality),  AV_OPT_TYPE_INT, { .i64 =  100 }, 0, 100,                         VE           },
	{ "webp_pass"     ,"number of entropy-analysis passes (in [1..10]).",                OFFSET(user_config.pass),  AV_OPT_TYPE_INT, { .i64 =  1 }, 1, 10,                         VE           },
	{ "webp_show_compressed"     ,"if true, export the compressed picture back. In-loop filtering is not applied.",                OFFSET(user_config.show_compressed),  AV_OPT_TYPE_INT, { .i64 =  0 }, 0, 1,                         VE           },
	{ "webp_preprocessing"     ,"preprocessing filter: 0=none, 1=segment-smooth, 2=pseudo-random dithering",                OFFSET(user_config.preprocessing),  AV_OPT_TYPE_INT, { .i64 =  0 }, 0, 2,                         VE           },
	{ "webp_partitions"     ,"log2(number of token partitions) in [0..3]. Default is set to 0 for easier progressive decoding.",                OFFSET(user_config.partitions),  AV_OPT_TYPE_INT, { .i64 =  0 }, 0, 3,                         VE           },
	{ "webp_partition_limit"     ,"quality degradation allowed to fit the 512k limit on prediction modes coding (0: no degradation, 100: maximum possible degradation)",                OFFSET(user_config.partition_limit),  AV_OPT_TYPE_INT, { .i64 =  0 }, 0, 100,                         VE           },
	{ "webp_emulate_jpeg_size"     ,"If true, compression parameters will be remapped to better match the expected output size from JPEG compression. Generally, the output size will be similar but the degradation will be lower.",                OFFSET(user_config.emulate_jpeg_size),  AV_OPT_TYPE_INT, { .i64 =  0 }, 0, 1,                         VE           },
	{ "webp_thread_level"     ,"If non-zero, try and use multi-threaded encoding.",                OFFSET(user_config.thread_level),  AV_OPT_TYPE_INT, { .i64 =  0 }, 0, 1,                       VE           },
	{ "webp_low_memory"     ,"If set, reduce memory usage (but increase CPU use).",                OFFSET(user_config.low_memory),  AV_OPT_TYPE_INT, { .i64 =  0 }, 0, 1,                       VE           },
	{ "webp_near_lossless"     ,"Near lossless encoding [0 = max loss .. 100 = off (default)].",                OFFSET(user_config.near_lossless),  AV_OPT_TYPE_INT, { .i64 =  100 }, 0, 100,                       VE           },
	{ "webp_exact"     ,"if non-zero, preserve the exact RGB values under transparent area. Otherwise, discard this invisible RGB information for better compression. The default value is 0.",                OFFSET(user_config.exact),  AV_OPT_TYPE_INT, { .i64 =  0 }, 0, 1,                       VE           },
	{ "webp_use_sharp_yuv"     ,"if needed, use sharp (and slow) RGB->YUV conversion",                OFFSET(user_config.use_sharp_yuv),  AV_OPT_TYPE_INT, { .i64 =  0 }, 0, 1,                       VE           },
	{ "webp_qmin"     ,"minimum permissible quality factor",                OFFSET(user_config.qmin),  AV_OPT_TYPE_INT, { .i64 =  0 }, 0, INT_MAX,                       VE           },
	{ "webp_qmax"     ,"maximum permissible quality factor",                OFFSET(user_config.qmax),  AV_OPT_TYPE_INT, { .i64 =  100 }, 0, INT_MAX,                       VE           },
    { NULL },
};

const AVClass ff_libwebpenc_class = {
    .class_name = "libwebp encoder",
    .item_name  = av_default_item_name,
    .option     = options,
    .version    = LIBAVUTIL_VERSION_INT,
};

const enum AVPixelFormat ff_libwebpenc_pix_fmts[] = {
    AV_PIX_FMT_RGB32,
    AV_PIX_FMT_YUV420P, AV_PIX_FMT_YUVA420P,
    AV_PIX_FMT_NONE
};

int ff_libwebp_error_to_averror(int err)
{
    switch (err) {
    case VP8_ENC_ERROR_OUT_OF_MEMORY:
    case VP8_ENC_ERROR_BITSTREAM_OUT_OF_MEMORY:
        return AVERROR(ENOMEM);
    case VP8_ENC_ERROR_NULL_PARAMETER:
    case VP8_ENC_ERROR_INVALID_CONFIGURATION:
    case VP8_ENC_ERROR_BAD_DIMENSION:
        return AVERROR(EINVAL);
    }
    return AVERROR_UNKNOWN;
}

av_cold int ff_libwebp_encode_init_common(AVCodecContext *avctx)
{
    LibWebPContextCommon *s = avctx->priv_data;
    int ret;

    if (avctx->global_quality >= 0)
        s->quality = av_clipf(avctx->global_quality / (float)FF_QP2LAMBDA,
                              0.0f, 100.0f);

    if (avctx->compression_level < 0 || avctx->compression_level > 6) {
        av_log(avctx, AV_LOG_WARNING, "invalid compression level: %d\n",
               avctx->compression_level);
        avctx->compression_level = av_clip(avctx->compression_level, 0, 6);
    }

    if (s->preset >= WEBP_PRESET_DEFAULT) {
        ret = WebPConfigPreset(&s->config, s->preset, s->quality);
        if (!ret)
            return AVERROR_UNKNOWN;
        s->lossless              = s->config.lossless;
        s->quality               = s->config.quality;
        avctx->compression_level = s->config.method;
    } else {
        ret = WebPConfigInit(&s->config);
        if (!ret)
            return AVERROR_UNKNOWN;

        s->config = s->user_config;

        ret = WebPValidateConfig(&s->config);
        if (!ret)
            return AVERROR(EINVAL);
    }

    av_log(avctx, AV_LOG_DEBUG, "%s - quality=%.1f method=%d\n",
           s->lossless ? "Lossless" : "Lossy", s->quality,
           avctx->compression_level);

    return 0;
}

int ff_libwebp_get_frame(AVCodecContext *avctx, LibWebPContextCommon *s,
                         const AVFrame *frame, AVFrame **alt_frame_ptr,
                         WebPPicture **pic_ptr) {
    int ret;
    WebPPicture *pic = NULL;
    AVFrame *alt_frame = NULL;

    if (avctx->width > WEBP_MAX_DIMENSION || avctx->height > WEBP_MAX_DIMENSION) {
        av_log(avctx, AV_LOG_ERROR, "Picture size is too large. Max is %dx%d.\n",
               WEBP_MAX_DIMENSION, WEBP_MAX_DIMENSION);
        return AVERROR(EINVAL);
    }

    *pic_ptr = av_malloc(sizeof(*pic));
    pic = *pic_ptr;
    if (!pic)
        return AVERROR(ENOMEM);

    ret = WebPPictureInit(pic);
    if (!ret) {
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    pic->width  = avctx->width;
    pic->height = avctx->height;

    if (avctx->pix_fmt == AV_PIX_FMT_RGB32) {
        if (!s->lossless) {
            /* libwebp will automatically convert RGB input to YUV when
               encoding lossy. */
            if (!s->conversion_warning) {
                av_log(avctx, AV_LOG_WARNING,
                       "Using libwebp for RGB-to-YUV conversion. You may want "
                       "to consider passing in YUV instead for lossy "
                       "encoding.\n");
                s->conversion_warning = 1;
            }
        }
        pic->use_argb    = 1;
        pic->argb        = (uint32_t *)frame->data[0];
        pic->argb_stride = frame->linesize[0] / 4;
    } else {
        if (frame->linesize[1] != frame->linesize[2] || s->cr_threshold) {
            if (!s->chroma_warning && !s->cr_threshold) {
                av_log(avctx, AV_LOG_WARNING,
                       "Copying frame due to differing chroma linesizes.\n");
                s->chroma_warning = 1;
            }
            *alt_frame_ptr = av_frame_alloc();
            alt_frame = *alt_frame_ptr;
            if (!alt_frame) {
                ret = AVERROR(ENOMEM);
                goto end;
            }
            alt_frame->width  = frame->width;
            alt_frame->height = frame->height;
            alt_frame->format = frame->format;
            if (s->cr_threshold)
                alt_frame->format = AV_PIX_FMT_YUVA420P;
            ret = av_frame_get_buffer(alt_frame, 0);
            if (ret < 0)
                goto end;
            alt_frame->format = frame->format;
            av_frame_copy(alt_frame, frame);
            frame = alt_frame;
            if (s->cr_threshold) {
                int x,y, x2, y2, p;
                int bs = s->cr_size;

                if (!s->ref) {
                    s->ref = av_frame_clone(frame);
                    if (!s->ref) {
                        ret = AVERROR(ENOMEM);
                        goto end;
                    }
                }

                alt_frame->format = AV_PIX_FMT_YUVA420P;
                for (y = 0; y < frame->height; y+= bs) {
                    for (x = 0; x < frame->width; x+= bs) {
                        int skip;
                        int sse = 0;
                        for (p = 0; p < 3; p++) {
                            int bs2 = bs >> !!p;
                            int w = AV_CEIL_RSHIFT(frame->width , !!p);
                            int h = AV_CEIL_RSHIFT(frame->height, !!p);
                            int xs = x >> !!p;
                            int ys = y >> !!p;
                            for (y2 = ys; y2 < FFMIN(ys + bs2, h); y2++) {
                                for (x2 = xs; x2 < FFMIN(xs + bs2, w); x2++) {
                                    int diff =  frame->data[p][frame->linesize[p] * y2 + x2]
                                              -s->ref->data[p][frame->linesize[p] * y2 + x2];
                                    sse += diff*diff;
                                }
                            }
                        }
                        skip = sse < s->cr_threshold && frame->data[3] != s->ref->data[3];
                        if (!skip)
                            for (p = 0; p < 3; p++) {
                                int bs2 = bs >> !!p;
                                int w = AV_CEIL_RSHIFT(frame->width , !!p);
                                int h = AV_CEIL_RSHIFT(frame->height, !!p);
                                int xs = x >> !!p;
                                int ys = y >> !!p;
                                for (y2 = ys; y2 < FFMIN(ys + bs2, h); y2++) {
                                    memcpy(&s->ref->data[p][frame->linesize[p] * y2 + xs],
                                            & frame->data[p][frame->linesize[p] * y2 + xs], FFMIN(bs2, w-xs));
                                }
                            }
                        for (y2 = y; y2 < FFMIN(y+bs, frame->height); y2++) {
                            memset(&frame->data[3][frame->linesize[3] * y2 + x],
                                    skip ? 0 : 255,
                                    FFMIN(bs, frame->width-x));
                        }
                    }
                }
            }
        }

        pic->use_argb  = 0;
        pic->y         = frame->data[0];
        pic->u         = frame->data[1];
        pic->v         = frame->data[2];
        pic->y_stride  = frame->linesize[0];
        pic->uv_stride = frame->linesize[1];
        if (frame->format == AV_PIX_FMT_YUVA420P) {
            pic->colorspace = WEBP_YUV420A;
            pic->a          = frame->data[3];
            pic->a_stride   = frame->linesize[3];
            if (alt_frame)
                WebPCleanupTransparentArea(pic);
        } else {
            pic->colorspace = WEBP_YUV420;
        }

        if (s->lossless) {
            /* We do not have a way to automatically prioritize RGB over YUV
               in automatic pixel format conversion based on whether we're
               encoding lossless or lossy, so we do conversion with libwebp as
               a convenience. */
            if (!s->conversion_warning) {
                av_log(avctx, AV_LOG_WARNING,
                       "Using libwebp for YUV-to-RGB conversion. You may want "
                       "to consider passing in RGB instead for lossless "
                       "encoding.\n");
                s->conversion_warning = 1;
            }

#if (WEBP_ENCODER_ABI_VERSION <= 0x201)
            /* libwebp should do the conversion automatically, but there is a
               bug that causes it to return an error instead, so a work-around
               is required.
               See https://code.google.com/p/webp/issues/detail?id=178 */
            pic->memory_ = (void*)1;  /* something non-null */
            ret = WebPPictureYUVAToARGB(pic);
            if (!ret) {
                av_log(avctx, AV_LOG_ERROR,
                       "WebPPictureYUVAToARGB() failed with error: %d\n",
                       pic->error_code);
                ret = libwebp_error_to_averror(pic->error_code);
                goto end;
            }
            pic->memory_ = NULL;  /* restore pointer */
#endif
        }
    }
end:
    return ret;
}
