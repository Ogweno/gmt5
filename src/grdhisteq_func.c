/*--------------------------------------------------------------------
 *	$Id: grdhisteq_func.c,v 1.6 2011-04-23 02:14:12 guru Exp $
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
 * Brief synopsis: read a grid file and find the values which divide its range
 * into n_cell number of quantiles.
 *
 * Author:	W.H.F. Smith
 * Date: 	31 May 1990
 * Version:	5 API
 */
 
#include "gmt.h"

struct GRDHISTEQ_CTRL {
	struct In {
		GMT_LONG active;
		char *file;
	} In;
	struct C {	/* -C<n_cells>*/
		GMT_LONG active;
		GMT_LONG value;
	} C;
	struct D {	/* -D */
		GMT_LONG active;
	} D;
	struct G {	/* -G<file> */
		GMT_LONG active;
		char *file;
	} G;
	struct N {	/* -N[<norm>] */
		GMT_LONG active;
		double norm;
	} N;
	struct Q {	/* -Q */
		GMT_LONG active;
	} Q;
};

struct	INDEXED_DATA {
	float x;
	GMT_LONG i;
};

struct	CELL {
	float low;
	float high;
};

void *New_grdhisteq_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDHISTEQ_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDHISTEQ_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	return ((void *)C);
}

