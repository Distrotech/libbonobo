
#ifndef __GNOME_PROPERTY_H__
#define __GNOME_PROPERTY_H__

/*
 * Every function and data structure in this file is private and should never be accessed
 * directly by a PropertyBag user.  So stop peeking.
 */

#include <bonobo/gnome-property-bag.h>

PortableServer_Servant gnome_property_servant_new     (PortableServer_POA adapter, GnomePropertyBag *pb,
							char *property_name);
void		       gnome_property_servant_destroy (PortableServer_Servant servant);

typedef struct {
	char			*name;
	char			*type;
	gpointer		 value;
	gpointer		 default_value;
	char			*docstring;
	GnomePropertyFlags	 flags;
} GnomeProperty;


#endif /* ! __GNOME_PROPERTY_H__ */
