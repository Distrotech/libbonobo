/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * gnome-stream-mem.c: Memory based stream
 *
 * Author:
 *   Miguel de Icaza (miguel@gnu.org)
 */
#include <config.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <libgnome/gnome-defs.h>
#include <libgnome/gnome-util.h>
#include <bonobo/gnome-stream-memory.h>
#include <errno.h>

static GnomeStreamClass *gnome_stream_mem_parent_class;

static void
mem_truncate (GnomeStream *stream,
	      const CORBA_long new_size, 
	      CORBA_Environment *ev)
{
	GnomeStreamMem *smem = GNOME_STREAM_MEM (stream);
	void *newp;
	
	if (smem->read_only)
		return;

	newp = g_realloc (smem->buffer, new_size);
	if (newp == NULL){
		g_warning ("Signal exception");
		return;
	}

	smem->buffer = newp;
	smem->size = new_size;

	if (smem->pos > new_size)
		smem->pos = new_size;
}

static CORBA_long
mem_write (GnomeStream *stream, const GNOME_Stream_iobuf *buffer,
	   CORBA_Environment *ev)
{
	GnomeStreamMem *smem = GNOME_STREAM_MEM (stream);
	long len = buffer->_length;

	if (smem->read_only){
		g_warning ("Should signal an exception here");
		return 0;
	}

	if (smem->pos + len > smem->size){
		mem_truncate (stream, smem->pos + len, ev);
		g_warning ("Should check for an exception here");
	}

	if (smem->pos + len > smem->size)
		len = smem->size - smem->pos;
	
	memcpy (smem->buffer, buffer->_buffer, len);
	smem->pos += len;
		
	return len;
}

static CORBA_long
mem_read (GnomeStream *stream, CORBA_long count,
	  GNOME_Stream_iobuf ** buffer,
	  CORBA_Environment *ev)
{
	GnomeStreamMem *smem = GNOME_STREAM_MEM (stream);

	if (smem->pos + count > smem->size)
		count = smem->size - smem->pos;
	    
	*buffer = GNOME_Stream_iobuf__alloc ();
	CORBA_sequence_set_release (*buffer, TRUE);
	(*buffer)->_buffer = CORBA_sequence_CORBA_octet_allocbuf (count);
	(*buffer)->_length = count;
	
	memcpy ((*buffer)->_buffer, smem->buffer, count);

	smem->pos += count;
	
	return count;
}

static CORBA_long
mem_seek (GnomeStream *stream,
	  CORBA_long offset, GNOME_Stream_SeekType whence,
	  CORBA_Environment *ev)
{
	GnomeStreamMem *smem = GNOME_STREAM_MEM (stream);
	int pos = 0;
	
	switch (whence){
	case GNOME_Stream_SEEK_SET:
		pos = offset;
		break;

	case GNOME_Stream_SEEK_CUR:
		pos = smem->pos + offset;
		break;

	case GNOME_Stream_SEEK_END:
		pos = smem->size + offset;
		break;

	default:
		g_warning ("Signal exception");
	}

	if (pos > smem->size){
		mem_truncate (stream, pos, ev);
	}
	smem->pos = pos;
	return pos;
}

static void
mem_copy_to  (GnomeStream *stream,
	      const CORBA_char *dest,
	      const CORBA_long bytes,
	      CORBA_long *read,
	      CORBA_long *written,
	      CORBA_Environment *ev)
{
	g_warning ("Implement me");
}

static void
mem_commit   (GnomeStream *stream,
	      CORBA_Environment *ev)
{
}

static void
mem_destroy (GtkObject *object)
{
	GnomeStreamMem *smem = GNOME_STREAM_MEM (object);
	
	if (smem->buffer)
		g_free (smem->buffer);
	
	GTK_OBJECT_CLASS (gnome_stream_mem_parent_class)->destroy (object);
}

static void
gnome_stream_mem_class_init (GnomeStreamMemClass *class)
{
	GtkObjectClass *object_class = (GtkObjectClass *) class;
	GnomeStreamClass *sclass = GNOME_STREAM_CLASS (class);
	
	gnome_stream_mem_parent_class = gtk_type_class (gnome_stream_get_type ());

	object_class->destroy = mem_destroy;
	
	sclass->write    = mem_write;
	sclass->read     = mem_read;
	sclass->seek     = mem_seek;
	sclass->truncate = mem_truncate;
	sclass->copy_to  = mem_copy_to;
	sclass->commit   = mem_commit;
}

/**
 * gnome_stream_mem_get_type:
 *
 * Returns: the GtkType of the GnomeStreamMem class.
 */
GtkType
gnome_stream_mem_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"IDL:GNOME/StreamMem:1.0",
			sizeof (GnomeStreamMem),
			sizeof (GnomeStreamMemClass),
			(GtkClassInitFunc) gnome_stream_mem_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gnome_stream_get_type (), &info);
	}

	return type;
}

static GNOME_Stream
create_gnome_stream_mem (GnomeObject *object)
{
	POA_GNOME_Stream *servant;
	CORBA_Environment ev;

	servant = (POA_GNOME_Stream *) g_new0 (GnomeObjectServant, 1);
	servant->vepv = &gnome_stream_vepv;

	CORBA_exception_init (&ev);

	POA_GNOME_Stream__init ((PortableServer_Servant) servant, &ev);
	if (ev._major != CORBA_NO_EXCEPTION){
                g_free (servant);
		CORBA_exception_free (&ev);
                return CORBA_OBJECT_NIL;
        }

	CORBA_exception_free (&ev);
	return (GNOME_Stream) gnome_object_activate_servant (object, servant);
}

/**
 * gnome_stream_mem_create:
 * @buffer: The memory buffer for which a GnomeStreamMem object is to be created.
 * @size: The size in bytes of @buffer.
 * @read_only: Specifies whether or not the returned GnomeStreamMem
 * object should allow write() operations.
 *
 * Creates a new GnomeStreamMem object which is bound to
 * the provided memory buffer @buffer.  When data is read
 * out of or written into the returned GnomeStream object,
 * the read() and write() operations operate on @buffer.
 *
 * Returns: the constructed GnomeStream object which operates on the specified memory buffer.
 */
GnomeStream *
gnome_stream_mem_create (char *buffer, size_t size, gboolean read_only)
{
	GnomeStreamMem *stream_mem;
	GNOME_Stream corba_stream;
	char *copy;
	
	g_return_val_if_fail (buffer != NULL, NULL);

	if (read_only)
		copy = buffer;
	else {
		copy = g_malloc (size);
		if (!copy)
			return NULL;
	}
	
	stream_mem = gtk_type_new (gnome_stream_mem_get_type ());
	if (stream_mem == NULL){
		g_free (copy);
		return NULL;
	}

	stream_mem->buffer = copy;
	stream_mem->size = size;
	stream_mem->pos = 0;
	stream_mem->read_only = read_only;
	
	corba_stream = create_gnome_stream_mem (
		GNOME_OBJECT (stream_mem));

	if (corba_stream == CORBA_OBJECT_NIL){
		gtk_object_destroy (GTK_OBJECT (stream_mem));
		return NULL;
	}

	gnome_object_construct (
		GNOME_OBJECT (stream_mem), corba_stream);
	return GNOME_STREAM (stream_mem);
}



