#!/bin/sh
#		GMT EXAMPLE 04
#
#		$Id: job04.sh,v 1.2 2002-01-30 03:40:55 ben Exp $
#
# Purpose:	3-D mesh plot of Hawaiian topography and geoid
# GMT progs:	grdcontour, grdview, pscoast, pstext, psxyz
# Unix progs:	echo, rm
#
#CPS: topo.cpt
#CPS: geoid.cpt
echo '-10     255     0       255' > zero.cpt
echo '0       100     10      100' >> zero.cpt
grdcontour HI_geoid4.grd -Jm0.45i -E60/30 -R195/210/18/25 -C1 -A5 -G4i -K -P -X1.5i -Y1.5i -U/-1.25i/-1.25i/"Example 4 in Cookbook" > example_04.ps
pscoast -Jm -E60/30 -R -B2/2NEsw -G0 -O -K >> example_04.ps
echo '205 26 0 0 1.1' | psxyz -Jm -E60/30 -R -SV0.2i/0.5i/0.4ii -W1p -O -K -N >> example_04.ps
echo '205 29.2 36 -90 1 LM N' | pstext -Jm -E60/30 -R -O -K -N >> example_04.ps
grdview HI_topo4.grd -Jm -Jz0.34i -Czero.cpt -E60/30 -R195/210/18/25/-6/4 -N-6/200/200/200 -Qsm -O -K -B2/2/2:"Topo (km)":neswZ -Y2.2i >> example_04.ps
echo '3.25 5.75 60 0.0 33 BC H@#awaiian@# R@#idge' | pstext -R0/10/0/10 -Jx1i -O >> example_04.ps
\rm -f zero.cpt
sh job4c.sh
