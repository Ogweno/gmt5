REM		GMT EXAMPLE 10
REM
REM		$Id: job10.bat,v 1.2 2002-01-30 03:40:56 ben Exp $
REM
REM Purpose:	Make 3-D bar graph on top of perspective map
REM GMT progs:	pscoast, pstext, psxyz
REM DOS calls:	echo, del, gawk
REM
echo GMT EXAMPLE 10
set master=y
if exist job10.bat set master=n
if %master%==y cd ex10
pscoast -R-180/180/-90/90 -JX8i/5id -Dc -G0 -E200/40 -K -U"Example 10 in Cookbook" > example_10.ps
psxyz agu.d -R-180/180/-90/90/1/100000 -JX -JZ2.5il -So0.3ib1 -G140 -W0.5p -B60g60/30g30/a1p:Memberships:WSneZ -O -K -E200/40 >> example_10.ps
gawk "{print $1-10, $2, 20, 0, 0, 7, $3}" agu.d | pstext -R-180/180/-90/90 -JX -O -K -E200/40 -G255 -S0.5p >> example_10.ps
echo 4.5 6 30 0 5 BC AGU 1991 Membership Distribution | pstext -R0/11/0/8.5 -Jx1i -O >> example_10.ps
del .gmt*
if %master%==y cd ..
