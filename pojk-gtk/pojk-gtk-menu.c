/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2013 Nick Schermer <nick@xfce.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libbladeutil/libbladeutil.h>
#include <libbladeui/libbladeui.h>

#include <pojk-gtk/pojk-gtk-menu.h>

#define STR_IS_EMPTY(str) ((str) == NULL || *(str) == '\0')


/**
 * SECTION: pojk-gtk-menu
 * @title: PojkGtkMenu
 * @short_description: Create a GtkMenu for a PojkMenu.
 * @include: pojk-gtk/pojk-gtk.h
 *
 * Create a complete GtkMenu for the given PojkMenu
 **/



/* Property identifiers */
enum
{
  PROP_0,
  PROP_MENU,
  PROP_SHOW_GENERIC_NAMES,
  PROP_SHOW_MENU_ICONS,
  PROP_SHOW_TOOLTIPS,
  PROP_SHOW_DESKTOP_ACTIONS,
  PROP_RIGHT_CLICK_EDITS,
  N_PROPERTIES
};



static void                 pojk_gtk_menu_finalize                    (GObject                 *object);
static void                 pojk_gtk_menu_get_property                (GObject                 *object,
                                                                         guint                    prop_id,
                                                                         GValue                  *value,
                                                                         GParamSpec              *pspec);
static void                 pojk_gtk_menu_set_property                (GObject                 *object,
                                                                         guint                    prop_id,
                                                                         const GValue            *value,
                                                                         GParamSpec              *pspec);
static void                 pojk_gtk_menu_show                        (GtkWidget               *widget);
static void                 pojk_gtk_menu_load                        (PojkGtkMenu           *menu);



struct _PojkGtkMenuPrivate
{
  PojkMenu *menu;

  guint is_loaded : 1;

  /* reload idle */
  guint reload_id;

  /* settings */
  guint show_generic_names : 1;
  guint show_menu_icons : 1;
  guint show_tooltips : 1;
  guint show_desktop_actions : 1;
  guint right_click_edits : 1;
};



static const GtkTargetEntry dnd_target_list[] = {
  { "text/uri-list", 0, 0 }
};



static GParamSpec *menu_props[N_PROPERTIES] = { NULL, };



G_DEFINE_TYPE_WITH_PRIVATE (PojkGtkMenu, pojk_gtk_menu, GTK_TYPE_MENU)



static void
pojk_gtk_menu_class_init (PojkGtkMenuClass *klass)
{
  GObjectClass   *gobject_class;
  GtkWidgetClass *gtkwidget_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = pojk_gtk_menu_finalize;
  gobject_class->get_property = pojk_gtk_menu_get_property;
  gobject_class->set_property = pojk_gtk_menu_set_property;

  gtkwidget_class = GTK_WIDGET_CLASS (klass);
  gtkwidget_class->show = pojk_gtk_menu_show;

  /**
   * PojkMenu:menu:
   *
   *
   **/
  menu_props[PROP_MENU] =
    g_param_spec_object ("menu",
                         "menu",
                         "menu",
                         POJK_TYPE_MENU,
                         G_PARAM_READWRITE
                         | G_PARAM_STATIC_STRINGS);

  /**
   * PojkMenu:show-generic-names:
   *
   *
   **/
  menu_props[PROP_SHOW_GENERIC_NAMES] =
    g_param_spec_boolean ("show-generic-names",
                          "show-generic-names",
                          "show-generic-names",
                          FALSE,
                          G_PARAM_READWRITE
                         | G_PARAM_STATIC_STRINGS);

  /**
   * PojkMenu:show-menu-icons:
   *
   *
   **/
  menu_props[PROP_SHOW_MENU_ICONS] =
    g_param_spec_boolean ("show-menu-icons",
                          "show-menu-icons",
                          "show-menu-icons",
                          TRUE,
                          G_PARAM_READWRITE
                          | G_PARAM_STATIC_STRINGS);

  /**
   * PojkMenu:show-tooltips:
   *
   *
   **/
  menu_props[PROP_SHOW_TOOLTIPS] =
    g_param_spec_boolean ("show-tooltips",
                          "show-tooltips",
                          "show-tooltips",
                          FALSE,
                          G_PARAM_READWRITE
                          | G_PARAM_STATIC_STRINGS);

  /**
   * PojkMenu:show-desktop-actions:
   *
   *
   **/
  menu_props[PROP_SHOW_DESKTOP_ACTIONS] =
    g_param_spec_boolean ("show-desktop-actions",
                          "show-desktop-actions",
                          "show desktop actions in a submenu",
                          FALSE,
                          G_PARAM_READWRITE
                          | G_PARAM_STATIC_STRINGS);

  /**
   * PojkMenu:right-click-edits:
   *
   *
   **/
  menu_props[PROP_RIGHT_CLICK_EDITS] =
    g_param_spec_boolean ("right-click-edits",
                          "right-click-edits",
                          "right click to edit menu items",
                          FALSE,
                          G_PARAM_READWRITE
                          | G_PARAM_STATIC_STRINGS);

  /* install all properties */
  g_object_class_install_properties (gobject_class, N_PROPERTIES, menu_props);
}



