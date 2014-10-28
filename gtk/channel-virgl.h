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
#ifndef __SPICE_CLIENT_VIRGL_CHANNEL_H__
#define __SPICE_CLIENT_VIRGL_CHANNEL_H__

#include <gio/gio.h>
#include "spice-client.h"

G_BEGIN_DECLS

#define SPICE_TYPE_VIRGL_CHANNEL            (spice_virgl_channel_get_type())
#define SPICE_VIRGL_CHANNEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), SPICE_TYPE_VIRGL_CHANNEL, SpiceVirglChannel))
#define SPICE_VIRGL_CHANNEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), SPICE_TYPE_VIRGL_CHANNEL, SpiceVirglChannelClass))
#define SPICE_IS_VIRGL_CHANNEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), SPICE_TYPE_VIRGL_CHANNEL))
#define SPICE_IS_VIRGL_CHANNEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SPICE_TYPE_VIRGL_CHANNEL))
#define SPICE_VIRGL_CHANNEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), SPICE_TYPE_VIRGL_CHANNEL, SpiceVirglChannelClass))

typedef struct _SpiceVirglChannel SpiceVirglChannel;
typedef struct _SpiceVirglChannelClass SpiceVirglChannelClass;
typedef struct _SpiceVirglChannelPrivate SpiceVirglChannelPrivate;

#define SPICE_TYPE_VIRGL_SCANOUT (spice_virgl_scanout_get_type ())

typedef struct _SpiceVirglScanout SpiceVirglScanout;

struct _SpiceVirglScanout {
    gint fd;
    guint32 width;
    guint32 height;
    guint32 stride;
    guint32 format;
    guint32 x;
    guint32 y;
    guint32 w;
    guint32 h;
    gboolean y0top;
};

/**
 * SpiceVirglChannel:
 *
 * The #SpiceVirglChannel struct is opaque and should not be accessed directly.
 */
struct _SpiceVirglChannel {
    SpiceChannel parent;

    /*< private >*/
    SpiceVirglChannelPrivate *priv;
    /* Do not add fields to this struct */
};

/**
 * SpiceVirglChannelClass:
 * @parent_class: Parent class.
 *
 * Class structure for #SpiceVirglChannel.
 */
struct _SpiceVirglChannelClass {
    SpiceChannelClass parent_class;

    /*< private >*/
    /* Do not add fields to this struct */
};

GType                     spice_virgl_channel_get_type     (void) G_GNUC_CONST;
const SpiceVirglScanout*  spice_virgl_channel_get_scanout  (SpiceVirglChannel *channel);

GType                     spice_virgl_scanout_get_type     (void) G_GNUC_CONST;
void                      spice_virgl_scanout_free         (SpiceVirglScanout *scanout);

G_END_DECLS

#endif /* __SPICE_CLIENT_VIRGL_CHANNEL_H__ */
