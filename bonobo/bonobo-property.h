
#ifndef __BONOBO_PROPERTY_H__
#define __BONOBO_PROPERTY_H__

/*
 * Every function and data structure in this file is private and
 * should never be accessed directly by a PropertyBag user, unless
 * he is implementing his own persistence mechanism.  Otherwise,
 * no peeking.
 */

#include <bonobo/bonobo-property-bag.h>

PortableServer_Servant bonobo_property_servant_new     (PortableServer_POA adapter, BonoboPropertyBag *pb,
							char *property_name);
void		       bonobo_property_servant_destroy (PortableServer_Servant servant);

typedef struct {
	char			*name;
	int                      idx;
	BonoboArgType            type;
	BonoboArg               *default_value;
	char			*docstring;
	BonoboPropertyFlags	 flags;

	BonoboPropertyGetFn      get_prop;
	BonoboPropertySetFn      set_prop;
	gpointer                 user_data;
} BonoboProperty;

#endif /* ! __BONOBO_PROPERTY_H__ */