static void
pojk_gtk_menu_init (PojkGtkMenu *menu)
{
  menu->priv = pojk_gtk_menu_get_instance_private (menu);

  menu->priv->show_generic_names = FALSE;
  menu->priv->show_menu_icons = TRUE;
  menu->priv->show_tooltips = FALSE;
  menu->priv->show_desktop_actions = FALSE;
  menu->priv->right_click_edits = FALSE;

  gtk_menu_set_reserve_toggle_size (GTK_MENU (menu), FALSE);
}



static void
pojk_gtk_menu_finalize (GObject *object)
{
  PojkGtkMenu *menu = POJK_GTK_MENU (object);

  /* Stop pending reload */
  if (menu->priv->reload_id != 0)
    g_source_remove (menu->priv->reload_id);

  /* Release menu */
  if (menu->priv->menu != NULL)
    g_object_unref (menu->priv->menu);

  (*G_OBJECT_CLASS (pojk_gtk_menu_parent_class)->finalize) (object);
}



static void
pojk_gtk_menu_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  PojkGtkMenu *menu = POJK_GTK_MENU (object);

  switch (prop_id)
    {
    case PROP_MENU:
      g_value_set_object (value, menu->priv->menu);
      break;

    case PROP_SHOW_GENERIC_NAMES:
      g_value_set_boolean (value, menu->priv->show_generic_names);
      break;

    case PROP_SHOW_MENU_ICONS:
      g_value_set_boolean (value, menu->priv->show_menu_icons);
      break;

    case PROP_SHOW_TOOLTIPS:
      g_value_set_boolean (value, menu->priv->show_tooltips);
      break;

    case PROP_SHOW_DESKTOP_ACTIONS:
      g_value_set_boolean (value, menu->priv->show_desktop_actions);
      break;

    case PROP_RIGHT_CLICK_EDITS:
      g_value_set_boolean (value, menu->priv->right_click_edits);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
pojk_gtk_menu_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  PojkGtkMenu *menu = POJK_GTK_MENU (object);

  switch (prop_id)
    {
    case PROP_MENU:
      pojk_gtk_menu_set_menu (menu, g_value_get_object (value));
      break;

    case PROP_SHOW_GENERIC_NAMES:
      pojk_gtk_menu_set_show_generic_names (menu, g_value_get_boolean (value));
      break;

    case PROP_SHOW_MENU_ICONS:
      pojk_gtk_menu_set_show_menu_icons (menu, g_value_get_boolean (value));
      break;

    case PROP_SHOW_TOOLTIPS:
      pojk_gtk_menu_set_show_tooltips (menu, g_value_get_boolean (value));
      break;

    case PROP_SHOW_DESKTOP_ACTIONS:
      pojk_gtk_menu_set_show_desktop_actions (menu, g_value_get_boolean (value));
      break;

    case PROP_RIGHT_CLICK_EDITS:
      pojk_gtk_menu_set_right_click_edits (menu, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
pojk_gtk_menu_show (GtkWidget *widget)
{
  PojkGtkMenu *menu = POJK_GTK_MENU (widget);

  /* try to load the menu if needed */
  if (!menu->priv->is_loaded)
    pojk_gtk_menu_load (menu);

  (*GTK_WIDGET_CLASS (pojk_gtk_menu_parent_class)->show) (widget);
}



static void
pojk_gtk_menu_append_quoted (GString     *string,
                               const gchar *unquoted)
{
  gchar *quoted;

  quoted = g_shell_quote (unquoted);
  g_string_append (string, quoted);
  g_free (quoted);
}



static void
pojk_gtk_menu_item_activate_real (GtkWidget            *mi,
                                    PojkMenuItem       *item,
                                    PojkMenuItemAction *action)
{
  GString      *string;
  const gchar  *command;
  const gchar  *p;
  const gchar  *tmp;
  gchar       **argv;
  gboolean      result = FALSE;
  gchar        *uri;
  GError       *error = NULL;

  g_return_if_fail (GTK_IS_WIDGET (mi));
  g_return_if_fail (POJK_IS_MENU_ITEM (item));

  if (action != NULL)
    {
      command = pojk_menu_item_action_get_command (action);
    }
  else
    {
      command = pojk_menu_item_get_command (item);
    }

  if (STR_IS_EMPTY (command))
    return;

  string = g_string_sized_new (100);

  if (pojk_menu_item_requires_terminal (item))
    g_string_append (string, "blxo-open --launch TerminalEmulator ");

  /* expand the field codes */
  for (p = command; *p != '\0'; ++p)
    {
      if (G_UNLIKELY (p[0] == '%' && p[1] != '\0'))
        {
          switch (*++p)
            {
            case 'f': case 'F':
            case 'u': case 'U':
              /* TODO for dnd, not a regression, xfdesktop never had this */
              break;

            case 'i':
              tmp = pojk_menu_item_get_icon_name (item);
              if (!STR_IS_EMPTY (tmp))
                {
                  g_string_append (string, "--icon ");
                  pojk_gtk_menu_append_quoted (string, tmp);
                }
              break;

            case 'c':
              tmp = pojk_menu_item_get_name (item);
              if (!STR_IS_EMPTY (tmp))
                pojk_gtk_menu_append_quoted (string, tmp);
              break;

            case 'k':
              uri = pojk_menu_item_get_uri (item);
              if (!STR_IS_EMPTY (uri))
                pojk_gtk_menu_append_quoted (string, uri);
              g_free (uri);
              break;

            case '%':
              g_string_append_c (string, '%');
              break;
            }
        }
      else
        {
          g_string_append_c (string, *p);
        }
    }

  /* parse and spawn command */
  if (g_shell_parse_argv (string->str, NULL, &argv, &error))
    {
      result = xfce_spawn_on_screen (gtk_widget_get_screen (mi),
                                     pojk_menu_item_get_path (item),
                                     argv, NULL, G_SPAWN_SEARCH_PATH,
                                     pojk_menu_item_supports_startup_notification (item),
                                     gtk_get_current_event_time (),
                                     pojk_menu_item_get_icon_name (item),
                                     &error);

      g_strfreev (argv);
    }

  if (G_UNLIKELY (!result))
    {
      xfce_dialog_show_error (NULL, error, _("Failed to execute command \"%s\"."), command);
      g_error_free (error);
    }

  g_string_free (string, TRUE);
}



static void
pojk_gtk_menu_item_edit_launcher (PojkMenuItem *item)
{
  GFile   *file;
  gchar   *uri, *cmd;
  GError  *error = NULL;

  file = pojk_menu_item_get_file (item);

  if (file)
    {
      uri = g_file_get_uri (file);
      cmd = g_strdup_printf ("blxo-desktop-item-edit \"%s\"", uri);

      if (!xfce_spawn_command_line_on_screen (NULL, cmd, FALSE, FALSE, &error))
        {
          xfce_message_dialog (NULL,
                               _("Launch Error"),
                               "dialog-error",
                              _("Unable to launch \"blxo-desktop-item-edit\", which is required to create and edit menu items."),
                              error->message,
                              XFCE_BUTTON_TYPE_MIXED, "window-close", _("_Close"), GTK_RESPONSE_ACCEPT,
                              NULL);

          g_clear_error (&error);
        }

      g_free(uri);
      g_free(cmd);
      g_object_unref(file);
    }
}


static void
pojk_gtk_menu_item_activate (GtkWidget      *mi,
                               PojkMenuItem *item)
{
  PojkGtkMenu  *menu = g_object_get_data (G_OBJECT (mi), "PojkGtkMenu");
  GdkEventButton *evt;
  guint           button;
  gboolean        right_click = FALSE;

  evt = (GdkEventButton *)gtk_get_current_event();

  /* See if we're trying to edit the launcher */
   if(menu->priv->right_click_edits && evt && GDK_BUTTON_RELEASE == evt->type)
    {
      button = evt->button;

      /* right click or Shift + left can optionally edit launchers */
      if (button == 3 || (button == 1 && (evt->state & GDK_SHIFT_MASK)))
        {
          pojk_gtk_menu_item_edit_launcher (item);
          right_click = TRUE;
        }
    }

  if (!right_click)
    {
      /* normal action, launch the application */
      pojk_gtk_menu_item_activate_real (mi, item, NULL);
    }

  if (evt)
    {
      gdk_event_free((GdkEvent*)evt);
    }
}



static void
pojk_gtk_menu_item_action_activate (GtkWidget            *mi,
                                      PojkMenuItemAction *action)
{
  PojkMenuItem *item = g_object_get_data (G_OBJECT (action), "PojkMenuItem");

  if (item == NULL)
    {
      g_critical ("pojk_gtk_menu_item_action_activate: Failed to get the PojkMenuItem\n");
      return;
    }

  pojk_gtk_menu_item_activate_real (mi, item, action);
}



static void
pojk_gtk_menu_item_drag_begin (PojkMenuItem *item,
                                 GdkDragContext *drag_context)
{
  const gchar *icon_name;

  g_return_if_fail (POJK_IS_MENU_ITEM (item));

  icon_name = pojk_menu_item_get_icon_name (item);
  if (!STR_IS_EMPTY (icon_name))
    gtk_drag_set_icon_name (drag_context, icon_name, 0, 0);
}



static void
pojk_gtk_menu_item_drag_data_get (PojkMenuItem   *item,
                                    GdkDragContext   *drag_context,
                                    GtkSelectionData *selection_data,
                                    guint             info,
                                    guint             drag_time)
{
  gchar *uris[2] = { NULL, NULL };

  g_return_if_fail (POJK_IS_MENU_ITEM (item));

  uris[0] = pojk_menu_item_get_uri (item);
  if (G_LIKELY (uris[0] != NULL))
    {
      gtk_selection_data_set_uris (selection_data, uris);
      g_free (uris[0]);
    }
}



static void
pojk_gtk_menu_item_drag_end (PojkGtkMenu *menu)
{
  g_return_if_fail (GTK_IS_MENU (menu));

  /* make sure the menu is not visible */
  gtk_menu_popdown (GTK_MENU (menu));

  /* always emit this signal */
  g_signal_emit_by_name (G_OBJECT (menu), "selection-done", 0);
}



static void
pojk_gtk_menu_deactivate (GtkWidget     *submenu,
                            PojkGtkMenu *menu)
{
  pojk_gtk_menu_item_drag_end (menu);
}



static gboolean
pojk_gtk_menu_reload_idle (gpointer data)
{
  PojkGtkMenu *menu = POJK_GTK_MENU (data);
  GList         *children;

  /* wait until the menu is hidden */
  if (gtk_widget_get_visible (GTK_WIDGET (menu)))
    return TRUE;

  /* destroy all menu item */
  children = gtk_container_get_children (GTK_CONTAINER (menu));
  g_list_free_full (children, (GDestroyNotify) gtk_widget_destroy);

  /* reload the menu */
  pojk_gtk_menu_load (menu);

  /* reset */
  menu->priv->reload_id = 0;

  return FALSE;
}



static void
pojk_gtk_menu_reload (PojkGtkMenu *menu)
{
  /* schedule a menu reload */
  if (menu->priv->reload_id == 0
      && menu->priv->is_loaded)
    {
      menu->priv->reload_id = g_timeout_add (100, pojk_gtk_menu_reload_idle, menu);
    }
}



static GtkWidget*
pojk_gtk_menu_load_icon (const gchar *icon_name)
{
  GtkWidget *image = NULL;
  gint w, h, size;
  gchar *p, *name = NULL;
  GdkPixbuf *pixbuf = NULL;
  GtkIconTheme *icon_theme = gtk_icon_theme_get_default ();

  gtk_icon_size_lookup (GTK_ICON_SIZE_MENU, &w, &h);
  size = MIN (w, h);

  if (gtk_icon_theme_has_icon (icon_theme, icon_name))
    {
	  pixbuf = gtk_icon_theme_load_icon (icon_theme, icon_name, size, 0, NULL);;
    }
  else
    {
      if (g_path_is_absolute (icon_name))
        {
          pixbuf = gdk_pixbuf_new_from_file_at_scale (icon_name, w, h, TRUE, NULL);
        }
      else
        {
          /* try to lookup names like application.png in the theme */
          p = strrchr (icon_name, '.');
          if (p)
            {
              name = g_strndup (icon_name, p - icon_name);
              pixbuf = gtk_icon_theme_load_icon (icon_theme, name, size, 0, NULL);
              g_free (name);
              name = NULL;
            }

          /* maybe they point to a file in the pixbufs folder */
          if (G_UNLIKELY (pixbuf == NULL))
            {
              gchar *filename;

              filename = g_build_filename ("pixmaps", icon_name, NULL);
              name = xfce_resource_lookup (XFCE_RESOURCE_DATA, filename);
              g_free (filename);
            }

          if (name)
            {
              pixbuf = gdk_pixbuf_new_from_file_at_scale (name, w, h, TRUE, NULL);
              g_free (name);
            }
        }
    }

  /* Turn the pixbuf into a gtk_image */
  if (G_LIKELY (pixbuf))
    {
      /* scale the pixbuf down if it needs it */
      GdkPixbuf *pixbuf_scaled = gdk_pixbuf_scale_simple (pixbuf, w, h, GDK_INTERP_BILINEAR);
      g_object_unref (G_OBJECT (pixbuf));

      image = gtk_image_new_from_pixbuf (pixbuf_scaled);
      g_object_unref (G_OBJECT (pixbuf_scaled));
    }
  else
    {
	  /* display the placeholder at least */
	  image = gtk_image_new_from_icon_name (icon_name, GTK_ICON_SIZE_MENU);
    }

  return image;
}



static GtkWidget*
pojk_gtk_menu_create_menu_item (PojkGtkMenu *menu,
                                  const gchar *name,
                                  const gchar *icon_name)
{
  GtkWidget *mi;
  GtkWidget *box;
  GtkWidget *image;
  GtkWidget *label;

  /* create item */
  mi = gtk_menu_item_new ();
  label = gtk_label_new (name);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
#if GTK_CHECK_VERSION (3, 0, 0)
  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_label_set_xalign (GTK_LABEL (label), 0.0);
  gtk_widget_set_halign (label, GTK_ALIGN_START);
#else
  box = gtk_hbox_new (FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5f);
#endif

  if (menu->priv->show_menu_icons)
    {
      image = pojk_gtk_menu_load_icon (icon_name);
      gtk_widget_show (image);
    }
  else
    {
      image = gtk_image_new ();
    }

  /* Add the image and label to the box, add the box to the menu item */
  gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 6);
  gtk_widget_show_all (box);
  gtk_container_add (GTK_CONTAINER (mi), box);

  return mi;
}



static GtkWidget*
pojk_gtk_menu_add_actions (PojkGtkMenu  *menu,
                             PojkMenuItem *menu_item,
                             GList          *actions,
                             const gchar    *parent_icon_name)
{
  GtkWidget *submenu, *mi;
  GList     *iter;

  submenu = gtk_menu_new ();
  gtk_menu_set_reserve_toggle_size (GTK_MENU (submenu), FALSE);

  /* Add the parent item again, this time something the user can click to execute */
  mi = pojk_gtk_menu_create_menu_item (menu, pojk_menu_item_get_name (menu_item), parent_icon_name);
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), mi);
  /* we need to store the PojkGtkMenu with this item so we can
   * use it if the user wants to edit a menu item */
  g_object_set_data (G_OBJECT (mi), "PojkGtkMenu", menu);
  g_signal_connect (G_OBJECT (mi), "activate",
                    G_CALLBACK (pojk_gtk_menu_item_activate), menu_item);
  gtk_widget_show (mi);

  /* Add all the individual actions to the menu */
  for (iter = g_list_first(actions); iter != NULL; iter = g_list_next (iter))
    {
      PojkMenuItemAction *action = pojk_menu_item_get_action (menu_item, iter->data);
      const gchar          *action_icon_name;

      if (action == NULL)
        continue;

      /* If there's a custom icon associated with the action, use it.
       * Otherwise default to the parent's icon.
       */
      action_icon_name = pojk_menu_item_action_get_icon_name (action);
      if (action_icon_name == NULL)
        {
          action_icon_name = parent_icon_name;
        }

      mi = pojk_gtk_menu_create_menu_item (menu,
                                             pojk_menu_item_action_get_name (action),
                                             action_icon_name);

      gtk_menu_shell_append (GTK_MENU_SHELL (submenu), mi);
      g_signal_connect (G_OBJECT (mi), "activate",
                        G_CALLBACK (pojk_gtk_menu_item_action_activate), action);
      /* we need to store the parent associated with this item so we can
       * activate it properly */
      g_object_set_data (G_OBJECT (action), "PojkMenuItem", menu_item);
      gtk_widget_show (mi);
    }

  return submenu;
}



