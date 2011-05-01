#!/bin/bash
#	$Id: GMT_App_O_1.sh,v 1.7 2011-05-01 18:06:37 remko Exp $
#
#	Makes Fig 1 for Appendix O (labeled lines)
#
. ./functions.sh

pscoast -R50/160/-15/15 -JM5.3i -Glightgray -A500 -K -P > GMT_App_O_1.ps
grdcontour geoid.nc -J -O -B20f10WSne -C10 -A20+s8 -Gd1.5i -S10 -T:LH >> GMT_App_O_1.ps
