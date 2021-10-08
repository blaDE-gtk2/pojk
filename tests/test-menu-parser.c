/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2009 Jannis Pohlmann <jannis@xfce.org>
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
 * You should have received a copy of the GNU Library General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <glib.h>

#include <pojk/pojk.h>



#define FILENAME "/etc/xdg/menus/applications.menu"



static gboolean print_node (GNode   *node,
                            gpointer depth);



static const gchar *
node_name (GNode *node)
{
  switch (pojk_menu_node_tree_get_node_type (node))
    {
      case POJK_MENU_NODE_TYPE_MENU: return "Menu"; break;
      case POJK_MENU_NODE_TYPE_INCLUDE: return "Include"; break;
      case POJK_MENU_NODE_TYPE_EXCLUDE: return "Exclude"; break;
      case POJK_MENU_NODE_TYPE_OR: return "Or"; break;
      case POJK_MENU_NODE_TYPE_AND: return "And"; break;
      case POJK_MENU_NODE_TYPE_NOT: return "Not"; break;
      case POJK_MENU_NODE_TYPE_MOVE: return "Move"; break;
      case POJK_MENU_NODE_TYPE_DEFAULT_LAYOUT: return "DefaultLayout"; break;
      case POJK_MENU_NODE_TYPE_LAYOUT: return "Layout"; break;
      default: return NULL; break;
    }
}



static void
print_child_nodes (GNode *node,
                   gint   depth)
{
  GNode *child;

  for (child = g_node_first_child (node); child != NULL; child = g_node_next_sibling (child))
    {
      g_node_traverse (child, G_PRE_ORDER, G_TRAVERSE_ALL, 1,
                       (GNodeTraverseFunc) print_node, GINT_TO_POINTER (depth+2));
    }
}



static gboolean
print_node (GNode    *node,
            gpointer  data)
{
  gint i;
  gint depth = GPOINTER_TO_INT (data);

#define INDENT {for (i = 0; i < depth; ++i) g_print (" ");}

  if (pojk_menu_node_tree_get_node_type (node) == POJK_MENU_NODE_TYPE_MENU ||
      pojk_menu_node_tree_get_node_type (node) == POJK_MENU_NODE_TYPE_INCLUDE ||
      pojk_menu_node_tree_get_node_type (node) == POJK_MENU_NODE_TYPE_EXCLUDE ||
      pojk_menu_node_tree_get_node_type (node) == POJK_MENU_NODE_TYPE_OR ||
      pojk_menu_node_tree_get_node_type (node) == POJK_MENU_NODE_TYPE_AND ||
      pojk_menu_node_tree_get_node_type (node) == POJK_MENU_NODE_TYPE_NOT ||
      pojk_menu_node_tree_get_node_type (node) == POJK_MENU_NODE_TYPE_MOVE ||
      pojk_menu_node_tree_get_node_type (node) == POJK_MENU_NODE_TYPE_DEFAULT_LAYOUT ||
      pojk_menu_node_tree_get_node_type (node) == POJK_MENU_NODE_TYPE_LAYOUT)
    {
      INDENT; g_print ("<%s>\n", node_name (node));
      print_child_nodes (node, depth);
      INDENT; g_print ("</%s>\n", node_name (node));
    }
  else
    {
      switch (pojk_menu_node_tree_get_node_type (node))
        {
        case POJK_MENU_NODE_TYPE_NAME:
          INDENT; g_print ("<Name>%s</Name>\n", pojk_menu_node_tree_get_string (node));
          break;
        case POJK_MENU_NODE_TYPE_DIRECTORY:
          INDENT; g_print ("<Directory>%s</Directory>\n", pojk_menu_node_tree_get_string (node));
          break;
        case POJK_MENU_NODE_TYPE_DIRECTORY_DIR:
          INDENT; g_print ("<DirectoryDir>%s</DirectoryDir>\n", pojk_menu_node_tree_get_string (node));
          break;
        case POJK_MENU_NODE_TYPE_DEFAULT_DIRECTORY_DIRS:
          INDENT; g_print ("<DefaultDirectoryDirs/>\n");
          break;
        case POJK_MENU_NODE_TYPE_APP_DIR:
          INDENT; g_print ("<AppDir>%s</AppDir>\n", pojk_menu_node_tree_get_string (node));
          break;
        case POJK_MENU_NODE_TYPE_DEFAULT_APP_DIRS:
          INDENT; g_print ("<DefaultAppDirs/>\n");
          break;
        case POJK_MENU_NODE_TYPE_ONLY_UNALLOCATED:
          INDENT; g_print ("<OnlyUnallocated/>\n");
          break;
        case POJK_MENU_NODE_TYPE_NOT_ONLY_UNALLOCATED:
          INDENT; g_print ("<NotOnlyUnallocated/>\n");
          break;
        case POJK_MENU_NODE_TYPE_DELETED:
          INDENT; g_print ("<Deleted/>\n");
          break;
        case POJK_MENU_NODE_TYPE_NOT_DELETED:
          INDENT; g_print ("<NotDeleted/>\n");
          break;
        case POJK_MENU_NODE_TYPE_FILENAME:
          INDENT; g_print ("<Filename>%s</Filename>\n", pojk_menu_node_tree_get_string (node));
          break;
        case POJK_MENU_NODE_TYPE_CATEGORY:
          INDENT; g_print ("<Category>%s</Category>\n", pojk_menu_node_tree_get_string (node));
          break;
        case POJK_MENU_NODE_TYPE_OLD:
          INDENT; g_print ("<Old>%s</Old>\n", pojk_menu_node_tree_get_string (node));
          break;
        case POJK_MENU_NODE_TYPE_NEW:
          INDENT; g_print ("<New>%s</New>\n", pojk_menu_node_tree_get_string (node));
          break;
        case POJK_MENU_NODE_TYPE_MENUNAME:
          INDENT; g_print ("<Menuname>%s</Menuname>\n", pojk_menu_node_tree_get_string (node));
          break;
        case POJK_MENU_NODE_TYPE_SEPARATOR:
          INDENT; g_print ("<Separator/>\n");
          break;
        case POJK_MENU_NODE_TYPE_MERGE:
          INDENT;
          switch (pojk_menu_node_tree_get_layout_merge_type (node))
            {
            case POJK_MENU_LAYOUT_MERGE_ALL:
              g_print ("<Merge type=\"all\"/>\n");
              break;
            case POJK_MENU_LAYOUT_MERGE_MENUS:
              g_print ("<Merge type=\"menus\"/>\n");
              break;
            case POJK_MENU_LAYOUT_MERGE_FILES:
              g_print ("<Merge type=\"files\"/>\n");
              break;
            }
          break;
        case POJK_MENU_NODE_TYPE_MERGE_FILE:
          INDENT;
          switch (pojk_menu_node_tree_get_merge_file_type (node))
            {
            case POJK_MENU_MERGE_FILE_PATH:
              g_print ("<MergeFile type=\"path\">%s</MergeFile>\n",
                       pojk_menu_node_tree_get_merge_file_filename (node));
              break;
            case POJK_MENU_MERGE_FILE_PARENT:
              g_print ("<MergeFile type=\"parent\"/>\n");
              break;
            }
          break;
        case POJK_MENU_NODE_TYPE_MERGE_DIR:
          INDENT; g_print ("<MergeDir>%s</MergeDir>\n", pojk_menu_node_tree_get_string (node));
          break;
        case POJK_MENU_NODE_TYPE_DEFAULT_MERGE_DIRS:
          INDENT; g_print ("<DefaultMergeDirs/>\n");
          break;
        default:
          break;
        }
    }

  return FALSE;
}



