/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Skeleton files. Not yet finished
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * Copyright 1999 Helix Code, Inc.
 */
 
#include <config.h>
#include <bonobo/gnome-embeddable-io.h>

BonoboEmbeddable *
bonobo_embeddable_load (BonoboStorage *storage, const char *interface, BonoboClientSite *client_site)
{
	/*
	 * 1. Get the class ID from the open Storage, by
	 * using IStorage::Stat.
	 */

	/*
	 * 2. Create an instance of the object.
	 */

	/*
	 * 3. Call Embeddable::SetClientSite to inform about the client
	 * site
	 */

	/*
	 * 4. Call the object's QueryInterface and find out about
	 * the PersistStorage pointer.  If found, use
	 * PersistStorage::Load to load the state from.
	 */

	/*
	 * 5. Queries for @interface and returns this
	 */
}

/*
 *
 */
BonoboEmbeddable *
bonobo_embeddable_load_from_stream (BonoboStream *stream, const char *interface)
{
	/*
	 * 1. Load the class id from @stream
	 */

	/*
	 * 2. Create an instance of the object
	 */

	/*
	 * 3. QueryInterface for PersistStream
	 */

	/*
	 * 4. Call PersistStream::Load
	 */
}

int
bonobo_embeddable_save (BonoboEmbeddable *bonobo_object, BonoboStorage *storage, gboolean same_as_loaded)
{
	GnomePersisStorage *persist_storage;
	char *class;
	
	persist_storage = bonobo_object_query_interface (
		bonobo_object, "IDL:Bonobo/PersistStorage:1.0");

	if (persist_storage == NULL)
		return -1;

	class = bonobo_persist_storage_get_class_id (persist_storage);
	bonobo_storage_write_class_id (persist_storage, class);
	
	if (bonobo_persist_storage_save (persist_storage, same_as_loaded) == 0){
		bonobo_persist_storage_commit (persist_storage);
	}

	return 0;
}

int
bonobo_embeddable_save_to_stream (BonoboEmbeddable *bonobo_object, BonoboStream *stream)
{
	GnomePersisStream *persist_stream;
	const char *goad_id;
	
	persist_stream = bonobo_object_query_interface (
		bonobo_object, "IDL:Bonobo/PersistStream:1.0");

	if (persist_stream == NULL)
		return -1;

	goad_id = bonobo_persist_stream_get_goad_id (persist_stream);
	bonobo_stream_write_goad_id (persist_stream, goad_id);
	
	bonobo_persist_stream_save (persist_stream);

	return 0;
}
