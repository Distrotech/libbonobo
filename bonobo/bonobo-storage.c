

gnome_save (GnomeObject *object, GnomeStorage *storage)
{
	GnomePersistStorage *persist;
	
	g_return_val_if_fail (object != NULL);
	g_return_val_if_fail (storage != NULL);

	persist = gtk_object_query_interface (
		GTK_OBJECT (object), gnome_persist_storage_get_type ());
	if (!persist)
		return SOME_ERROR;

	id = gnome_persist_storage_getid (persist);
	gnome_persist_storage_save (persist, storage);
	
	if (gnome_persist_storage_save (persist, storage) == NO_ERROR)
		gnome_persist_storage_commit (persist);
}
