#/bin/sh
# $Id: nightly_gmt_cvsupdate.sh,v 1.7 2011-04-19 17:35:06 guru Exp $
# This script is used to get the latest GMT CVS changes and compile and install everything.
# We do that by first getting the changes and then build and install executables.
# First set some environment parameters since cron will not process login settings
export NETCDFHOME=/sw
export MATLAB=/Applications/MATLAB_R2010a.app
export GMTHOME=/Users/pwessel/UH/RESEARCH/CVSPROJECTS/GMTdev/GMT5
export AWK=awk
export RGBDEF=/usr/X11/share/X11/rgb.txt
export PATH=${PATH}:/sw/bin:$GMTHOME/bin
#-------------------------------------------------------------
cd $GMTHOME
make update
make spotless
make site
