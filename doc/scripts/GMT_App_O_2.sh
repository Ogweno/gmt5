#!/bin/bash
#	$Id: GMT_App_O_2.sh,v 1.6 2011-02-28 00:58:03 remko Exp $
#
#	Makes Fig 2 for Appendix O (labeled lines)
#
. functions.sh

pscoast -R50/160/-15/15 -JM5.3i -Glightgray -A500 -K -P > GMT_App_O_2.ps
grdcontour geoid.nc -J -O -B20f10WSne -C10 -A20+s8 -Gn1/1i -S10 -T:LH >> GMT_App_O_2.ps
