/* -*- Mode: C; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
   Copyright (C) 2014 Red Hat, Inc.

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
#include "config.h"

#include <math.h>

#define EGL_EGLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES

#include "spice-widget.h"
#include "spice-widget-priv.h"
#include <libdrm/drm_fourcc.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

static const char *spice_egl_vertex_src =       \
"                                               \
  attribute vec4        position;               \
  attribute vec2        texcoords;              \
  varying vec2          tcoords;                \
  uniform mat4          mproj;                  \
                                                \
  void main()                                   \
  {                                             \
    tcoords = texcoords;                        \
    gl_Position = mproj * position;             \
  }                                             \
";

static const char *spice_egl_fragment_src =     \
"                                               \
  varying highp vec2    tcoords;                \
  uniform sampler2D     samp;                   \
                                                \
  void  main()                                  \
  {                                             \
    gl_FragColor = texture2D(samp, tcoords);    \
  }                                             \
";

static void apply_ortho(guint mproj, float left, float right,
                        float bottom, float top, float near, float far)

{
    float a = 2.0f / (right - left);
    float b = 2.0f / (top - bottom);
    float c = -2.0f / (far - near);

    float tx = - (right + left) / (right - left);
    float ty = - (top + bottom) / (top - bottom);
    float tz = - (far + near) / (far - near);

    float ortho[16] = {
        a, 0, 0, 0,
        0, b, 0, 0,
        0, 0, c, 0,
        tx, ty, tz, 1
    };

    glUniformMatrix4fv(mproj, 1, GL_FALSE, &ortho[0]);
}

static int spice_egl_init_shaders(SpiceDisplay *display)
{
    SpiceDisplayPrivate *d = SPICE_DISPLAY_GET_PRIVATE(display);
    GLuint fs, vs, prog;
    GLint stat;

    fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, (const char **)&spice_egl_fragment_src, NULL);
    glCompileShader(fs);
    glGetShaderiv(fs, GL_COMPILE_STATUS, &stat);
    if (!stat) {
        g_critical("Failed to compile FS");
        return -1;
    }

    vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, (const char **)&spice_egl_vertex_src, NULL);
    glCompileShader(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &stat);
    if (!stat) {
        g_critical("failed to compile VS");
        return -1;
    }

    prog = glCreateProgram();
    glAttachShader(prog, fs);
    glAttachShader(prog, vs);
    glLinkProgram(prog);

    glGetProgramiv(prog, GL_LINK_STATUS, &stat);
    if (!stat) {
        char log[1000];
        GLsizei len;
        glGetProgramInfoLog(prog, 1000, &len, log);
        g_critical("Error linking: %s", log);
        return -1;
    }

    glUseProgram(prog);

    d->egl.attr_pos = glGetAttribLocation(prog, "position");
    d->egl.attr_tex = glGetAttribLocation(prog, "texcoords");
    d->egl.tex_loc = glGetUniformLocation(prog, "samp");
    d->egl.mproj = glGetUniformLocation(prog, "mproj");

    glUniform1i(d->egl.tex_loc, 0);

    glGenBuffers(1, &d->egl.vbuf_id);
    glBindBuffer(GL_ARRAY_BUFFER, d->egl.vbuf_id);
    glBufferData(GL_ARRAY_BUFFER,
                 (sizeof(GLfloat) * 4 * 4) +
                 (sizeof(GLfloat) * 4 * 2),
                 NULL,
                 GL_STATIC_DRAW);

    return 0;
}

int spice_egl_init(SpiceDisplay *display)
{
    SpiceDisplayPrivate *d = SPICE_DISPLAY_GET_PRIVATE(display);
    Display *dpy;
    static const EGLint conf_att[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 0,
        EGL_NONE,
    };
    static const EGLint ctx_att[] = {
#ifdef EGL_CONTEXT_MAJOR_VERSION
        EGL_CONTEXT_MAJOR_VERSION, 3,
        EGL_CONTEXT_MINOR_VERSION, 1,
#else
        EGL_CONTEXT_CLIENT_VERSION, 3,
#endif
        EGL_NONE
    };
    EGLBoolean b;
    EGLenum api;
    EGLint major, minor, n;

    dpy = gdk_x11_get_default_xdisplay();
    d->egl.display = eglGetDisplay((EGLNativeDisplayType)dpy);
    if (d->egl.display == EGL_NO_DISPLAY) {
        g_critical("Failed to get EGL display");
        return -1;
    }

    if (!eglInitialize(d->egl.display, &major, &minor)) {
        g_critical("Failed to init EGL display");
        return -1;
    }
#if 0
    fprintf(stderr, "EGL major/minor: %d.%d\n", major, minor);
    fprintf(stderr, "EGL version: %s\n",
            eglQueryString(d->egl.display, EGL_VERSION));
    fprintf(stderr, "EGL vendor: %s\n",
            eglQueryString(d->egl.display, EGL_VENDOR));
    fprintf(stderr, "EGL extensions: %s\n",
            eglQueryString(d->egl.display, EGL_EXTENSIONS));
#endif
    api = EGL_OPENGL_ES_API;
    b = eglBindAPI(api);
    if (!b) {
        g_critical("cannot bind OpenGLES API");
        return -1;
    }

    b = eglChooseConfig(d->egl.display, conf_att, &d->egl.conf,
                        1, &n);

    if (!b || n != 1) {
        g_critical("cannot find suitable EGL config");
        return -1;
    }

    d->egl.ctx = eglCreateContext(d->egl.display,
                                  d->egl.conf,
                                  EGL_NO_CONTEXT,
                                  ctx_att);
    if (!d->egl.ctx) {
        g_critical("cannot create EGL context");
        return -1;
    }

    eglMakeCurrent(d->egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                   EGL_NO_CONTEXT);

    return 0;
}

static int spice_widget_init_egl_win(SpiceDisplay *display)
{
    GtkWidget *widget = GTK_WIDGET(display);
    SpiceDisplayPrivate *d = SPICE_DISPLAY_GET_PRIVATE(display);
    GdkWindow *gdk_win;
    Window xwin;
    EGLBoolean b;

    if (d->egl.surface)
        return 0;

    gdk_win = gtk_widget_get_window(widget);
    if (!gdk_win)
        return -1;
    xwin = gdk_x11_window_get_xid(gdk_win);

    d->egl.surface = eglCreateWindowSurface(d->egl.display,
                                            d->egl.conf,
                                            (EGLNativeWindowType)xwin, NULL);

    if (!d->egl.surface) {
        g_critical("failed to init egl surface\n");
        return -1;
    }

    b = eglMakeCurrent(d->egl.display,
                       d->egl.surface,
                       d->egl.surface,
                       d->egl.ctx);
    if (!b) {
        g_critical("failed to activate context\n");
        return -1;
    }

    return 0;
}

#if GTK_CHECK_VERSION (2, 91, 0)
static inline void gdk_drawable_get_size(GdkWindow *w, gint *ww, gint *wh)
{
    *ww = gdk_window_get_width(w);
    *wh = gdk_window_get_height(w);
}
#endif

int spice_egl_realize_display(SpiceDisplay *display)
{
    int ret;
    int ww, wh;

    SPICE_DEBUG("egl realize");
    ret = spice_widget_init_egl_win(display);
    if (ret)
        return ret;
    ret = spice_egl_init_shaders(display);
    if (ret)
        return ret;

    gdk_drawable_get_size(gtk_widget_get_window(GTK_WIDGET(display)), &ww, &wh);
    spice_egl_resize_display(display, ww, wh);

    return 0;
}

void spice_egl_unrealize_display(SpiceDisplay *display)
{
    SpiceDisplayPrivate *d = SPICE_DISPLAY_GET_PRIVATE(display);

    SPICE_DEBUG("egl unrealize %p", d->egl.surface);

    eglMakeCurrent(d->egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                   EGL_NO_CONTEXT);

    if (d->egl.surface != EGL_NO_SURFACE) {
        eglDestroySurface(d->egl.display, d->egl.surface);
        d->egl.surface = EGL_NO_SURFACE;
    }
}

void spice_egl_resize_display(SpiceDisplay *display, int w, int h)
{
    SpiceDisplayPrivate *d = SPICE_DISPLAY_GET_PRIVATE(display);

    apply_ortho(d->egl.mproj, 0, w, 0, h, -1, 1);
    glViewport(0, 0, w, h);

    spice_egl_update_display(display);
}

static void
draw_rect_from_arrays(SpiceDisplay *display, const void *verts, const void *tex)
{
    SpiceDisplayPrivate *d = SPICE_DISPLAY_GET_PRIVATE(display);

    glBindBuffer(GL_ARRAY_BUFFER, d->egl.vbuf_id);

    if (verts) {
        glBufferSubData(GL_ARRAY_BUFFER,
                        0,
                        sizeof(GLfloat) * 4 * 4,
                        verts);
        glVertexAttribPointer(d->egl.attr_pos, 4, GL_FLOAT,
                              GL_FALSE, 0, 0);
        glEnableVertexAttribArray(d->egl.attr_pos);
    }
    if (tex) {
        glBufferSubData(GL_ARRAY_BUFFER,
                        sizeof(GLfloat) * 4 * 4,
                        sizeof(GLfloat) * 4 * 2,
                        tex);
        glVertexAttribPointer(d->egl.attr_tex, 2, GL_FLOAT,
                              GL_FALSE, 0,
                              (void *)(sizeof(GLfloat) * 4 * 4));
        glEnableVertexAttribArray(d->egl.attr_tex);
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    if (verts)
        glDisableVertexAttribArray(d->egl.attr_pos);
    if (tex)
        glDisableVertexAttribArray(d->egl.attr_tex);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static GLvoid
client_draw_rect_tex(SpiceDisplay *display,
                     float x, float y, float w, float h,
                     float tx, float ty, float tw, float th)
{
    float verts[4][4];
    float tex[4][2];

    verts[0][0] = x;
    verts[0][1] = y;
    verts[0][2] = 0.0;
    verts[0][3] = 1.0;
    tex[0][0] = tx;
    tex[0][1] = ty;
    verts[1][0] = x + w;
    verts[1][1] = y;
    verts[1][2] = 0.0;
    verts[1][3] = 1.0;
    tex[1][0] = tx + tw;
    tex[1][1] = ty;
    verts[2][0] = x;
    verts[2][1] = y + h;
    verts[2][2] = 0.0;
    verts[2][3] = 1.0;
    tex[2][0] = tx;
    tex[2][1] = ty + th;
    verts[3][0] = x + w;
    verts[3][1] = y + h;
    verts[3][2] = 0.0;
    verts[3][3] = 1.0;
    tex[3][0] = tx + tw;
    tex[3][1] = ty + th;

    draw_rect_from_arrays(display, verts, tex);
}

G_GNUC_INTERNAL
void spice_egl_update_display(SpiceDisplay *display)
{
    SpiceDisplayPrivate *d = SPICE_DISPLAY_GET_PRIVATE(display);
    double s;
    int x, y, w, h;
    gdouble tx, ty, tw, th;

    g_return_if_fail(d->egl.image != NULL);
    g_return_if_fail(d->egl.surface != NULL);

    spice_display_get_scaling(display, &s, &x, &y, &w, &h);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    /* FIXME: bad floating arithmetic? */
    tx = ((float)d->egl.scanout.x / (float)d->egl.scanout.width);
    ty = ((float)d->egl.scanout.y / (float)d->egl.scanout.height);
    tw = ((float)d->egl.scanout.w / (float)d->egl.scanout.width);
    th = ((float)d->egl.scanout.h / (float)d->egl.scanout.height);
    ty += 1 - th;
    if (!d->egl.scanout.y0top) {
        ty = 1 - ty;
        th = -1 * th;
    }
    SPICE_DEBUG("update %f +%d+%d %dx%d +%f+%f %fx%f", s, x, y, w, h,
                tx, ty, tw, th);

    client_draw_rect_tex(display, x, y, w, h,
                         tx, ty, tw, th);
    eglSwapBuffers(d->egl.display, d->egl.surface);
}


