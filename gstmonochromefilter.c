/* GStreamer
 * Copyright (C) 2017 FIXME <fixme@example.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
/**
 * SECTION:element-gstmonochromefilter
 *
 * The monochromefilter element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! monochromefilter ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>
#include "gstmonochromefilter.h"

GST_DEBUG_CATEGORY_STATIC (gst_monochrome_filter_debug_category);
#define GST_CAT_DEFAULT gst_monochrome_filter_debug_category

#define DEFAULT_FILTER_MODE GRAY_FILTER
#define TYPE_FILTER_MODE (filter_mode_get_type ())

/* prototypes */


static void gst_monochrome_filter_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_monochrome_filter_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);
static void gst_monochrome_filter_dispose (GObject * object);
static void gst_monochrome_filter_finalize (GObject * object);

static gboolean gst_monochrome_filter_start (GstBaseTransform * trans);
static gboolean gst_monochrome_filter_stop (GstBaseTransform * trans);
static gboolean gst_monochrome_filter_set_info (GstVideoFilter * filter,
    GstCaps * incaps, GstVideoInfo * in_info, GstCaps * outcaps,
    GstVideoInfo * out_info);
/*static GstFlowReturn gst_monochrome_filter_transform_frame (GstVideoFilter *
    filter, GstVideoFrame * inframe, GstVideoFrame * outframe);*/
static GstFlowReturn gst_monochrome_filter_transform_frame_ip (GstVideoFilter *
    filter, GstVideoFrame * frame);




enum
{
  PROP_0,
  PROP_FILTER_MODE,
};

/* pad templates */

/* FIXME: add/remove formats you can handle */
#define VIDEO_SRC_CAPS \
    GST_VIDEO_CAPS_MAKE("{ RGB, RGBx, xRGB, ARGB, RGBA }")

/* FIXME: add/remove formats you can handle */
#define VIDEO_SINK_CAPS \
    GST_VIDEO_CAPS_MAKE("{ RGB, RGBx, xRGB, ARGB, RGBA }")

static GType
filter_mode_get_type (void)
{
  static GType filter_mode = 0;

  if (!filter_mode) {
    static const GEnumValue modes[] = {
      {GRAY_FILTER, "Grayscale image", "gray"},
      {RED_FILTER,  "Red-tone image", "red"},
      {GREEN_FILTER,  "Green-tone image", "green"},
      {BLUE_FILTER,  "Blue-tone image", "blue"},

      {0, NULL, NULL}
    };

    filter_mode = g_enum_register_static ("Filter_Mode", modes);
  }

  return filter_mode;
}


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstMonochromeFilter, gst_monochrome_filter,
    GST_TYPE_VIDEO_FILTER,
    GST_DEBUG_CATEGORY_INIT (gst_monochrome_filter_debug_category,
        "monochromefilter", 0, "debug category for monochromefilter element"));


static void
gst_monochrome_filter_class_init (GstMonochromeFilterClass * klass)
{



  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS (klass);
  GstVideoFilterClass *video_filter_class = GST_VIDEO_FILTER_CLASS (klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
      gst_pad_template_new ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
        gst_caps_from_string (VIDEO_SRC_CAPS)));
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS(klass),
      gst_pad_template_new ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
        gst_caps_from_string (VIDEO_SINK_CAPS)));

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS (klass),
      "Monochrome filter", "Generic", "Apply a monochrome filter to a video source",
      "Obi wan Kenobi <fakeupdate.com>");

  gobject_class->set_property = gst_monochrome_filter_set_property;
  gobject_class->get_property = gst_monochrome_filter_get_property;
  gobject_class->dispose = gst_monochrome_filter_dispose;
  gobject_class->finalize = gst_monochrome_filter_finalize;
  base_transform_class->start = GST_DEBUG_FUNCPTR (gst_monochrome_filter_start);
  base_transform_class->stop = GST_DEBUG_FUNCPTR (gst_monochrome_filter_stop);
  video_filter_class->set_info =
      GST_DEBUG_FUNCPTR (gst_monochrome_filter_set_info);
  video_filter_class->transform_frame_ip =
      GST_DEBUG_FUNCPTR (gst_monochrome_filter_transform_frame_ip);


