#!/bin/sh
#	$Id: mask.sh,v 1.2 2007-09-11 22:56:12 remko Exp $
#
# Test grdlandmask for proper wrapping.

echo -n "$0: Test grdlandmask when wrapping over 0-360:			"

ps=mask.ps
echo "-10000	white	+10000	white" > $$.cpt
echo "N black" >> $$.cpt
# Must split a 5x5 degree shore bin across L and R boundary
grdlandmask -G$$.i2=bs -I60m -R-3/357/-90/90 -Di -N1/NaN/NaN/NaN/NaN -A500/1/1
grdimage $$.i2=bs -Jx0.017id -B30g180/30g90WSnE -C$$.cpt -Xc -Y0.75i -P -K > $ps
#
grdlandmask -G$$.i2=bs -I60m -Rg -Di -N1/NaN/NaN/NaN/NaN -A500/1/1
grdimage $$.i2=bs -J -B30g180/30g90WsnE -C$$.cpt -Y3.25i -O -K >> $ps
#
grdlandmask -G$$.i2=bs -I60m -Rd -Di -N1/NaN/NaN/NaN/NaN -A500/1/1
grdimage $$.i2=bs -J -B30g180/30g90WsnE -C$$.cpt -Y3.25i -O >> $ps

compare -density 100 -metric PSNR {,orig/}$ps mask_diff.png > log 2>&1
grep inf log > fail
if [ ! -s fail ]; then
        echo "[FAIL]"
	echo $0 >> ../fail_count.d
else
        echo "[PASS]"
        rm -f fail mask_diff.png log
fi
rm -f $$.*
