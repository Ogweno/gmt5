#!/bin/csh
#
#	$Id: guruenv.csh,v 1.6 2001-03-08 00:59:19 pwessel Exp $
#
#	Environmental variables needed by GMT gurus
#	Stick these in your environment
#
setenv AWK gawk								# nawk, gawk, or compatible
setenv MATLAB /usr/local/matlab						# Set to NONE if you do not have Matlab
setenv MAN2HTML "man2html"						# Use this for Linux (Standard setting)
#setenv MAN2HTML "man2html -leftm 1 -topm 8 -botm 6 -pgsize 65"		# Use this for HPUX
#setenv MAN2HTML "man2html -sun"					# Use this for Sun/Solaris
setenv CVSROOT :pserver:anonymous@gmt.soest.hawaii.edu:/home/gmt/cvs	# anonymous for reading; writers need a specific account
alias gurumake 'make -f guru/Makefile.guru'				# Simple shorthand for guru making
