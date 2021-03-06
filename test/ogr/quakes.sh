#!/bin/bash
#       $Id$
#
# Test gmt psxy for proper handling of -a [OGR].  We read an OGR file
# with depth and magnitude among the aspatial values and we wish to
# use that as input cols 2 and 3, then scale col 3 (mag) by a log10
# transform to get symbol sizes.

ps=quakes.ps

gmt makecpt -Crainbow -T0/300/25 -Z > t.cpt
gmt psxy quakes.gmt -R15/25/15/25 -JM6i -B5 -Sci -Ct.cpt -P -K -Wthin -a2=depth,3=magnitude -i0,1,2,3s0.05l -Yc -Xc > $ps
gmt pstext quakes.gmt -R -J -O -K -a2=name -F+jCT -Dj0/0.2i >> $ps
gmt psscale -Ct.cpt -D3i/-0.5i/6i/0.1ih -O -Bx+l"Epicenter Depth" -By+lkm >> $ps