void Free_grdhisteq_Ctrl (struct GMT_CTRL *GMT, struct GRDHISTEQ_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free ((void *)C->In.file);	
	if (C->G.file) free ((void *)C->G.file);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_grdhisteq_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdhisteq %s [API] - Histogram equalization for grid files\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdhisteq <infile> [-G<outfile>] [-C<n_cells>] [-D] [-N[<norm>]] [-Q] [%s] [%s]\n", GMT_Rgeo_OPT, GMT_V_OPT);
	
	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);
	
	GMT_message (GMT, "\t<infile> is name of input grid file.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-C Sets how many cells (divisions) of data range to make.\n");
	GMT_message (GMT, "\t-D Dump level information to stdout.\n");
	GMT_message (GMT, "\t-G Create an equalized output grid file called <outfile>.\n");
	GMT_message (GMT, "\t-N Use with -G to make an output grid file with standard normal scores.\n");
	GMT_message (GMT, "\t   Append <norm> to normalize the scores to <-1,+1>.\n");
	GMT_message (GMT, "\t-Q Use quadratic intensity scaling [Default is linear].\n");
	GMT_explain_options (GMT, "RV.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdhisteq_parse (struct GMTAPI_CTRL *C, struct GRDHISTEQ_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdhisteq and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			case '<':	/* Input file (only one is accepted) */
				Ctrl->In.active = TRUE;
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Get # of cells */
				Ctrl->C.active = TRUE;
				Ctrl->C.value = atoi (opt->arg);
				break;
			case 'D':	/* Dump info to stdout */
				Ctrl->D.active = TRUE;
				break;
			case 'G':	/* Output file for equalized grid */
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'N':	/* Get normalized scores */
				Ctrl->N.active = TRUE;
				Ctrl->N.norm = atof (opt->arg);
				break;
			case 'Q':	/* Use quadratic scaling */
				Ctrl->Q.active = TRUE;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, n_files > 1, "Syntax error: Must specify a single input grid file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input grid file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.active && !Ctrl->G.file, "Syntax error -N option: Must also specify output grid file with -G\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->N.active && Ctrl->C.value <= 0, "Syntax error -C option: n_cells must be positive\n");
	n_errors += GMT_check_condition (GMT, !strcmp (Ctrl->In.file, "="), "Syntax error: Piping of input grid file not supported!\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

float get_cell (float x, struct CELL *cell, GMT_LONG n_cells_m1, GMT_LONG last_cell)
{
	GMT_LONG low, high, i;

	low = 0;
	high = n_cells_m1;
	i = last_cell;

	do {
		if (cell[i].low <= x && cell[i].high >= x) {
			last_cell = i;
			return ((float)i);
		}
		else if (cell[low].low <= x && cell[low].high >= x) {
			return ((float)low);
		}
		else if (cell[high].low <= x && cell[high].high >= x) {
			return ((float)high);
		}
		else if (cell[i].low > x) {
			high = i;
			i = (low + high) / 2;
		}
		else if (cell[i].high < x) {
			low = i;
			i = (low + high) / 2;
		}
	} while (TRUE);
	return (0.0);	/* Cannot get here - just used to quiet compiler */
}

GMT_LONG do_usual (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, char *infile, char *outfile, GMT_LONG n_cells, GMT_LONG quadratic, GMT_LONG dump_intervals)
{
	GMT_LONG last_cell, n_cells_m1 = 0, current_cell, i, j, nxy, pad[4];
	char format[BUFSIZ];
	double delta_cell, target;
	struct CELL *cell = NULL;
	
	sprintf (format, "%s\t%s\t%%ld\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
	
	cell = GMT_memory (GMT, NULL, n_cells, struct CELL);

	/* Sort the data and find the division points */

	GMT_memcpy (pad, Grid->header->pad, 4, GMT_LONG);	/* Save the original pad */
	GMT_grd_pad_off (GMT, Grid);	/* Undo pad if one existed so we can sort */
	GMT_sort_array ((void *)Grid->data, Grid->header->nm, GMT_FLOAT_TYPE);
	
	nxy = Grid->header->nm;
	while (nxy > 0 && GMT_is_fnan (Grid->data[nxy-1])) nxy--;	/* Only deal with real numbers */

	last_cell = n_cells / 2;
	n_cells_m1 = n_cells - 1;

	current_cell = i = 0;
	delta_cell = ((double)nxy) / ((double)n_cells);

	while (current_cell < n_cells) {

		if (current_cell == (n_cells - 1))
			j = nxy - 1;
		else if (quadratic) {	/* Use y = 2x - x**2 scaling  */
			target = ((double) (current_cell + 1)) / ((double) n_cells);
			j = (GMT_LONG)floor (nxy * (1.0 - sqrt (1.0 - target)));
		}
		else	/* Use simple linear scale  */
			j = (GMT_LONG)(floor ((current_cell + 1) * delta_cell)) - 1;

		cell[current_cell].low = Grid->data[i];
		cell[current_cell].high = Grid->data[j];

		if (dump_intervals) GMT_fprintf (GMT->session.std[GMT_OUT], format, Grid->data[i], Grid->data[j], current_cell);

		i = j;
		current_cell++;
	}

	if (outfile) {	/* Must re-read the grid and evaluate since it got sorted and trodden on... */
		GMT_Destroy_Data (GMT->parent, GMT_ALLOCATED, (void **)&Grid);
		if (GMT_Get_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, (void **)&infile, (void **)&Grid)) return (GMT_DATA_READ_ERROR);
		GMT_grd_pad_off (GMT, Grid);	/* Undo pad if one existed (again) */

		for (i = 0; i < Grid->header->nm; i++) Grid->data[i] = (GMT_is_fnan (Grid->data[i])) ? GMT->session.f_NaN : get_cell (Grid->data[i], cell, n_cells_m1, last_cell);
	}

	GMT_grd_pad_on (GMT, Grid, pad);	/* Reinstate the oroginal pad */
	GMT_free (GMT, cell);
	return (0);
}

int compare_indexed_floats (const void *point_1, const void *point_2)
{
	if (((struct INDEXED_DATA *)point_1)->x < ((struct INDEXED_DATA *)point_2)->x) return (-1);
	if (((struct INDEXED_DATA *)point_1)->x > ((struct INDEXED_DATA *)point_2)->x) return (1);
	return (0);
}

int compare_indices (const void *point_1, const void *point_2)
{
	if (((struct INDEXED_DATA *)point_1)->i < ((struct INDEXED_DATA *)point_2)->i) return (-1);
	if (((struct INDEXED_DATA *)point_1)->i > ((struct INDEXED_DATA *)point_2)->i) return (1);
	return (0);
}

GMT_LONG do_gaussian (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, char *infile, char *outfile, double norm)
{
	GMT_LONG i = 0, j = 0, ij, row, col, nxy;
	double dnxy;
	struct INDEXED_DATA *indexed_data = NULL;

	indexed_data = GMT_memory (GMT, NULL, Grid->header->nm, struct INDEXED_DATA);

	nxy = Grid->header->nm;
	GMT_grd_loop (Grid, row, col, ij) {
		if (GMT_is_fnan (Grid->data[ij])) {	/* Put NaNs in the back */
			nxy--;
			indexed_data[nxy].i = ij;
			indexed_data[nxy].x = Grid->data[ij];
		}
		else {
			indexed_data[j].i = ij;
			indexed_data[j].x = Grid->data[ij];
			j++;
		}
	}

	/* Sort on data value  */

	qsort ((void *)indexed_data, (size_t)nxy, sizeof (struct INDEXED_DATA), compare_indexed_floats);

	dnxy = 1.0 / (nxy + 1);

	if (norm != 0.0) norm /= fabs (GMT_zcrit (GMT, (double)dnxy));	/* Normalize by abs(max score) */

	for (i = 0; i < nxy; i++) {
		indexed_data[i].x = (float)GMT_zcrit (GMT, (double)((i + 1) * dnxy));
		if (norm != 0.0) indexed_data[i].x *= (float)norm;
	}

	/* Sort on data index  */

	qsort ((void *)indexed_data, (size_t)Grid->header->nm, sizeof (struct INDEXED_DATA), compare_indices);

	i = 0;
	GMT_grd_loop (Grid, row, col, ij) Grid->data[ij] = indexed_data[i++].x;	/* Load up the grid */

	GMT_free (GMT, indexed_data);
	return (0);
}

#define Return(code) {Free_grdhisteq_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_grdhisteq (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG error = FALSE;

	double wesn[4];
	
	struct GMT_GRID *Grid = NULL;
	struct GRDHISTEQ_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grdhisteq_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grdhisteq_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdhisteq", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VR", "", options))) Return (error);
	Ctrl = (struct GRDHISTEQ_CTRL *) New_grdhisteq_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdhisteq_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdhisteq main code ----------------------------*/

	GMT_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Current -R setting, if any */
	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
	if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(Ctrl->In.file), (void **)&Grid)) Return (GMT_DATA_READ_ERROR);
	if (GMT_is_subset (Grid->header, wesn)) GMT_err_fail (GMT, GMT_adjust_loose_wesn (GMT, wesn, Grid->header), "");	/* Subset requested; make sure wesn matches header spacing */
	if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn, GMT_GRID_DATA, (void **)&(Ctrl->In.file), (void **)&Grid)) Return (GMT_DATA_READ_ERROR);	/* Get subset */
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);				/* Disables further data input */
	GMT_grd_init (GMT, Grid->header, options, TRUE);

	if (Ctrl->N.active)
		error = do_gaussian (GMT, Grid, Ctrl->In.file, Ctrl->G.file, Ctrl->N.norm);
	else {
		if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, options))) Return (error);	/* Registers default output destination, unless already set */
		if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_SET))) Return (error);				/* Enables data input and sets access mode */
		if ((error = do_usual (GMT, Grid, Ctrl->In.file, Ctrl->G.file, Ctrl->C.value, Ctrl->Q.active, Ctrl->D.active))) {	/* Read error */
			GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Grid);
			Return (EXIT_FAILURE);
		}
		if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);				/* Disables further data input */
	}

	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_REC))) Return (error);				/* Enables data output and sets access mode */
	if (Ctrl->G.active && GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, (void **)&(Ctrl->G.file), (void **)&Grid)) Return (GMT_DATA_READ_ERROR);	/* Get header only */
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);				/* Disables further data output */

	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Grid);

	Return (EXIT_SUCCESS);
}
