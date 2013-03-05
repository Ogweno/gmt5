#!/bin/sh
#	$Id$
#
# Test the output of grdseamount for conical shapes
ps=cseamounts.ps
gmtset MAP_FRAME_TYPE plain
# 4 panels of bathymetry:
# Left column is Cartesian, right column is Geographic
# Top row is elliptical, bottom row is circular
# Each panel shows to seamounts, the lower right being
# truncated with a flattening of 0.3
# NOT FINISHED
makecpt -Crainbow -T-3000/-2000/100 -Z > t.cpt
# LL. Input file for Cartesian circular
cat << EOF > cart_circ.txt
-50	50	50	1000	0
50	-50	50	1000	0.3
EOF
grdseamount -C cart_circ.txt -R-100/100/-100/100 -I1 -T -Z-3000 -r -Gcart_circ.nc -V
grdimage cart_circ.nc -JX3i -P -BafWSne -Ct.cpt -K -X1.1i -Y1.25i > $ps
grdtrack -Gcart_circ.nc -ELT/RB > cart_circ.trk
psxy -R -J -O -K -W1p cart_circ.trk >> $ps
psscale -Ct.cpt -D3.25i/-0.4i/5i/0.1ih -B200f100 -O -K >> $ps
# LR. Input file for Geographic circular
cat << EOF > geo_circ.txt
-0.5	0.5	50	1000	0
0.5	-0.5	50	1000	0.3
EOF
grdseamount -C geo_circ.txt -R-1/1/-1/1 -I0.01 -T -Z-3000 -r -Ggeo_circ.nc -fg -V
grdimage geo_circ.nc -JX3id -P -Ba1f0.5wESn -Ct.cpt -O -K -X3.5i >> $ps
grdtrack -Ggeo_circ.nc -ELT/RB > geo_circ.trk
psxy -R -J -O -K -W1p cart_circ.trk >> $ps
# UL. Input file for Cartesian elliptical
cat << EOF > cart_ellipse.txt
-50	50	45	50	30	1000	0
50	-50	-45	50	30	1000	0.3
EOF
grdseamount -C cart_ellipse.txt -R-100/100/-100/100 -I1 -E -T -Z-3000 -r -Gcart_ellipse.nc -V
grdimage cart_ellipse.nc -JX3i -P -BafWsne -Ct.cpt -O -K  -X-3.5i -Y3.2i >> $ps
grdtrack -Gcart_ellipse.nc -ELT/RB > cart_ellipse.trk
psxy -R -J -O -K -W1p,- cart_ellipse.trk >> $ps
# UR. Input file for Geographic elliptical
cat << EOF > geo_ellipse.txt
-0.5	0.5	45	50	30	1000	0
0.5	-0.5	-45	50	30	1000	0.3
EOF
grdseamount -C geo_ellipse.txt -R-1/1/-1/1 -I0.01 -E -T -Z-3000 -r -Ggeo_ellipse.nc -fg -V
grdimage geo_ellipse.nc -JX3id -P -Ba1f0.5wEsn -Ct.cpt -O -K -X3.5i >> $ps
grdtrack -Ggeo_ellipse.nc -ELT/RB > geo_ellipse.trk
psxy -R -J -O -K -W1p,- geo_ellipse.trk >> $ps
# Add Cartesian crossections
psxy -R0/282/-3100/-1900 -JX3i/2.5i -O -K -W1p -i2,3 cart_circ.trk -Bafg1000WsNe -X-3.5i -Y3.2i >> $ps
psxy -R -J -O -K -W1p,- -i2,3 cart_ellipse.trk >> $ps
echo "141 -1950 CARTESIAN" | pstext -R -J -O -K -F+jCM+f12p >> $ps
# Add Geographic crossections
psxy -R0/313/-3100/-1900 -JX3i/2.5i -O -K -W1p -i2,3 geo_circ.trk -Bafg1000wsNe -X3.5i >> $ps
psxy -R -J -O -K -W1p,- -i2,3 geo_ellipse.trk >> $ps
echo "141 -1950 GEOGRAPHIC" | pstext -R -J -O -K -F+jCM+f12p >> $ps
# Finalize plot
psxy -R -J -O -T >> $ps