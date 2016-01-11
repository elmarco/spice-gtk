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
#ifndef __SPICE_CLIENT_WIDGET_H__
#define __SPICE_CLIENT_WIDGET_H__

#if !defined(__SPICE_CLIENT_GTK_H_INSIDE__) && !defined(SPICE_COMPILATION)
#warning "Only <spice-client-gtk.h> can be included directly"
#endif

#include "spice-client.h"

#include <gtk/gtk.h>
#include "spice-grabsequence.h"
#include "spice-widget-enums.h"
#include "spice-util.h"
#include "spice-gtk-session.h"

G_BEGIN_DECLS

#define SPICE_TYPE_DISPLAY            (spice_display_get_type())
#define SPICE_DISPLAY(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), SPICE_TYPE_DISPLAY, SpiceDisplay))
#define SPICE_DISPLAY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), SPICE_TYPE_DISPLAY, SpiceDisplayClass))
#define SPICE_IS_DISPLAY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), SPICE_TYPE_DISPLAY))
#define SPICE_IS_DISPLAY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SPICE_TYPE_DISPLAY))
#define SPICE_DISPLAY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), SPICE_TYPE_DISPLAY, SpiceDisplayClass))


typedef struct _SpiceDisplay SpiceDisplay;
typedef struct _SpiceDisplayClass SpiceDisplayClass;

/**
 * SpiceDisplayKeyEvent:
 * @SPICE_DISPLAY_KEY_EVENT_PRESS: key press
 * @SPICE_DISPLAY_KEY_EVENT_RELEASE: key release
 * @SPICE_DISPLAY_KEY_EVENT_CLICK: key click (press and release)
 *
 * Constants for key events.
 */
typedef enum
{
	SPICE_DISPLAY_KEY_EVENT_PRESS = 1,
	SPICE_DISPLAY_KEY_EVENT_RELEASE = 2,
	SPICE_DISPLAY_KEY_EVENT_CLICK = 3,
} SpiceDisplayKeyEvent;

GType	        spice_display_get_type(void);

SpiceDisplay* spice_display_new(SpiceSession *session, int channel_id);
SpiceDisplay* spice_display_new_with_monitor(SpiceSession *session, gint channel_id, gint monitor_id);

void spice_display_mouse_ungrab(SpiceDisplay *display);
void spice_display_set_grab_keys(SpiceDisplay *display, SpiceGrabSequence *seq);
SpiceGrabSequence *spice_display_get_grab_keys(SpiceDisplay *display);
void spice_display_send_keys(SpiceDisplay *display, const guint *keyvals,
                             int nkeyvals, SpiceDisplayKeyEvent kind);
GdkPixbuf *spice_display_get_pixbuf(SpiceDisplay *display);

G_END_DECLS

#endif /* __SPICE_CLIENT_WIDGET_H__ */
