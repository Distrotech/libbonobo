#include <config.h>
#include <bonobo/gnome-embeddable-io.h>

GnomeEmbeddable *
gnome_embeddable_load (GnomeStorage *storage, const char *interface, GnomeClientSite *client_site)
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
GnomeEmbeddable *
gnome_embeddable_load_from_stream (GnomeStream *stream, const char *interface)
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
gnome_embeddable_save (GnomeEmbeddable *bonobo_object, GnomeStorage *storage, gboolean same_as_loaded)
{
	GnomePersisStorage *persist_storage;
	char *class;
	
	persist_storage = gnome_object_query_interface (
		bonobo_object, "IDL:GNOME/PersistStorage:1.0");

	if (persist_storage == NULL)
		return -1;

	class = gnome_persist_storage_get_class_id (persist_storage);
	gnome_storage_write_class_id (persist_storage, class);
	
	if (gnome_persist_storage_save (persist_storage, same_as_loaded) == 0){
		gnome_persist_storage_commit (persist_storage);
	}

	return 0;
}

int
gnome_embeddable_save_to_stream (GnomeEmbeddable *bonobo_object, GnomeStream *stream)
{
	GnomePersisStream *persist_stream;
	char *class;
	
	persist_stream = gnome_object_query_interface (
		bonobo_object, "IDL:GNOME/PersistStream:1.0");

	if (persist_stream == NULL)
		return -1;

	class = gnome_persist_stream_get_class_id (persist_stream);
	gnome_stream_write_class_id (persist_stream, class);
	
	gnome_persist_stream_save (persist_stream);

	return 0;
}
