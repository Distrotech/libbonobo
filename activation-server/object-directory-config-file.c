/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */

#include "od-utils.h"
#include <tree.h> /* gnome-xml */
#include <parser.h> /* gnome-xml */
#include <malloc.h>
#include <xmlmemory.h> /* guess what ? gnome-xml !! */
#include <glib.h>

#define OAF_CONFIG_FILE "/oaf/oaf-config.xml"
#define OAF_CONFIG_DEBUG 0

void deb_print (char *string);

void deb_print (char *string)
{
        if (OAF_CONFIG_DEBUG)
                g_print ("%s\n", string);
}

char *
od_utils_load_config_file (void)
{
        char *oaf_config_file;
        char *result;
        xmlDocPtr doc;
        xmlNodePtr search_node;

        oaf_config_file = g_strconcat (OAF_CONFDIR, OAF_CONFIG_FILE, NULL);
        doc = xmlParseFile (oaf_config_file);

        /* check if the document was read successfully. */
        if (doc == NULL) {
                g_warning ("The OAF configuration file was not read "
                           "successfully. Please, check it is valid in: %s",
                           oaf_config_file);
                return NULL;
        }


        search_node = doc->root->childs;
        result = "";
        while (search_node != NULL) {
                if (strcmp (search_node->name, "searchpath") == 0) {
                        xmlNodePtr item_node;
                        item_node = search_node->childs;
                        while (item_node != NULL) {
                                if (strcmp (item_node->name, "item") == 0) {
                                        char *directory;
                                        /* FIXME: this may be slow and has probably
                                           a direct influence on startup time. */
                                        directory = xmlNodeGetContent (item_node);
                                        result = g_strconcat (result, ":", directory, NULL);
                                        xmlFree (directory);
                                        deb_print (result);
                                }
                                item_node = item_node->next;
                        }
                }
                search_node = search_node->next;
        }

        xmlFreeDoc (doc);
        return result;
}


