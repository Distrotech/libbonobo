#include <liboaf/liboaf.h>
#include <stdlib.h>
#include "empty.h"

int main(int argc, char *argv[])
{
        OAF_ServerInfoList *result;
	CORBA_Environment ev;
	OAF_ActivationID aid;
	char *query;
	int i;

	CORBA_exception_init(&ev);
	oaf_init(argc, argv);

	if (argc > 1) {
	  query = argv[1];
	} else {
	  query = "repo_ids.has('IDL:Empty:1.0')";
	}

	// putenv("OAF_BARRIER_INIT=1");
	result = oaf_query (query, NULL, &ev);

	// result = oaf_query ("iid == 'OAFIID:Empty:19991025'", NULL, &ev);

	if (result == NULL) {
	  puts ("query failed");
	} else {
	  printf ("number of results: %d\n", result->_length);

	  for (i = 0; i < result->_length; i++) {
	    puts ((result->_buffer[i]).iid);
	  }
	  
	}

	CORBA_exception_free(&ev);

	return 0;
}
