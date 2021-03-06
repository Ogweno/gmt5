#!/bin/bash
#	$Id$
#

ps=psimage.ps

cat > tt.in <<%
0 0
1 0 :FwhiteBblack
0 1 :FblackB-
1 1 :F-Bblack
0 2 :FwhiteB-
1 2 :F-Bwhite
0 3 :FredB-
1 3 :F-Bred
0 4 :FredByellow
1 4 :FyellowBred
%
gmt psxy -R0/3/0/5 -Jx1.5i -Gp128/${GMT_SOURCE_DIR}/share/psldemo/circuit.ras -P -K > $ps <<%
0 0
2 0
3 1
3 4
2 5
0 5
%
$AWK '{ x0=$1;x1=x0+1;y0=$2;y1=y0+1;c=$3; \
	printf "> -Gp80/10%s\n%i %i\n%i %i\n%i %i\n",c,x0,y0,x1,y1,x0,y1 ; \
	printf "> -GP80/10%s\n%i %i\n%i %i\n%i %i\n",c,x0,y0,x1,y1,x1,y0}' < tt.in \
	| gmt psxy -R -J -O -K >> $ps
gmt psxy -R -J -O -K <<% >> $ps
> -Gyellow
2 4
2.5 4
2.5 4.5
> -Gred -W2p,blue
2 4
2.5 4.5
2 4.5
%
$AWK '{ x0=$1+0.5;y0=$2+0.5;c=$3; \
	printf "%g %g BR p%s\n",x0,y0,c ; \
	printf "%g %g TL P%s\n",x0,y0,c}' < tt.in \
	| gmt pstext -F+f7p,Helvetica-Bold,purple+j -R -J -O -K >> $ps
gmt psimage -E80 -C3i/3i/BL ${GMT_SOURCE_DIR}/share/pattern/ps_pattern_10.ras -Gfred -Gb- -O -K >> $ps
gmt psimage -E80 -C3i/3i/TL ${GMT_SOURCE_DIR}/share/pattern/ps_pattern_10.ras -O >> $ps

