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

OAF_ActivationID oaf_actinfo_stringify(const OAFActivationInfo *actinfo);
OAFActivationInfo *oaf_actid_parse(const OAF_ActivationID actid);
OAFActivationInfo *oaf_actinfo_new(void);
void oaf_actinfo_free(OAFActivationInfo *actinfo);

CORBA_ORB oaf_init(int argc, char **argv);
CORBA_ORB oaf_orb_get(void);
CORBA_Context oaf_context_get(void); /* Just makes getting hold of the default context a bit easier */
CORBA_Object oaf_activation_context_get(void); /* Internal use, remove me from header file */

#define oaf_username_get() g_get_user_name()
const char *oaf_hostname_get(void);
const char *oaf_session_name_get(void);
const char *oaf_domain_get(void);

typedef struct {
  const char   *iid;

  /* This routine should call oaf_plugin_use(servant, impl_ptr), as should all routines which activate CORBA objects
     implemented by this shared library. This needs to be done before making any CORBA calls on the object, or
     passing that object around. First thing after servant creation always works. :) */
  CORBA_Object (*activate) (PortableServer_POA poa,
			    const char *iid,
			    gpointer impl_ptr, /* This pointer should be stored by the implementation
						  to be passed to oaf_plugin_unuse() in the implementation's
						  destruction routine. */
			    CORBA_Environment *ev);
} OAFPluginObject;

typedef struct {
  const OAFPluginObject *plugin_object_list;
  const char *description;
} OAFPlugin;

void oaf_plugin_use(PortableServer_Servant servant, gpointer impl_ptr);
void oaf_plugin_unuse(gpointer impl_ptr);

CORBA_Object oaf_activate(const char *requirements, const char **selection_order,
			  OAF_ActivationFlags flags, CORBA_Environment *ev);
CORBA_Object oaf_activate_from_id(const OAF_ActivationID aid, OAF_ActivationFlags flags, CORBA_Environment *ev);

/* oaf-registration.c - not intended for application use */
typedef struct {
  const char *name;
  const char *session_name;
  const char *username, *hostname, *domain;
} OAFRegistrationCategory;

typedef struct _OAFRegistrationLocation OAFRegistrationLocation;
struct _OAFRegistrationLocation {
  void   (*lock)       (const OAFRegistrationLocation *regloc, gpointer user_data);
  void   (*unlock)     (const OAFRegistrationLocation *regloc, gpointer user_data);
  char * (*check)      (const OAFRegistrationLocation *regloc, const OAFRegistrationCategory *regcat,
			int *ret_distance, gpointer user_data);

  void   (*register_new)(const OAFRegistrationLocation *regloc, const char *ior,
			 const OAFRegistrationCategory *regcat, gpointer user_data);
  void   (*unregister) (const OAFRegistrationLocation *regloc, const char *ior,
			const OAFRegistrationCategory *regcat, gpointer user_data);
};

void oaf_registration_location_add(const OAFRegistrationLocation *regloc, int priority, gpointer user_data);

CORBA_Object oaf_registration_check(const OAFRegistrationCategory *regcat, CORBA_Environment *ev);
void oaf_registration_set(const OAFRegistrationCategory *regcat, CORBA_Object obj, CORBA_Environment *ev);
void oaf_registration_unset(const OAFRegistrationCategory *regcat, CORBA_Object obj, CORBA_Environment *ev);

/* Do not release() the returned value */
CORBA_Object oaf_service_get(const OAFRegistrationCategory *regcat);

typedef CORBA_Object (*OAFServiceActivator)(const OAFRegistrationCategory *regcat, const char **cmd,
					    int ior_fd, CORBA_Environment *ev);
void oaf_registration_activator_add(OAFServiceActivator act_func, int priority);

/* oaf-servreg.c */
OAF_RegistrationResult oaf_active_server_register(const char *iid, CORBA_Object obj);
void oaf_active_server_unregister(const char *iid, CORBA_Object obj);

/* Optional stuff for libgnome to use */
#ifdef HAVE_POPT_H
#include <popt.h>
#endif

#ifdef POPT_AUTOHELP
extern struct poptOptions oaf_popt_options[];
#endif

CORBA_ORB oaf_orb_init(int *argc, char **argv);
void oaf_preinit(gpointer app, gpointer mod_info);
void oaf_postinit(gpointer app, gpointer mod_info);
const char *oaf_activation_iid_get(void);
extern const char liboaf_version[];

#endif
