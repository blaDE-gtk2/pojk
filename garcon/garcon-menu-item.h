/* $Id$ */
/*-
 * vi:set et ai sts=2 sw=2 cindent:
 *
 * Copyright (c) 2006-2009 Jannis Pohlmann <jannis@xfce.org>
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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#if !defined(GARCON_INSIDE_GARCON_H) && !defined(GARCON_COMPILATION)
#error "Only <garcon/garcon.h> can be included directly. This file may disappear or change contents."
#endif

#ifndef __GARCON_MENU_ITEM_H__
#define __GARCON_MENU_ITEM_H__

#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define GARCON_TYPE_MENU_ITEM            (garcon_menu_item_get_type())
#define GARCON_MENU_ITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GARCON_TYPE_MENU_ITEM, GarconMenuItem))
#define GARCON_MENU_ITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GARCON_TYPE_MENU_ITEM, GarconMenuItemClass))
#define GARCON_IS_MENU_ITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GARCON_TYPE_MENU_ITEM))
#define GARCON_IS_MENU_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GARCON_TYPE_MENU_ITEM))
#define GARCON_MENU_ITEM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GARCON_TYPE_MENU_ITEM, GarconMenuItemClass))

typedef struct _GarconMenuItemPrivate GarconMenuItemPrivate;
typedef struct _GarconMenuItemClass   GarconMenuItemClass;
typedef struct _GarconMenuItem        GarconMenuItem;

GType           garcon_menu_item_get_type                          (void) G_GNUC_CONST;

GarconMenuItem *garcon_menu_item_new                               (GFile          *file) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
GarconMenuItem *garcon_menu_item_new_for_path                      (const gchar    *filename) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
GarconMenuItem *garcon_menu_item_new_for_uri                       (const gchar    *uri) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

GFile          *garcon_menu_item_get_file                          (GarconMenuItem *item);

gchar          *garcon_menu_item_get_uri                           (GarconMenuItem *item) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

const gchar    *garcon_menu_item_get_desktop_id                    (GarconMenuItem *item);
void            garcon_menu_item_set_desktop_id                    (GarconMenuItem *item,
                                                                    const gchar    *desktop_id);

const gchar    *garcon_menu_item_get_command                       (GarconMenuItem *item);
void            garcon_menu_item_set_command                       (GarconMenuItem *item,
                                                                    const gchar    *command);
const gchar    *garcon_menu_item_get_try_exec                      (GarconMenuItem *item);
void            garcon_menu_item_set_try_exec                      (GarconMenuItem *item,
                                                                    const gchar    *try_exec);
const gchar    *garcon_menu_item_get_name                          (GarconMenuItem *item);
void            garcon_menu_item_set_name                          (GarconMenuItem *item,
                                                                    const gchar    *name);
const gchar    *garcon_menu_item_get_generic_name                  (GarconMenuItem *item);
void            garcon_menu_item_set_generic_name                  (GarconMenuItem *item,
                                                                    const gchar    *generic_name);
const gchar    *garcon_menu_item_get_comment                       (GarconMenuItem *item);
void            garcon_menu_item_set_comment                       (GarconMenuItem *item,
                                                                    const gchar    *comment);
const gchar    *garcon_menu_item_get_icon_name                     (GarconMenuItem *item);
void            garcon_menu_item_set_icon_name                     (GarconMenuItem *item,
                                                                    const gchar    *icon_name);
const gchar    *garcon_menu_item_get_path                          (GarconMenuItem *item);
void            garcon_menu_item_set_path                          (GarconMenuItem *item,
                                                                    const gchar    *path);
gboolean        garcon_menu_item_requires_terminal                 (GarconMenuItem *item);
void            garcon_menu_item_set_requires_terminal             (GarconMenuItem *item,
                                                                    gboolean        requires_terminal);
gboolean        garcon_menu_item_get_no_display                    (GarconMenuItem *item);
void            garcon_menu_item_set_no_display                    (GarconMenuItem *item,
                                                                    gboolean        no_display);
gboolean        garcon_menu_item_supports_startup_notification     (GarconMenuItem *item);
void            garcon_menu_item_set_supports_startup_notification (GarconMenuItem *item,
                                                                    gboolean        supports_startup_notification);
GList          *garcon_menu_item_get_categories                    (GarconMenuItem *item);
void            garcon_menu_item_set_categories                    (GarconMenuItem *item,
                                                                    GList          *categories);
gboolean        garcon_menu_item_has_category                      (GarconMenuItem *item,
                                                                    const gchar    *category);
gboolean        garcon_menu_item_get_show_in_environment           (GarconMenuItem *item);
gboolean        garcon_menu_item_only_show_in_environment          (GarconMenuItem *item);
void            garcon_menu_item_ref                               (GarconMenuItem *item);
void            garcon_menu_item_unref                             (GarconMenuItem *item);
gint            garcon_menu_item_get_allocated                     (GarconMenuItem *item);
void            garcon_menu_item_increment_allocated               (GarconMenuItem *item);
void            garcon_menu_item_decrement_allocated               (GarconMenuItem *item);

G_END_DECLS

#endif /* !__GARCON_MENU_ITEM_H__ */