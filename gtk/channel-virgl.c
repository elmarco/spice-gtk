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

#include "spice-client.h"
#include "spice-common.h"
#include "spice-channel-priv.h"
#include "spice-marshal.h"
#include "glib-compat.h"

/**
 * SECTION:channel-virgl
 * @short_description: virgl/3d display channel
 * @title: Virgl Channel
 * @section_id:
 * @see_also: #SpiceChannel
 * @stability: Stable
 * @include: channel-virgl.h

 * Since: FIXME
 */

#define SPICE_VIRGL_CHANNEL_GET_PRIVATE(obj)                             \
    (G_TYPE_INSTANCE_GET_PRIVATE((obj), SPICE_TYPE_VIRGL_CHANNEL, SpiceVirglChannelPrivate))

struct _SpiceVirglChannelPrivate {
    SpiceVirglScanout scanout;
};

G_DEFINE_TYPE(SpiceVirglChannel, spice_virgl_channel, SPICE_TYPE_CHANNEL)

/* Properties */
enum {
    PROP_0,
    PROP_SCANOUT,
};

/* Signals */
enum {
    SIGNAL_VIRGL_UPDATE,
    SIGNAL_LAST,
};

static guint signals[SIGNAL_LAST];
static void channel_set_handlers(SpiceChannelClass *klass);
static SpiceVirglScanout* spice_virgl_scanout_copy(const SpiceVirglScanout *scanout);

G_DEFINE_BOXED_TYPE(SpiceVirglScanout, spice_virgl_scanout,
                    (GBoxedCopyFunc)spice_virgl_scanout_copy,
                    (GBoxedFreeFunc)spice_virgl_scanout_free)

static void spice_virgl_channel_init(SpiceVirglChannel *channel)
{
    SpiceVirglChannelPrivate *c = SPICE_VIRGL_CHANNEL_GET_PRIVATE(channel);

    channel->priv = c;
    c->scanout.fd = -1;
}

const SpiceVirglScanout*
spice_virgl_channel_get_scanout(SpiceVirglChannel *channel)
{
    g_return_val_if_fail(SPICE_IS_VIRGL_CHANNEL(channel), NULL);

    return &channel->priv->scanout;
}

static void spice_virgl_get_property(GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
    SpiceVirglChannel *channel = SPICE_VIRGL_CHANNEL(object);

    switch (prop_id) {
    case PROP_SCANOUT:
        g_value_set_boxed(value, spice_virgl_channel_get_scanout(channel));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void spice_virgl_channel_finalize(GObject *object)
{
    SpiceVirglChannelPrivate *c = SPICE_VIRGL_CHANNEL(object)->priv;

    if (c->scanout.fd >= 0)
        close(c->scanout.fd);

    if (G_OBJECT_CLASS(spice_virgl_channel_parent_class)->finalize)
        G_OBJECT_CLASS(spice_virgl_channel_parent_class)->finalize(object);
}

static void spice_virgl_channel_reset(SpiceChannel *channel, gboolean migrating)
{
    /* SpiceVirglChannelPrivate *c = SPICE_VIRGL_CHANNEL(channel)->priv; */

    SPICE_CHANNEL_CLASS(spice_virgl_channel_parent_class)->channel_reset(channel, migrating);
}

static void spice_virgl_channel_class_init(SpiceVirglChannelClass *klass)
{
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    SpiceChannelClass *channel_class = SPICE_CHANNEL_CLASS(klass);

    gobject_class->finalize     = spice_virgl_channel_finalize;
    gobject_class->get_property = spice_virgl_get_property;
    channel_class->channel_reset = spice_virgl_channel_reset;

    g_object_class_install_property
        (gobject_class, PROP_SCANOUT,
         g_param_spec_boxed("scanout",
                            "Virgl scanout",
                            "Virgl scanout",
                            SPICE_TYPE_VIRGL_SCANOUT,
                            G_PARAM_READABLE |
                            G_PARAM_STATIC_STRINGS));

    /**
     * SpiceVirglChannel::update:
     * @virgl: the #SpiceVirglChannel that emitted the signal
     * @x: x position
     * @y: y position
     * @width: width
     * @height: height
     *
     * The #SpiceVirglChannel::update signal is emitted
     * when the rectangular region x/y/w/h of the primary buffer is
     * updated.
     **/
    signals[SIGNAL_VIRGL_UPDATE] =
        g_signal_new("update",
                     G_OBJECT_CLASS_TYPE(gobject_class),
                     0, 0, NULL, NULL,
                     g_cclosure_user_marshal_VOID__UINT_UINT_UINT_UINT,
                     G_TYPE_NONE,
                     4,
                     G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT);

    g_type_class_add_private(klass, sizeof(SpiceVirglChannelPrivate));
    channel_set_handlers(SPICE_CHANNEL_CLASS(klass));
}


#ifdef G_OS_UNIX
/* coroutine context */
static void spice_virgl_handle_scanout_unix(SpiceChannel *channel, SpiceMsgIn *in)
{
    SpiceVirglChannelPrivate *c = SPICE_VIRGL_CHANNEL(channel)->priv;
    SpiceMsgVirglScanoutUnix *scanout = spice_msg_in_parsed(in);

    scanout->fd = -1;
    if (scanout->format != 0) {
        scanout->fd = spice_channel_unix_read_fd(channel);
        CHANNEL_DEBUG(channel, "scanout unix fd: %d", scanout->fd);
    }

    c->scanout.y0top = scanout->flags & SPICE_SCANOUT_FLAGS_Y0TOP;
    if (c->scanout.fd >= 0)
        close(c->scanout.fd);
    c->scanout.fd = scanout->fd;
    c->scanout.width = scanout->width;
    c->scanout.height = scanout->height;
    c->scanout.stride = scanout->stride;
    c->scanout.format = scanout->format;
    c->scanout.x = scanout->x;
    c->scanout.y = scanout->y;
    c->scanout.w = scanout->w;
    c->scanout.h = scanout->h;

    g_coroutine_object_notify(G_OBJECT(channel), "scanout");
}
#endif

/* coroutine context */
static void spice_virgl_handle_update(SpiceChannel *channel, SpiceMsgIn *in)
{
    SpiceMsgVirglUpdate *update = spice_msg_in_parsed(in);

    CHANNEL_DEBUG(channel, "update %dx%d+%d+%d",
                  update->w, update->h, update->x, update->y);

    g_coroutine_signal_emit(channel, signals[SIGNAL_VIRGL_UPDATE], 0,
                            update->x, update->y,
                            update->w, update->h);
}

static void channel_set_handlers(SpiceChannelClass *klass)
{
    static const spice_msg_handler handlers[] = {
#ifdef G_OS_UNIX
        [ SPICE_MSG_VIRGL_SCANOUT_UNIX ]       = spice_virgl_handle_scanout_unix,
#endif
        [ SPICE_MSG_VIRGL_UPDATE ]             = spice_virgl_handle_update,
    };

    spice_channel_set_handlers(klass, handlers, G_N_ELEMENTS(handlers));
}

/**
 * spice_virgl_scanout_copy:
 * @scanout:
 *
 **/
static SpiceVirglScanout*
spice_virgl_scanout_copy(const SpiceVirglScanout *scanout)
{
    return g_slice_dup(SpiceVirglScanout, scanout);
}

void
spice_virgl_scanout_free(SpiceVirglScanout *scanout)
{
    g_slice_free(SpiceVirglScanout, scanout);
}
