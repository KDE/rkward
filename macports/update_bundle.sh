#!/bin/bash
SRCDATE=$(date +%Y-%m-%d)
SRCPATH=/opt/ports
# specify macports installation path
MPTINST=/opt/rkward
# specify work directory
WORKDIR=/opt/ports/kde/rkward/work
# specify the target port
PTARGET=rkward-devel
# specify local public directory
LPUBDIR=~/Public/rkward
# specify application dir used
APPLDIR=/Applications/RKWard

SVNREPO=http://svn.code.sf.net/p/rkward/code/trunk
OLDWD=$(pwd)

if [[ $1 == "" ]] ; then
 echo "Usage: update_bundle.sh OPTION
          OPTIONS:
           -D (build target rkward instead of rkward-devel)
           -X (completely!!! wipe ${MPTINST})
           -F <MacPorts version> (do an all fresh installation of <MacPorts version>)
           -f (full -- all of the below)
           -l (remove static port libraries)
           -p (update macports, remove inactive)
           -r (update port ${PTARGET})
           -m (create .mdmg of ${PTARGET})
           -s (create sources .tar)
           -c (copy .mdmg and src.tar to ${LPUBDIR}, if created)
           -x (completely!!! wipe ${MPTINST}/var/macports/distfiles)"
fi

# get the options
while getopts ":DflprmscxXF:" OPT; do
  case $OPT in
    D) PTARGET=rkward >&2 ;;
    F)
       FRESHMCP=TRUE >&2
       MCPVERS=$OPTARG >&2 ;;
    f)
       RMSTLIBS=TRUE >&2
       UPMPORTS=TRUE >&2
       UPRKWARD=TRUE >&2
       MAKEMDMD=TRUE >&2
       MKSRCTAR=TRUE >&2
       COPYMDMD=TRUE >&2
       WIPEDSTF=TRUE >&2
       ;;
    l) RMSTLIBS=TRUE >&2 ;;
    p) UPMPORTS=TRUE >&2 ;;
    r) UPRKWARD=TRUE >&2 ;;
    m) MAKEMDMD=TRUE >&2 ;;
    s) MKSRCTAR=TRUE >&2 ;;
    c) COPYMDMD=TRUE >&2 ;;
    x) WIPEDSTF=TRUE >&2 ;;
    X)
       WIPEDSTF=FALSE >&2
       WIPEINST=TRUE >&2 ;;
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


# remove MacPorts completely
if [[ $WIPEINST ]] ; then
  if [ -d ${MPTINST} ] ; then
    echo "removing ${MPTINST}..."
    sudo rm -rf ${MPTINST} || exit 1
  fi
  if [ -d ${APPLDIR} ] ; then
    echo "removing ${APPLDIR}..."
    sudo rm -rf ${APPLDIR} || exit 1
  fi
  # these leftovers would conflict with port installation
  if [ -f /Library/LaunchDaemons/org.freedesktop.dbus-system.plist ] ; then
    sudo rm /Library/LaunchDaemons/org.freedesktop.dbus-system.plist
  fi
  if [ -f /Library/LaunchAgents/org.freedesktop.dbus-session.plist ] ; then
    sudo rm /Library/LaunchAgents/org.freedesktop.dbus-session.plist
  fi
  if [ -f /Library/LaunchDaemons/org.freedesktop.avahi-daemon.plist ] ; then
    sudo rm /Library/LaunchDaemons/org.freedesktop.avahi-daemon.plist
  fi
  if [ -f /Library/LaunchDaemons/org.freedesktop.avahi-dnsconfd.plist ] ; then
    sudo rm /Library/LaunchDaemons/org.freedesktop.avahi-dnsconfd.plist
  fi
  if [ -f /Library/LaunchAgents/org.macports.kdecache.plist ] ; then
    sudo rm /Library/LaunchAgents/org.macports.kdecache.plist
  fi
fi

