/* -*- Mode: C; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/*
   Copyright (C) 2012 Red Hat, Inc.

   Red Hat Authors:
   Hans de Goede <hdegoede@redhat.com>

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
#ifndef __SPICE_USB_DEVICE_WIDGET_H__
#define __SPICE_USB_DEVICE_WIDGET_H__

#if !defined(__SPICE_CLIENT_GTK_H_INSIDE__) && !defined(SPICE_COMPILATION)
#warning "Only <spice-client-gtk.h> can be included directly"
#endif

#include <gtk/gtk.h>
#include "spice-client.h"

G_BEGIN_DECLS

#define SPICE_TYPE_USB_DEVICE_WIDGET            (spice_usb_device_widget_get_type ())
#define SPICE_USB_DEVICE_WIDGET(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), SPICE_TYPE_USB_DEVICE_WIDGET, SpiceUsbDeviceWidget))
#define SPICE_USB_DEVICE_WIDGET_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), SPICE_TYPE_USB_DEVICE_WIDGET, SpiceUsbDeviceWidgetClass))
#define SPICE_IS_USB_DEVICE_WIDGET(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), SPICE_TYPE_USB_DEVICE_WIDGET))
#define SPICE_IS_USB_DEVICE_WIDGET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), SPICE_TYPE_USB_DEVICE_WIDGET))
#define SPICE_USB_DEVICE_WIDGET_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), SPICE_TYPE_USB_DEVICE_WIDGET, SpiceUsbDeviceWidgetClass))

typedef struct _SpiceUsbDeviceWidget SpiceUsbDeviceWidget;
typedef struct _SpiceUsbDeviceWidgetClass SpiceUsbDeviceWidgetClass;
typedef struct _SpiceUsbDeviceWidgetPrivate SpiceUsbDeviceWidgetPrivate;

GType spice_usb_device_widget_get_type(void);
GtkWidget *spice_usb_device_widget_new(SpiceSession    *session,
                                       const gchar     *device_format_string);

G_END_DECLS

#endif /* __SPICE_USB_DEVICE_WIDGET_H__ */
