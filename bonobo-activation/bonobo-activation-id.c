/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
#include "config.h"

#include "liboaf-private.h"

OAFActivationInfo *
oaf_actinfo_new (void)
{
        return g_new0 (OAFActivationInfo, 1);
}

OAFActivationInfo *
oaf_servinfo_to_actinfo (const OAF_ServerInfo * servinfo)
{
	OAFActivationInfo *retval = oaf_actinfo_new ();

	retval->iid = g_strdup (servinfo->iid);
	retval->user = g_strdup (servinfo->username);
	retval->host = g_strdup (servinfo->hostname);
	retval->domain = g_strdup (servinfo->domain);

	return retval;
}

void
oaf_actinfo_free (OAFActivationInfo * actinfo)
{
	g_free (actinfo->iid);
	g_free (actinfo->user);
	g_free (actinfo->host);
	g_free (actinfo->domain);
	g_free (actinfo);
}

OAFActivationInfo *
oaf_actid_parse (const OAF_ActivationID actid)
{
	OAFActivationInfo *retval;
	char *splitme, *ctmp, *ctmp2;
	char **parts[4];
	const int nparts = sizeof (parts) / sizeof (parts[0]);
	int bracket_count, partnum;

	g_return_val_if_fail (actid, NULL);
	if (strncmp (actid, "OAFAID:", sizeof ("OAFAID:") - 1))
		return NULL;

	ctmp = (char *) (actid + sizeof ("OAFAID:") - 1);
	if (*ctmp != '[')
		return NULL;

	retval = oaf_actinfo_new ();

	splitme = oaf_alloca (strlen (ctmp) + 1);
	strcpy (splitme, ctmp);

	parts[0] = &(retval->iid);
	parts[1] = &(retval->user);
	parts[2] = &(retval->host);
	parts[3] = &(retval->domain);
	for (partnum = bracket_count = 0, ctmp = ctmp2 = splitme;
	     bracket_count >= 0 && *ctmp && partnum < nparts; ctmp++) {

		switch (*ctmp) {
		case '[':
			if (bracket_count <= 0)
				ctmp2 = ctmp + 1;
			bracket_count++;
			break;
		case ']':
			bracket_count--;
			if (bracket_count <= 0) {
				*ctmp = '\0';
				if (*ctmp2)
					*parts[partnum++] = g_strdup (ctmp2);
			}
			break;
		case ',':
			if (bracket_count == 1) {
				*ctmp = '\0';
				if (*ctmp2)
					*parts[partnum++] = g_strdup (ctmp2);
				ctmp2 = ctmp + 1;
			}
			break;
		default:
			break;
		}
	}

	return retval;
}

char *
oaf_actinfo_stringify (const OAFActivationInfo * actinfo)
{
	g_return_val_if_fail (actinfo, NULL);

	return g_strconcat ("OAFAID:[",
			    actinfo->iid ? actinfo->iid : "",
			    ",",
			    actinfo->user ? actinfo->user : "",
			    ",",
			    actinfo->host ? actinfo->host : "",
			    ",",
			    actinfo->domain ? actinfo->domain : "",
			    "]", NULL);
}