static gboolean
pojk_gtk_menu_add (PojkGtkMenu *menu,
                     GtkMenu       *gtk_menu,
                     PojkMenu    *pojk_menu)
{
  GList               *elements, *li;
  GtkWidget           *mi;
  const gchar         *name, *icon_name;
  const gchar         *comment;
  GtkWidget           *submenu;
  gboolean             has_children = FALSE;
  const gchar         *command;
  PojkMenuDirectory *directory;

  g_return_val_if_fail (POJK_GTK_IS_MENU (menu), FALSE);
  g_return_val_if_fail (GTK_IS_MENU (gtk_menu), FALSE);
  g_return_val_if_fail (POJK_IS_MENU (pojk_menu), FALSE);

  elements = pojk_menu_get_elements (pojk_menu);
  for (li = elements; li != NULL; li = li->next)
    {
      g_assert (POJK_IS_MENU_ELEMENT (li->data));

      if (POJK_IS_MENU_ITEM (li->data))
        {
          GList *actions = NULL;

          /* watch for changes */
          g_signal_connect_swapped (G_OBJECT (li->data), "changed",
              G_CALLBACK (pojk_gtk_menu_reload), menu);

          /* skip invisible items */
          if (!pojk_menu_element_get_visible (li->data))
            continue;

          /* get element name */
          name = NULL;
          if (menu->priv->show_generic_names)
            name = pojk_menu_item_get_generic_name (li->data);
          if (name == NULL)
            name = pojk_menu_item_get_name (li->data);

          if (G_UNLIKELY (name == NULL))
            continue;

          icon_name = pojk_menu_item_get_icon_name (li->data);
          if (STR_IS_EMPTY (icon_name))
            icon_name = "applications-other";

          /* build the menu item */
          mi = pojk_gtk_menu_create_menu_item (menu, name, icon_name);
          gtk_menu_shell_append (GTK_MENU_SHELL (gtk_menu), mi);

          /* if the menu item has actions such as "Private browsing mode"
           * show them as well */
          if (menu->priv->show_desktop_actions)
            {
              actions = pojk_menu_item_get_actions (li->data);
            }

          if (actions != NULL)
            {
              submenu = pojk_gtk_menu_add_actions (menu, li->data, actions, icon_name);
              gtk_menu_item_set_submenu (GTK_MENU_ITEM (mi), submenu);
              g_list_free (actions);
            }
          else
            {
              g_signal_connect (G_OBJECT (mi), "activate",
                                G_CALLBACK (pojk_gtk_menu_item_activate), li->data);
              /* we need to store the PojkGtkMenu with this item so we can
               * use it if the user wants to edit a menu item */
              g_object_set_data (G_OBJECT (mi), "PojkGtkMenu", menu);
            }

          gtk_widget_show (mi);

          if (menu->priv->show_tooltips)
            {
              comment = pojk_menu_item_get_comment (li->data);
              if (!STR_IS_EMPTY (comment))
                gtk_widget_set_tooltip_text (mi, comment);
            }

          /* support for dnd item to for example the blade-bar */
          gtk_drag_source_set (mi, GDK_BUTTON1_MASK, dnd_target_list,
              G_N_ELEMENTS (dnd_target_list), GDK_ACTION_COPY);
          g_signal_connect_swapped (G_OBJECT (mi), "drag-begin",
              G_CALLBACK (pojk_gtk_menu_item_drag_begin), li->data);
          g_signal_connect_swapped (G_OBJECT (mi), "drag-data-get",
              G_CALLBACK (pojk_gtk_menu_item_drag_data_get), li->data);
          g_signal_connect_swapped (G_OBJECT (mi), "drag-end",
              G_CALLBACK (pojk_gtk_menu_item_drag_end), menu);

          /* doesn't happen, but anyway... */
          command = pojk_menu_item_get_command (li->data);
          if (STR_IS_EMPTY (command))
            gtk_widget_set_sensitive (mi, FALSE);

          /* atleast 1 visible child */
          has_children = TRUE;
        }
      else if (POJK_IS_MENU_SEPARATOR (li->data))
        {
          mi = gtk_separator_menu_item_new ();
          gtk_menu_shell_append (GTK_MENU_SHELL (gtk_menu), mi);
          gtk_widget_show (mi);
        }
      else if (POJK_IS_MENU (li->data))
        {
          /* the element check for menu also copies the item list to
           * check if all the elements are visible, we do that with the
           * return value of this function, so avoid that and only check
           * the visibility of the menu directory */
          directory = pojk_menu_get_directory (li->data);
          if (directory != NULL
              && !pojk_menu_directory_get_visible (directory))
            continue;

          submenu = gtk_menu_new ();
          gtk_menu_set_reserve_toggle_size (GTK_MENU (submenu), FALSE);
          if (pojk_gtk_menu_add (menu, GTK_MENU (submenu), li->data))
            {
              /* attach submenu */
              name = pojk_menu_element_get_name (li->data);

              icon_name = pojk_menu_element_get_icon_name (li->data);
              if (STR_IS_EMPTY (icon_name))
                icon_name = "applications-other";

              /* build the menu item */
              mi = pojk_gtk_menu_create_menu_item (menu, name, icon_name);

              gtk_menu_shell_append (GTK_MENU_SHELL (gtk_menu), mi);
              gtk_menu_item_set_submenu (GTK_MENU_ITEM (mi), submenu);
              g_signal_connect (G_OBJECT (submenu), "selection-done",
                  G_CALLBACK (pojk_gtk_menu_deactivate), menu);
              gtk_widget_show (mi);

              /* atleast 1 visible child */
              has_children = TRUE;
            }
          else
            {
              /* no visible element in the menu */
              gtk_widget_destroy (submenu);
            }
        }
    }

  g_list_free (elements);

  return has_children;
}



