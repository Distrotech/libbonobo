/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-ui-node.h: Code to manipulate BonoboNode objects
 *
 * Author:
 *	Havoc Pennington <hp@redhat.com>
 *
 * Copyright 2000 Red Hat, Inc.
 */
#ifndef _BONOBO_NODE_H_
#define _BONOBO_NODE_H_

#include <glib.h>

typedef struct _BonoboNode BonoboNode;

BonoboNode *bonobo_node_new           (const char   *name);
BonoboNode *bonobo_node_new_child     (BonoboNode *parent,
				       const char   *name);
BonoboNode *bonobo_node_copy          (BonoboNode *node,
				       gboolean      recursive);
void        bonobo_node_free          (BonoboNode *node);
void        bonobo_node_set_data      (BonoboNode *node,
				       gpointer      data);
gpointer    bonobo_node_get_data      (BonoboNode *node);
void        bonobo_node_set_attr      (BonoboNode *node,
				       const char   *name,
				       const char   *value);
char *      bonobo_node_get_attr      (BonoboNode *node,
				       const char   *name);
gboolean    bonobo_node_has_attr      (BonoboNode *node,
				       const char   *name);
void        bonobo_node_remove_attr   (BonoboNode *node,
				       const char   *name);
void        bonobo_node_add_child     (BonoboNode *parent,
				       BonoboNode *child);
void        bonobo_node_insert_before (BonoboNode *sibling,
				       BonoboNode *prev_sibling);
void        bonobo_node_unlink        (BonoboNode *node);
void        bonobo_node_replace       (BonoboNode *old_node,
				       BonoboNode *new_node);
void        bonobo_node_set_content   (BonoboNode *node,
				       const char   *content);
char       *bonobo_node_get_content   (BonoboNode *node);
BonoboNode *bonobo_node_next          (BonoboNode *node);
BonoboNode *bonobo_node_prev          (BonoboNode *node);
BonoboNode *bonobo_node_children      (BonoboNode *node);
BonoboNode *bonobo_node_parent        (BonoboNode *node);
const char *bonobo_node_get_name      (BonoboNode *node);
gboolean    bonobo_node_has_name      (BonoboNode *node,
				       const char   *name);
gboolean    bonobo_node_transparent   (BonoboNode *node);
void        bonobo_node_copy_attrs    (BonoboNode *src,
				       BonoboNode *dest);

/* This blows. libxml2 fixes it I guess. */
void        bonobo_node_free_string   (char *str);
void        bonobo_node_strip         (BonoboNode **node);

char       *bonobo_node_to_string     (BonoboNode *node,
				       gboolean      recurse);
BonoboNode *bonobo_node_from_string   (const char *str);
BonoboNode *bonobo_node_from_file     (const char *filename);

#endif /* _BONOBO_NODE_H_ */
