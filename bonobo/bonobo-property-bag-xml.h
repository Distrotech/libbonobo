#ifndef __BONOBO_PROPERTY_BAG_XML_H__
#define __BONOBO_PROPERTY_BAG_XML_H__

#include <bonobo/bonobo-property-bag.h>

BEGIN_GNOME_DECLS

typedef char	  *(*BonoboPropertyBagValueToXMLStringFn)      (const char *type, gpointer value, gpointer user_data);
typedef gpointer   (*BonoboPropertyBagValueFromXMLStringFn)    (const char *type, const char *str, gpointer user_data);


void     bonobo_property_bag_xml_register  (BonoboPropertyBag *pb);

gboolean bonobo_property_bag_xml_persist   (BonoboPropertyBag *pb, const Bonobo_Stream stream,
					   gpointer user_data);

gboolean bonobo_property_bag_xml_depersist (BonoboPropertyBag *pb, const Bonobo_Stream stream,
					   gpointer user_data);

void     bonobo_property_bag_xml_add_type  (BonoboPropertyBag *pb, const char *type,
					   BonoboPropertyBagValueToXMLStringFn   to_string,
					   BonoboPropertyBagValueFromXMLStringFn from_string,
					   gpointer user_data);

END_GNOME_DECLS

#endif /* ! __BONOBO_PROPERTY_BAG_XML_H__ */
