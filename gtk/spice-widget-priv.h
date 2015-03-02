/* -*- Mode: C; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
   Copyright (C) 2010 Red Hat, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __SPICE_WIDGET_PRIV_H__
#define __SPICE_WIDGET_PRIV_H__

G_BEGIN_DECLS

#include "config.h"

#ifdef WITH_X11
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <gdk/gdkx.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif

#ifdef USE_EGL
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#endif

#include "spice-widget.h"
#include "spice-common.h"
#include "spice-gtk-session.h"
#include "channel-virgl.h"

#define SPICE_DISPLAY_GET_PRIVATE(obj)                                  \
    (G_TYPE_INSTANCE_GET_PRIVATE((obj), SPICE_TYPE_DISPLAY, SpiceDisplayPrivate))

struct _SpiceDisplayPrivate {
    gint                    channel_id;
    gint                    monitor_id;

    /* options */
    bool                    keyboard_grab_enable;
    gboolean                keyboard_grab_inhibit;
    bool                    mouse_grab_enable;
    bool                    resize_guest_enable;

    /* state */
    gboolean                ready;
    gboolean                monitor_ready;
    enum SpiceSurfaceFmt    format;
    gint                    width, height, stride;
    gint                    shmid;
    gpointer                data_origin; /* the original display image data */
    gpointer                data; /* converted if necessary to 32 bits */

    GdkRectangle            area;
    /* window border */
    gint                    ww, wh, mx, my;

    bool                    convert;
    bool                    have_mitshm;
    gboolean                allow_scaling;
    gboolean                only_downscale;
    gboolean                disable_inputs;

    /* TODO: make a display object instead? */
#ifdef WITH_X11
    Display                 *dpy;
    XVisualInfo             *vi;
    XImage                  *ximage;
    XShmSegmentInfo         *shminfo;
    GC                      gc;
#else
    cairo_surface_t         *ximage;
#endif

    SpiceSession            *session;
    SpiceGtkSession         *gtk_session;
    SpiceMainChannel        *main;
    SpiceChannel            *display;
    SpiceCursorChannel      *cursor;
    SpiceInputsChannel      *inputs;
    SpiceSmartcardChannel   *smartcard;

    enum SpiceMouseMode     mouse_mode;
    int                     mouse_grab_active;
    bool                    mouse_have_pointer;
    GdkCursor               *mouse_cursor;
    GdkPixbuf               *mouse_pixbuf;
    GdkPoint                mouse_hotspot;
    GdkCursor               *show_cursor;
    int                     mouse_last_x;
    int                     mouse_last_y;
    int                     mouse_guest_x;
    int                     mouse_guest_y;

    bool                    keyboard_grab_active;
    bool                    keyboard_have_focus;

    const guint16          *keycode_map;
    size_t                  keycode_maplen;
    uint32_t                key_state[512 / 32];
    int                     key_delayed_scancode;
    guint                   key_delayed_id;
    SpiceGrabSequence         *grabseq; /* the configured key sequence */
    gboolean                *activeseq; /* the currently pressed keys */
    gboolean                seq_pressed;
    gboolean                keyboard_grab_released;
    gint                    mark;
#ifdef WIN32
    HHOOK                   keyboard_hook;
    int                     win_mouse[3];
    int                     win_mouse_speed;
#endif
    guint                   keypress_delay;
    gint                    zoom_level;
#ifdef GDK_WINDOWING_X11
    int                     x11_accel_numerator;
    int                     x11_accel_denominator;
    int                     x11_threshold;
#endif
#ifdef USE_EGL
    struct {
        gboolean            enabled;
        EGLSurface          surface;
        EGLDisplay          display;
        EGLConfig           conf;
        EGLContext          ctx;
        guint32             tex_loc;
        guint32             mproj;
        guint32             attr_pos, attr_tex;
        guint32             vbuf_id;
        guint               tex_id;
        EGLImageKHR         image;
        SpiceVirglScanout   scanout;
    } egl;
#endif
    SpiceVirglChannel      *virgl;
};

int      spicex_image_create                 (SpiceDisplay *display);
void     spicex_image_destroy                (SpiceDisplay *display);
#if GTK_CHECK_VERSION (2, 91, 0)
void     spicex_draw_event                   (SpiceDisplay *display, cairo_t *cr);
#else
void     spicex_expose_event                 (SpiceDisplay *display, GdkEventExpose *ev);
#endif
gboolean spicex_is_scaled                    (SpiceDisplay *display);
void     spice_display_get_scaling           (SpiceDisplay *display, double *s, int *x, int *y, int *w, int *h);
int      spice_egl_init                      (SpiceDisplay *display);
int      spice_egl_realize_display           (SpiceDisplay *display);
void     spice_egl_unrealize_display         (SpiceDisplay *display);
void     spice_egl_update_display            (SpiceDisplay *display);
void     spice_egl_resize_display            (SpiceDisplay *display, int w, int h);
int      spice_egl_update_scanout            (SpiceDisplay *display,
                                              const SpiceVirglScanout *scanout);

G_END_DECLS

#endif
