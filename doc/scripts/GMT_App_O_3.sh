#!/bin/sh
#	$Id: GMT_App_O_3.sh,v 1.1 2004-06-12 08:15:27 pwessel Exp $
#
#	Makes Fig 3 for Appendix O (labeled lines)
#

PS=`basename $0 .sh`
pscoast -R50/160/-15/15 -JM5i -Glightgray -A500 -K -P > $PS.ps
grdcontour geoid.grd -J -O -B20f10WSne -C10 -A20+d -Gffix.d/0.1i -S10 -T:LH >> $PS.ps
