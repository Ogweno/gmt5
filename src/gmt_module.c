/* $Id$
 *
 * Copyright (c) 2012-2013
 * by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis, and F. Wobbe
 * See LICENSE.TXT file for copying and redistribution conditions.
 */

/* gmt_module.c populates the external array of GMT module parameters such as name
 * and purpose strings, and function pointers to call the module functions.
 * This file also contains the following convenience functions to access the module
 * parameter array:
 *   void gmt_module_show_all(struct GMTAPI_CTRL *API); - Pretty print all module names and their
 *           purposes
 *   void gmt_module_show_name_and_purpose(struct GMTAPI_CTRL *API, enum GMT_MODULE_ID module);
 *           - Pretty print module names and purposes
 *   enum GMT_MODULE_ID gmt_module_lookup (const char *candidate); - Lookup module id by
 *           name
 *   const char *gmt_module_name (struct GMT_CTRL *gmt_ctrl); - Get module name
 *
 * DO NOT edit this file directly! Instead edit gmt_moduleinfo.txt and regenerate
 * this file with gmt_make_module_src.sh. */

#include <stdio.h>
#include <string.h>

#include "gmt_dev.h"

/* sorted array with program parameters for all GMT modules */
struct Gmt_moduleinfo g_module[] = {
	{"backtracker", "spotter", "Generate forward and backward flowlines and hotspot tracks", &GMT_backtracker},
	{"blockmean", "core", "Block average (x,y,z) data tables by L2 norm", &GMT_blockmean},
	{"blockmedian", "core", "Block average (x,y,z) data tables by L1 norm (spatial median)", &GMT_blockmedian},
	{"blockmode", "core", "Block average (x,y,z) data tables by mode estimation", &GMT_blockmode},
	{"colmath", "core", "Do mathematics on columns from data tables", &GMT_colmath},
	{"dimfilter", "misc", "Directional filtering of grids in the Space domain", &GMT_dimfilter},
	{"filter1d", "core", "Do time domain filtering of 1-D data tables", &GMT_filter1d},
	{"fitcircle", "core", "Find mean position and best-fitting great- or small-circle to points on sphere", &GMT_fitcircle},
	{"gmt2kml", "core", "Convert GMT data tables to KML files for Google Earth", &GMT_gmt2kml},
	{"gmtconvert", "core", "Convert, paste, or extract columns from data tables", &GMT_gmtconvert},
	{"gmtdefaults", "core", "List current GMT default parameters", &GMT_gmtdefaults},
	{"gmtget", "core", "Get individual GMT default parameters", &GMT_gmtget},
	{"gmtgravmag3d", "potential", "Compute the gravity/magnetic anomaly of a body by the method of Okabe", &GMT_gmtgravmag3d},
	{"gmtmath", "core", "Reverse Polish Notation (RPN) calculator for data tables", &GMT_gmtmath},
	{"gmtread", "core", "Read from file into memory location [API only]", &GMT_gmtread},
	{"gmtselect", "core", "Select data table subsets based on multiple spatial criteria", &GMT_gmtselect},
	{"gmtset", "core", "Change individual GMT default parameters", &GMT_gmtset},
	{"gmtsimplify", "core", "Line reduction using the Douglas-Peucker algorithm", &GMT_gmtsimplify},
	{"gmtspatial", "core", "Do geospatial operations on lines and polygons", &GMT_gmtspatial},
	{"gmtstitch", "core", "Join individual lines whose end points match within tolerance", &GMT_gmtstitch},
	{"gmtvector", "core", "Basic manipulation of Cartesian vectors", &GMT_gmtvector},
	{"gmtwhich", "core", "Find full path to specified files", &GMT_gmtwhich},
	{"gmtwrite", "core", "Write to file from memory location [API only]", &GMT_gmtwrite},
	{"gravfft", "potential", "Compute gravitational attraction of 3-D surfaces and a little more (ATTENTION z positive up)", &GMT_gravfft},
	{"grd2cpt", "core", "Make linear or histogram-equalized color palette table from grid", &GMT_grd2cpt},
	{"grd2rgb", "core", "Write r/g/b grid files from a grid file, a raw RGB file, or SUN rasterfile", &GMT_grd2rgb},
	{"grd2xyz", "core", "Convert grid file to data table", &GMT_grd2xyz},
	{"grdblend", "core", "Blend several partially over-lapping grids into one larger grid", &GMT_grdblend},
	{"grdclip", "core", "Clip the range of grids", &GMT_grdclip},
	{"grdcontour", "core", "Make contour map using a grid", &GMT_grdcontour},
	{"grdcut", "core", "Extract subregion from a grid", &GMT_grdcut},
	{"grdedit", "core", "Modify header or content of a grid", &GMT_grdedit},
	{"grdfft", "core", "Do mathematical operations on grids in the wavenumber (or frequency) domain", &GMT_grdfft},
	{"grdfilter", "core", "Filter a grid in the space (or time) domain", &GMT_grdfilter},
	{"grdgradient", "core", "Compute directional gradients from a grid", &GMT_grdgradient},
	{"grdgravmag3d", "potential", "Computes the gravity effect of one (or two) grids by the method of Okabe", &GMT_grdgravmag3d},
	{"grdhisteq", "core", "Perform histogram equalization for a grid", &GMT_grdhisteq},
	{"grdimage", "core", "Project grids or images and plot them on maps", &GMT_grdimage},
	{"grdinfo", "core", "Extract information from grids", &GMT_grdinfo},
	{"grdlandmask", "core", "Create a \"wet-dry\" mask grid from shoreline data base", &GMT_grdlandmask},
	{"grdmask", "core", "Create mask grid from polygons or point coverage", &GMT_grdmask},
	{"grdmath", "core", "Reverse Polish Notation (RPN) calculator for grids (element by element)", &GMT_grdmath},
	{"grdpaste", "core", "Join two grids along their common edge", &GMT_grdpaste},
	{"grdpmodeler", "spotter", "Evaluate a plate model on a geographic grid", &GMT_grdpmodeler},
	{"grdproject", "core", "Forward and inverse map transformation of grids", &GMT_grdproject},
	{"grdraster", "dbase", "Extract subregion from a binary raster and save as a GMT grid", &GMT_grdraster},
	{"grdredpol", "potential", "Compute the Continuous Reduction To the Pole, AKA differential RTP", &GMT_grdredpol},
	{"grdreformat", "core", "Convert between different grid formats", &GMT_grdreformat},
	{"grdrotater", "spotter", "Finite rotation reconstruction of geographic grid", &GMT_grdrotater},
	{"grdsample", "core", "Resample a grid onto a new lattice", &GMT_grdsample},
	{"grdseamount", "potential", "Compute synthetic seamount (Gaussian or cone, circular or elliptical) bathymetry", &GMT_grdseamount},
	{"grdspotter", "spotter", "Create CVA image from a gravity or topography grid", &GMT_grdspotter},
	{"grdtrack", "core", "Sample grids at specified (x,y) locations", &GMT_grdtrack},
	{"grdtrend", "core", "Fit trend surface to grids and compute residuals", &GMT_grdtrend},
	{"grdvector", "core", "Plot vector field from two component grids", &GMT_grdvector},
	{"grdview", "core", "Create 3-D perspective image or surface mesh from a grid", &GMT_grdview},
	{"grdvolume", "core", "Calculate grid volume and area constrained by a contour", &GMT_grdvolume},
	{"greenspline", "core", "Interpolate using Green's functions for splines in 1-3 dimensions", &GMT_greenspline},
	{"gshhg", "gshhg", "Extract data tables from binary GSHHS or WDBII data files", &GMT_gshhg},
	{"hotspotter", "spotter", "Create CVA image from seamount locations", &GMT_hotspotter},
	{"img2grd", "imgsrc", "Extract a subset from an img file in Mercator or Geographic format", &GMT_img2grd},
	{"kml2gmt", "core", "Extract GMT table data from Google Earth KML files", &GMT_kml2gmt},
	{"makecpt", "core", "Make GMT color palette tables", &GMT_makecpt},
	{"mapproject", "core", "Do forward and inverse map transformations, datum conversions and geodesy", &GMT_mapproject},
	{"mgd77convert", "mgd77", "Convert MGD77 data to other file formats", &GMT_mgd77convert},
	{"mgd77info", "mgd77", "Extract information about MGD77 files", &GMT_mgd77info},
	{"mgd77list", "mgd77", "Extract data from MGD77 files", &GMT_mgd77list},
	{"mgd77magref", "mgd77", "Evaluate the IGRF or CM4 magnetic field models", &GMT_mgd77magref},
	{"mgd77manage", "mgd77", "Manage the content of MGD77+ files", &GMT_mgd77manage},
	{"mgd77path", "mgd77", "Return paths to MGD77 cruises and directories", &GMT_mgd77path},
	{"mgd77sniffer", "mgd77", "Along-track quality control of MGD77 cruises", &GMT_mgd77sniffer},
	{"mgd77track", "mgd77", "Plot track-line map of MGD77 cruises", &GMT_mgd77track},
	{"minmax", "core", "Find extreme values in data tables", &GMT_minmax},
	{"nearneighbor", "core", "Grid table data using a \"Nearest neighbor\" algorithm", &GMT_nearneighbor},
	{"originator", "spotter", "Associate seamounts with nearest hotspot point sources", &GMT_originator},
	{"project", "core", "Project table data onto lines or great circles, generate tracks, or translate coordinates", &GMT_project},
	{"ps2raster", "core", "Convert [E]PS file(s) to other formats using GhostScript.", &GMT_ps2raster},
	{"psbasemap", "core", "Plot PostScript base maps", &GMT_psbasemap},
	{"psclip", "core", "Initialize or terminate polygonal clip paths", &GMT_psclip},
	{"pscoast", "core", "Plot continents, shorelines, rivers, and borders on maps", &GMT_pscoast},
	{"pscontour", "core", "Contour table data by direct triangulation", &GMT_pscontour},
	{"pscoupe", "meca", "Plot cross-sections of focal mechanisms", &GMT_pscoupe},
	{"pshistogram", "core", "Calculate and plot histograms", &GMT_pshistogram},
	{"psimage", "core", "Place images or EPS files on maps", &GMT_psimage},
	{"pslegend", "core", "Plot legends on maps", &GMT_pslegend},
	{"psmask", "core", "Use data tables to clip or mask map areas with no coverage", &GMT_psmask},
	{"psmeca", "meca", "Plot focal mechanisms on maps", &GMT_psmeca},
	{"pspolar", "meca", "Plot polarities on the inferior focal half-sphere on maps", &GMT_pspolar},
	{"psrose", "core", "Plot a polar histogram (rose, sector, windrose diagrams)", &GMT_psrose},
	{"psscale", "core", "Plot a gray-scale or color-scale on maps", &GMT_psscale},
	{"pssegy", "segy", "Plot a SEGY file on a map", &GMT_pssegy},
	{"pssegyz", "segy", "Plot a SEGY file in PostScript", &GMT_pssegyz},
	{"pstext", "core", "Plot or typeset text on maps", &GMT_pstext},
	{"psvelo", "meca", "Plot velocity vectors, crosses, and wedges on maps", &GMT_psvelo},
	{"pswiggle", "core", "Plot z = f(x,y) anomalies along tracks", &GMT_pswiggle},
	{"psxy", "core", "Plot lines, polygons, and symbols on maps", &GMT_psxy},
	{"psxyz", "core", "Plot lines, polygons, and symbols in 3-D", &GMT_psxyz},
	{"rotconverter", "spotter", "Manipulate total reconstruction and stage rotations", &GMT_rotconverter},
	{"sample1d", "core", "Resample 1-D table data using splines", &GMT_sample1d},
	{"segy2grd", "segy", "Converting SEGY data to a GMT grid", &GMT_segy2grd},
	{"spectrum1d", "core", "Compute auto- [and cross-] spectra from one [or two] timeseries", &GMT_spectrum1d},
	{"sph2grd", "core", "Compute grid from spherical harmonic coefficients", &GMT_sph2grd},
	{"sphdistance", "core", "Make grid of distances to nearest points on a sphere", &GMT_sphdistance},
	{"sphinterpolate", "core", "Spherical gridding in tension of data on a sphere", &GMT_sphinterpolate},
	{"sphtriangulate", "core", "Delaunay or Voronoi construction of spherical lon,lat data", &GMT_sphtriangulate},
	{"splitxyz", "core", "Split xyz[dh] data tables into individual segments", &GMT_splitxyz},
	{"surface", "core", "Grid table data using adjustable tension continuous curvature splines", &GMT_surface},
	{"trend1d", "core", "Fit a [weighted] [robust] polynomial [or Fourier] model for y = f(x) to xy[w] data", &GMT_trend1d},
	{"trend2d", "core", "Fit a [weighted] [robust] polynomial for z = f(x,y) to xyz[w] data", &GMT_trend2d},
	{"triangulate", "core", "Do optimal (Delaunay) triangulation and gridding of Cartesian table data", &GMT_triangulate},
	{"x2sys_binlist", "x2sys", "Create bin index listing from track data files", &GMT_x2sys_binlist},
	{"x2sys_cross", "x2sys", "Calculate crossovers between track data files", &GMT_x2sys_cross},
	{"x2sys_datalist", "x2sys", "Extract content of track data files", &GMT_x2sys_datalist},
	{"x2sys_get", "x2sys", "Get track listing from track index database", &GMT_x2sys_get},
	{"x2sys_init", "x2sys", "Initialize a new x2sys track database", &GMT_x2sys_init},
	{"x2sys_list", "x2sys", "Extract subset from crossover data base", &GMT_x2sys_list},
	{"x2sys_merge", "x2sys", "Merge an updated COEs table (smaller) into the main table (bigger)", &GMT_x2sys_merge},
	{"x2sys_put", "x2sys", "Update track index database from track bin file", &GMT_x2sys_put},
	{"x2sys_report", "x2sys", "Report statistics from crossover data base", &GMT_x2sys_report},
	{"x2sys_solve", "x2sys", "Determine least-squares systematic correction from crossovers", &GMT_x2sys_solve},
	{"xyz2grd", "core", "Convert data table to a grid file", &GMT_xyz2grd},
	{NULL, NULL, NULL, NULL} /* last element == NULL detects end of array */
};

/* sorted array with shorter aliases for modules starting with "gmt" */
struct Gmt_alias gmt_alias[] =
{	/* Alias:	Full name */
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
	{NULL,		NULL}
};

/* sorted array with replacement names for some modules */
struct Gmt_alias gmt_oldname[] =
{	/* Old:		New: */
	{"gmtdp",	"gmtsimplify"},
	{NULL,		NULL}
};

/* Look out for modules given by their aliases */
const char * gmt_formal_name (struct GMTAPI_CTRL *API, const char *module) {
	int i = 0;
	while (gmt_alias[i].alias != NULL) {
		if (!strcmp (module, gmt_alias[i].alias)) return gmt_alias[i].name;
		i++;
	}
	if (GMT_compat_check (API->GMT, 4)) {
		i = 0;
		while (gmt_oldname[i].alias != NULL) {
			if (!strcmp (module, gmt_oldname[i].alias)) return gmt_oldname[i].name;
			i++;
		}
	}
	return module;
}

/* Pretty print all module names and their purposes */
void gmt_module_show_all(struct GMTAPI_CTRL *API) {
	enum GMT_MODULE_ID module_id = 0; /* Module ID */
	char module_name_comp[GMT_TEXT_LEN64], message[GMT_TEXT_LEN256];

	GMT_Message (API, GMT_TIME_NONE, "Program                Purpose of Program\n");
	while (g_module[module_id].name != NULL) {
		snprintf (module_name_comp, GMT_TEXT_LEN64, "%s(%s)",
				g_module[module_id].name, g_module[module_id].component);
		sprintf (message, "%-22s %s\n",
				module_name_comp, g_module[module_id].purpose);
		GMT_Message (API, GMT_TIME_NONE, message);
		++module_id;
	}
}

/* Pretty print module names and purposes */
void gmt_module_show_name_and_purpose(struct GMTAPI_CTRL *API, enum GMT_MODULE_ID module_id) {
	char message[GMT_TEXT_LEN256];
	assert (module_id != GMT_ID_NONE);
	sprintf (message, "%s(%s) %s - %s\n\n",
			g_module[module_id].name,
			g_module[module_id].component,
			GMT_version(),
			g_module[module_id].purpose);
	GMT_Message (API, GMT_TIME_NONE, message);
}

/* Lookup module id by name */
enum GMT_MODULE_ID gmt_module_lookup (struct GMTAPI_CTRL *API, const char *candidate) {
	enum GMT_MODULE_ID module_id = 0; /* Module ID */
	const char *actual_name = gmt_formal_name (API, candidate);

	/* Match actual_name against g_module[module_id].name */
	while ( g_module[module_id].name != NULL &&
			strcmp (actual_name, g_module[module_id].name) )
		++module_id;

	/* Return matching Module ID or GMT_ID_NONE */
	return (module_id == GMT_N_MODULES) ? GMT_ID_NONE : module_id;
}

/* Get module name */
const char *gmt_module_name (struct GMT_CTRL *gmt_ctrl) {
	static const char no_module[] = "core"; /* when called before GMT_begin_module */
	const char *module_name;
	module_name = gmt_ctrl->init.module_id == GMT_ID_NONE ?
			gmt_ctrl->init.module_name : g_module[gmt_ctrl->init.module_id].name;
	if (module_name == NULL)
		/* when called before GMT_begin_module or after GMT_end_module */
		return no_module;
	return module_name;
}
