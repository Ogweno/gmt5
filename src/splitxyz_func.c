/*--------------------------------------------------------------------
 *	$Id: splitxyz_func.c,v 1.5 2011-04-23 02:14:13 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
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

#include "gmt.h"

#define SPLITXYZ_F_RES			1000	/* Number of points in filter halfwidth  */
#define SPLITXYZ_N_OUTPUT_CHOICES	5

struct SPLITXYZ_CTRL {
	struct Out {	/* -> */
		GMT_LONG active;
		char *file;
	} Out;
	struct A {	/* -A<azimuth>/<tolerance> */
		GMT_LONG active;
		double azimuth, tolerance;
	} A;
	struct C {	/* -C<course_change> */
		GMT_LONG active;
		double value;
	} C;
	struct D {	/* -D<mindist> */
		GMT_LONG active;
		double value;
	} D;
	struct F {	/* -F<xy_filter>/<z_filter> */
		GMT_LONG active;
		double xy_filter, z_filter;
	} F;
	struct G {	/* -D<gap> */
		GMT_LONG active;
		double value;
	} G;
	struct M {	/* -M */
		GMT_LONG active;
	} M;
	struct N {	/* -N<namestem> */
		GMT_LONG active;
		char *name;
	} N;
	struct Q {	/* -Q[<xyzdg>] */
		GMT_LONG active;
		char col[SPLITXYZ_N_OUTPUT_CHOICES];	/* Character codes for desired output in the right order */
	} Q;
	struct S {	/* -S */
		GMT_LONG active;
	} S;
	struct Z {	/* -Z */
		GMT_LONG active;
	} Z;
};

void filterxy_setup (double *fwork)
{
	GMT_LONG i;
	double tmp, sum = 0.0;

	tmp = M_PI / SPLITXYZ_F_RES;
	for (i = 0; i < SPLITXYZ_F_RES; i++) {
		fwork[i] = 1.0 + cos (i * tmp);
		sum += fwork[i];
	}
	for (i = 1; i < SPLITXYZ_F_RES; i++) fwork[i] /= sum;
}

