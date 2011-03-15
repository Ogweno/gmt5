#!/bin/bash
#	$Id: GMT_App_O_4.sh,v 1.6 2011-03-15 02:06:29 guru Exp $
#
#	Makes Fig 4 for Appendix O (labeled lines)
#
. functions.sh

pscoast -R50/160/-15/15 -JM5.3i -Glightgray -A500 -K -P > GMT_App_O_4.ps
grdcontour geoid.nc -J -O -B20f10WSne -C10 -A20+d+s8 -GLZ-/Z+ -S10 -T:LH >> GMT_App_O_4.ps