static void
print_tree (PojkMenuTreeProvider *provider)
{
  GNode *tree;

  g_return_if_fail (POJK_IS_MENU_TREE_PROVIDER (provider));

  tree = pojk_menu_tree_provider_get_tree (provider);
  print_node (tree, 0);
  pojk_menu_node_tree_free (tree);
}



int
main (int    argc,
      char **argv)
{
  PojkMenuParser *parser;
  PojkMenuMerger *merger;
  const gchar      *prefix;
  GError           *error = NULL;
  GFile            *file = NULL;
  gchar            *filename;
  gchar            *relative_filename;
  gint              result = EXIT_SUCCESS;

#if !GLIB_CHECK_VERSION (2, 36, 0)
  /* Initialize the type system */
  g_type_init ();
#endif

#if !GLIB_CHECK_VERSION(2,32,0)
  if (!g_thread_supported ())
    g_thread_init (NULL);
#endif

  pojk_set_environment ("XFCE");

  if (argc > 1)
    file = g_file_new_for_path (argv[1]);
  else
    {
      prefix = g_getenv ("XDG_MENU_PREFIX");
      relative_filename = g_strconcat ("menus", G_DIR_SEPARATOR_S,
                                       prefix != NULL ? prefix : "", "applications.menu",
                                       NULL);

      /* Search for the menu file */
      filename = pojk_config_lookup (relative_filename);
      if (G_UNLIKELY (filename != NULL))
        {
          /* Try to load the root menu from this file */
          file = g_file_new_for_path (filename);
          g_free (filename);

          if (!g_file_query_exists (file, NULL))
            {
              g_object_unref (file);
              file = NULL;
            }
        }

      g_free (relative_filename);
    }

  filename = g_file_get_path (file);
  g_print ("\nread menu from \"%s\"\n", filename);
  g_free (filename);

  parser = pojk_menu_parser_new (file);
  g_object_unref (file);

  if (G_LIKELY (pojk_menu_parser_run (parser, NULL, &error)))
    {
      g_print("\n\nafter parsing:\n\n");

      print_tree (POJK_MENU_TREE_PROVIDER (parser));

      merger = pojk_menu_merger_new (POJK_MENU_TREE_PROVIDER (parser));

      if (G_LIKELY (pojk_menu_merger_run (merger, NULL, NULL, NULL, &error)))
        {
          g_print ("\n\nafter merging:\n\n");
          print_tree (POJK_MENU_TREE_PROVIDER (merger));
        }
      else
        {
          if (error != NULL)
            {
              g_error ("Could not merge menus in %s: %s",
                       argc > 1 ? argv[1] : FILENAME,
                       error->message);

              g_error_free (error);
            }

          result = EXIT_FAILURE;
        }

      g_object_unref (merger);
    }
  else
    {
      if (error != NULL)
        {
          g_error ("Could not parse %s: %s", argc > 1 ? argv[1] : FILENAME, error->message);
          g_error_free (error);
        }

      result = EXIT_FAILURE;
    }

  g_object_unref (parser);

  return result;
}