static void
pojk_gtk_menu_load (PojkGtkMenu *menu)
{
  GError    *error = NULL;

  g_return_if_fail (POJK_GTK_IS_MENU (menu));
  g_return_if_fail (menu->priv->menu == NULL || POJK_IS_MENU (menu->priv->menu));

  if (menu->priv->menu == NULL)
    return;

  if (pojk_menu_load (menu->priv->menu, NULL, &error))
    {
      pojk_gtk_menu_add (menu, GTK_MENU (menu), menu->priv->menu);

      /* watch for changes */
      g_signal_connect_swapped (G_OBJECT (menu->priv->menu), "reload-required",
        G_CALLBACK (pojk_gtk_menu_reload), menu);
    }
  else
    {
       xfce_dialog_show_error (NULL, error, _("Failed to load the applications menu"));
       g_error_free (error);
    }

  menu->priv->reload_id = 0;
  menu->priv->is_loaded = TRUE;
}



/**
 * pojk_gtk_menu_new:
 * @pojk_menu  :
 *
 * Creates a new #PojkMenu for the .menu file referred to by @file.
 * This operation only fails @file is invalid. To load the menu
 * tree from the file, you need to call pojk_gtk_menu_load() with the
 * returned #PojkMenu.
 *
 * The caller is responsible to destroy the returned #PojkMenu
 * using g_object_unref().
 *
 * For more information about the usage @see pojk_gtk_menu_new().
 *
 * Returns: a new #PojkMenu for @file.
 **/
