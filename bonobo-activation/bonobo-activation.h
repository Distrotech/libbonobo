#ifndef LIBOAF_H
#define LIBOAF_H 1

#include <liboaf/oaf.h>

/* Need to conditionalize this */
#include <orb/orbit.h>

/* If you wish to manipulate the internals of this structure, please
   use g_malloc/g_free to allocate memory. */
typedef struct {
  char *iid; /* Implementation ID */
  char *user; /* You owe this field to boc */
  char *host; /* DNS name or IP address */
  char *domain; /* This is not a DNS domain, but an activation domain */
} OAFActivationInfo;

OAF_ActivationID oaf_actinfo_stringify(OAFActivationInfo *actinfo);
OAFActivationInfo *oaf_actid_parse(OAF_ActivationID actid);
OAFActivationInfo *oaf_actinfo_new(void);
void oaf_actinfo_free(OAFActivationInfo *actinfo);

CORBA_ORB oaf_orb_init(int *argc, char **argv);
CORBA_ORB oaf_orb_get(void);

CORBA_Context oaf_context_get(void); /* Just makes getting hold of the default context a bit easier */

#endif
