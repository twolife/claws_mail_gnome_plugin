/*
 * Gnome plugin for Claws-Mail
 *  Copyright (C) 2009 Holger Berndt
 *  Copyright (C) 2012-2016 Sébastien Noel
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "common/version.h"
#include "common/plugin.h"
#include "common/hooks.h"
#include "account.h"
#include "addr_compl.h"
#include "common/utils.h"

#include <libsecret/secret.h>
#include <libebook/libebook.h>

static guint hook_address_completion;
static guint hook_password;


// LIBSECRET

const SecretSchema * get_generic_schema (void)
{
    static const SecretSchema the_schema = {
        "org.freedesktop.Secret.Generic", SECRET_SCHEMA_NONE,
        {
            {  "user", SECRET_SCHEMA_ATTRIBUTE_STRING },
            {  "server", SECRET_SCHEMA_ATTRIBUTE_STRING },
            {  "protocol", SECRET_SCHEMA_ATTRIBUTE_STRING },
            {  NULL, 0 },
        }
    };
    return &the_schema;
}
#define GENERIC_SCHEMA  get_generic_schema ()

static gboolean password_get_hook(gpointer source, gpointer hook_data) {
    PasswordRequest *req = source;
    GError* error = NULL;
    gchar *pass;

    // "full search"
    debug_print("LIBSECRET:: user: %s, server: %s, proto: %s \n",
                             req->user, req->server, req->protocol);
    pass = secret_password_lookup_sync(GENERIC_SCHEMA, NULL, &error,
            "user", req->user,
            "server", req->server,
            "protocol", req->protocol,
            NULL);
    if (pass != NULL) {
        req->password = g_strdup(pass);
        secret_password_free(pass);
        return TRUE;
    }
    else if (error != NULL) {
        debug_print("libsecret access failed: %s.", error->message);
        g_error_free(error);
        return FALSE;
    }

    // fallback
    debug_print("LIBSECRET:: user: %s, server: %s \n",
                             req->user, req->server);
    pass = secret_password_lookup_sync(GENERIC_SCHEMA, NULL, &error,
            "user", req->user,
            "server", req->server,
            NULL);
    if (pass != NULL) {
        req->password = g_strdup(pass);
        secret_password_free(pass);
        return TRUE;
    }
    else if (error != NULL) {
        debug_print("libsecret access failed: %s.", error->message);
        g_error_free(error);
        return FALSE;
    }

    return FALSE;
}


// EDS

static gboolean my_address_completion_build_list_hook(gpointer, gpointer);
static GList *eds_books = NULL;
static gboolean eds_waiting = FALSE;

static void eds_contacts_added_cb(EBookView *view, const GList *contacts, gpointer data)
{
  const GList *walk;
  GList **address_list = (GList**) data;

  for(walk = contacts; walk; walk = walk->next) {
    const char *name;
    GList *email_list, *email_entry;
    EContact *contact = walk->data;

    if(!E_IS_CONTACT(contact))
      continue;

    name = e_contact_get_const(contact, E_CONTACT_FULL_NAME);
    email_list = e_contact_get(contact, E_CONTACT_EMAIL);
    for(email_entry = email_list; email_entry; email_entry = email_entry->next) {
      address_entry *ae;
      const char *email_address = email_entry->data;

      ae = g_new0(address_entry, 1);
      ae->name = g_strdup(name);
      ae->address = g_strdup(email_address);
      ae->grp_emails = NULL;
      *address_list = g_list_prepend(*address_list, ae);

      addr_compl_add_address1(name, ae);
      if(email_address && *email_address != '\0')
        addr_compl_add_address1(email_address, ae);
    }
  }
}

static void eds_sequence_complete_cb(EBookView *view, const GList *contacts, gpointer data)
{
  eds_waiting = FALSE;
}

static void add_gnome_addressbook(GList **address_list)
{
  ESourceRegistry * registry = NULL;
  GError *error = NULL;
  GList *a;

  registry = e_source_registry_new_sync (NULL, &error);

  if (!registry || error) {
    debug_print("Error: Failed to get access to source registry: %s\n", error->message);
    g_error_free(error);
    return;
  }

  // create book accessor if necessary
  if(!eds_books) {
    GList *list_sources = e_source_registry_list_sources (registry, E_SOURCE_EXTENSION_ADDRESS_BOOK);
    for (a = list_sources; a; a = a->next) {
      ESource *source = E_SOURCE (a->data);
      if (e_source_get_enabled(source)) {
        EBook *eds_book = e_book_new(source, &error);

        if(!eds_book) {
          g_list_free_full(list_sources, g_object_unref);
          debug_print("Error: Could not get eds addressbook: %s\n", error->message);
          g_error_free(error);
          return;
        }
        eds_books = g_list_append (eds_books, eds_book);
      }
    }
    g_list_free_full(list_sources, g_object_unref);
  }

  for (a = eds_books; a; a = a->next) {
    EBook *eds_book = a->data;
    EBookQuery *query;
    EBookView *view;

    // open book if necessary
    if(!e_book_is_opened(eds_book) && !e_book_open(eds_book, TRUE, &error)) {
      debug_print("Error: Could not open eds addressbook: %s\n", error->message);
      g_error_free(error);
      return;
    }

    // query book
    query = e_book_query_field_exists(E_CONTACT_EMAIL);
    if(!e_book_get_book_view(eds_book, query, NULL, 0, &view, &error)) {
      debug_print("Error: Could not get eds addressbook view: %s\n", error->message);
      g_error_free(error);
    }
    e_book_query_unref(query);

    g_signal_connect(G_OBJECT(view), "contacts-added", G_CALLBACK(eds_contacts_added_cb), address_list);
    g_signal_connect(G_OBJECT(view), "sequence-complete", G_CALLBACK(eds_sequence_complete_cb), NULL);

    eds_waiting = TRUE;
    e_book_view_start(view);

    while(eds_waiting)
      gtk_main_iteration();

    e_book_view_stop(view);
    g_object_unref(view);
  }

}

static gboolean my_address_completion_build_list_hook(gpointer source, gpointer data)
{
  add_gnome_addressbook(source);
  return FALSE;
}


// MAIN

gint plugin_init(gchar **error)
{
  /* Version check */
  if(!check_plugin_version(MAKE_NUMERIC_VERSION(3,9,3,0),
                           VERSION_NUMERIC, "Gnome", error))
    return -1;

  hook_address_completion = hooks_register_hook(ADDDRESS_COMPLETION_BUILD_ADDRESS_LIST_HOOKLIST,
                                                my_address_completion_build_list_hook, NULL);

  if(hook_address_completion == (guint) -1) {
    *error = g_strdup("Failed to register address completion hook in the Gnome plugin");
    return -1;
  }

  hook_password = hooks_register_hook(PASSWORD_GET_HOOKLIST, &password_get_hook, NULL);
  if(hook_password == (guint) -1) {
    *error = g_strdup("Failed to register password hook in the Gnome plugin");
    return -1;
  }

  debug_print("Gnome plugin loaded\n");

  return 0;
}