GtkWidget *
pojk_gtk_menu_new (PojkMenu *pojk_menu)
{
  g_return_val_if_fail (pojk_menu == NULL || POJK_IS_MENU (pojk_menu), NULL);
  return g_object_new (POJK_GTK_TYPE_MENU, "menu", pojk_menu, NULL);
}



/**
 * pojk_gtk_menu_set_menu:
 * @menu  : A #PojkGtkMenu
 * @pojk_menu : The #PojkMenu to use
 *
 **/
void
pojk_gtk_menu_set_menu (PojkGtkMenu *menu,
                          PojkMenu    *pojk_menu)
{
  g_return_if_fail (POJK_GTK_IS_MENU (menu));
  g_return_if_fail (pojk_menu == NULL || POJK_IS_MENU (pojk_menu));

  if (menu->priv->menu == pojk_menu)
    return;

  if (menu->priv->menu != NULL)
    {
      g_signal_handlers_disconnect_by_func (G_OBJECT (menu->priv->menu), pojk_gtk_menu_reload, menu);
      g_object_unref (G_OBJECT (menu->priv->menu));
    }

  if (pojk_menu != NULL)
    menu->priv->menu = POJK_MENU (g_object_ref (G_OBJECT (pojk_menu)));
  else
    menu->priv->menu = NULL;

  g_object_notify_by_pspec (G_OBJECT (menu), menu_props[PROP_MENU]);

  pojk_gtk_menu_reload (menu);
}



