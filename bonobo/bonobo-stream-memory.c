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
#include <bonobo/bonobo-stream-memory.h>
#include <errno.h>

static BonoboStreamClass *bonobo_stream_mem_parent_class;

static void
mem_truncate (BonoboStream *stream,
	      const CORBA_long new_size, 
	      CORBA_Environment *ev)
{
	BonoboStreamMem *smem = BONOBO_STREAM_MEM (stream);
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
mem_write (BonoboStream *stream, const Bonobo_Stream_iobuf *buffer,
	   CORBA_Environment *ev)
{
	BonoboStreamMem *smem = BONOBO_STREAM_MEM (stream);
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
	
	memcpy (smem->buffer + smem->pos, buffer->_buffer, len);
	smem->pos += len;
		
	return len;
}

static CORBA_long
mem_read (BonoboStream *stream, CORBA_long count,
	  Bonobo_Stream_iobuf ** buffer,
	  CORBA_Environment *ev)
{
	BonoboStreamMem *smem = BONOBO_STREAM_MEM (stream);

	if (smem->pos + count > smem->size)
		count = smem->size - smem->pos;
	    
	*buffer = Bonobo_Stream_iobuf__alloc ();
	CORBA_sequence_set_release (*buffer, TRUE);
	(*buffer)->_buffer = CORBA_sequence_CORBA_octet_allocbuf (count);
	(*buffer)->_length = count;
	
	memcpy ((*buffer)->_buffer, smem->buffer + smem->pos, count);

	smem->pos += count;
	
	return count;
}

static CORBA_long
mem_seek (BonoboStream *stream,
	  CORBA_long offset, Bonobo_Stream_SeekType whence,
	  CORBA_Environment *ev)
{
	BonoboStreamMem *smem = BONOBO_STREAM_MEM (stream);
	int pos = 0;
	
	switch (whence){
	case Bonobo_Stream_SEEK_SET:
		pos = offset;
		break;

	case Bonobo_Stream_SEEK_CUR:
		pos = smem->pos + offset;
		break;

	case Bonobo_Stream_SEEK_END:
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
mem_copy_to  (BonoboStream *stream,
	      const CORBA_char *dest,
	      const CORBA_long bytes,
	      CORBA_long *read,
	      CORBA_long *written,
	      CORBA_Environment *ev)
{
	BonoboStreamMem *smem = BONOBO_STREAM_MEM (stream);
	gint fd_out;
	gint w;
	
	*read = smem->size - smem->pos;
	*written = 0;
	
	/* create the output file */
	fd_out = creat(dest, 0666);
	if(fd_out == -1) {
		g_warning ("unable to create output file\n");
		return;
	}
	
	/* write the memory stream buffer to the output file */
	do {
		w = write (fd_out, smem->buffer, *read);
	} while (w == -1 && errno == EINTR);
	
	if (w != -1)
		*written = w;
	else if (errno != EINTR) {
		/* should probably do something to signal an error here */
		g_warning ("ouput file write failed\n");
	}
	
	
	close(fd_out);

}

static void
mem_commit   (BonoboStream *stream,
	      CORBA_Environment *ev)
{
}

static void
mem_destroy (GtkObject *object)
{
	BonoboStreamMem *smem = BONOBO_STREAM_MEM (object);
	
	if (smem->buffer)
		g_free (smem->buffer);
	
	GTK_OBJECT_CLASS (bonobo_stream_mem_parent_class)->destroy (object);
}

static void
bonobo_stream_mem_class_init (BonoboStreamMemClass *klass)
{
	GtkObjectClass *object_class = (GtkObjectClass *) klass;
	BonoboStreamClass *sclass = BONOBO_STREAM_CLASS (klass);
	
	bonobo_stream_mem_parent_class = gtk_type_class (bonobo_stream_get_type ());

	object_class->destroy = mem_destroy;
	
	sclass->write    = mem_write;
	sclass->read     = mem_read;
	sclass->seek     = mem_seek;
	sclass->truncate = mem_truncate;
	sclass->copy_to  = mem_copy_to;
	sclass->commit   = mem_commit;
}

/**
 * bonobo_stream_mem_get_type:
 *
 * Returns: the GtkType of the BonoboStreamMem class.
 */
GtkType
bonobo_stream_mem_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"BonoboStreamMem",
			sizeof (BonoboStreamMem),
			sizeof (BonoboStreamMemClass),
			(GtkClassInitFunc) bonobo_stream_mem_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_stream_get_type (), &info);
	}

	return type;
}

static Bonobo_Stream
create_bonobo_stream_mem (BonoboObject *object)
{
	POA_Bonobo_Stream *servant;
	CORBA_Environment ev;

	servant = (POA_Bonobo_Stream *) g_new0 (BonoboObjectServant, 1);
	servant->vepv = &bonobo_stream_vepv;

	CORBA_exception_init (&ev);

	POA_Bonobo_Stream__init ((PortableServer_Servant) servant, &ev);
	if (ev._major != CORBA_NO_EXCEPTION){
                g_free (servant);
		CORBA_exception_free (&ev);
                return CORBA_OBJECT_NIL;
        }

	CORBA_exception_free (&ev);
	return (Bonobo_Stream) bonobo_object_activate_servant (object, servant);
}

/**
 * bonobo_stream_mem_create:
 * @buffer: The memory buffer for which a BonoboStreamMem object is to be created.
 * @size: The size in bytes of @buffer.
 * @read_only: Specifies whether or not the returned BonoboStreamMem
 * object should allow write() operations.
 *
 * Creates a new BonoboStreamMem object which is bound to
 * the provided memory buffer @buffer.  When data is read
 * out of or written into the returned BonoboStream object,
 * the read() and write() operations operate on @buffer.
 *
 * Returns: the constructed BonoboStream object which operates on the specified memory buffer.
 */
BonoboStream *
bonobo_stream_mem_create (char *buffer, size_t size, gboolean read_only)
{
	BonoboStreamMem *stream_mem;
	Bonobo_Stream corba_stream;
	char *copy;
	
	g_return_val_if_fail (buffer != NULL, NULL);

	if (read_only)
		copy = buffer;
	else {
		copy = g_malloc (size);
		if (!copy)
			return NULL;
		memcpy (copy, buffer, size);
	}
	
	stream_mem = gtk_type_new (bonobo_stream_mem_get_type ());
	if (stream_mem == NULL){
		g_free (copy);
		return NULL;
	}

	stream_mem->buffer = copy;
	stream_mem->size = size;
	stream_mem->pos = 0;
	stream_mem->read_only = read_only;
	
	corba_stream = create_bonobo_stream_mem (
		BONOBO_OBJECT (stream_mem));

	if (corba_stream == CORBA_OBJECT_NIL){
		gtk_object_destroy (GTK_OBJECT (stream_mem));
		return NULL;
	}

	bonobo_object_construct (
		BONOBO_OBJECT (stream_mem), corba_stream);
	return BONOBO_STREAM (stream_mem);
}



