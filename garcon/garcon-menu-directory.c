/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006-2009 Jannis Pohlmann <jannis@xfce.org>
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

#include <locale.h>
#include <glib.h>

#include <garcon/garcon-environment.h>
#include <garcon/garcon-menu-directory.h>



#define GARCON_MENU_DIRECTORY_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), GARCON_TYPE_MENU_DIRECTORY, GarconMenuDirectoryPrivate))



/* Desktop entry keys */
#if 0
static const gchar *desktop_entry_keys[] =
{
  "Name",
  "Comment",
  "Icon",
  "Categories",
  "OnlyShowIn",
  "NotShowIn",
  "NoDisplay",
  "Hidden",
  NULL
};
#endif



/* Property identifiers */
enum
{
  PROP_0,
  PROP_FILE,
  PROP_NAME,
  PROP_COMMENT,
  PROP_NO_DISPLAY,
  PROP_ICON,
};



static void garcon_menu_directory_finalize     (GObject                  *object);
static void garcon_menu_directory_get_property (GObject                  *object,
                                                guint                     prop_id,
                                                GValue                   *value,
                                                GParamSpec               *pspec);
static void garcon_menu_directory_set_property (GObject                  *object,
                                                guint                     prop_id,
                                                const GValue             *value,
                                                GParamSpec               *pspec);



struct _GarconMenuDirectoryPrivate
{
  /* Directory file */
  GFile  *file;

  /* Directory name */
  gchar  *name;

  /* Directory description (comment) */
  gchar  *comment;

  /* Icon */
  gchar  *icon;

  /* Environments in which the menu should be displayed only */
  gchar **only_show_in;

  /* Environments in which the menu should be hidden */
  gchar **not_show_in;

  /* Whether the menu should be ignored completely */
  guint   hidden : 1;

  /* Whether the menu should be hidden */
  guint   no_display : 1;
};



G_DEFINE_TYPE (GarconMenuDirectory, garcon_menu_directory, G_TYPE_OBJECT)



static void
garcon_menu_directory_class_init (GarconMenuDirectoryClass *klass)
{
  GObjectClass *gobject_class;

  g_type_class_add_private (klass, sizeof(GarconMenuDirectoryPrivate));

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = garcon_menu_directory_finalize;
  gobject_class->get_property = garcon_menu_directory_get_property;
  gobject_class->set_property = garcon_menu_directory_set_property;

  /**
   * GarconMenuDirectory:filename:
   *
   * The @GFile of an %GarconMenuDirectory.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_FILE,
                                   g_param_spec_object ("file",
                                                        "File",
                                                        "File",
                                                        G_TYPE_FILE,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS |
                                                        G_PARAM_CONSTRUCT_ONLY));

  /**
   * GarconMenuDirectory:name:
   *
   * Name of the directory.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_NAME,
                                   g_param_spec_string ("name",
                                                        "Name",
                                                        "Directory name",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS));

  /**
   * GarconMenuDirectory:comment:
   *
   * Directory description (comment).
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_COMMENT,
                                   g_param_spec_string ("comment",
                                                        "Description",
                                                        "Directory description",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS));

  /**
   * GarconMenuDirectory:icon:
   *
   * Icon associated with this directory.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_ICON,
                                   g_param_spec_string ("icon",
                                                        "Icon",
                                                        "Directory icon",
                                                        NULL,
                                                        G_PARAM_READWRITE |
                                                        G_PARAM_STATIC_STRINGS));

  /**
   * GarconMenuDirectory:no-display:
   *
   * Whether this menu item is hidden in menus.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_NO_DISPLAY,
                                   g_param_spec_boolean ("no-display",
                                                         "No Display",
                                                         "Visibility state of the related menu",
                                                         FALSE,
                                                         G_PARAM_READWRITE |
                                                         G_PARAM_STATIC_STRINGS));

}



static void
garcon_menu_directory_init (GarconMenuDirectory *directory)
{
  directory->priv = GARCON_MENU_DIRECTORY_GET_PRIVATE (directory);
  directory->priv->file = NULL;
  directory->priv->name = NULL;
  directory->priv->icon = NULL;
  directory->priv->only_show_in = NULL;
  directory->priv->not_show_in = NULL;
  directory->priv->hidden = FALSE;
  directory->priv->no_display = FALSE;
}



static void
garcon_menu_directory_finalize (GObject *object)
{
  GarconMenuDirectory *directory = GARCON_MENU_DIRECTORY (object);

  /* Free name */
  g_free (directory->priv->name);

  /* Free comment */
  g_free (directory->priv->comment);

  /* Free icon */
  g_free (directory->priv->icon);

  /* Free environment lists */
  g_strfreev (directory->priv->only_show_in);
  g_strfreev (directory->priv->not_show_in);

  /* Free file */
  g_object_unref (directory->priv->file);

  (*G_OBJECT_CLASS (garcon_menu_directory_parent_class)->finalize) (object);
}



