/*--------------------------------------------------------------------
 *	$Id$
 *
 *   Copyright (c) 1999-2015 by P. Wessel
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation; version 3 or any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   Contact info: www.soest.hawaii.edu/pwessel
 *--------------------------------------------------------------------*/
/*
 * HOTSPOTTER will (1) read ascii file(s) with records for each seamount
 * (2) read an ascii file with stage or total reconstruction rotations,
 * (3) convolve the flowline of each seamount with its gravimetric shape, and
 * (4) build a cumulative volcano amplitude (CVA) grid.  The grid is
 * written out in GMT format and can be processed and plotted with GMT.
 * HOTSPOTTER is part of the SPOTTER supplemental GMT package and should
 * be installed accordingly, see README.spotter.
 *
 * Author:	Paul Wessel, SOEST, Univ. of Hawaii, Honolulu, HI, USA
 * Date:	22-JUN-1999
 * Version:	1.0
 *
 *-------------------------------------------------------------------------
 * An ASCII stage pole (Euler) file must have following format:
 *
 * 1. Any number of comment lines starting with # in first column
 * 2. Any number of blank lines (just carriage return, no spaces)
 * 2. Any number of stage pole records which each have the format:
 *    lon(deg)  lat(deg)  tstart(Ma)  tstop(Ma)  ccw-angle(deg)
 * 3. stage records must go from oldest to youngest rotation
 * 4. Note tstart is larger (older) that tstop for each record
 * 5. No gaps allowed: tstart must equal the previous records tstop
 *
 * Example: Duncan & Clague [1985] Pacific-Hotspot rotations:
 *
 * # Time in Ma, angles in degrees
 * # lon  lat	tstart	tend	ccw-angle
 * 165     85	150	100	24.0
 * 284     36	100	74	15.0
 * 265     22	74	65	7.5
 * 253     17	65	42	14.0
 * 285     68	42	0	34.0
 *
 * AN ASCII total reconstruction file must have the following format:
*
 * 1. Any number of comment lines starting with # in first column
 * 2. Any number of blank lines (just carriage return, no spaces)
 * 2. Any number of finite pole records which each have the format:
 *    lon(deg)  lat(deg)  tstop(Ma)  ccw-angle(deg)
 * 3. total reconstructions rotations must go from youngest to oldest
 *
 * Example: Duncan & Clague [1985] Pacific-Hotspot rotations:
 *
 * # Time in Ma, angles in degrees
 * #longitude	latitude	time(My)	angle(deg)
 * 285.00000	 68.00000	 42.0000	 34.0000
 * 275.66205	 53.05082	 65.0000	 43.5361
 * 276.02501	 48.34232	 74.0000	 50.0405
 * 279.86436	 46.30610	100.0000	 64.7066
 * 265.37800	 55.69932	150.0000	 82.9957
 *
 *
 * Seamount data file(s) must have the following format:
 * 1. Any number of comment lines starting with # in first column
 * 2. Any number of blank lines (just carriage return, no spaces)
 * 3. Any number of seamount records which each have the format:
 *    lon(deg)  lat(deg)  amplitude radius(km) age(Ma)
 * 4. The amplitude is in user units (m, mGal, km^3, whatever)
 * 5. Age represents the upper possible age for seamount, which
 *    is usually the age of the seafloor beneath it.  If the
 *    crustal age is not known, set it to NaN in the file.
 *    NaN values will be replaced with -N (see usage)
 *
 * Example: Wessel & Lyons [1997] Pacific seamounts (just a few records):
 * # Pacific seamounts > 100 Eotvos amplitude in Vertical Gravity Gradient
 * # From Wessel & Lyons [1997]
 * #LON		  FAA	 	 VGG	RADIUS	CRUST_AGE
 * 134.38333	0.9436415	120.5	 22.97	37.606796
 * 136.05	7.6325042	102.7	 18.67	NaN
 * 131.28333	1.1423035	129.0	 17.16	NaN
 * .....
 *
 *------------------------------------------------------------------------------
 * REFERENCES:
 *
 * -> The hotspotting technique:
 *
 * Aslanian, D., L. Geli, and J.-L. Olivet, 1998, Hotspotting called into
 *    question, Nature, 396, 127.
 * Wessel, P., and L. W. Kroenke, 1997, A geometric technique for
 *    relocating hotspots and refining absolute plate motions, Nature,
 *    387, 365-369.
 * Wessel, P., and L. W. Kroenke, 1998a, Factors influencing the locations
 *    of hot spots determined by the hot-spotting technique, Geophys. Res.
 *    Lett., 25, 55-58.
 * Wessel, P., and L. W. Kroenke, 1998b, The geometric relationship between
 *    hot spots and seamounts: implications for Pacific hot spots, Earth
 *    Planet. Sci. Lett., 158, 1-18.
 * Wessel, P., and L. W. Kroenke, 1998c, Hotspotting called into question
 *    - Reply, Nature, 396, 127-128.
 *
 * -> Seamount data set:
 *
 * Wessel, P., and S. Lyons, 1997, Distribution of large Pacific seamounts
 *    from Geosat/ERS-1: Implications for the history of intraplate volcanism,
 *    J. Geophys. Res., 102,22,459-22,475.
 * Wessel, P., 1997, Sizes and ages of seamounts using remote sensing:
 *    Implications for intraplate volcanism, Science, 277, 802-805.
 *
 * -> Plate motion models (stage poles):
 *
 * Duncan, R.A., and D. Clague, 1985, Pacific plate motion recorded by linear
 *    volcanic chains, in: A.E.M. Nairn, F. G. Stehli, S. Uyeda (eds.), The
 *    Ocean Basins and Margins, Vol. 7A, Plenum, New York, pp. 89-121.
 * Wessel and Kroenke, 1997, (see above)
 *
 */
 