void filter_cols (struct GMT_CTRL *C, double *data[], GMT_LONG begin, GMT_LONG end, GMT_LONG d_col, GMT_LONG n_cols, GMT_LONG cols[], double filter_width, double *fwork)
{
	GMT_LONG i, j, k, p, istart, istop, ndata, hilow;
	double half_width, dt, sum, **w = NULL;

	if (filter_width == 0.0) return;	/* No filtering */
	hilow = (filter_width < 0.0);
	half_width = 0.5 * fabs (filter_width);
	dt = SPLITXYZ_F_RES / half_width;
	ndata = end - begin;
	w = GMT_memory (C, NULL, n_cols, double *);
	for (k = 0; k < n_cols; k++) w[k] = GMT_memory (C, NULL, ndata, double);	/* Initialized to zeros */
	j = istart = istop = begin;
	while (j < end) {
		while (istart < end && data[d_col][istart] - data[d_col][j] <= -half_width) istart++;
		while (istop  < end && data[d_col][istop]  - data[d_col][j] <   half_width) istop++;
		for (i = istart, sum = 0.0; i < istop; i++) {
			k = (GMT_LONG)floor (dt * fabs (data[d_col][i] - data[d_col][j]));
			if (k < 0 || k >= SPLITXYZ_F_RES) continue;	/* Safety valve */
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
	for (p = 0; p < n_cols; p++) GMT_free (C, w[p]);
	GMT_free (C, w);
}

void *New_splitxyz_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SPLITXYZ_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct SPLITXYZ_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
        C->A.azimuth = 90.0;
	C->A.tolerance = 360.0;
	C->G.value = DBL_MAX;	/* No gaps */
	return ((void *)C);
}

void Free_splitxyz_Ctrl (struct GMT_CTRL *GMT, struct SPLITXYZ_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->Out.file) free ((void *)C->Out.file);	
	if (C->N.name) free ((void *)C->N.name);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_splitxyz_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	/* This displays the splitxyz synopsis and optionally full usage information */

	GMT_message (GMT, "splitxyz %s [API] - Split xyz[dh] files into segments\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: splitxyz [<xyz[dh]file>] -C<course_change> [-A<azimuth>/<tolerance>]\n");
	GMT_message (GMT, "\t[-D<minimum_distance>] [-F<xy_filter>/<z_filter>] [-G<gap>] [-M]\n");
	GMT_message (GMT, "\t[-N<namestem>] [-Q<flags>] [-S] [%s] [-Z] [%s] [%s] [%s] [%s] [%s] [%s]\n\n",
		GMT_V_OPT, GMT_b_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\tGive xyz[dh]file name or read stdin.\n");
	GMT_message (GMT, "\t-C Profile ends when change of heading exceeds <course_change>.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-A Only write profile if mean direction is w/in +/- <tolerance>\n");
	GMT_message (GMT, "\t   of <azimuth>. [Default = All].\n");
	GMT_message (GMT, "\t-D Only write profile if length is at least <minimum_distance> [0].\n");
	GMT_message (GMT, "\t-F Filter the data.  Give full widths of cosine arch filters for xy and z.\n");
	GMT_message (GMT, "\t   Defaults are both widths = 0, giving no filtering.\n");
	GMT_message (GMT, "\t   Use negative width to highpass.\n");
	GMT_message (GMT, "\t-G Do not let profiles have gaps exceeding <gap>. [Default = 10 dist units].\n");
	GMT_message (GMT, "\t-M Map units TRUE; x,y in degrees, dist units in km.  [Default dist unit = x,y unit].\n");
	GMT_message (GMT, "\t-N Write output to separate files named <namestem>.profile#.\n");
	GMT_message (GMT, "\t   [Default all to stdout, separated by >].\n");
	GMT_message (GMT, "\t-Q Indicate what output you want as one or more of xyzdh in any order;\n");
	GMT_message (GMT, "\t   where x,y,z refer to input data locations and optional z-value(s),\n");
	GMT_message (GMT, "\t   and d,h are the distance and heading along track.\n");
	GMT_message (GMT, "\t   [Default is all fields, i.e. -Qxyzdh (or -Qxydh if -Z is set)]\n");
	GMT_message (GMT, "\t-S d,h is supplied.  Input is 5 col x,y,z,d,h with d non-decreasing.\n");
	GMT_message (GMT, "\t   [Default input is 3 col x,y,z only and computes d,h from the data].\n");
	GMT_message (GMT, "\t-Z No z-values.  Input is 2 col x,y only.\n");
	GMT_explain_options (GMT, "VC0");
	GMT_message (GMT, "\t     Default input columns is set given -S and -Z options.\n");
	GMT_explain_options (GMT, "D0fghi:.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_splitxyz_parse (struct GMTAPI_CTRL *C, struct SPLITXYZ_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to splitxyz and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG j, n_errors = 0, n_outputs = 0, n_files = 0, z_selected = FALSE;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				break;
			case '>':	/* Got named output file */
				if (n_files++ == 0) Ctrl->Out.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = TRUE;
				n_errors += GMT_check_condition (GMT,  (sscanf(opt->arg, "%lf/%lf", &Ctrl->A.azimuth, &Ctrl->A.tolerance)) != 2, "Syntax error -A option: Can't decipher values\n");
				break;
			case 'C':
				Ctrl->C.active = TRUE;
				n_errors += GMT_check_condition (GMT,  (sscanf(opt->arg, "%lf", &Ctrl->C.value)) != 1, "Syntax error -C option: Can't decipher value\n");
				break;
			case 'D':
				Ctrl->D.active = TRUE;
				n_errors += GMT_check_condition (GMT,  (sscanf(opt->arg, "%lf", &Ctrl->D.value)) != 1, "Syntax error -D option: Can't decipher value\n");
				break;
			case 'F':
				Ctrl->F.active = TRUE;
				n_errors += GMT_check_condition (GMT,  (sscanf(opt->arg, "%lf/%lf", &Ctrl->F.xy_filter, &Ctrl->F.z_filter)) != 2, "Syntax error -F option: Can't decipher values\n");
				break;
			case 'G':
				Ctrl->G.active = TRUE;
				n_errors += GMT_check_condition (GMT,  (sscanf(opt->arg, "%lf", &Ctrl->G.value)) != 1, "Syntax error -G option: Can't decipher value\n");
				break;
			case 'M':
				Ctrl->M.active = TRUE;
				break;
			case 'N':
				Ctrl->N.active = TRUE;
				if (opt->arg[0])
					Ctrl->N.name = strdup (opt->arg);
				else {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -N option: Append a name stem\n");
					n_errors++;
				}
				break;
			case 'Q':
				Ctrl->Q.active = TRUE;
				for (j = 0; opt->arg[j]; j++) {
					if (j < SPLITXYZ_N_OUTPUT_CHOICES) {
						Ctrl->Q.col[j] = opt->arg[j];
						if (!strchr ("xyzdh", Ctrl->Q.col[j])) {
							GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -Q option: Unrecognized output choice %c\n", Ctrl->Q.col[j]);
							n_errors++;
						}
						if (opt->arg[j] == 'z') z_selected = TRUE;
						n_outputs++;
					}
					else {
						GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -Q option: Too many output columns selected: Choose from -Qxyzdg\n");
						n_errors++;
					}
				}
				break;
			case 'S':
				Ctrl->S.active = TRUE;
				break;
			case 'Z':
				Ctrl->Z.active = TRUE;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
               }
	}

	n_errors += GMT_check_condition (GMT, Ctrl->D.value < 0.0, "Syntax error -D option: Minimum segment distance must be positive\n");
	n_errors += GMT_check_condition (GMT, Ctrl->C.value <= 0.0, "Syntax error -C option: Course change tolerance must be positive\n");
	n_errors += GMT_check_condition (GMT, Ctrl->A.tolerance < 0.0, "Syntax error -A option: Azimuth tolerance must be positive\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.value < 0.0, "Syntax error -G option: Data gap distance must be positive\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Z.active && Ctrl->S.active, "Syntax error -Z option: Cannot be used with -S option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Z.active && Ctrl->F.z_filter != 0.0, "Syntax error -F option: Cannot specify z-filter while using -Z option\n");
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_OUT] && !Ctrl->N.name, "Syntax error: Binary output requires a namestem in -N\n");
	n_errors += GMT_check_condition (GMT, n_outputs > 0 && z_selected && Ctrl->Z.active, "Syntax error -Q option: Cannot request z if -Z have been specified\n");
	n_errors += GMT_check_binary_io (GMT, (Ctrl->S.active) ? 5 : ((Ctrl->Z.active) ? 2 : 3));
	n_errors += GMT_check_condition (GMT, n_files > 1, "Syntax error: Only one output destination can be specified\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_splitxyz_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_splitxyz (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG i, tbl, seg, begin, row, col, end, d_col, h_col, z_cols, xy_cols[2] = {0, 1};
	GMT_LONG k, output_choice[SPLITXYZ_N_OUTPUT_CHOICES], n_outputs = 0, n_columns = 0, out_ID;
	GMT_LONG error = FALSE, ok, io_mode = 0, nprofiles = 0;

	double dy, dx, last_d, last_c, last_s, csum, ssum, this_c, this_s, dotprod;
	double mean_azim, fwork[SPLITXYZ_F_RES];

	struct GMT_DATASET *D = NULL;
	struct GMT_TABLE *T = NULL;
	struct GMT_LINE_SEGMENT *S = NULL;
	struct SPLITXYZ_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_splitxyz_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_splitxyz_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, "GMT_splitxyz", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-Vbf:", "ghis>" GMT_OPT("H"), options))) Return (error);
	Ctrl = (struct SPLITXYZ_CTRL *)New_splitxyz_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_splitxyz_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the splitxyz main code ----------------------------*/

	GMT_memset (output_choice, SPLITXYZ_N_OUTPUT_CHOICES, GMT_LONG);

	for (k = n_outputs = 0; k < SPLITXYZ_N_OUTPUT_CHOICES && Ctrl->Q.col[k]; k++) {
		switch (Ctrl->Q.col[k]) {
			case 'x':
				output_choice[k] = 0;
				break;
			case 'y':
				output_choice[k] = 1;
				break;
			case 'z':
				output_choice[k] = 2;
				break;
			case 'd':
				output_choice[k] = 3;
				break;
			case 'h':
				output_choice[k] = 4;
				break;
		}
		n_outputs++;
	}
	if (Ctrl->M.active) {
		GMT->current.io.col_type[GMT_IN][GMT_X] = GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT_IS_LON;
		GMT->current.io.col_type[GMT_IN][GMT_Y] = GMT->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_LAT;
	}
	if (n_outputs == 0) {	/* Generate default -Q setting (all) */
		n_outputs = 5 - Ctrl->Z.active;
		for (i = 0; i < 2; i++) output_choice[i] = (GMT_LONG)i;
		for (i = 2; i < n_outputs; i++) output_choice[i] = (GMT_LONG)i + Ctrl->Z.active;
	}

	Ctrl->A.tolerance *= D2R;
	/* if (Ctrl->A.azimuth > 180.0) Ctrl->A.azimuth -= 180.0; */	/* Put in Easterly strikes  */
	Ctrl->A.azimuth = D2R * (90.0 - Ctrl->A.azimuth);	/* Work in cartesian angle and radians  */
	Ctrl->C.value *= D2R;
	filterxy_setup (fwork);
	if (Ctrl->N.active) {
		GMT_LONG n_formats;
		for (col = n_formats = 0; Ctrl->N.name[col]; col++) if (Ctrl->N.name[col] == '%') n_formats++;
		io_mode = (n_formats == 2) ? GMT_WRITE_TABLE_SEGMENTS: GMT_WRITE_SEGMENTS;
	}

	if ((error = GMT_set_cols (GMT, GMT_IN, 3))) Return (error);
	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Establishes data input */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
	if ((error = GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, NULL, GMT_FILE_BREAK, NULL, (void **)&D))) Return (error);
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	if (!Ctrl->S.active) {	/* Must extend table with 2 cols to hold d and az */
		n_columns = D->n_columns + 2;
		d_col = D->n_columns;
		h_col = d_col + 1;
	}
	else {	/* Comes with d and az in file */
		d_col = Ctrl->Z.active + 2;
		h_col = Ctrl->Z.active + 3;
	}
	z_cols = 2;
	for (tbl = 0; tbl < D->n_tables; tbl++) {
		T = D->table[tbl];
		if (!Ctrl->S.active) {	/* Must extend table with 2 cols to hold d and az */
			T->min = GMT_memory (GMT, T->min, n_columns, double);
			T->max = GMT_memory (GMT, T->max, n_columns, double);
		}
		GMT_report (GMT, GMT_MSG_FATAL, "Working on file %s\n", T->file[GMT_IN]);

		for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {	/* For each segment in the table */
			S = T->segment[seg];
			if (!Ctrl->S.active) {	/* Must extend table with 2 cols to hold d and az */
				S->coord = GMT_memory (GMT, S->coord, n_columns, double *);
				S->min = GMT_memory (GMT, S->min, n_columns, double);
				S->max = GMT_memory (GMT, S->max, n_columns, double);
				for (col = D->n_columns; col < n_columns; col++) S->coord[col] = GMT_memory (GMT, NULL, S->n_rows, double);
			}
			
			if (Ctrl->S.active)S->coord[h_col][0] = D2R * (90.0 - S->coord[h_col][0]);	/* Angles are stored as CCW angles in radians */
			for (row = 1; row < S->n_rows; row++) {
				if (!Ctrl->S.active) {	/* Must extend table with 2 cols to hold d and az */
					dx = (S->coord[GMT_X][row] - S->coord[GMT_X][row-1]);
					dy = (S->coord[GMT_Y][row] - S->coord[GMT_Y][row-1]);
					if (Ctrl->M.active) {
						dy *= GMT->current.proj.DIST_KM_PR_DEG;
						dx *= (GMT->current.proj.DIST_KM_PR_DEG * cosd (0.5 * (S->coord[GMT_Y][row] + S->coord[GMT_Y][row-1])));
					}
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

			filter_cols (GMT, S->coord, 0, S->n_rows, d_col, 1, &z_cols, Ctrl->F.z_filter, fwork);

			/* Now we are ready to search for segments.  */

			nprofiles = begin = end = 0;
			while (end < S->n_rows-1) {
				last_d = S->coord[d_col][begin];
				sincos (S->coord[h_col][begin], &last_s, &last_c);
				csum = last_c;	ssum = last_s;
				ok = TRUE;
				while (ok && end < S->n_rows-1) {
					end++;
					if (S->coord[d_col][end] - last_d > Ctrl->G.value) {	/* Fails due to too much distance gap  */
						ok = FALSE;
						continue;
					}
					sincos (S->coord[h_col][end], &this_s, &this_c);
					dotprod = this_c * last_c + this_s * last_s;
					if (fabs (dotprod) > 1.0) dotprod = copysign (1.0, dotprod);
					if (d_acos (dotprod) > Ctrl->C.value) {	/* Fails due to too much change in azimuth  */
						ok = FALSE;
						continue;
					}
					/* Get here when this point belongs with last one */
					csum += this_c;
					ssum += this_s;
					last_c = this_c;
					last_s = this_s;
					last_d = S->coord[d_col][end];
				}

				/* Get here when we have found a beginning and end  */

				if (ok) end++;	/* Last point in input should be included in this segment  */

				if (end - begin - 1) { /* There are at least two points in the list.  */
					if ((S->coord[d_col][end-1] - S->coord[d_col][begin]) >= Ctrl->D.value) {
						/* List is long enough.  Check strike. Compute mean_azim in range [-pi/2, pi/2] */

						mean_azim = d_atan2 (ssum, csum);
						mean_azim = fabs (mean_azim - Ctrl->A.azimuth);
						if (mean_azim <= Ctrl->A.tolerance) {	/* List has acceptable strike.  */
							filter_cols (GMT, S->coord, begin, end, d_col, 2, xy_cols, Ctrl->F.xy_filter, fwork);
							nprofiles++;
						}
					}
				}
				begin = end;
			}

		}
	}
	
	/* Get here when all profiles have been found and written.  */

	GMT_report (GMT, GMT_MSG_NORMAL, " Split %ld data into %ld files.\n", D->n_records, nprofiles);
	if (Ctrl->N.active) {
		D->io_mode = io_mode;
		/* The io_Mode tells the i/o function to split segments into files */
		/* Register output as a file with the template filename */
		GMT_Register_IO (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_OUT, (void **)&Ctrl->N.name, NULL, D, &out_ID);
	}
	else {	/* Just register output to stdout */
		if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, options))) Return (error);	/* Establishes data output */
	}
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
	if ((error = GMT_Put_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, NULL, io_mode, (void **)&Ctrl->Out.file, (void *)D))) Return (error);
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */

	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&D);
	GMT_free (GMT, fwork);

	Return (GMT_OK);
}