g_object_class_install_property (gobject_class, PROP_FILTER_MODE,
    g_param_spec_enum ("filter", "Video filter",
    "Color filter to apply",
    TYPE_FILTER_MODE, DEFAULT_FILTER_MODE,
    G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

}

static void
gst_monochrome_filter_init (GstMonochromeFilter * monochromefilter)
{
}


void
gst_monochrome_filter_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstMonochromeFilter *monochromefilter = GST_MONOCHROME_FILTER (object);

  GST_DEBUG_OBJECT (monochromefilter, "set_property");

  switch (property_id) {
    case PROP_FILTER_MODE:
      monochromefilter->filter_mode = g_value_get_enum (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_monochrome_filter_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstMonochromeFilter *monochromefilter = GST_MONOCHROME_FILTER (object);

  GST_DEBUG_OBJECT (monochromefilter, "get_property");

  switch (property_id) {
    case PROP_FILTER_MODE:
      g_value_set_enum (value, monochromefilter->filter_mode);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_monochrome_filter_dispose (GObject * object)
{
  GstMonochromeFilter *monochromefilter = GST_MONOCHROME_FILTER (object);

  GST_DEBUG_OBJECT (monochromefilter, "dispose");

  /* clean up as possible.  may be called multiple times */

  G_OBJECT_CLASS (gst_monochrome_filter_parent_class)->dispose (object);
}

void
gst_monochrome_filter_finalize (GObject * object)
{
  GstMonochromeFilter *monochromefilter = GST_MONOCHROME_FILTER (object);

  GST_DEBUG_OBJECT (monochromefilter, "finalize");

  /* clean up object here */

  G_OBJECT_CLASS (gst_monochrome_filter_parent_class)->finalize (object);
}

static gboolean
gst_monochrome_filter_start (GstBaseTransform * trans)
{
  GstMonochromeFilter *monochromefilter = GST_MONOCHROME_FILTER (trans);

  GST_DEBUG_OBJECT (monochromefilter, "start");

  return TRUE;
}

static gboolean
gst_monochrome_filter_stop (GstBaseTransform * trans)
{
  GstMonochromeFilter *monochromefilter = GST_MONOCHROME_FILTER (trans);

  GST_DEBUG_OBJECT (monochromefilter, "stop");

  return TRUE;
}

static gboolean
gst_monochrome_filter_set_info (GstVideoFilter * filter, GstCaps * incaps,
    GstVideoInfo * in_info, GstCaps * outcaps, GstVideoInfo * out_info)
{
  GstMonochromeFilter *monochromefilter = GST_MONOCHROME_FILTER (filter);

  GST_DEBUG_OBJECT (monochromefilter, "set_info");

  return TRUE;
}

/* transform */
/*static GstFlowReturn
gst_monochrome_filter_transform_frame (GstVideoFilter * filter,
    GstVideoFrame * inframe, GstVideoFrame * outframe)
{
  GstMonochromeFilter *monochromefilter = GST_MONOCHROME_FILTER (filter);

  GST_DEBUG_OBJECT (monochromefilter, "transform_frame");

  return GST_FLOW_OK;
}*/

static GstFlowReturn
gst_monochrome_filter_transform_frame_ip (GstVideoFilter * filter, GstVideoFrame * frame)
{
  GstMonochromeFilter *monochromefilter = GST_MONOCHROME_FILTER (filter);

  GST_DEBUG_OBJECT (monochromefilter, "transform_frame_ip");

  gint r, g, b, gray;

  gint width, height;
  gint i, j;
  gint row_padding;

  guint8 rgb_offsets[3];
  rgb_offsets[0] = GST_VIDEO_FRAME_COMP_OFFSET (frame, GST_VIDEO_COMP_R);
  rgb_offsets[1] = GST_VIDEO_FRAME_COMP_OFFSET (frame, GST_VIDEO_COMP_G);
  rgb_offsets[2] = GST_VIDEO_FRAME_COMP_OFFSET (frame, GST_VIDEO_COMP_B);

  guint8 *data;

  width = GST_VIDEO_FRAME_WIDTH(frame);
  height = GST_VIDEO_FRAME_HEIGHT(frame);

  data = GST_VIDEO_FRAME_PLANE_DATA(frame, 0);

  row_padding = GST_VIDEO_FRAME_PLANE_STRIDE(frame, 0) - width * GST_VIDEO_FRAME_COMP_PSTRIDE(frame, 0);

  for (i = 0; i < height; ++i) {
    for (j = 0; j < width; ++j) {
      r = data[rgb_offsets[0]];
      g = data[rgb_offsets[1]];
      b = data[rgb_offsets[2]];

      if (monochromefilter->filter_mode == GRAY_FILTER) {
        gray = (r + g + b) / 3;

        data[rgb_offsets[0]] = gray;
        data[rgb_offsets[1]] = gray;
        data[rgb_offsets[2]] = gray;

      } else if (monochromefilter->filter_mode == RED_FILTER) {
        data[rgb_offsets[0]] = r;
        data[rgb_offsets[1]] = 0;
        data[rgb_offsets[2]] = 0;

      } else if (monochromefilter->filter_mode == GREEN_FILTER) {
        data[rgb_offsets[0]] = 0;
        data[rgb_offsets[1]] = g;
        data[rgb_offsets[2]] = 0;

      } else if (monochromefilter->filter_mode == BLUE_FILTER) {
        data[rgb_offsets[0]] = 0;
        data[rgb_offsets[1]] = 0;
        data[rgb_offsets[2]] = b;

      } else {
        GST_WARNING_OBJECT (monochromefilter, "unknown video filter mode, no transformation is done");
      }

      data += GST_VIDEO_FRAME_COMP_PSTRIDE(frame, 0);
    }

    data += row_padding;
  }

  return GST_FLOW_OK;
}
