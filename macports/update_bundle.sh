#!/bin/bash
SRCDATE=$(date +%Y-%m-%d)
SRCPATH=/opt/ports
SRCFILE=${SRCPATH}/sources_bundle_${SRCDATE}.tar
# specify macports installation path
MPTINST=/opt/rkward
# specify the target port
PTARGET=rkward-devel

if [[ $1 == "" ]] ; then
 echo "Usage: update_bundle.sh OPTION
          OPTIONS:
           -f (full -- all of the below)
           -p (update macports, remove inactive)
           -r (update port ${PTARGET})
           -m (create .mdmg of ${PTARGET})
           -s (create sources .tar)"
fi

# get the options
while getopts ":fprms" OPT; do
  case $OPT in
    f)
       UPMPORTS=TRUE >&2
       UPRKWARD=TRUE >&2
       MAKEMDMD=TRUE >&2
       MKSRCTAR=TRUE >&2
       ;;
    p) UPMPORTS=TRUE >&2 ;;
    r) UPRKWARD=TRUE >&2 ;;
    m) MAKEMDMD=TRUE >&2 ;;
    s) MKSRCTAR=TRUE >&2 ;;
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
  sudo port uninstall inactive
  sudo port clean inactive
fi

# remove previous installation and its build left-overs
if [[ $UPRKWARD ]] ; then
  sudo port uninstall $PTARGET
  sudo port clean $PTARGET
  # build and install recent version
  sudo port -v install $PTARGET
fi

# make meta-package including dependencies
if [[ $MAKEMDMD ]] ; then
  sudo port -v mdmg $PTARGET
fi

# archive sources
if [[ $MKSRCTAR ]] ; then
  if [ -f $SRCFILE ] ; then
    rm $SRCFILE || exit 1
  fi
  tar cvf $SRCFILE ${MPTINST}/var/macports/software || exit 1
fi

exit 0
