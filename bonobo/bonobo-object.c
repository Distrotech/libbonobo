/**
 * GNOME object
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 */

enum {
	DO_VERB,
	SET_HOST_NAME,
	CLOSE,
	LAST_SIGNAL
};

static guint gnome_object_signals [ITEM_LAST_SIGNAL];
static GtkObjectClass *gnome_object_parent_class;

void
gnome_object_advise_notify (GnomeObject *object, int status)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_OBJECT (object));

	if (object->advise_sink == CORBA_OBJECT_NIL)
		return;

	GNOME_AdviseSink_notify (object->advise_sink, status, &ev);
}

void
gnome_object_set_verb_list (GnomeObject *object, GList *VerbInfo_list)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_OBJECT (object));

	object->VerbInfo_list = VerbInfo_list;
}

void
gnome_object_set_view_factory (GnomeObject *object, GnomeObjectViewFactory *factory)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_OBJECT (object));

	object->view_factory = factory;
}

void
gnome_object_set_save_in_storage (GnomeObject *object,
				  GnomeObjectSaveInStorage *storage_save)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_OBJECT (object));

	object->storage_save = storage_save;
}

void
gnome_object_set_save_in_file (GnomeObject *object,
			       GnomeObjectSaveInFile *file_save)
{
	g_return_if_fail (object != NULL);
	g_return_if_fail (GNOME_IS_OBJECT (object));

	object->save_in_file = file_save;
}

/*
 * GnomeObject::do_verb default handler
 */
static void
default_do_verb (GnomeObject *object, int verb, char *name)
{
	/* nothing */
}

static void
default_set_host_name (GnomeObject *object, char *title, char *container_app_name)
{
	if (object->window_title)
		g_free (object->window_title);
	object->window_title = g_strdup (title);

	if (object->container_name)
		g_free (object->container_name);
	object->container_app_name = g_strdup (container_app_name);
}

static void
object_class_init (GnomeObjectClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;

	gnome_object_parent_class = gtk_type_class (gtk_object_get_type ());

	gnome_object_signals [DO_VERB] =
		gtk_signal_new ("do_verb",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET(GnomeObject), do_verb,
				gtk_marshall_NONE__INT_POINTER,
				GTK_TYPE_NONE, 2,
				GTK_TYPE_INT,
				GTK_TYPE_STRING);
	gnome_object_signals [SET_HOST_NAME] =
		gtk_signal_new ("set_host_name",
				GTK_RUN_LAST,
				object_class->type,
				GTK_SIGNAL_OFFSET(GnomeObject), set_host_name,
				gtk_marshall_NONE__POINTER_POINTER,
				GTK_TYPE_NONE, 2
				GTK_TYPE_STRING,
				GTK_TYPE_STRING);
	gtk_object_class_add_signals (object_class, gnome_object_signals, LAST_SIGNAL);

	class->set_host_name = default_set_host_name;
	class->do_verb = default_do_verb;
	class->save_in_storage = default_save_in_storage;
	class->save_in_file = default_save_in_file;
}

static void
object_init (GnomeObject *object)
{
	CORBA_exception_init (&object->ev);
}

GtkType
gnome_object_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"GnomeObject",
			sizeof (GnomeObject),
			sizeof (GnomeObjectClass),
			(GtkClassInitFunc) object_class_init,
			(GtkObjectInitFunc) object_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gtk_object_get_type (), &info);
	}

	return type;
}

GnomeObject *
gnome_object_new (void)
{
	GnomeObject *object;

	object = gtk_type_new (gnome_object_get_type ());

	component->
	return object;
}

