/*
 * Copyright © 2022 Matthew Leeds
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *       Matthew Leeds <mwleeds@protonmail.com>
 */

#include "config.h"

#include <locale.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <glib/gi18n.h>

#include "libglnx.h"

#include "flatpak-builtins.h"
#include "flatpak-builtins-utils.h"
#include "flatpak-cli-transaction.h"
#include "flatpak-quiet-transaction.h"
#include "flatpak-utils-private.h"
#include "flatpak-error.h"
#include "flatpak-table-printer.h"

static gboolean opt_remove;

static GOptionEntry options[] = {
  { "remove", 0, 0, G_OPTION_ARG_NONE, &opt_remove, N_("Remove matching pins"), NULL },
  { NULL }
};

gboolean
flatpak_builtin_alias (int argc, char **argv, GCancellable *cancellable, GError **error)
{
  g_autoptr(GOptionContext) context = NULL;
  g_autoptr(GPtrArray) dirs = NULL;
  FlatpakDir *dir;

  context = g_option_context_new (_("REF ALIAS - Add an alias for running the app REF"));
  g_option_context_set_translation_domain (context, GETTEXT_PACKAGE);

  if (!flatpak_option_context_parse (context, options, &argc, &argv,
                                     FLATPAK_BUILTIN_FLAG_ONE_DIR,
                                     &dirs, cancellable, error))
    return FALSE;

  dir = g_ptr_array_index (dirs, 0);

  g_print ("argc = %d\n", argc);
  if (argc == 1)
    {
      g_autoptr(GHashTable) aliases = NULL; /* alias → app-id */

      aliases = flatpak_dir_get_aliases (dir);

      if (g_hash_table_size (aliases) == 0)
        {
          if (flatpak_fancy_output ())
            g_print (_("No aliases\n"));
        }
      else
        {
          g_autoptr(FlatpakTablePrinter) printer = NULL;

          printer = flatpak_table_printer_new ();
          flatpak_table_printer_set_column_title (printer, 0, _("Alias"));
          flatpak_table_printer_set_column_title (printer, 1, _("App"));

          GLNX_HASH_TABLE_FOREACH_KV (aliases, const char *, alias, const char *, app_id)
            {
              flatpak_table_printer_add_column (printer, alias);
              flatpak_table_printer_add_column (printer, app_id);
              flatpak_table_printer_finish_row (printer);
            }

          flatpak_table_printer_print (printer);
        }
    }
  else if (argc != 3)
    return usage_error (context, _("Wrong number of arguments"), error);
  else
    {
      //TODO actually handle creating or removing an alias
    }

  return TRUE;
}

gboolean
flatpak_complete_alias (FlatpakCompletion *completion)
{
  g_autoptr(GOptionContext) context = NULL;
  g_autoptr(GPtrArray) dirs = NULL;
  FlatpakDir *dir;

  context = g_option_context_new ("");
  if (!flatpak_option_context_parse (context, options, &completion->argc, &completion->argv,
                                     FLATPAK_BUILTIN_FLAG_ONE_DIR | FLATPAK_BUILTIN_FLAG_OPTIONAL_REPO,
                                     &dirs, NULL, NULL))
    return FALSE;

  dir = g_ptr_array_index (dirs, 0);

  switch (completion->argc)
    {
    case 0:
    case 1: /* REF */
      flatpak_complete_options (completion, global_entries);
      flatpak_complete_options (completion, options);
      flatpak_complete_options (completion, user_entries);
      flatpak_complete_partial_ref (completion, FLATPAK_KINDS_APP, NULL /* arch */,
                                    dir, NULL /* remote */);
      break;
    }

  return TRUE;
}