/**
 * pojk_gtk_menu_get_menu:
 * @menu  : A #PojkGtkMenu
 *
 * The #PojkMenu used to create the #GtkMenu.
 *
 * The caller is responsible to releasing the returned #PojkMenu
 * using g_object_unref().
 *
 * Returns: the #PojkMenu for @menu.
 **/
PojkMenu *
pojk_gtk_menu_get_menu (PojkGtkMenu *menu)
{
  g_return_val_if_fail (POJK_GTK_IS_MENU (menu), NULL);
  if (menu->priv->menu != NULL)
    return POJK_MENU (g_object_ref (G_OBJECT (menu->priv->menu)));
  return NULL;
}



/**
 * pojk_gtk_menu_set_show_generic_names:
 * @menu               : A #PojkGtkMenu
 * @show_generic_names : new value
 *
 **/
void
pojk_gtk_menu_set_show_generic_names (PojkGtkMenu *menu,
                                        gboolean       show_generic_names)
{
  g_return_if_fail (POJK_GTK_IS_MENU (menu));

  if (menu->priv->show_generic_names == show_generic_names)
    return;

  menu->priv->show_generic_names = !!show_generic_names;
  g_object_notify_by_pspec (G_OBJECT (menu), menu_props[PROP_SHOW_GENERIC_NAMES]);

  pojk_gtk_menu_reload (menu);
}