#define THIS_MODULE_NAME	"hotspotter"
#define THIS_MODULE_LIB		"spotter"
#define THIS_MODULE_PURPOSE	"Create CVA image from seamount locations"

#include "spotter.h"

#define GMT_PROG_OPTIONS "-:>RVbghirs" GMT_OPT("FHMm")

struct HOTSPOTTER_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct D {	/* -D<factor> */
		bool active;
		double value;	/* Resampling factor */
	} D;
	struct E {	/* -E[+]rotfile */
		bool active;
		bool mode;
		char *file;
	} E;
	struct G {	/* -Goutfile */
		bool active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct N {	/* -N */
		bool active;
		double t_upper;
	} N;
	struct S {	/* -S */
		bool active;
		char *file;
	} S;
	struct T {	/* -T<tzero> */
		bool active;
		double t_zero;	/* Set zero age*/
	} T;
};

void *New_hotspotter_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct HOTSPOTTER_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct HOTSPOTTER_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	
	C->D.value = 0.5;	/* Sets how many points along flowline per node */
	C->N.t_upper = 180.0;	/* Upper age assigned to seamounts on undated seafloor */
	return (C);
}

void Free_hotspotter_Ctrl (struct GMT_CTRL *GMT, struct HOTSPOTTER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->E.file) free (C->E.file);	
	if (C->G.file) free (C->G.file);	
	if (C->S.file) free (C->S.file);	
	GMT_free (GMT, C);	
}

