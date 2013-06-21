#!/bin/bash
#
# $Id$
#
# Copyright (c) 2012-2013
# by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
# See LICENSE.TXT file for copying and redistribution conditions.
#
# Below, <X> is either core, suppl, or a users custom shared lib tag
#
# Run this script after adding a new GMT <X> module and updating the file
# gmt_<X>moduleinfo.txt in order to generate two files:
# gmt_<X>module.h and gmt_<X>module.c.
# Any new aliases or namechanges (with backwards compabitility)
# should be added below to the gmt_<X>alias[] and gmt_<X>oldname[] arrays.
#
# Note: gmt_<X>module.h and gmt_<X>module.c are in svn.
#

if [ $# -ne 1 ]; then
cat << EOF
usage: gmt_make_module_src.sh tag
	tag is the name of the set of modules.
	It is core or suppl for the GMT developers;
	It is whatever you call it for your custom extension.
EOF
	exit 0
fi
set -e

# Make sure we get both upper- and lower-case versions of the tag
U_TAG=`echo $1 | tr '[a-z]' '[A-Z]'`
L_TAG=`echo $1 | tr '[A-Z]' '[a-z]'`
FILE_MODULEINFO=gmt_${L_TAG}moduleinfo.txt

if [ ! -f $FILE_MODULEINFO ]; then
	echo "gmt_make_module_src.sh: Cannot find $FILE_MODULEINFO - Aborting"
	exit -1
fi

FILE_GMT_MODULE_C=gmt_${L_TAG}module.c
FILE_GMT_MODULE_H=gmt_${L_TAG}module.h
FILE_GMT_MODULE_TROFF=explain_gmt_${L_TAG}modules.txt
COPY_YEAR=$(date +%Y)

#
# Generate FILE_GMT_MODULE_H
#

cat << EOF > ${FILE_GMT_MODULE_H}
/* \$Id\$
 *
 * Copyright (c) 2012-${COPY_YEAR}
 * by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
 * See LICENSE.TXT file for copying and redistribution conditions.
 */

/* gmt_${L_TAG}module.h declares the prototypes for ${L_TAG} module functions
 * and the array that contains ${L_TAG} GMT module parameters such as name and purpose strings.
 * DO NOT edit this file directly! Instead edit gmt_${L_TAG}moduleinfo.txt and regenerate
 * this file with gmt_make_module_src.sh ${L_TAG}. */

#pragma once
#ifndef _GMT_${U_TAG}MODULE_H
#define _GMT_${U_TAG}MODULE_H

#ifdef __cplusplus /* Basic C++ support */
extern "C" {
#endif

/* CMake definitions: This must be first! */
#include "gmt_config.h"

/* Declaration modifiers for DLL support (MSC et al) */
#include "declspec.h"

/* Pretty print all module names and their purposes */
EXTERN_MSC void gmt_${L_TAG}module_show_all (struct GMTAPI_CTRL *API);

#ifdef __cplusplus
}
#endif

#endif /* !_GMT_${U_TAG}MODULE_H */
EOF

#
# Generate FILE_GMT_MODULE_C
#

cat << EOF > ${FILE_GMT_MODULE_C}
/* \$Id\$
 *
 * Copyright (c) 2012-${COPY_YEAR}
 * by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
 * See LICENSE.TXT file for copying and redistribution conditions.
 */

/* gmt_${L_TAG}module.c populates the external array of GMT ${L_TAG} module parameters such as name
 * and purpose strings, and function pointers to call the module functions.
 * This file also contains the following convenience function to display all module purposes:
 *   void gmt_${L_TAG}module_show_all(struct GMTAPI_CTRL *API); - Pretty print all ${L_TAG} module names and their
 *           purposes
 *
 * DO NOT edit this file directly! Instead edit gmt_${L_TAG}moduleinfo.txt and regenerate
 * this file with gmt_make_${L_TAG}module_src.sh. */

#include <stdio.h>
#include <string.h>

#include "gmt_dev.h"

/* sorted array with program parameters for all GMT ${L_TAG} modules */
struct Gmt_moduleinfo g_${L_TAG}module[] = {
EOF

# $1 = name, $2 = ${L_TAG}, $3 = Api_mode, $4 = purpose
gawk '
	BEGIN {
		FS = "\t";
	}
	!/^[ \t]*#/ {
		printf "\t{\"%s\", \"%s\", \"%s\"},\n", $1, $2, $3;
	}' ${FILE_MODULEINFO} >> ${FILE_GMT_MODULE_C}

cat << EOF >> ${FILE_GMT_MODULE_C}
	{NULL, NULL, NULL} /* last element == NULL detects end of array */
};

