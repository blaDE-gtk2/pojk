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

#include <garcon/garcon-menu-element.h>



GType
garcon_menu_element_get_type (void)
{
  static volatile gsize type__volatile = 0;
  GType                 type;

  if (g_once_init_enter (&type__volatile))
    {
      type = g_type_register_static_simple (G_TYPE_INTERFACE,
                                            g_intern_static_string ("GarconMenuElement"),
                                            sizeof (GarconMenuElementIface),
                                            NULL,
                                            0,
                                            NULL,
                                            0);

      g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);

      g_once_init_leave (&type__volatile, type);
    }

  return type__volatile;
}



const gchar*
garcon_menu_element_get_name (GarconMenuElement *element)
{
  g_return_val_if_fail (GARCON_IS_MENU_ELEMENT (element), NULL);
  return (*GARCON_MENU_ELEMENT_GET_IFACE (element)->get_name) (element);
}



const gchar*
garcon_menu_element_get_comment (GarconMenuElement *element)
{
  g_return_val_if_fail (GARCON_IS_MENU_ELEMENT (element), NULL);
  return (*GARCON_MENU_ELEMENT_GET_IFACE (element)->get_comment) (element);
}



const gchar*
garcon_menu_element_get_icon_name (GarconMenuElement *element)
{
  g_return_val_if_fail (GARCON_IS_MENU_ELEMENT (element), NULL);
  return (*GARCON_MENU_ELEMENT_GET_IFACE (element)->get_icon_name) (element);
}



gboolean
garcon_menu_element_get_visible (GarconMenuElement *element)
{
  g_return_val_if_fail (GARCON_IS_MENU_ELEMENT (element), FALSE);
  return (*GARCON_MENU_ELEMENT_GET_IFACE (element)->get_visible) (element);
}



gboolean
garcon_menu_element_get_show_in_environment (GarconMenuElement *element)
{
  g_return_val_if_fail (GARCON_IS_MENU_ELEMENT (element), FALSE);
  return (*GARCON_MENU_ELEMENT_GET_IFACE (element)->get_show_in_environment) (element);
}



gboolean
garcon_menu_element_get_no_display (GarconMenuElement *element)
{
  g_return_val_if_fail (GARCON_IS_MENU_ELEMENT (element), FALSE);
  return (*GARCON_MENU_ELEMENT_GET_IFACE (element)->get_no_display) (element);
}
