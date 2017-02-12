# -*-Mode: makefile;-*-

#*BeginCopyright*************************************************************
#
# $HeadURL: https://pastec.gtri.gatech.edu/svn/svn-dpc/INNC/projects/PERFECT-TAV-ES/Makefile $
# $Id: Makefile 8296 2013-09-23 21:53:35Z tallent $
#
#***************************************************************EndCopyright*

#****************************************************************************
# $HeadURL: https://pastec.gtri.gatech.edu/svn/svn-dpc/INNC/projects/PERFECT-TAV-ES/Makefile $
#****************************************************************************

MK_SUBDIRS = \
	suite

include Makefile-template.mk

#****************************************************************************

TAR = $(if $(findstring Darwin, $(shell uname)), gnutar, tar)
TARNM = perfect-suite-0.0.4

dist :
	nm_cur=`basename ${PWD}` ; \
	nm_new=$(TARNM) ; \
	cd .. ; \
	ln -s $${nm_cur} $${nm_new} ; \
	${TAR} zcvf $${nm_new}.tar.gz \
	  --dereference --exclude=".svn" \
	  --exclude="suite/sar/tools" \
	  --exclude="doc/src" \
	  $${nm_new}
