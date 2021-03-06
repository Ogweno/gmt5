/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Brief synopsis: read a file of lon, lat, zvalue[, distance, azimuth]
 * and split it into profile segments.
 * 
 * Author:	W. H. F. Smith
 * Date:	1 JAN 2010
 * Version:	5 API
 */

#define THIS_MODULE_NAME	"splitxyz"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Split xyz[dh] data tables into individual segments"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:>Vbfghis" GMT_OPT("H")

#define SPLITXYZ_F_RES			1000	/* Number of points in filter halfwidth  */
#define SPLITXYZ_N_OUTPUT_CHOICES	5

int gmt_parse_g_option (struct GMT_CTRL *GMT, char *txt);

struct SPLITXYZ_CTRL {
	struct Out {	/* -> */
		bool active;
		char *file;
	} Out;
	struct A {	/* -A<azimuth>/<tolerance> */
		bool active;
		double azimuth, tolerance;
	} A;
	struct C {	/* -C<course_change> */
		bool active;
		double value;
	} C;
	struct D {	/* -D<mindist> */
		bool active;
		double value;
	} D;
	struct F {	/* -F<xy_filter>/<z_filter> */
		bool active;
		double xy_filter, z_filter;
	} F;
	struct N {	/* -N<namestem> */
		bool active;
		char *name;
	} N;
	struct Q {	/* -Q[<xyzdg>] */
		bool active;
		bool z_selected;
		char col[SPLITXYZ_N_OUTPUT_CHOICES];	/* Character codes for desired output in the right order */
	} Q;
	struct S {	/* -S */
		bool active;
	} S;
};

double *filterxy_setup (struct GMT_CTRL *GMT)
{
	unsigned int i;
	double tmp, sum = 0.0, *fwork = NULL;

	fwork = GMT_memory (GMT, NULL, SPLITXYZ_F_RES, double);	/* Initialized to zeros */
	tmp = M_PI / SPLITXYZ_F_RES;
	for (i = 0; i < SPLITXYZ_F_RES; i++) {
		fwork[i] = 1.0 + cos (i * tmp);
		sum += fwork[i];
	}
	for (i = 1; i < SPLITXYZ_F_RES; i++) fwork[i] /= sum;
	return (fwork);
}

void filter_cols (struct GMT_CTRL *GMT, double *data[], uint64_t begin, uint64_t end, unsigned int d_col, unsigned int n_cols, unsigned int cols[], double filter_width, double *fwork)
{
	uint64_t i, j, k, p, istart, istop, ndata;
	int64_t kk;
	bool hilow;
	double half_width, dt, sum, **w = NULL;

	if (filter_width == 0.0) return;	/* No filtering */
	hilow = (filter_width < 0.0);
	half_width = 0.5 * fabs (filter_width);
	dt = SPLITXYZ_F_RES / half_width;
	ndata = end - begin;
	w = GMT_memory (GMT, NULL, n_cols, double *);
	for (k = 0; k < n_cols; k++) w[k] = GMT_memory (GMT, NULL, ndata, double);	/* Initialized to zeros */
	j = istart = istop = begin;
	while (j < end) {
		while (istart < end && data[d_col][istart] - data[d_col][j] <= -half_width) istart++;
		while (istop  < end && data[d_col][istop]  - data[d_col][j] <   half_width) istop++;
		for (i = istart, sum = 0.0; i < istop; i++) {
			kk = lrint (floor (dt * fabs (data[d_col][i] - data[d_col][j])));
			if (kk < 0 || kk >= SPLITXYZ_F_RES) continue;	/* Safety valve */
			k = kk;
			sum += fwork[k];
			for (p = 0; p < n_cols; p++) w[p][j] += (data[cols[p]][i] * fwork[k]);
		}
		for (p = 0; p < n_cols; p++) w[p][j] /= sum;
		j++;
	}
	if (hilow) {
		for (i = begin; i < end; i++) for (p = 0; p < n_cols; p++) data[cols[p]][i] -= w[p][i];
	}
	else {
		for (i = begin; i < end; i++) for (p = 0; p < n_cols; p++) data[cols[p]][i] = w[p][i];
	}
	for (p = 0; p < n_cols; p++) GMT_free (GMT, w[p]);
	GMT_free (GMT, w);
}

