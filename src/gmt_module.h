/* $Id$
 *
 * Copyright (c) 2012-2013
 * by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
 * See LICENSE.TXT file for copying and redistribution conditions.
 */

/* gmt_module.h declares structures needed by the various modules libraries. */

#pragma once
#ifndef _GMT_MODULE_H
#define _GMT_MODULE_H

#ifdef __cplusplus /* Basic C++ support */
extern "C" {
#endif

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

/* Info for each GMT shared library. This array is filled out when parsing GMT_CUSTOM_LIBS at end of GMT_Create_Session */

struct Gmt_libinfo {
	char *name;	/* Library tag name [without leading "lib" and extension], e.g. "gmt", "gmtsuppl" */
	char *path;	/* Full path to library as given in GMT_CUSTOM_LIBS */
	bool skip;	/* true if we tried open it and it was not available the first time */
	void *handle;	/* Handle to the shared library, returned by dlopen */
};

/* name, purpose, Api_mode, and function pointer for each GMT module */
struct Gmt_moduleinfo {
	const char *name;             /* Program name */
	const char *component;        /* Component (core, supplement, custom) */
	const char *purpose;          /* Program purpose */
};

/* Name and shorthand or replacement name for legacy module names */
struct Gmt_alias {
	const char *alias;
	const char *name;
};

#ifdef __cplusplus
}
#endif

#endif /* !_GMT_MODULE_H */
