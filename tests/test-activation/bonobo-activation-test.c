#include <liboaf/liboaf.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	CORBA_Object obj;
	CORBA_Environment ev;

	CORBA_exception_init(&ev);
	oaf_orb_init(&argc, argv);

//	putenv("OAF_BARRIER_INIT=1");
	obj = oaf_activate("repo_ids.has('IDL:GNOME/Unknown:1.0')", NULL, 0, &ev);

	if(CORBA_Object_is_nil(obj, &ev)) {
		g_warning("Activation failed!");
	} else if(ev._major != CORBA_NO_EXCEPTION) {
		g_warning("Activation failed: %s\n", CORBA_exception_id(&ev));
	} else
		while(1) g_main_iteration(TRUE);

	CORBA_exception_free(&ev);

	return 0;
}