void *New_splitxyz_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SPLITXYZ_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct SPLITXYZ_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->A.azimuth = 90.0;
	C->A.tolerance = 360.0;
	C->C.value = 180.0;	/* Tolerate any course change */
	return (C);
}

void Free_splitxyz_Ctrl (struct GMT_CTRL *GMT, struct SPLITXYZ_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->Out.file) free (C->Out.file);	
	if (C->N.name) free (C->N.name);	
	GMT_free (GMT, C);	
}

int GMT_splitxyz_usage (struct GMTAPI_CTRL *API, int level)
{
	/* This displays the splitxyz synopsis and optionally full usage information */

	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: splitxyz [<table>] [-A<azimuth>/<tolerance>] [-C<course_change>] [-D<minimum_distance>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-F<xy_filter>/<z_filter>] [-N<template>] [-Q<flags>] [-S] [%s]\n", GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s]\n\n",
		GMT_b_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_s_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\tGive xyz[dh]file name or read stdin.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<table> is one or more data files (in ASCII, binary, netCDF) with 2, 3 or 5 columns.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no files are given, standard input is read.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Only write profile if mean direction is w/in +/- <tolerance>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   of <azimuth> [Default = All].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Profile ends when change of heading exceeds <course_change> [ignore course changes].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Only write profile if length is at least <minimum_distance> [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Filter the data.  Give full widths of cosine arch filters for xy and z.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Defaults are both widths = 0, giving no filtering.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use negative width to highpass.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Write individual segments to separate files [Default writes one\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   multisegment file to stdout].  Append file name template which MUST\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   contain a C-style format for a long integer (e.g., %%d) that represents\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a sequential segment number across all tables (if more than one table).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default uses splitxyz_segment_%%d.txt (or .bin for binary)].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, supply a template with two long formats and we will\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   replace them with the table number and table segment numbers.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Indicate what output you want as one or more of xyzdh in any order;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   where x,y,z refer to input data locations and optional z-value(s),\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   and d,h are the distance and heading along track.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is all fields, i.e., -Qxyzdh (or -Qxydh if no z-column in the input)].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S d,h is supplied.  Input is 5 col x,y,z,d,h with d non-decreasing.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default input is 3 col x,y,z only and computes d,h from the data].\n");
	GMT_Option (API, "V,bi");
	GMT_Message (API, GMT_TIME_NONE, "\t     Default input columns is set via -S.\n");
	GMT_Option (API, "bo,f,g,h,i,s,:,.");
	
	return (EXIT_FAILURE);
}

