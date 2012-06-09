/*--------------------------------------------------------------------
 *    $Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * sph2grd evalutes a grid using a spherical harmonics model
 *
 * Author:	Paul Wessel
 * Date:	1-JUN-2006
 */
 
#include "gmt.h"

struct SPH2GRD_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct D {	/* -D */
		bool active;
		char mode;
	} D;
	struct E {	/* -E */
		bool active;
	} E;
	struct G {	/* -G<grdfile> */
		bool active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct L {	/* -L<lc>/<lp>/<hp>/<hc> or -L<lo>/<hi> */
		bool active;
		int mode;
		double lc, lp, hp, hc;
	} L;
	struct N {	/* -Ng|m|s */
		bool active;
		char mode;
	} N;
	struct Q {	/* -Q */
		bool active;
	} Q;
};

int main (int argc, char **argv)
{
	int i, j, ij, n_expected_fields, n_fields, n_files = 0, f_arg, error = 0, n_read = 0;
	
	float *grd;
	
	double *in, *lon, lat;
	
	FILE *fp;
	
	struct GRD_HEADER header;
	struct SPH2GRD_CTRL *Ctrl;

	void *New_sph2grd_Ctrl (), Free_sph2grd_Ctrl (struct SPH2GRD_CTRL *C);

	argc = GMT_begin (argc, argv);

	Ctrl = (struct SPH2GRD_CTRL *) New_sph2grd_Ctrl ();		/* Allocate and initialize defaults in a new control structure */
	
	GMT_grd_init (&header, argc, argv, false);

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
				/* Common parameters */
                      
				case 'R':
  				case 'b':
				case '\0':
					error += GMT_parse_common_options (argv[i], &header.x_min, &header.x_max, &header.y_min, &header.y_max);
					break;

				/* Supplemental parameters */

				case 'D':	/* Evaluate derivative solutions */
					Ctrl->D.active = true;
					Ctrl->D.mode = argv[i][2];
					break;
				case 'E':	/* Evaluate on ellipsoid */
					Ctrl->E.active = true;
					break;
				case 'G':
					Ctrl->G.active = true;
					Ctrl->G.file = strdup (&argv[i][2]);
					break;
				case 'I':
					Ctrl->I.active = true;
					if (GMT_getinc (&argv[i][2], Ctrl->I.inc)) {
						GMT_inc_syntax ('I', 1);
						error++;
					}
					break;
 				case 'L':	/* Bandpass or Gaussian filter */
					Ctrl->L.active = true;
					sscanf (&argv[i][2], "%lg/%lg/%lg/%lg", &Ctrl->L.lc, &Ctrl->L.lp, &Ctrl->L.hp, &Ctrl->L.hc);
					break;
				case 'N':
					Ctrl->N.active = true;
					Ctrl->N.mode = argv[i][2];
					break;
				case 'Q':
					Ctrl->Q.active = true;
					break;
				default:
					error++;
					GMT_default_error (argv[i][1]);
					break;
			}
		}
		else {
			n_files++;
			f_arg = i;
		}
	}
		
	if (argc == 1 || GMT->common.synopsis.active) {
		fprintf (stderr, "sph2grd %s - Evaluate spherical harmonic models on a grid\n\n", GMT_VERSION);
		fprintf (stderr, "usage: sph2grd [coeff_file] %s %s [-Dg|n]\n", GMT_I_OPT, GMT_Rgeo_OPT);
		fprintf (stderr, "\t[-E] [-F] [-G<grdfile>] [-L[d]<filter>] [-N<norm>] [-Q] [-V] [%s]\n\n", GMT_bi_OPT);
		if (GMT->common.synopsis.active) exit (EXIT_FAILURE);
		fprintf (stderr, "	coeff_file (or stdin) contains records of degree, order, cos, sin\n");
		GMT_explain_options ("R");
		GMT_inc_syntax ('I', 0);
		fprintf (stderr, "\n\tOPTIONS:\n");
		fprintf (stderr, "\t-D Will evaluate a derived field from a geopotential model.  Choose between\n");
		fprintf (stderr, "\t   -Dg will compute the gravitational field [Add -E for anomalies on ellipsoid]\n");
		fprintf (stderr, "\t   -Dn will compute the geoid [Add -E for anomalies on ellipsoid]\n");
		fprintf (stderr, "\t-E to evaluate expansion on the current ellipsoid [Default is sphere]\n");
		fprintf (stderr, "\t-F Force pixel registration [Default is gridline registration].\n");
		fprintf (stderr, "\t-G filename for output grid file\n");
		fprintf (stderr, "\t-L Filter coefficients according to one of two kinds of filter specifications:.\n");
		fprintf (stderr, "\t   Use -Ld if values are given in terms of coefficient degrees [Default is km]\n");
		fprintf (stderr, "\t   a) Cosine band-pass: Append four wavelengths <lc>/<lp>/<hp>/<hc>.\n");
		fprintf (stderr, "\t      coefficients outside <lc>/<hc> are cut; inside <lp>/<hp> are passed, rest are tapered.\n");
		fprintf (stderr, "\t      Replace wavelength by - to skip, e.g., -L-/-/500/100 is a low-pass filter.\n");
		fprintf (stderr, "\t   b) Gaussian band-pass: Append two wavelengths <lo>/<hi> where filter amplitudes = 0.5.\n");
		fprintf (stderr, "\t      Replace wavelength by - to skip, e.g., -L300/- is a high-pass Gaussian filter.\n");
		fprintf (stderr, "\t-N Normalization used for coefficients.  Choose among\n");
		fprintf (stderr, "\t   m: Mathematical normalization - inner products summed over surface equal 1 [Default]\n");
		fprintf (stderr, "\t   g: Geodesy normalization - inner products summed over surface equal 4pi\n");
		fprintf (stderr, "\t   s: Schmidt normalization - as used in geomagnetism\n");
		fprintf (stderr, "\t-Q Coefficients have phase convention from physics, i.e., the (-1)^m factor\n");
		GMT_explain_options ("VC4.");

		exit (EXIT_FAILURE);
	}
	
	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.active, &Ctrl->I.active);

	if (n_files > 1) {
		fprintf (stderr, "%s: Syntax error: Can only handle one input coefficient file\n", gmt_module_name(GMT));
		error++;
	}
	if (!GMT->common.R.active) {
		fprintf (stderr, "%s: Syntax error: Must specify -R option\n", gmt_module_name(GMT));
		error++;
	}
	if (!Ctrl->G.file) {
		fprintf (stderr, "%s: Syntax error -G option: Must specify output file\n", gmt_module_name(GMT));
		error++;
	}
	if (Ctrl->D.active && !(Ctrl->D.mode == 'g' || Ctrl->D.mode == 'n')) {
		fprintf (stderr, "%s: Syntax error -D option: Must append g or n\n", gmt_module_name(GMT));
		error++;
	}
	if (!(Ctrl->N.mode == 'm' || Ctrl->N.mode == 'g' || Ctrl->N.mode == 's')) {
		fprintf (stderr, "%s: Syntax error: -N Normalization must be one of m, g, or s\n", gmt_module_name(GMT));
		error++;
	}
	if (Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0) {
		fprintf (stderr, "%s: Syntax error -I option.  Must specify positive increment(s)\n", gmt_module_name(GMT));
		error++;
	}

	if (error) exit (EXIT_FAILURE);

	if (n_files == 1 && (fp = GMT_fopen (argv[f_arg], GMT->current.io.info.r_mode)) == NULL) {
		fprintf (stderr, "%s: Cannot open file %s\n", gmt_module_name(GMT), argv[f_arg]);
		exit (EXIT_FAILURE);
	}
	else {
		fp = GMT->session.stdin;
#ifdef SET_IO_MODE
		GMT_setmode (GMT, GMT_IN);
#endif
	}
	n_expected_fields = (GMT->current.io.info.binary[GMT_IN]) ? GMT->current.io.info.ncol[GMT_IN] : 4;
	while ((n_fields = GMT->current.io.input (GMT, fp, &n_expected_fields, &in)) >= 0 && !(GMT->current.io.info.status & GMT_IO_EOF)) {	/* Not yet EOF */
		n_read++;
		if (GMT->current.io.info.status & GMT_IO_MISMATCH) {
			fprintf (stderr, "%s: Mismatch between actual (%d) and expected (%d) fields near line %ld\n", gmt_module_name(GMT), n_fields, n_expected_fields, n_read);
			exit (EXIT_FAILURE);
		}
		/* Store coefficients somewhere */
	}
	GMT_fclose (fp);

	header.x_inc = Ctrl->I.inc[GMT_X];
	header.y_inc = Ctrl->I.inc[GMT_Y];
	header.registration = GMT->common.r.active;
	GMT_RI_prepare (&header);	/* Ensure -R -I consistency and set nx, ny */
	GMT_err_fail (GMT_grd_RI_verify (&header, 1), Ctrl->G.file);

	grd = GMT_memory (GMT, NULL, (size_t)(header.nx * header.ny), float);
	lon = GMT_memory (GMT, NULL, (size_t)header.nx, double);
	for (i = 0; i < header.nx; i++) lon[i] = GMT_col_to_x (GMT, i, header.x_min, header.x_max, header.x_inc, header.xy_off, header.nx);
		
	for (j = ij = 0; j < header.ny; j++) {
		lat = GMT_row_to_y (GMT, j, header.y_min, header.y_max, header.y_inc, header.xy_off, header.ny);
		if (GMT->current.setting.verbose) {
			fprintf (stderr, "Working on latitude: ");
			GMT_ascii_output_col (stderr, lat, GMT_Y);
			fprintf (stderr, "\r");
		}
		
		/* Compute the Legendre coefficients for this latitude */
			
		for (i = 0; i < header.nx; i++, ij++) grd[ij] = 0.0;
	}
	
	GMT_err_fail (GMT_write_grd (Ctrl->G.file, &header, grd, 0.0, 0.0, 0.0, 0.0, GMT->current.io.pad, false), Ctrl->G.file);
	
	GMT_free (grd);
	GMT_free (lon);
	
	Free_sph2grd_Ctrl (Ctrl);	/* Deallocate control structure */

	GMT_end (argc, argv);

	exit (EXIT_SUCCESS);
}

void *New_sph2grd_Ctrl () {	/* Allocate and initialize a new control structure */
	struct SPH2GRD_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct SPH2GRD_CTRL);
	
	C->N.mode = 'm';
	return (C);
}

void Free_sph2grd_Ctrl (struct SPH2GRD_CTRL *C) {	/* Deallocate control structure */
	if (C->G.file) GMT_free (C->G.file);	
	GMT_free (C);	
}