/* sorted array with shorter aliases for ${L_TAG} modules starting with "gmt" */

struct Gmt_alias gmt_${L_TAG}alias[] =
{	/* Alias:	Full name */
EOF
if [ "$L_TAG" = "core" ]; then
cat << EOF >> ${FILE_GMT_MODULE_C}
	{"2kml", 	"gmt2kml"},
	{"convert",	"gmtconvert"},
	{"defaults",	"gmtdefaults"},
	{"get",		"gmtget"},
	{"math",	"gmtmath"},
	{"select",	"gmtselect"},
	{"set",		"gmtset"},
	{"simplify",	"gmtsimplify"},
	{"spatial",	"gmtspatial"},
	{"stitch",	"gmtstitch"},
	{"vector",	"gmtvector"},
	{"which",	"gmtwhich"},
EOF
elif [ "$L_TAG" = "suppl" ]; then
cat << EOF >> ${FILE_GMT_MODULE_C}
	{"gravmag3d",	"gmtgravmag3d"},
EOF
fi
cat << EOF >> ${FILE_GMT_MODULE_C}
	{NULL,		NULL}
};

/* sorted array with replacement names for some ${L_TAG} modules */
struct Gmt_alias gmt_${L_TAG}oldname[] =
{	/* Old:		New: */
EOF
if [ "$L_TAG" = "core" ]; then
cat << EOF >> ${FILE_GMT_MODULE_C}
	{"gmtdp",	"gmtsimplify"},
EOF
fi
cat << EOF >> ${FILE_GMT_MODULE_C}
	{NULL,		NULL}
};

/* Look out for ${L_TAG} modules given by their aliases */
const char * gmt_${L_TAG}formal_name (struct GMTAPI_CTRL *API, const char *module) {
	unsigned int k = 0;
	while (gmt_${L_TAG}alias[k].alias != NULL) {
		if (!strcmp (module, gmt_${L_TAG}alias[k].alias)) return gmt_${L_TAG}alias[k].name;
		k++;
	}
	if (GMT_compat_check (API->GMT, 4)) {
		k = 0;
		while (gmt_${L_TAG}oldname[k].alias != NULL) {
			if (!strcmp (module, gmt_${L_TAG}oldname[k].alias)) return gmt_${L_TAG}oldname[k].name;
			k++;
		}
	}
	return module;
}

/* Pretty print all ${L_TAG} module names and their purposes */
void gmt_${L_TAG}module_show_all (struct GMTAPI_CTRL *API) {
	unsigned int module_id = 0;
	char module_name_comp[GMT_TEXT_LEN64], message[GMT_TEXT_LEN256];

	GMT_Message (API, GMT_TIME_NONE, "Program                Purpose of Program\n");
	while (g_${L_TAG}module[module_id].name != NULL) {
		snprintf (module_name_comp, GMT_TEXT_LEN64, "%s(%s)",
				g_${L_TAG}module[module_id].name, g_${L_TAG}module[module_id].component);
		sprintf (message, "%-22s %s\n",
				module_name_comp, g_${L_TAG}module[module_id].purpose);
		GMT_Message (API, GMT_TIME_NONE, message);
		++module_id;
	}
}
EOF

exit 0

# vim: set ft=c:
