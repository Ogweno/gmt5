#!/bin/bash
#	$Id: GMT_stereographic_general.sh,v 1.9 2011-05-01 18:06:37 remko Exp $
#
. ./functions.sh
gmtset MAP_ANNOT_OBLIQUE 0
pscoast -R100/-42/160/-8r -JS130/-30/4i -B30g10/15g15 -Dl -A500 -Ggreen -P \
	-Slightblue -Wthinnest > GMT_stereographic_general.ps