/**
 * pojk_gtk_menu_get_show_generic_names:
 * @menu  : A #PojkGtkMenu
 *
 * Return value: if generic names are shown
 **/
gboolean
pojk_gtk_menu_get_show_generic_names (PojkGtkMenu *menu)
{
  g_return_val_if_fail (POJK_GTK_IS_MENU (menu), FALSE);
  return menu->priv->show_generic_names;
}



/**
 * pojk_gtk_menu_set_show_menu_icons:
 * @menu            : A #PojkGtkMenu
 * @show_menu_icons : new value
 *
 *
 **/
void
pojk_gtk_menu_set_show_menu_icons (PojkGtkMenu *menu,
                                     gboolean       show_menu_icons)
{
  g_return_if_fail (POJK_GTK_IS_MENU (menu));

  if (menu->priv->show_menu_icons == show_menu_icons)
    return;

  menu->priv->show_menu_icons = !!show_menu_icons;
  g_object_notify_by_pspec (G_OBJECT (menu), menu_props[PROP_SHOW_MENU_ICONS]);

  pojk_gtk_menu_reload (menu);
}



/**
 * pojk_gtk_menu_get_show_menu_icons:
 * @menu  : A #PojkGtkMenu
 *
 * Return value: if menu icons are shown
 **/
gboolean
pojk_gtk_menu_get_show_menu_icons (PojkGtkMenu *menu)
{
  g_return_val_if_fail (POJK_GTK_IS_MENU (menu), FALSE);
  return menu->priv->show_menu_icons;
}



