
GnomeStorage   *gnome_storage_file_open    (const char *path,
					    const char *open_mode);
GnomeStorage   *gnome_storage_storage_open (GnomeStorage *storage,
					    const char *path,
					    const char *open_mode);
GnomeStream    *gnome_stream_storage_open (GnomeStorage *storage,
					    const char *path,
					    const char *open_mode);

void gnome_storage_write_class_id (GnomeStorage *storage,
				   char *class_id);

void gnome_stream_write_class_id  (GnomeStream *stream,
				   char *class_id);
					 