# prepare for a clean installation, remove all cached sources
if [[ $WIPEDSTF ]] ; then
  sudo rm -rf ${MPTINST}/var/macports/distfiles/*
fi

# do a full clean installation
if [[ $FRESHMCP ]] ; then
  echo "creating ${MPTINST}..."
  sudo mkdir -p ${MPTINST} || exit 1
  mkdir /tmp/MP && cd /tmp/MP
  curl "https://distfiles.macports.org/MacPorts/MacPorts-${MCPVERS}.tar.bz2" -o "MacPorts-${MCPVERS}.tar.bz2" || exit 1
  tar xjvf "MacPorts-${MCPVERS}.tar.bz2" || exit 1
  cd "MacPorts-${MCPVERS}" || exit 1
  ./configure --prefix=${MPTINST}  || exit 1
  make || exit 1
  sudo make install || exit 1
  cd $OLDWD || exit 1
  rm -rf /tmp/MP || exit 1
  echo "update MacPorts configuration"
  sudo sed -i -e "s+#\(portautoclean[[:space:]]*\)yes+\1no+" ${MPTINST}/etc/macports/macports.conf
  sudo sed -i -e "s+\(applications_dir[[:space:]]*\)/Applications/MacPorts+\1${APPLDIR}+" ${MPTINST}/etc/macports/macports.conf
  sudo port -v selfupdate || exit 1
  echo "adding local portfiles to ${MPTINST}/etc/macports/sources.conf..."
  sudo sed -i -e "s+rsync://rsync.macports.org.*\[default\]+file://${SRCPATH}/\\`echo -e '\n\r'`&+" ${MPTINST}/etc/macports/sources.conf || exit 1
  sudo port install subversion || \
    echo "configure.cc cc -L${MPTINST}/lib -I${MPTINST}/include -arch x86_64" >> \
    "${MPTINST}/var/macports/sources/rsync.macports.org/release/tarballs/ports/perl/p5-locale-gettext/Portfile" && \
    sudo port install subversion
  ## NOTE: there is some serious trouble with port:p5.12-locale-gettext which hasn't been fixed in months
  ## a workaround, if you run into it:
  ##   sudo port edit p5.12-locale-gettext
  ## and append
  ##   configure.cc cc -L${MPTINST}/lib -I${MPTINST}/include -arch x86_64
  ## to the portfile, or:
  ## echo "configure.cc cc -L${MPTINST}/lib -I${MPTINST}/include -arch x86_64" >> ${MPTINST}/var/macports/sources/rsync.macports.org/release/tarballs/ports/perl/p5-locale-gettext/Portfile
  sudo port -v selfupdate || exit 1
  echo "successfully completed reincarnation of ${MPTINST}!"
fi


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
  # make sure each instance of previous RKWard installations is removed first
  sudo port uninstall rkward
  sudo port uninstall rkward-devel
  sudo port clean rkward
  sudo port clean rkward-devel
  # build and install recent version
  sudo port -v install $PTARGET
fi

# remove static libraries, they're a waste of disk space
if [[ $RMSTLIBS ]] ; then
  echo "deleting all static libs in ${MPTINST}/lib/..."
  sudo rm ${MPTINST}/lib/*.a
  echo "deleting all static libs in ${MPTINST}/var/macports/build..."
  find "${MPTINST}/var/macports/build" -name "*.a" -exec sudo rm \{\} \;
fi

# set some variables
if [[ $COPYMDMD ]] ; then
  # get version information of installed ports
  PORTVERS=$(port list $PTARGET | sed -e "s/.*@//;s/[[:space:]].*//")
  if [ $PTARGET == "rkward-devel" ] ; then
    TARGETVERS=${PORTVERS}$(svn info "$SVNREPO" | grep "^Revision:" | sed "s/[^[:digit:]]*//")
  else
    TARGETVERS=$PORTVERS
  fi
  KDEVERS=$(port list kdelibs4 | sed -e "s/.*@//;s/[[:space:]].*//")
  RVERS=$(port list R-framework | sed -e "s/.*@//;s/[[:space:]].*//")
fi

# make meta-package including dependencies
if [[ $MAKEMDMD ]] ; then
  sudo port -v mdmg $PTARGET || exit 1
  # copy the image file to a public directory
  if [[ $COPYMDMD ]] ; then
    MDMGFILE=${WORKDIR}/${PTARGET}-${PORTVERS}.dmg
    TRGTFILE=${LPUBDIR}/RKWard-${TARGETVERS}_R-${RVERS}_KDE-${KDEVERS}_MacOSX_bundle.dmg
    echo "copying: $MDMGFILE to $TRGTFILE ..."
    cp -av $MDMGFILE $TRGTFILE
    echo "done."
  fi
fi

# archive sources
if [[ $MKSRCTAR ]] ; then
  if [[ ! $COPYMDMD ]] ; then
    # get version information of installed ports
    PORTVERS=$(port list $PTARGET | sed -e "s/.*@//;s/[[:space:]].*//")
  fi
  SRCFILE=${SRCPATH}/sources_bundle_RKWard-${PORTVERS}_${SRCDATE}.tar
  if [ -f $SRCFILE ] ; then
    rm $SRCFILE || exit 1
  fi
  tar cvf $SRCFILE ${MPTINST}/var/macports/distfiles || exit 1
  # copy the source archive to a public directory
  if [[ $COPYMDMD ]] ; then
    TRGSFILE=${LPUBDIR}/RKWard-${TARGETVERS}_R-${RVERS}_KDE-${KDEVERS}_src.tar
    echo "copying: $SRCFILE to $TRGSFILE ..."
    cp -av $SRCFILE $TRGSFILE
    echo "done."
  fi
fi

exit 0