int GMT_splitxyz_parse (struct GMT_CTRL *GMT, struct SPLITXYZ_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to splitxyz and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int j, n_errors = 0, n_outputs = 0, n_files = 0;
	char txt_a[GMT_LEN256] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;
			case '>':	/* Got named output file */
				if (n_files++ == 0) Ctrl->Out.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = true;
				n_errors += GMT_check_condition (GMT,  (sscanf(opt->arg, "%lf/%lf", &Ctrl->A.azimuth, &Ctrl->A.tolerance)) != 2, "Syntax error -A option: Can't decipher values\n");
				break;
			case 'C':
				Ctrl->C.active = true;
				n_errors += GMT_check_condition (GMT,  (sscanf(opt->arg, "%lf", &Ctrl->C.value)) != 1, "Syntax error -C option: Can't decipher value\n");
				break;
			case 'D':
				Ctrl->D.active = true;
				n_errors += GMT_check_condition (GMT,  (sscanf(opt->arg, "%lf", &Ctrl->D.value)) != 1, "Syntax error -D option: Can't decipher value\n");
				break;
			case 'F':
				Ctrl->F.active = true;
				n_errors += GMT_check_condition (GMT,  (sscanf(opt->arg, "%lf/%lf", &Ctrl->F.xy_filter, &Ctrl->F.z_filter)) != 2, "Syntax error -F option: Can't decipher values\n");
				break;
			case 'G':
				if (GMT_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: -G option is deprecated; use -g instead.\n");
					GMT->common.g.active = true;
					if (GMT_is_geographic (GMT, GMT_IN))	
						sprintf (txt_a, "D%sk", opt->arg);	/* Hardwired to be km */
					else
						sprintf (txt_a, "d%s", opt->arg);	/* Cartesian */
					n_errors += gmt_parse_g_option (GMT, txt_a);
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'M':
				if (GMT_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -M is deprecated; -fg was set instead, use this in the future.\n");
					if (!GMT_is_geographic (GMT, GMT_IN)) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Set -fg unless already set */
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'N':
				Ctrl->N.active = true;
				if (opt->arg[0]) Ctrl->N.name = strdup (opt->arg);
				break;
			case 'Q':
				Ctrl->Q.active = true;
				for (j = 0; opt->arg[j]; j++) {
					if (j < SPLITXYZ_N_OUTPUT_CHOICES) {
						Ctrl->Q.col[j] = opt->arg[j];
						if (!strchr ("xyzdh", Ctrl->Q.col[j])) {
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Q option: Unrecognized output choice %c\n", Ctrl->Q.col[j]);
							n_errors++;
						}
						if (opt->arg[j] == 'z') Ctrl->Q.z_selected = true;
						n_outputs++;
					}
					else {
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Q option: Too many output columns selected: Choose from -Qxyzdg\n");
						n_errors++;
					}
				}
				break;
			case 'S':
				Ctrl->S.active = true;
				break;
			case 'Z':
				if (GMT_compat_check (GMT, 5)) /* Warn and pass through */
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: -Z option is deprecated and not longer required.\n");
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, Ctrl->D.value < 0.0, "Syntax error -D option: Minimum segment distance must be positive\n");
	n_errors += GMT_check_condition (GMT, Ctrl->C.value <= 0.0, "Syntax error -C option: Course change tolerance must be positive\n");
	n_errors += GMT_check_condition (GMT, Ctrl->A.tolerance < 0.0, "Syntax error -A option: Azimuth tolerance must be positive\n");
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_OUT] && !Ctrl->N.name, "Syntax error: Binary output requires a namestem in -N\n");
	n_errors += GMT_check_binary_io (GMT, (Ctrl->S.active) ? 5 : 3);
	n_errors += GMT_check_condition (GMT, n_files > 1, "Syntax error: Only one output destination can be specified\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.active && Ctrl->N.name && !strstr (Ctrl->N.name, "%"), "Syntax error -N: Output template must contain %%d\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_splitxyz_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_splitxyz (void *V_API, int mode, void *args)
{
	unsigned int i, j, d_col, h_col, z_cols, xy_cols[2] = {0, 1};
	unsigned int output_choice[SPLITXYZ_N_OUTPUT_CHOICES], n_outputs = 0;
	int error = 0;
	bool ok, io_mode = 0, first = true, no_z_column;
	uint64_t dim[4] = {1, 0, 0, 0};
	
	size_t n_alloc_seg = 0, n_alloc = 0;
	uint64_t tbl, col, n_out = 0, k, n, row, seg, seg2 = 0, begin, end, n_total = 0, n_columns = 0, nprofiles = 0, *rec = NULL;

	double dy, dx, last_c, last_s, csum, ssum, this_c, this_s, dotprod;
	double mean_azim, *fwork = NULL;
	
	char header[GMT_LEN64] = {""};

	struct GMT_DATASET *D[2] = {NULL, NULL};
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATASEGMENT *S = NULL, *S_out = NULL;
	struct SPLITXYZ_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_splitxyz_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_splitxyz_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_splitxyz_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_splitxyz_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_splitxyz_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the splitxyz main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");
	if ((error = GMT_set_cols (GMT, GMT_IN, 3)) != GMT_OK) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data input */
		Return (API->error);
	}
	if ((D[GMT_IN] = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_FILE_BREAK, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}

	GMT_memset (output_choice, SPLITXYZ_N_OUTPUT_CHOICES, int);
	no_z_column = (D[GMT_IN]->n_columns == 2);
	
	if (no_z_column && Ctrl->S.active) {
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: The -S option requires a 3rd data column\n");
		Return (GMT_PARSE_ERROR);
	}
	if (no_z_column && Ctrl->F.z_filter != 0.0) {
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: The -F option requires a 3rd data column\n");
		Return (GMT_PARSE_ERROR);
	}
	if (Ctrl->Q.z_selected && no_z_column) {
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Q option: Cannot request z if unless data have a 3rd column\n");
		Return (GMT_PARSE_ERROR);
	}
	
	for (k = n_outputs = 0; k < SPLITXYZ_N_OUTPUT_CHOICES && Ctrl->Q.col[k]; k++) {
		switch (Ctrl->Q.col[k]) {
			case 'x':
				output_choice[k] = 0;
				break;
			case 'y':
				output_choice[k] = 1;
				break;
			case 'z':
				if (no_z_column) {
					GMT_Report (API, GMT_MSG_NORMAL, "Cannot specify z when there is no z column!\n");
					Return (-1);
				}
				output_choice[k] = 2;
				break;
			case 'd':
				output_choice[k] = 3 - no_z_column;
				break;
			case 'h':
				output_choice[k] = 4 - no_z_column;
				break;
		}
		n_outputs++;
	}
	if (GMT_is_geographic (GMT, GMT_IN))
		GMT_set_geographic (GMT, GMT_OUT);

	if (n_outputs == 0) {	/* Generate default -Q setting (all) */
		n_outputs = 5 - no_z_column;
		for (i = 0; i < 2; i++) output_choice[i] = i;
		if (!no_z_column) output_choice[2] = 2;
		for (i = 3-no_z_column; i < n_outputs; i++) output_choice[i] = i;
	}

	Ctrl->A.tolerance *= D2R;
	/* if (Ctrl->A.azimuth > 180.0) Ctrl->A.azimuth -= 180.0; */	/* Put in Easterly strikes  */
	Ctrl->A.azimuth = D2R * (90.0 - Ctrl->A.azimuth);	/* Work in cartesian angle and radians  */
	Ctrl->C.value *= D2R;
	if (Ctrl->F.active) fwork = filterxy_setup (GMT);
	if (Ctrl->N.active) {
		int n_formats;
		for (col = n_formats = 0; Ctrl->N.name[col]; col++) if (Ctrl->N.name[col] == '%') n_formats++;
		io_mode = (n_formats == 2) ? GMT_WRITE_TABLE_SEGMENT: GMT_WRITE_SEGMENT;
	}
	else
		GMT_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */

	if ((error = GMT_set_cols (GMT, GMT_OUT, n_outputs)) != GMT_OK) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_PLP, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Registers default output destination, unless already set */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {
		Return (API->error);	/* Enables data output and sets access mode */
	}

	if (!Ctrl->S.active) {	/* Must extend table with 2 cols to hold d and az */
		n_columns = D[GMT_IN]->n_columns + 2;
		d_col = (unsigned int)D[GMT_IN]->n_columns;
		h_col = d_col + 1;
	}
	else {	/* Comes with d and az in file */
		d_col = no_z_column + 2;
		h_col = no_z_column + 3;
	}
	z_cols = 2;
	S_out = GMT_memory (GMT, NULL, 1, struct GMT_DATASEGMENT);
	
	nprofiles = 0;
	for (tbl = 0; tbl < D[GMT_IN]->n_tables; tbl++) {
		T = D[GMT_IN]->table[tbl];
		if (!Ctrl->S.active) {	/* Must extend table with 2 cols to hold d and az */
			T->min = GMT_memory (GMT, T->min, n_columns, double);
			T->max = GMT_memory (GMT, T->max, n_columns, double);
		}
		GMT_Report (API, GMT_MSG_VERBOSE, "Working on file %s\n", T->file[GMT_IN]);

		for (seg = 0; seg < D[GMT_IN]->table[tbl]->n_segments; seg++) {	/* For each segment in the table */
			S = T->segment[seg];
			if (!Ctrl->S.active) {	/* Must extend table with 2 cols to hold d and az */
				S->coord = GMT_memory (GMT, S->coord, n_columns, double *);
				S->min = GMT_memory (GMT, S->min, n_columns, double);
				S->max = GMT_memory (GMT, S->max, n_columns, double);
				for (col = D[GMT_IN]->n_columns; col < n_columns; col++) S->coord[col] = GMT_memory (GMT, NULL, S->n_rows, double);
			}
			
			if (Ctrl->S.active) S->coord[h_col][0] = D2R * (90.0 - S->coord[h_col][0]);	/* Angles are stored as CCW angles in radians */
			for (row = 1; row < S->n_rows; row++) {
				if (!Ctrl->S.active) {	/* Must extend table with 2 cols to hold d and az */
					dy = S->coord[GMT_Y][row] - S->coord[GMT_Y][row-1];
					if (GMT_is_geographic (GMT, GMT_IN)) {
						GMT_set_delta_lon (S->coord[GMT_X][row-1], S->coord[GMT_X][row], dx);
						dy *= GMT->current.proj.DIST_KM_PR_DEG;
						dx *= (GMT->current.proj.DIST_KM_PR_DEG * cosd (0.5 * (S->coord[GMT_Y][row] + S->coord[GMT_Y][row-1])));
					}
					else
						dx = S->coord[GMT_X][row] - S->coord[GMT_X][row-1];
					if (dy == 0.0 && dx == 0.0) {
						S->coord[d_col][row] = S->coord[d_col][row-1];
						S->coord[h_col][row] = S->coord[h_col][row-1];
					}
					else {
						S->coord[d_col][row] = S->coord[d_col][row-1] + hypot (dx,dy);
						S->coord[h_col][row] = d_atan2(dy,dx);	/* Angles are stored as CCW angles in radians */
					}
				}
				else 
					S->coord[h_col][row] = D2R * (90.0 - S->coord[h_col][row]);	/* Angles are stored as CCW angles in radians */
			}
			if (!Ctrl->S.active) S->coord[h_col][0] = S->coord[h_col][1];
			
			/* Here a complete segment is ready for further processing */
			/* Now we have read the data and can filter z, if necessary.  */

			if (Ctrl->F.active) filter_cols (GMT, S->coord, 0, S->n_rows, d_col, 1, &z_cols, Ctrl->F.z_filter, fwork);

			/* Now we are ready to search for segments.  */

			begin = end = 0;
			while (end < S->n_rows-1) {
				sincos (S->coord[h_col][begin], &last_s, &last_c);
				csum = last_c;	ssum = last_s;
				ok = true;
				while (ok && end < S->n_rows-1) {
					end++;
					sincos (S->coord[h_col][end], &this_s, &this_c);
					dotprod = this_c * last_c + this_s * last_s;
					if (fabs (dotprod) > 1.0) dotprod = copysign (1.0, dotprod);
					if (d_acos (dotprod) > Ctrl->C.value) {	/* Fails due to too much change in azimuth  */
						ok = false;
						continue;
					}
					/* Get here when this point belongs with last one */
					csum += this_c;
					ssum += this_s;
					last_c = this_c;
					last_s = this_s;
				}

				/* Get here when we have found a beginning and end  */

				if (ok) end++;	/* Last point in input should be included in this segment  */

				if (end - begin - 1) { /* There are at least two points in the list.  */
					if ((S->coord[d_col][end-1] - S->coord[d_col][begin]) >= Ctrl->D.value) {
						/* List is long enough.  Check strike. Compute mean_azim in range [-pi/2, pi/2] */

						mean_azim = d_atan2 (ssum, csum);
						mean_azim = fabs (mean_azim - Ctrl->A.azimuth);
						if (mean_azim <= Ctrl->A.tolerance) {	/* List has acceptable strike.  */
							if (Ctrl->F.active) filter_cols (GMT, S->coord, begin, end, d_col, 2, xy_cols, Ctrl->F.xy_filter, fwork);
							nprofiles++;

							n_out = end - begin;
							if ((n_total + n_out) >= n_alloc) {
								n_alloc = (first) ? D[GMT_IN]->n_records : n_alloc * 2;
								GMT_alloc_segment (GMT, S_out, n_alloc, n_outputs, first);
								first = false;
							}

							for (row = begin; row < end; row++, k++) {
								for (j = 0; j < n_outputs; j++) {	/* Remember to convert CCW angles back to azimuths */
									S_out->coord[j][k] = (output_choice[j] == h_col) ? 90.0 - R2D * S->coord[h_col][row] : S->coord[output_choice[j]][row];
								}
							}
							if (seg2 == n_alloc_seg) {
								n_alloc_seg = (n_alloc_seg == 0) ? D[GMT_IN]->n_segments : n_alloc_seg * 2;
								rec = GMT_memory (GMT, rec, n_alloc_seg, uint64_t);
							}
							rec[seg2++] = k;
							n_total += n_out;
						}
					}
				}
				begin = end;
			}
			if (!Ctrl->S.active) {	/* Must remove the 2 cols we added  */
				for (col = D[GMT_IN]->n_columns; col < n_columns; col++) GMT_free (GMT, S->coord[col]);
				S->coord = GMT_memory (GMT, S->coord, S->n_columns, double *);
				S->min = GMT_memory (GMT, S->min, S->n_columns, double);
				S->max = GMT_memory (GMT, S->max, S->n_columns, double);
			}
		}
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
		Return (API->error);
	}
	
	/* Get here when all profiles have been found and written.  */

	if (nprofiles > 1) GMT_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */

	dim[GMT_SEG] = seg2;	dim[GMT_COL] = n_outputs;
	if ((D[GMT_OUT] = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_LINE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (API->error);	/* An empty table */
	for (seg = 0; seg < seg2; seg++) {	/* We fake a table by setting the coord pointers to point to various points in our single S_out arrays */
		S = D[GMT_OUT]->table[0]->segment[seg];
		k = (seg == 0) ? 0 : rec[seg-1];
		n = (seg == 0) ? rec[seg] : rec[seg] - rec[seg-1];
		for (j = 0; j < n_outputs; j++) S->coord[j] = &(S_out->coord[j][k]);
		S->n_rows = n;
		sprintf (header, "Profile %" PRIu64" -I%" PRIu64, seg, seg);
		S->header = strdup (header);
		S->id = seg;
	}

	GMT_Report (API, GMT_MSG_VERBOSE, " Split %" PRIu64 " data into %" PRIu64 " segments.\n", D[GMT_IN]->n_records, nprofiles);
	if (Ctrl->N.active) {
		int n_formats = 0;
		if (!Ctrl->N.name) Ctrl->N.name = (GMT->common.b.active[GMT_OUT]) ? strdup ("splitxyz_segment_%d.bin") : strdup ("splitxyz_segment_%d.txt");
		for (k = 0; Ctrl->N.name[k]; k++) if (Ctrl->N.name[k] == '%') n_formats++;
		D[GMT_OUT]->io_mode = (n_formats == 2) ? GMT_WRITE_TABLE_SEGMENT: GMT_WRITE_SEGMENT;
		/* The io_mode tells the i/o function to split segments into files */
		if (Ctrl->Out.file) free ((void*)Ctrl->Out.file);
		Ctrl->Out.file = strdup (Ctrl->N.name);
	}
	if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_PLP, io_mode, NULL, Ctrl->Out.file, D[GMT_OUT]) != GMT_OK) {
		Return (API->error);
	}

	/* Must set coord pointers to NULL since they were not allocated */
	for (seg = 0; seg < seg2; seg++) for (j = 0; j < n_outputs; j++) D[GMT_OUT]->table[0]->segment[seg]->coord[j] = NULL;
	GMT_free_segment (GMT, &S_out, GMT_ALLOCATED_BY_GMT);
	if (Ctrl->F.active) GMT_free (GMT, fwork);
	GMT_free (GMT, rec);

	Return (GMT_OK);
}
