#ifndef __GNOME_PROPERTY_BAG_XML_H__
#define __GNOME_PROPERTY_BAG_XML_H__

#include <bonobo/gnome-property-bag.h>

BEGIN_GNOME_DECLS

typedef char	  *(*GnomePropertyBagValueToXMLStringFn)      (const char *type, gpointer value, gpointer user_data);
typedef gpointer   (*GnomePropertyBagValueFromXMLStringFn)    (const char *type, const char *str, gpointer user_data);


void     gnome_property_bag_xml_register  (GnomePropertyBag *pb);

gboolean gnome_property_bag_xml_persist   (GnomePropertyBag *pb, const GNOME_Stream stream,
					   gpointer user_data);

gboolean gnome_property_bag_xml_depersist (GnomePropertyBag *pb, const GNOME_Stream stream,
					   gpointer user_data);

void     gnome_property_bag_xml_add_type  (GnomePropertyBag *pb, const char *type,
					   GnomePropertyBagValueToXMLStringFn   to_string,
					   GnomePropertyBagValueFromXMLStringFn from_string,
					   gpointer user_data);

END_GNOME_DECLS

#endif /* ! __GNOME_PROPERTY_BAG_XML_H__ */
