#include <config.h>
#include <stdlib.h>
#include <libbonobo.h>

int
main (int argc, char *argv [])
{
	BonoboObject *object;

	free (malloc (8));

	if (bonobo_init (&argc, argv) == FALSE)
		g_error ("Can not bonobo_init");

	object = BONOBO_OBJECT (g_object_new (bonobo_moniker_get_type (), NULL));
	
	g_assert (bonobo_object (object) == object);
	g_assert (bonobo_object (&object->servant) == object);

	bonobo_object_unref (BONOBO_OBJECT (object));

	return 0;
}
