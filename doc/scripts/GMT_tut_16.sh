#!/bin/bash
#	$Id$
#
gmt makecpt -Crainbow -T1000/5000/500 -Z > topo.cpt
gmt grdgradient "${tut:-../tutorial}"/us.nc -Ne0.8 -A100 -fg -Gus_i.nc
gmt grdimage "${tut:-../tutorial}"/us.nc -Ius_i.nc -JM6i -P -Ba -Ctopo.cpt -K > GMT_tut_16.ps
gmt psscale -D3i/8.5i/5i/0.25ih -Ctopo.cpt -I0.4 -By+lm -O >> GMT_tut_16.ps
