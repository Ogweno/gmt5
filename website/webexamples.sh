#!/bin/sh
#-----------------------------------------------------------------------------
#	 $Id: webexamples.sh,v 1.5 2003-10-21 18:13:01 pwessel Exp $
#
#	webexamples.csh - Automatic generation of the GMT examples pages
#
#	To be run from the GMT/website directory
#
#	Author:	Paul Wessel
#	Date:	21-MAR-2001
#
#	Script that creates the examples web pages from
#	the scripts files and resulting postscript files
#	Files created:
#
#	gmt/gmt_examples.html
#	gmt/examples/gmt_example_??.html
#	gmt/examples/gmt_example_??.ps (copy from examples directory)
#	gmt/examples/example_??_50dpi.gif (using convert)
#	gmt/examples/example_??_100dpi.gif (using convert)
#-----------------------------------------------------------------------------

GMT050dpi="convert -density 50x50"
GMT100dpi="convert -density 100x100"

if [ $# -eq 1 ]; then
	gush=0
else
	gush=1
fi

cd ..
if [ ! -d src ]; then
	echo "Must be run from main GMT directory - aborts"
	exit
fi

mkdir -p www/gmt
cd www/gmt

cat << EOF > gmt_examples.html
<!-- gmt_example.html - Automatically generated by webexamples.csh  -->
<title>GMT Examples</title>
<body bgcolor="#ffffff">
<H2>Examples of GMT output</H2>
Each of the $n_examples examples in the GMT Technical Reference and Cookbook may
be viewed individually by clicking on the desired example. The images are
reduced to speed up transmission; clicking on these images will open up
a more detailed image. You will also have the option of viewing the responsible
GMT script and download the complete PostScript illustration.
<OL>
<LI>
<A HREF="examples/ex01/gmt_example_01.html">Contour map of global data
set.</A></LI>

<LI>
<A HREF="examples/ex02/gmt_example_02.html">Color images of gridded data.</A></LI>

<LI>
<A HREF="examples/ex03/gmt_example_03.html">Spectral comparisons and x-y
plots.</A></LI>

<LI>
<A HREF="examples/ex04/gmt_example_04.html">3-D perspective, mesh-line surface</A></LI>

<LI>
<A HREF="examples/ex05/gmt_example_05.html">3-D perspective, artificially
illuminated grayshaded image.</A></LI>

<LI>
<A HREF="examples/ex06/gmt_example_06.html">Two types of histograms.</A></LI>

<LI>
<A HREF="examples/ex07/gmt_example_07.html">A typical location map.</A></LI>

<LI>
<A HREF="examples/ex08/gmt_example_08.html">A 3-D bar graph.</A></LI>

<LI>
<A HREF="examples/ex09/gmt_example_09.html">Time/space-series plotted along
tracks.</A></LI>

<LI>
<A HREF="examples/ex10/gmt_example_10.html">Superposition of 3-D bargraph
and map.</A></LI>

<LI>
<A HREF="examples/ex11/gmt_example_11.html">The RGB color cube.</A></LI>

<LI>
<A HREF="examples/ex12/gmt_example_12.html">Delaunay triangulation, contouring,
and imaging.</A></LI>

<LI>
<A HREF="examples/ex13/gmt_example_13.html">Plotting of vector fields.</A></LI>

<LI>
<A HREF="examples/ex14/gmt_example_14.html">Gridding, contouring, and trend
surfaces.</A></LI>

<LI>
<A HREF="examples/ex15/gmt_example_15.html">Gridding with missing data.</A></LI>

<LI>
<A HREF="examples/ex16/gmt_example_16.html">More gridding options.</A></LI>

<LI>
<A HREF="examples/ex17/gmt_example_17.html">Blending images using clipping.</A></LI>

<LI>
<A HREF="examples/ex18/gmt_example_18.html">Volumes and spatial selections.</A></LI>

<LI>
<A HREF="examples/ex19/gmt_example_19.html">GMT color patterns</A></LI>

<LI>
<A HREF="examples/ex20/gmt_example_20.html">Extending GMT with custom symbols.</A></LI>
</OL>
<P><CENTER><IMG SRC="images/gmt_bar.gif" ALT="---------------------------------" ></CENTER>
<BR>
<A HREF="http://gmt.soest.hawaii.edu">
<IMG SRC="images/gmt_small_logo.gif" ALT="RETURN">
Go to GMT home page.
</A>
EOF

mkdir -p examples
cd examples

i=1
n_examples=20

#	Go over all examples and generate HTML, GIF etc

while [ $i -le $n_examples ]; do

	number=`echo $i | awk '{printf "%2.2d\n", $1}'`
	dir=`echo "ex$number"`

	if [ $gush ]; then
		echo "Working on example $number"
	fi

	mkdir -p $dir
	cd $dir

#	Extract cshell example script and rename

	\cp -f ../../../../examples/$dir/job${number}.csh job${number}.csh.txt

#	Copy over the example PS file

	\cp ../../../../examples/$dir/example_${number}.ps .

#	Make the GIF at both 50 and 100 dpi, rotating the landscape ones

	if [ `grep "612 0 T 90 R" example_${number}.ps | wc -l` -eq 1 ]; then
		rot="-rotate 90"
	else
		rot=""
	fi
	$GMT100dpi $rot example_${number}.ps example_${number}_100dpi.gif
	$GMT050dpi $rot example_${number}.ps example_${number}_50dpi.gif

#	Write the html file

cat << EOF > gmt_example_${number}.html
<HTML>
<!-- gmt_example_${number}.html - Automatically generated by webexamples.sh  -->
<title>GMT - Example ${number}</title>
<body bgcolor="#ffffff">
<center><H2>Example ${number}</H2>
<A HREF="example_${number}_100dpi.gif">
<img src="example_${number}_50dpi.gif">
</a><p></center>
EOF
	tail +2 ../../../../website/job${number}.txt >> gmt_example_${number}.html

cat << EOF >> gmt_example_${number}.html
<p>
<A HREF="job${number}.csh.txt"><IMG SRC="../../images/gmt_script.gif" ALT="RETURN">View GMT script.</A>
<A HREF="example_${number}.ps"><IMG SRC="../../images/gmt_ps.gif" ALT="RETURN">Download PostScript version.</A>
<A HREF="../../gmt_examples.html"><IMG SRC="../../images/gmt_back.gif" ALT="RETURN">Back</A>
<HR>
<A HREF="http://gmt.soest.hawaii.edu">
<IMG SRC="../../images/gmt_small_logo.gif" ALT="RETURN">
Return to GMT home page.
</A>
</HTML>
EOF

	cd ..

	i=`expr $i + 1`
done
