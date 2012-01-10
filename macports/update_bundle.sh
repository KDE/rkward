#!/bin/bash
# get SVN revision number
echo "get SVN revision number..."
SVNREV=$(svn info http://rkward.svn.sourceforge.net/svnroot/rkward/trunk 2>&1 | grep "^Revision:" | sed -e 's/[^[:digit:]]*//g')
echo "Revision: $SVNREV"
SRCDATE=$(date +%Y-%m-%d)
SRCPATH=/opt/ports
SRCFILE=${SRCPATH}/sources_bundle_svn$SVNREV_${SRCDATE}.tar
# specify macports installation path
MPTINST=/opt/rkward
# specify work directory
WORKDIR=/opt/ports/kde/rkward/work
# specify the target port
PTARGET=rkward-devel
# specify local public directory
LPUBDIR=~/Public/rkward

if [[ $1 == "" ]] ; then
 echo "Usage: update_bundle.sh OPTION
          OPTIONS:
           -f (full -- all of the below)
           -p (update macports, remove inactive)
           -r (update port ${PTARGET})
           -m (create .mdmg of ${PTARGET})
           -s (create sources .tar)
           -c (copy .mdmg and src.tar to ${LPUBDIR}, if created)"
fi

# get the options
while getopts ":fprmsc" OPT; do
  case $OPT in
    f)
       UPMPORTS=TRUE >&2
       UPRKWARD=TRUE >&2
       MAKEMDMD=TRUE >&2
       MKSRCTAR=TRUE >&2
       COPYMDMD=TRUE >&2
       ;;
    p) UPMPORTS=TRUE >&2 ;;
    r) UPRKWARD=TRUE >&2 ;;
    m) MAKEMDMD=TRUE >&2 ;;
    s) MKSRCTAR=TRUE >&2 ;;
    c) COPYMDMD=TRUE >&2 ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
    :)
      echo "Option -$OPTARG requires an argument." >&2
      exit 1
      ;;
  esac
done


# update installed ports
if [[ $UPMPORTS ]] ; then
  sudo port selfupdate
  sudo port -v upgrade outdated
  # get rid of inactive stuff
  sudo port clean inactive
  sudo port -f uninstall inactive
fi

# remove previous installation and its build left-overs
if [[ $UPRKWARD ]] ; then
  sudo port uninstall $PTARGET
  sudo port clean $PTARGET
  # build and install recent version
  sudo port -v install $PTARGET
fi

# set some variables
if [[ $COPYMDMD ]] ; then
  # get version information of installed ports
  PORTVERS=$(port list $PTARGET | sed -e "s/.*@//;s/[[:space:]].*//")
  KDEVERS=$(port list kde4-baseapps | sed -e "s/.*@//;s/[[:space:]].*//")
  RVERS=$(port list R | sed -e "s/.*@//;s/[[:space:]].*//")
fi

# make meta-package including dependencies
if [[ $MAKEMDMD ]] ; then
  sudo port -v mdmg $PTARGET
  # copy the image file to a public directory
  if [[ $COPYMDMD ]] ; then
    MDMGFILE=${WORKDIR}/${PTARGET}-${PORTVERS}.dmg
    TRGTFILE=${LPUBDIR}/RKWard-${PORTVERS}${SVNREV}_R-${RVERS}_KDE-${KDEVERS}_MacOSX_bundle.dmg
    echo "copying: $MDMGFILE to $TRGTFILE ..."
    cp -av $MDMGFILE $TRGTFILE
    echo "done."
  fi
fi

# archive sources
if [[ $MKSRCTAR ]] ; then
  if [ -f $SRCFILE ] ; then
    rm $SRCFILE || exit 1
  fi
  tar cvf $SRCFILE ${MPTINST}/var/macports/distfiles || exit 1
  # copy the source archive to a public directory
  if [[ $COPYMDMD ]] ; then
    TRGSFILE=${LPUBDIR}/RKWard-${PORTVERS}${SVNREV}_R-${RVERS}_KDE-${KDEVERS}_src.tar
    echo "copying: $SRCFILE to $TRGSFILE ..."
    cp -av $SRCFILE $TRGSFILE
    echo "done."
  fi
fi

exit 0