static void
garcon_menu_directory_get_property (GObject    *object,
                                    guint       prop_id,
                                    GValue     *value,
                                    GParamSpec *pspec)
{
  GarconMenuDirectory *directory = GARCON_MENU_DIRECTORY (object);

  switch (prop_id)
    {
    case PROP_FILE:
      g_value_set_object (value, directory->priv->file);
      break;

    case PROP_NAME:
      g_value_set_string (value, garcon_menu_directory_get_name (directory));
      break;

    case PROP_COMMENT:
      g_value_set_string (value, garcon_menu_directory_get_comment (directory));
      break;

    case PROP_ICON:
      g_value_set_string (value, garcon_menu_directory_get_icon (directory));
      break;

    case PROP_NO_DISPLAY:
      g_value_set_boolean (value, garcon_menu_directory_get_no_display (directory));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
garcon_menu_directory_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  GarconMenuDirectory *directory = GARCON_MENU_DIRECTORY (object);

  switch (prop_id)
    {
    case PROP_FILE:
      directory->priv->file = g_object_ref (g_value_get_object (value));
      break;

    case PROP_NAME:
      garcon_menu_directory_set_name (directory, g_value_get_string (value));
      break;

    case PROP_COMMENT:
      garcon_menu_directory_set_comment (directory, g_value_get_string (value));
      break;

    case PROP_ICON:
      garcon_menu_directory_set_icon (directory, g_value_get_string (value));
      break;

    case PROP_NO_DISPLAY:
      garcon_menu_directory_set_no_display (directory, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



GarconMenuDirectory *
garcon_menu_directory_new (GFile *file)
{
  GarconMenuDirectory *directory = NULL;
  gchar               *contents;
  gsize                length;
  GKeyFile            *rc;
  gchar               *name;
  gchar               *comment;
  gchar               *icon;
  gboolean             no_display;
  gboolean             succeed;

  g_return_val_if_fail (G_IS_FILE (file), NULL);

  /* Load the contents of the file */
  if (!g_file_load_contents (file, NULL, &contents, &length, NULL, NULL))
    return NULL;

  /* Open the keyfile */
  rc = g_key_file_new ();
  succeed = g_key_file_load_from_data (rc, contents, length, G_KEY_FILE_NONE, NULL);
  g_free (contents);
  if (G_UNLIKELY (!succeed))
    {
      /* Cleanup and leave */
      g_key_file_free (rc);
      return NULL;
    }

  /* Parse name, exec command and icon name */
  name = g_key_file_get_locale_string (rc, G_KEY_FILE_DESKTOP_GROUP,
                                       G_KEY_FILE_DESKTOP_KEY_NAME, NULL, NULL);
  comment = g_key_file_get_locale_string (rc, G_KEY_FILE_DESKTOP_GROUP,
                                          G_KEY_FILE_DESKTOP_KEY_COMMENT, NULL, NULL);
  icon = g_key_file_get_locale_string (rc, G_KEY_FILE_DESKTOP_GROUP,
                                       G_KEY_FILE_DESKTOP_KEY_ICON, NULL, NULL);
  no_display = g_key_file_get_boolean (rc, G_KEY_FILE_DESKTOP_GROUP,
                                       G_KEY_FILE_DESKTOP_KEY_NO_DISPLAY, NULL);

  /* Allocate a new directory instance */
  directory = g_object_new (GARCON_TYPE_MENU_DIRECTORY,
                            "file", file,
                            "name", name,
                            "comment", comment,
                            "icon", icon,
                            "no-display", no_display,
                            NULL);

  /* Free strings */
  g_free (name);
  g_free (comment);
  g_free (icon);

  /* Set rest of the private data directly */
  directory->priv->only_show_in = g_key_file_get_string_list (rc, G_KEY_FILE_DESKTOP_GROUP,
                                                              G_KEY_FILE_DESKTOP_KEY_ONLY_SHOW_IN, NULL, NULL);
  directory->priv->not_show_in = g_key_file_get_string_list (rc, G_KEY_FILE_DESKTOP_GROUP,
                                                            G_KEY_FILE_DESKTOP_KEY_NOT_SHOW_IN, NULL, NULL);
  directory->priv->hidden = g_key_file_get_boolean (rc, G_KEY_FILE_DESKTOP_GROUP,
                                                    G_KEY_FILE_DESKTOP_KEY_HIDDEN, NULL);

  /* Cleanup */
  g_key_file_free (rc);

  return directory;
}



/**
 * garcon_menu_directory_get_file:
 *
 * Get the file for @directory.
 *
 * Return value: a #GFile. The returned object
 * should be unreffed with g_object_unref() when no longer needed.
 */
GFile *
garcon_menu_directory_get_file (GarconMenuDirectory *directory)
{
  g_return_val_if_fail (GARCON_IS_MENU_DIRECTORY (directory), NULL);
  return g_object_ref (directory->priv->file);
}



const gchar*
garcon_menu_directory_get_name (GarconMenuDirectory *directory)
{
  g_return_val_if_fail (GARCON_IS_MENU_DIRECTORY (directory), NULL);
  return directory->priv->name;
}



void
garcon_menu_directory_set_name (GarconMenuDirectory *directory,
                                const gchar         *name)
{
  g_return_if_fail (GARCON_IS_MENU_DIRECTORY (directory));
  g_return_if_fail (name != NULL);

  /* Free old name */
  if (G_UNLIKELY (directory->priv->name != NULL))
    g_free (directory->priv->name);

  /* Set the new filename */
  directory->priv->name = g_strdup (name);

  /* Notify listeners */
  g_object_notify (G_OBJECT (directory), "name");
}



const gchar*
garcon_menu_directory_get_comment (GarconMenuDirectory *directory)
{
  g_return_val_if_fail (GARCON_IS_MENU_DIRECTORY (directory), NULL);
  return directory->priv->comment;
}



void
garcon_menu_directory_set_comment (GarconMenuDirectory *directory,
                                   const gchar         *comment)
{
  g_return_if_fail (GARCON_IS_MENU_DIRECTORY (directory));

  /* Free old name */
  if (G_UNLIKELY (directory->priv->comment != NULL))
    g_free (directory->priv->comment);

  /* Set the new filename */
  directory->priv->comment = g_strdup (comment);

  /* Notify listeners */
  g_object_notify (G_OBJECT (directory), "comment");
}


const gchar*
garcon_menu_directory_get_icon (GarconMenuDirectory *directory)
{
  g_return_val_if_fail (GARCON_IS_MENU_DIRECTORY (directory), NULL);
  return directory->priv->icon;
}



void
garcon_menu_directory_set_icon (GarconMenuDirectory *directory,
                                const gchar         *icon)
{
  g_return_if_fail (GARCON_IS_MENU_DIRECTORY (directory));

  /* Free old name */
  if (G_UNLIKELY (directory->priv->icon != NULL))
    g_free (directory->priv->icon);

  /* Set the new filename */
  directory->priv->icon = g_strdup (icon);

  /* Notify listeners */
  g_object_notify (G_OBJECT (directory), "icon");
}



gboolean
garcon_menu_directory_get_no_display (GarconMenuDirectory *directory)
{
  g_return_val_if_fail (GARCON_IS_MENU_DIRECTORY (directory), FALSE);
  return directory->priv->no_display;
}



void
garcon_menu_directory_set_no_display (GarconMenuDirectory *directory,
                                      gboolean             no_display)
{
  g_return_if_fail (GARCON_IS_MENU_DIRECTORY (directory));

  /* Abort if old and new value are equal */
  if (directory->priv->no_display == no_display)
    return;

  /* Assign new value */
  directory->priv->no_display = no_display;

  /* Notify listeners */
  g_object_notify (G_OBJECT (directory), "no-display");
}



gboolean
garcon_menu_directory_get_hidden (GarconMenuDirectory *directory)
{
  g_return_val_if_fail (GARCON_IS_MENU_DIRECTORY (directory), FALSE);
  return directory->priv->hidden;
}



gboolean
garcon_menu_directory_get_show_in_environment (GarconMenuDirectory *directory)
{
  const gchar *env;
  guint        i;
  gboolean     show = TRUE;

  g_return_val_if_fail (GARCON_IS_MENU_DIRECTORY (directory), FALSE);

  /* Determine current environment */
  env = garcon_get_environment ();

  /* If no environment has been set, the menu is displayed no matter what
   * OnlyShowIn or NotShowIn contain */
  if (G_UNLIKELY (env == NULL))
    return TRUE;

  /* According to the spec there is either a OnlyShowIn or a NotShowIn list */
  if (G_UNLIKELY (directory->priv->only_show_in != NULL))
    {
      /* Check if your environemnt is in OnlyShowIn list */
      for (i = 0, show = FALSE; !show && directory->priv->only_show_in[i] != NULL; i++)
        if (g_utf8_collate (directory->priv->only_show_in[i], env) == 0)
          show = TRUE;
    }
  else if (G_UNLIKELY (directory->priv->not_show_in != NULL))
    {
      /* Check if your environemnt is in NotShowIn list */
      for (i = 0, show = TRUE; show && directory->priv->not_show_in[i] != NULL; i++)
        if (g_utf8_collate (directory->priv->not_show_in[i], env) == 0)
          show = FALSE;
    }

  return show;
}



gboolean
garcon_menu_directory_get_visible (GarconMenuDirectory *directory)
{
  g_return_val_if_fail (GARCON_IS_MENU_DIRECTORY (directory), FALSE);

  if (!garcon_menu_directory_get_show_in_environment (directory))
    return FALSE;
  else if (garcon_menu_directory_get_hidden (directory))
    return FALSE;
  else if (garcon_menu_directory_get_no_display (directory))
    return FALSE;

  return TRUE;
}



gboolean
garcon_menu_directory_equal (GarconMenuDirectory *directory,
                             GarconMenuDirectory *other)
{
  g_return_val_if_fail (GARCON_IS_MENU_DIRECTORY (directory), FALSE);
  g_return_val_if_fail (GARCON_IS_MENU_DIRECTORY (other), FALSE);
  return g_file_equal (directory->priv->file, other->priv->file);
}