gboolean plugin_done(void)
{
  hooks_unregister_hook(PASSWORD_GET_HOOKLIST, hook_password);
  hooks_unregister_hook(ADDDRESS_COMPLETION_BUILD_ADDRESS_LIST_HOOKLIST, hook_address_completion);

  if(eds_books) {
    g_list_free_full(eds_books, g_object_unref);
    eds_books = NULL;
  }

  /* Returning FALSE here means that g_module_close() will not be called on the plugin.
   * This is necessary, as libebook is not designed to be unloaded (static type registration,
   * async callbacks registered in the main loop without a finalize() cleanup function etc.). */
  debug_print("Gnome plugin done and hidden.\n");
  return FALSE;
}

const gchar *plugin_name(void)
{
  return "Gnome";
}

const gchar *plugin_desc(void)
{
  return "This plugin provides Gnome integration features:\n"
           " - adding the Gnome addressbook to the address completion\n"
           " - obtains passwords for e-mail accounts from libsecret\n"
          ;
}

const gchar *plugin_type(void)
{
  return "GTK2";
}

const gchar *plugin_licence(void)
{
  return "GPL3+";
}

const gchar *plugin_version(void)
{
  return PACKAGE_VERSION;
}

struct PluginFeature *plugin_provides(void)
{
  static struct PluginFeature features[] =
    { {PLUGIN_UTILITY, "Gnome integration"},
      {PLUGIN_NOTHING, NULL}};
  return features;
}