/**
 * pojk_gtk_menu_set_show_tooltips:
 * @menu  : A #PojkGtkMenu
 *
 *
 **/
void
pojk_gtk_menu_set_show_tooltips (PojkGtkMenu *menu,
                                   gboolean       show_tooltips)
{
  g_return_if_fail (POJK_GTK_IS_MENU (menu));

  if (menu->priv->show_tooltips == show_tooltips)
    return;

  menu->priv->show_tooltips = !!show_tooltips;
  g_object_notify_by_pspec (G_OBJECT (menu), menu_props[PROP_SHOW_TOOLTIPS]);

  pojk_gtk_menu_reload (menu);
}



/**
 * pojk_gtk_menu_get_show_tooltips:
 * @menu  : A #PojkGtkMenu
 *
 * Return value: if descriptions are shown in tooltip
 **/
gboolean
pojk_gtk_menu_get_show_tooltips (PojkGtkMenu *menu)
{
  g_return_val_if_fail (POJK_GTK_IS_MENU (menu), FALSE);
  return menu->priv->show_tooltips;
}

/**
 * pojk_gtk_menu_set_show_desktop_actions:
 * @menu  : A #PojkGtkMenu
 * @show_desktop_actions : Toggle showing the desktop actions in a submenu.
 *
 **/
void
pojk_gtk_menu_set_show_desktop_actions (PojkGtkMenu *menu,
                                          gboolean       show_desktop_actions)
{
  g_return_if_fail (POJK_GTK_IS_MENU (menu));

  if (menu->priv->show_desktop_actions == show_desktop_actions)
    return;

  menu->priv->show_desktop_actions = !!show_desktop_actions;
  g_object_notify_by_pspec (G_OBJECT (menu), menu_props[PROP_SHOW_DESKTOP_ACTIONS]);

  pojk_gtk_menu_reload (menu);
}



/**
 * pojk_gtk_menu_get_show_desktop_actions:
 * @menu  : A #PojkGtkMenu
 *
 * Return value: if the desktop actions in a submenu
 **/
gboolean
pojk_gtk_menu_get_show_desktop_actions (PojkGtkMenu *menu)
{
  g_return_val_if_fail (POJK_GTK_IS_MENU (menu), FALSE);
  return menu->priv->show_desktop_actions;
}


/**
 * pojk_gtk_menu_set_right_click_edits:
 * @menu  : A #PojkGtkMenu
 * @enable_right_click_edits : Toggle showing wether to launch an editor
 * when the menu is clicked with the secondary mouse button.
 *
 **/
void
pojk_gtk_menu_set_right_click_edits (PojkGtkMenu *menu,
                                       gboolean       enable_right_click_edits)
{
  g_return_if_fail (POJK_GTK_IS_MENU (menu));

  if (menu->priv->right_click_edits == enable_right_click_edits)
    return;

  menu->priv->right_click_edits = !!enable_right_click_edits;
  g_object_notify_by_pspec (G_OBJECT (menu), menu_props[PROP_RIGHT_CLICK_EDITS]);

  pojk_gtk_menu_reload (menu);
}



/**
 * pojk_gtk_menu_get_right_click_edits:
 * @menu  : A #PojkGtkMenu
 *
 * Return value: if an editor will be launched on secondary mouse clicks.
 **/
gboolean
pojk_gtk_menu_get_right_click_edits (PojkGtkMenu *menu)
{
  g_return_val_if_fail (POJK_GTK_IS_MENU (menu), FALSE);
  return menu->priv->right_click_edits;
}