G_GNUC_INTERNAL
int spice_egl_update_scanout(SpiceDisplay *display,
                             const SpiceVirglScanout *scanout)
{
    SpiceDisplayPrivate *d = SPICE_DISPLAY_GET_PRIVATE(display);
    EGLint attrs[13];
    guint32 format;

    g_return_val_if_fail(scanout != NULL, -1);
    format = scanout->format;

    if (scanout->fd == -1)
        return 0;

    if (d->egl.image != NULL) {
        eglDestroyImageKHR(d->egl.display, d->egl.image);
        d->egl.image = NULL;
    }

    attrs[0] = EGL_DMA_BUF_PLANE0_FD_EXT;
    attrs[1] = scanout->fd;
    attrs[2] = EGL_DMA_BUF_PLANE0_PITCH_EXT;
    attrs[3] = scanout->stride;
    attrs[4] = EGL_DMA_BUF_PLANE0_OFFSET_EXT;
    attrs[5] = 0;
    attrs[6] = EGL_WIDTH;
    attrs[7] = scanout->width;
    attrs[8] = EGL_HEIGHT;
    attrs[9] = scanout->height;
    attrs[10] = EGL_LINUX_DRM_FOURCC_EXT;
    attrs[11] = format;
    attrs[12] = EGL_NONE;
    SPICE_DEBUG("fd:%d stride:%d y0:%d %dx%d format:0x%x (%c%c%c%c)",
                scanout->fd, scanout->stride, scanout->y0top,
                scanout->width, scanout->height, format,
                format & 0xff, (format >> 8) & 0xff, (format >> 16) & 0xff, format >> 24);

    d->egl.image = eglCreateImageKHR(d->egl.display,
                                       EGL_NO_CONTEXT,
                                       EGL_LINUX_DMA_BUF_EXT,
                                       (EGLClientBuffer)NULL,
                                       attrs);
    glGenTextures(1, &d->egl.tex_id);
    glBindTexture(GL_TEXTURE_2D, d->egl.tex_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)d->egl.image);
    d->egl.scanout = *scanout;

    return 0;
}