int GMT_hotspotter_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: hotspotter [<table>] -E[+]<rottable> -G<CVAgrid> %s\n", GMT_Id_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s [-D<factor>] [-N<upper_age>] [-S] [-T] [%s]\n", GMT_Rgeo_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s]\n\t[%s] [%s] [%s]\n\n",
		GMT_bi_OPT, GMT_h_OPT, GMT_i_OPT, GMT_r_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	spotter_rot_usage (API, 'E');
	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify file name for output CVA grid.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Specify grid interval(s); Append m [or c] to <dx> and/or <dy> for minutes [or seconds].\n");
	GMT_Option (API, "Rg");
	GMT_Message (API, GMT_TIME_NONE, "\t<table> (in ASCII, binary, or netCDF) has 5 or more columns.  If no file(s) is given,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   standard input is read.  Expects (x,y,z,r,t) records, with t in Ma.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Scale affecting distance between points along flowline [0.5].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Set upper age in m.y. for seamounts whose plate age is NaN [180].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Normalize CVA grid to percentages of the CVA maximum.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Truncate all ages to max age in stage pole model [Default extrapolates].\n");
	GMT_Option (API, "V,bi5,h,i,r,:,.");
	
	return (EXIT_FAILURE);
}

int GMT_hotspotter_parse (struct GMT_CTRL *GMT, struct HOTSPOTTER_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to hotspotter and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Supplemental parameters */
			case 'C':	/* Now done automatically in spotter_init */
				if (GMT_compat_check (GMT, 4))
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: -C is no longer needed as total reconstruction vs stage rotation is detected automatically.\n");
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;

			case 'D':
				Ctrl->D.active = true;
				Ctrl->D.value = atof (opt->arg);
				break;
			case 'E':
				Ctrl->E.active = true;	k = 0;
				if (opt->arg[0] == '+') { Ctrl->E.mode = true; k = 1;}
				if (GMT_check_filearg (GMT, 'E', &opt->arg[k], GMT_IN, GMT_IS_DATASET))
					Ctrl->E.file  = strdup (&opt->arg[k]);
				else
					n_errors++;
				break;
			case 'G':
				if ((Ctrl->G.active = GMT_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':
				Ctrl->I.active = true;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'N':
				Ctrl->N.active = true;
				Ctrl->N.t_upper = atof (opt->arg);
				break;
			case 'S':
				Ctrl->S.active = true;
				break;
			case 'T':
				Ctrl->T.active = true;
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.registration, &Ctrl->I.active);

        if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 5;
	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, !(Ctrl->G.active || Ctrl->G.file), "Syntax error option: Must specify output file\n");
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 5, "Syntax error option: Binary input data (-bi) must have at least 5 columns\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_hotspotter_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_hotspotter (void *V_API, int mode, void *args)
{

	uint64_t n_smts;		/* Number of seamounts read */
	uint64_t n_track;		/* Number of points along a single flowline */
	uint64_t n_read = 0;	/* Number of records read */
	uint64_t node, node_0;		/* Grid indices */
	uint64_t n_expected_fields;
	unsigned int n_stages;	/* Number of stage rotations (poles) */
	unsigned int row, col, kx, ky, m;
	int node_x_width;		/* Number of x-nodes covered by the seamount in question (y-dependent) */
	int node_y_width;		/* Number of y-nodes covered by the seamount */
	int d_col, d_row, col_0, row_0, nx, ny;
	int error = 0;			/* nonzero when arguments are wrong */
	

	double sampling_int_in_km;	/* Sampling interval along flowline (in km) */
	double x_smt;			/* Seamount longitude (input degrees, stored as radians) */
	double y_smt;			/* Seamount latitude (input degrees, stored as radians) */
	double z_smt;			/* Seamount amplitude (in user units) */
	double r_smt;			/* Seamount radius (input km, stored as radians) */
	double t_smt;			/* Seamount upper age (up to age of seafloor) */
	double norm;			/* Normalization factor based on r_smt */
	double *xpos = NULL, *ypos = NULL;		/* Coordinates of the output grid (in radians) */
	double *c = NULL;			/* Array with one flowline */
	double dx, dy;			/* x,y distance from projected seamount center to nearest node */
	double *latfactor = NULL, *ilatfactor = NULL;	/* Arrays of latitudinal-dependent x-scales (cos(lat)-stuff) */
	double x_part, y_part;		/* Components of radius from projected seamount center to current node */
	double y_part2;			/* y_part squared */
	double i_yinc_r;		/* 1/y_inc (in radians) */
	double r2;			/* Radius squared from projected seamount center to current node */
	double *in = NULL;		/* GMT read array */
	double wesn[4];			/* Map region in geocentric coordinates in radians */
	double yg;			/* Geodetic latitude */

	char *processed_node = NULL;	/* Pointer to array with true/false values for each grid node */

	struct GMT_GRID *G = NULL;	/* Grid structure for output CVA grid */
	struct GMT_GRID *G_rad = NULL;	/* Same but has radians in header (no grid) */

	struct EULER *p = NULL;		/* Array of structures with Euler stage rotations */
	struct GMT_OPTION *ptr = NULL;
	struct HOTSPOTTER_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_hotspotter_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_hotspotter_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_hotspotter_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if ((ptr = GMT_Find_Option (API, 'f', options)) == NULL) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Did not set -f, implicitly set -fg */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_hotspotter_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_hotspotter_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the hotspotter main code ----------------------------*/

	/* Load in the Euler stage poles */

	n_stages = spotter_init (GMT, Ctrl->E.file, &p, true, false, Ctrl->E.mode, &Ctrl->N.t_upper);

	/* Initialize the CVA grid and structure */

	if ((G = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, NULL, Ctrl->I.inc, \
		GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);

	/* Assign grid-region variables in radians to avoid conversions inside convolution loop */

	if ((G_rad = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_NONE, G)) == NULL) Return (API->error);
	G_rad->header->inc[GMT_X] = G->header->inc[GMT_X] * D2R;
	G_rad->header->inc[GMT_Y] = G->header->inc[GMT_Y] * D2R;
	G_rad->header->wesn[XLO]  = G->header->wesn[XLO] * D2R;
	G_rad->header->wesn[XHI]  = G->header->wesn[XHI] * D2R;
	G_rad->header->wesn[YLO]  = G->header->wesn[YLO] * D2R;
	G_rad->header->wesn[YHI]  = G->header->wesn[YHI] * D2R;

	/* Get geocentric wesn region in radians */
	GMT_memcpy (wesn, G_rad->header->wesn, 4, double);
	wesn[YLO] = D2R * GMT_lat_swap (GMT, R2D * wesn[YLO], GMT_LATSWAP_G2O);
	wesn[YHI] = D2R * GMT_lat_swap (GMT, R2D * wesn[YHI], GMT_LATSWAP_G2O);
	
	/* Initialize the CVA grid and structure */

	i_yinc_r = 1.0 / G_rad->header->inc[GMT_Y];

	/* Precalculate coordinates xpos[], ypos[] and scale factors(lat) on the grid */

	xpos = GMT_grd_coord (GMT, G_rad->header, GMT_X);
	ypos = GMT_grd_coord (GMT, G_rad->header, GMT_Y);

	latfactor  = GMT_memory (GMT, NULL, G->header->ny, double);
	ilatfactor = GMT_memory (GMT, NULL, G->header->ny, double);

	for (row = 0; row < G->header->ny; row++) {
		latfactor[row] = G_rad->header->inc[GMT_X] * cos (ypos[row]);
		ilatfactor[row] = 1.0 / latfactor[row];
	}

	/* Allocate T/F array */

	processed_node = GMT_memory (GMT, NULL, G->header->size, char);

	/* Set flowline sampling interval to 1/2 of the shortest distance between x-nodes */

	sampling_int_in_km = Ctrl->D.value * G_rad->header->inc[GMT_X] * EQ_RAD * ((fabs (G_rad->header->wesn[YHI]) > fabs (G_rad->header->wesn[YLO])) ? cos (G_rad->header->wesn[YHI]) : cos (G_rad->header->wesn[YLO]));
	GMT_Report (API, GMT_MSG_VERBOSE, "Flowline sampling interval = %.3f km\n", sampling_int_in_km);

	if (Ctrl->T.active) GMT_Report (API, GMT_MSG_VERBOSE, "Seamount ages truncated to %g\n", Ctrl->N.t_upper);

	/* Start to read input data */

	n_smts = 0;
	nx = G->header->nx;	ny = G->header->ny;	/* Signed integers */

	n_expected_fields = (GMT->common.b.ncol[GMT_IN]) ? GMT->common.b.ncol[GMT_IN] : 5;
	if ((error = GMT_set_cols (GMT, GMT_IN, n_expected_fields)) != GMT_OK) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Establishes data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_OK) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	do {	/* Keep returning records until we reach EOF */
		n_read++;
		if ((in = GMT_Get_Record (API, GMT_READ_DOUBLE, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			if (GMT_REC_IS_ANY_HEADER (GMT)) 	/* Skip all headers */
				continue;
			if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
				break;
		}

		/* Data record to process */
	
		/* STEP 1: Read information about a single seamount from input record */

		if (GMT_is_dnan (in[4]))	/* Age is NaN, assign value */
			t_smt = Ctrl->N.t_upper;
		else {			/* Assign given value, truncate if necessary */
			t_smt = in[4];
			if (t_smt > Ctrl->N.t_upper) {
				if (Ctrl->T.active)
					t_smt = Ctrl->N.t_upper;
				else {
					GMT_Report (API, GMT_MSG_VERBOSE, "Seamounts near line %" PRIu64 " has age (%g) > oldest stage (%g) (skipped)\n", n_read, t_smt, Ctrl->N.t_upper);
					continue;
				}
			}
		}

		y_smt = D2R * GMT_lat_swap (GMT, in[GMT_Y], GMT_LATSWAP_G2O);	/* Convert to geocentric, and radians */
		x_smt = in[GMT_X] * D2R;	/* Seamount positions in RADIANS */

		/* STEP 2: Calculate this seamount's flowline */

		if (spotter_forthtrack (GMT, &x_smt, &y_smt, &t_smt, 1, p, n_stages, sampling_int_in_km, 0.0, 0, NULL, &c) <= 0) {
			GMT_Report (API, GMT_MSG_NORMAL, "Nothing returned from spotter_forthtrack - aborting\n");
			Return (GMT_RUNTIME_ERROR);
		}

		/* Do some normalizations here to save processing inside convolution later */

		z_smt = in[GMT_Z];
		r_smt = in[3];
		r_smt /= EQ_RAD;				/* Converts radius in km to radians */
		norm = -4.5 / (r_smt * r_smt);			/* Gaussian normalization */
		node_y_width = irint (ceil (i_yinc_r * r_smt));	/* y-node coverage */

		/* STEP 3: Convolve this flowline with seamount shape and add to CVA grid */

		/* Our convolution is approximate:  We sample the flowline frequently and use
		 * one of the points on the flowline that are closest to the node.  Ideally we
		 * want the nearest distance from each node to the flowline. Later versions may
		 * improve on this situation */

		n_track = lrint (c[0]);				/* Number of point pairs making up this flowline */

		GMT_memset (processed_node, G->header->size, char);	/* Fresh start for this flowline convolution */

		for (m = 0, kx = 1; m < n_track; m++, kx += 2) {		/* For each point along flowline */

			ky = kx + 1;	/* Index for the y-coordinate */

			/* First throw out points outside specified grid region */

			if (c[ky] < wesn[YLO] || c[ky] > wesn[YHI]) continue;	/* Geocentric latitude outside region */

			if (c[kx] > wesn[XHI]) c[kx] -= TWO_PI;
			while (c[kx] < wesn[XLO]) c[kx] += TWO_PI;
			if (c[kx] > wesn[XHI]) continue;			/* Longitude outside region */

			/* OK, this point is within our region, get node index */

			col = (unsigned int)GMT_grd_x_to_col (GMT, c[kx], G_rad->header);
			yg = GMT_lat_swap (GMT, R2D * c[ky], GMT_LATSWAP_O2G);		/* Convert back to geodetic */
			row = (unsigned int)GMT_grd_y_to_row (GMT, yg, G->header);
			node = GMT_IJP (G->header, row, col);

			if (!processed_node[node]) {	/* Have not added to the CVA at this node yet */

				/* Shape is z_smt * exp (r^2 * norm) */

				node_x_width = irint (ceil (r_smt * ilatfactor[row]));
				dx = c[kx] - xpos[col];
				dy = c[ky] - ypos[row];

				/* Loop over a square that circumscribes this seamount's basal outline */

				for (d_row = -node_y_width, row_0 = row - node_y_width; d_row <= node_y_width; d_row++, row_0++) {

					if (row_0 < 0 || row_0 >= ny) continue;	/* Outside grid */

					y_part = d_row * G_rad->header->inc[GMT_Y] - dy;
					y_part2 = y_part * y_part;
					node_0 = GMT_IJP (G->header, row_0, 0);

					for (d_col = -node_x_width, col_0 = col - node_x_width; d_col <= node_x_width; d_col++, col_0++) {

						if (col_0 < 0 || col_0 >= nx) continue;	/* Outside grid */

						x_part = d_col * latfactor[row] - dx;
						r2 = (x_part * x_part + y_part2) * norm;
						G->data[node_0+col_0] += (float)(z_smt * exp (r2));
					}
				}
				processed_node[node] = 1;	/* Now we have visited this node */
			}
		}

		GMT_free (GMT, c);	/* Free the flowline vector */

		n_smts++;	/* Go to next seamount */

		if (!(n_smts%100)) GMT_Report (API, GMT_MSG_VERBOSE, "Processed %5ld seamounts\r", n_smts);
	} while (true);
	
	if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
		Return (API->error);
	}

	/* OK, Done processing, time to write out */

	GMT_Report (API, GMT_MSG_VERBOSE, "Processed %5ld seamounts\n", n_smts);

	if (Ctrl->S.active) {	/* Convert CVA values to percent of CVA maximum */
		uint64_t node;
		double scale;
		
		GMT_Report (API, GMT_MSG_VERBOSE, "Normalize CVS grid to percentages of max CVA\n");
		G->header->z_min = +DBL_MAX;
		G->header->z_max = -DBL_MAX;
		GMT_grd_loop (GMT, G, row, col, node) {	/* Loop over all output nodes */
			if (GMT_is_fnan (G->data[node])) continue;
			if (G->data[node] < G->header->z_min) G->header->z_min = G->data[node];
			if (G->data[node] > G->header->z_max) G->header->z_max = G->data[node];
		}
		GMT_Report (API, GMT_MSG_VERBOSE, "CVA min/max: %g %g -> ", G->header->z_min, G->header->z_max);
		scale = 100.0 / G->header->z_max;
		for (node = 0; node < G->header->size; node++) G->data[node] *= (float)scale;
		G->header->z_min *= scale;	G->header->z_max *= scale;
		GMT_Report (API, GMT_MSG_VERBOSE, "%g %g\n", G->header->z_min, G->header->z_max);
	}
	GMT_Report (API, GMT_MSG_VERBOSE, "Write CVA grid %s\n", Ctrl->G.file);

	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, G)) Return (API->error);
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, G) != GMT_OK) {
		Return (API->error);
	}

	/* Clean up memory */

	GMT_free (GMT, processed_node);
	GMT_free (GMT, latfactor);
	GMT_free (GMT, ilatfactor);
	GMT_free (GMT, xpos);
	GMT_free (GMT, ypos);
	GMT_free (GMT, p);
	if (GMT_Destroy_Data (API, &G_rad) != GMT_OK) {
		GMT_Report (API, GMT_MSG_NORMAL, "Failed to free G_rad\n");
	}

	GMT_Report (API, GMT_MSG_VERBOSE, "Done\n");

	Return (GMT_OK);
}
