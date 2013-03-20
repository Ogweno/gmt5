REM             GMT EXAMPLE 34
REM             $Id
REM
REM Purpose:    Illustrate pscoast with DCW country polygons
REM GMT progs:  pscoast, makecpt, grdimage, grdgradient
REM DOS calls:  del
REM
set ps=example_34.ps

REM Extract a subset of ETOPO2m for this part of Europe
REM grdcut etopo2m_grd.nc -R -GFR+IT.nc=ns
gmtset FORMAT_GEO_MAP dddF
pscoast -JM4.5i -R-6/20/35/52 -FFR,IT+fP300/8 -Glightgray -BafWSne -P -K -X2i -U/-1.75i/-0.75i/"Example 34 in Cookbook" > %ps%
makecpt -Cglobe -T-5000/5000/500 -Z > z.cpt
grdgradient FR+IT.nc -A15 -Ne0.75 -GFR+IT_int.nc
grdimage FR+IT.nc -IFR+IT_int.nc -Cz.cpt -J -O -K -Y4.5i -BafWsnE:."Franco-Italian Union, 2042-45": >> %ps%
pscoast -J -R -FFR,IT+fred@60 -O >> %ps%
REM cleanup
del gmt.conf FR+IT_int.nc z.cpt