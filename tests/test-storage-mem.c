/* -*- mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include <bonobo/bonobo-storage-memory.h>

#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-exception.h>

int
main (int argc, char *argv [])
{
	BonoboObject                 *storage;
	Bonobo_Storage                corba_storage;
	CORBA_Environment             ev;
	Bonobo_StorageInfo           *info;
	Bonobo_Storage_DirectoryList *dir_list;
	
	if (!bonobo_init (&argc, argv))
		g_error ("bonobo_init failed");

	storage = bonobo_storage_mem_create ();
	corba_storage = bonobo_object_corba_objref (storage);
	
	CORBA_exception_init (&ev);

	printf ("creating storage:\t");
	Bonobo_Storage_openStorage (corba_storage,
				   "/foo",
				   Bonobo_Storage_CREATE,
				   &ev);
	if (!BONOBO_EX (&ev))
		printf ("passed\t'/foo'\n");

	printf ("creating sub-storage:\t");
	Bonobo_Storage_openStorage (corba_storage,
				   "/foo/bar",
				   Bonobo_Storage_CREATE,
				   &ev);
	if (!BONOBO_EX (&ev))
		printf ("passed\t'/foo/bar'\n");
		
	
	printf ("creating stream:\t");
	Bonobo_Storage_openStream (corba_storage,
				   "/foo/bar/baz",
				   Bonobo_Storage_CREATE,
				   &ev);
	if (!BONOBO_EX (&ev))
		printf ("passed\t'/foo/bar/baz'\n");

	printf ("creating stream:\t");
	Bonobo_Storage_openStream (corba_storage,
				   "/foo/quux",
				   Bonobo_Storage_CREATE,
				   &ev);
	if (!BONOBO_EX (&ev))
		printf ("passed\t'/foo/quux'\n");

	printf ("opening stream:\t\t");
	Bonobo_Storage_openStream (corba_storage,
				   "/foo/quux",
				   Bonobo_Storage_READ,
				   &ev);
	if (!BONOBO_EX (&ev))
		printf ("passed\n");
	else {
		printf ("failed: %s\n", BONOBO_EX_REPOID (&ev));
		CORBA_exception_free (&ev);
		CORBA_exception_init (&ev);
	}

	printf ("opening missing stream:\t");
	Bonobo_Storage_openStream (corba_storage,
				   "/foo/dummy",
				   Bonobo_Storage_READ,
				   &ev);
	if (!BONOBO_EX (&ev))
		printf ("passed\n");
	else {
		printf ("failed: %s\n", BONOBO_EX_REPOID (&ev));
		CORBA_exception_free (&ev);
		CORBA_exception_init (&ev);
	}

	printf ("rename (storage):\t");
	Bonobo_Storage_rename (corba_storage,
			       "/foo", "/renamed",
			       &ev);
	if (!BONOBO_EX (&ev))
		printf ("passed\t'%s' -> '%s'\n",
			"/foo", "/renamed");
		
	else {
		printf ("failed: %s\n", BONOBO_EX_REPOID (&ev));
		CORBA_exception_free (&ev);
		CORBA_exception_init (&ev);
	}
	
	printf ("getInfo (storage):\t");
	info = Bonobo_Storage_getInfo (corba_storage,
				       "/renamed",
				       Bonobo_FIELD_TYPE,
				       &ev);
	if (!BONOBO_EX (&ev)) {
		printf ("passed\n");
		printf ("\t\t\t\tname:\t%s\n", info->name);
		printf ("\t\t\t\ttype:\t%s\n",
			info->type ? "storage" : "stream" );

		CORBA_free (info);
		
	} else {
		printf ("failed: %s\n", BONOBO_EX_REPOID (&ev));
		CORBA_exception_free (&ev);
		CORBA_exception_init (&ev);
	}

	printf ("getInfo (stream):\t");
	info = Bonobo_Storage_getInfo (corba_storage,
				       "/renamed/quux",
				       Bonobo_FIELD_TYPE | Bonobo_FIELD_SIZE | Bonobo_FIELD_CONTENT_TYPE,
				       &ev);
	if (!BONOBO_EX (&ev)) {
		printf ("passed\n");
		printf ("\t\t\t\tname:\t%s\n", info->name);
		printf ("\t\t\t\ttype:\t%s\n",
			info->type ? "storage" : "stream" );
		printf ("\t\t\t\tmime:\t%s\n", info->content_type);

		CORBA_free (info);
		
	} else {
		printf ("failed: %s\n", BONOBO_EX_REPOID (&ev));
		CORBA_exception_free (&ev);
		CORBA_exception_init (&ev);
	}

	printf ("getInfo (root):\t\t");
	info = Bonobo_Storage_getInfo (corba_storage,
				       "/",
				       Bonobo_FIELD_TYPE,
				       &ev);
	if (!BONOBO_EX (&ev)) {
		printf ("passed\n");
		printf ("\t\t\t\tname:\t'%s'\n", info->name);
		printf ("\t\t\t\ttype:\t%s\n",
			info->type ? "storage" : "stream" );

		CORBA_free (info);
		
	} else {
		printf ("failed: %s\n", BONOBO_EX_REPOID (&ev));
		CORBA_exception_free (&ev);
		CORBA_exception_init (&ev);
	}

	
	printf ("listContents:\t\t");
	dir_list = Bonobo_Storage_listContents (corba_storage,
						"/renamed",
						0,
						&ev);
	if (!BONOBO_EX (&ev)) {
		int i;

		printf ("passed\n");

		for (i = 0; i < dir_list->_length; i++)
			printf ("\t\t\t\t%s%c\n",
				dir_list->_buffer[i].name,
				dir_list->_buffer[i].type ? '/' : ' ');
		
		CORBA_free (dir_list);
		
	} else {
		printf ("failed: %s\n", BONOBO_EX_REPOID (&ev));
		CORBA_exception_free (&ev);
		CORBA_exception_init (&ev);
	}
	
	printf ("erase (non-empty strg):\t");
	Bonobo_Storage_erase (corba_storage,
			      "/renamed/bar",
			      &ev);
	if (!BONOBO_EX (&ev))
		printf ("passed\n");
	else {
		printf ("failed: %s\n", BONOBO_EX_REPOID (&ev));
		CORBA_exception_free (&ev);
		CORBA_exception_init (&ev);
	}

	printf ("erase (stream):\t\t");
	Bonobo_Storage_erase (corba_storage,
			      "/renamed/bar/baz",
			      &ev);
	if (!BONOBO_EX (&ev))
		printf ("passed\n");
	else {
		printf ("failed: %s\n", BONOBO_EX_REPOID (&ev));
		CORBA_exception_free (&ev);
		CORBA_exception_init (&ev);
	}

	printf ("erase (empty storage):\t");
	Bonobo_Storage_erase (corba_storage,
			      "/renamed/bar",
			      &ev);
	if (!BONOBO_EX (&ev))
		printf ("passed\n");
	else {
		printf ("failed: %s\n", BONOBO_EX_REPOID (&ev));
		CORBA_exception_free (&ev);
		CORBA_exception_init (&ev);
	}


	printf ("getInfo (dltd stream):\t");
	info = Bonobo_Storage_getInfo (corba_storage,
				       "/renamed/bar/baz",
				       Bonobo_FIELD_TYPE | Bonobo_FIELD_SIZE | Bonobo_FIELD_CONTENT_TYPE,
				       &ev);
	if (!BONOBO_EX (&ev)) {
		printf ("passed\n");
		printf ("\t\t\t\tname:\t%s\n", info->name);
		printf ("\t\t\t\ttype:\t%s\n",
			info->type ? "storage" : "stream" );
		printf ("\tmime:\t%s\n", info->content_type);

		CORBA_free (info);
		
	} else {
		printf ("failed: %s\n", BONOBO_EX_REPOID (&ev));
		CORBA_exception_free (&ev);
		CORBA_exception_init (&ev);
	}


	printf ("getInfo (dltd storage):\t");
	info = Bonobo_Storage_getInfo (corba_storage,
				       "/renamed/bar",
				       Bonobo_FIELD_TYPE,
				       &ev);
	if (!BONOBO_EX (&ev)) {
		printf ("passed\n");
		printf ("\t\t\t\tname:\t%s\n", info->name);
		printf ("\t\t\t\ttype:\t%s\n",
			info->type ? "storage" : "stream" );

		CORBA_free (info);
		
	} else {
		printf ("failed: %s\n", BONOBO_EX_REPOID (&ev));
		CORBA_exception_free (&ev);
		CORBA_exception_init (&ev);
	}


	printf ("listContents (deleted):\t");
	dir_list = Bonobo_Storage_listContents (corba_storage,
						"/renamed/bar",
						0,
						&ev);
	if (!BONOBO_EX (&ev)) {
		int i;

		printf ("passed\n");

		for (i = 0; i < dir_list->_length; i++)
			printf ("\t\t\t\t%s%c\n",
				dir_list->_buffer[i].name,
				dir_list->_buffer[i].type ? '/' : ' ');
		
		CORBA_free (dir_list);
		
	} else {
		printf ("failed: %s\n", BONOBO_EX_REPOID (&ev));
		CORBA_exception_free (&ev);
		CORBA_exception_init (&ev);
	}
	
	CORBA_exception_free (&ev);
	
	return 0;
}
