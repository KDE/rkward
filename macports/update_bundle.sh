#!/bin/bash
SRCDATE=$(date +%Y-%m-%d)
SRCPATH=/opt/ports
# specify macports installation path
MPTINST=/opt/rkward
# specify the target port
PTARGET=rkward-devel
PNSUFFX="-devel"
DEVEL=TRUE
# specify work directory
WORKDIR=/opt/ports/kde/${PTARGET}/work
# specify local public directory
LPUBDIR=~/Public/rkward
# specify application dir used
APPLDIR=/Applications/RKWard
# specify the prefix for build directories below ${MPTINST}/var/macports/build
BLDPRFX=_opt_rkward_var_macports_sources_rsync.macports.org_release_tarballs_ports_
# this array holds all packages who should not be included in the bundle
# declare -a EXCLPKG=(audio_flac audio_jack audio_lame audio_libmodplug audio_libopus audio_libsamplerate \
#   audio_libsndfile audio_libvorbis audio_speex \
#   databases_db46 databases_gdbm databases_sqlite3 devel_boost devel_soprano devel_strigi devel_virtuoso \
#   gnome_gobject-introspection gnome_gtk2 gnome_hicolor-icon-theme gnome_libglade2 \
#   multimedia_XviD multimedia_dirac multimedia_ffmpeg multimedia_libogg multimedia_libtheora multimedia_libvpx \
#   multimedia_schroedinger multimedia_x264 net_avahi net_kerberos5 security_cyrus-sasl2 sysutils_e2fsprogs )
declare -a EXCLPKG=(audio_flac audio_jack audio_lame audio_libmodplug audio_libopus audio_libsamplerate \
  audio_libsndfile audio_libvorbis audio_speex databases_db46 databases_gdbm databases_sqlite3 devel_boost \
  gnome_gobject-introspection gnome_gtk2 gnome_hicolor-icon-theme gnome_libglade2 \
  multimedia_XviD multimedia_dirac multimedia_ffmpeg multimedia_libogg multimedia_libtheora multimedia_libvpx \
  multimedia_schroedinger multimedia_x264 net_avahi net_kerberos5 security_cyrus-sasl2 sysutils_e2fsprogs )

#LLVMFIX="configure.compiler=llvm-gcc-4.2"

# to see the dependency tree of ports, run
# sudo port rdeps rkward-devel

SVNREPO=http://svn.code.sf.net/p/rkward/code/trunk
OLDWD=$(pwd)

if [[ $1 == "" ]] ; then
 echo "Usage: update_bundle.sh OPTIONS
          OPTIONS:

           the following must always be combined with r/m/s/c:
           -D (build target rkward instead of rkward-devel)
           -d (build subport 'debug')
           -b (build subport 'binary')

           these work on their own:
           -X (completely!!! wipe ${MPTINST})
           -F <MacPorts version> (do an all fresh installation of <MacPorts version>)
           -f (list disk usage for all includable ports)
           -l (remove static port libraries)
           -L (don't bundle probably superfluous ports)
           -p (update macports, remove inactive)
           -r (update port ${PTARGET})
           -m (create .mdmg of ${PTARGET})
           -s (create sources .tar)
           -c (copy .mdmg and src.tar to ${LPUBDIR}, if created)
           -x (completely!!! wipe ${MPTINST}/var/macports/distfiles)"
exit 0
fi

# get the options
while getopts ":DdbflLprmscxXF:" OPT; do
  case $OPT in
    D) PTARGET=rkward >&2
       WORKDIR="/opt/ports/kde/${PTARGET}/work" >&2
       PNSUFFX="" >&2
       DEVEL=FALSE >&2 ;;
    d) DEBUG=TRUE >&2
       PTARGET=${PTARGET}-debug >&2
       PNSUFFX="${PNSUFFX}-debug" >&2 ;;
    b) BINARY=TRUE >&2
       PTARGET=${PTARGET}-binary >&2
       PNSUFFX="${PNSUFFX}-binary" >&2 ;;
    F) FRESHMCP=TRUE >&2
       MCPVERS=$OPTARG >&2 ;;
    f) LSDSKUSG=TRUE >&2 ;;
    l) RMSTLIBS=TRUE >&2 ;;
    L) DOEXCPCK=TRUE >&2 ;;
    p) UPMPORTS=TRUE >&2 ;;
    r) UPRKWARD=TRUE >&2 ;;
    m) RPATHFIX=TRUE >&2
       MAKEMDMD=TRUE >&2 ;;
    s) MKSRCTAR=TRUE >&2 ;;
    c) COPYMDMD=TRUE >&2 ;;
    x) WIPEDSTF=TRUE >&2 ;;
    X) WIPEDSTF=FALSE >&2
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

# correct setting of RPATHFIX workaround, it's not needed
# for binary subports since they don't include R.framework
if [[ $BINARY ]] ; then
  unset RPATHFIX
fi

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
  if [ -d /Applications/MacPorts ] ; then
    echo "removing /Applications/MacPorts..."
    sudo rm -rf /Applications/MacPorts || exit 1
  fi
  # these leftovers would conflict with port installation
  for libsymlink in \
    /Library/LaunchDaemons/org.freedesktop.dbus-system.plist \
    /Library/LaunchAgents/org.freedesktop.dbus-session.plist \
    /Library/LaunchDaemons/org.freedesktop.avahi-daemon.plist \
    /Library/LaunchDaemons/org.freedesktop.avahi-dnsconfd.plist \
    /Library/LaunchAgents/org.macports.kdecache.plist \
    /Library/LaunchDaemons/org.macports.mysql5.plist \
    /Library/LaunchDaemons/org.macports.slapd.plist
  do
    if [ -L "${libsymlink}" ] ; then
      echo "removing symbolic link ${libsymlink}..."
      sudo rm "${libsymlink}"
    fi
  done
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
  # install a needed gcc/clang first?
  if [[ $CMPLR ]] ; then
    sudo port -v install ${CMPLR} ${LLVMFIX} || exit 1
  fi
  if [[ $CLANG ]] ; then
    sudo port -v install ${CLANG} ${LLVMFIX} || exit 1
  fi
#  # if you don't have the latest Xcode, some dependencies of subversion might need certain compilers
#  sudo port -v install subversion ${LLVMFIX} || exit 1
  sudo port -v install subversion || exit 1
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
  INSTALLEDPORTS=$(port installed)
  # make sure each instance of previous RKWard installations is removed first
  for i in rkward rkward-devel rkward-binary rkward-devel-binary rkward-debug rkward-devel-debug ; do
    if [[ $(echo $INSTALLEDPORTS | grep "[[:space:]]${i}[[:space:]]" 2> /dev/null ) ]] ; then
      sudo port uninstall ${i}
      sudo port clean ${i}
    fi
  done
  # build and install recent version
  sudo port -v install ${PTARGET} || exit 1
fi

# remove static libraries, they're a waste of disk space
if [[ $RMSTLIBS ]] ; then
  echo "deleting all static libs in ${MPTINST}/lib/..."
  sudo rm ${MPTINST}/lib/*.a
  echo "deleting all static libs in ${MPTINST}/var/macports/build..."
  #find "${MPTINST}/var/macports/build" -name "*.a" -exec sudo rm \{\} \;
  # only remove libs in destroot/libs/
  find -E "${MPTINST}/var/macports/build" -type f -regex '.*/destroot'${MPTINST}'/lib/[^/]*\.a' -exec sudo rm \{\} \;
fi

# list disk usage of ports
if [[ $LSDSKUSG ]] ; then
  cd ${MPTINST}/var/macports/build/
  SBFLDRS=$(ls)
  for i in ${SBFLDRS} ; do
    echo $(du -sh ${i}/$(ls ${i}/)/work/destroot | sed -e "s+\(${BLDPRFX}\)\(.*\)\(/work/destroot\)+\2+")
  done
fi

# set some variables
if [[ $COPYMDMD ]] ; then
  # get version information of installed ports
  PORTVERS=$(port list $PTARGET | sed -e "s/.*@//;s/[[:space:]].*//")
  if [[ $DEVEL ]] ; then
    TARGETVERS=${PORTVERS}$(svn info "$SVNREPO" | grep "^Revision:" | sed "s/[^[:digit:]]*//")
  else
    TARGETVERS=$PORTVERS
  fi
  KDEVERS=$(port list kdelibs4 | sed -e "s/.*@//;s/[[:space:]].*//")
fi

# get R version, long and short
if [[ $BINARY ]] ; then
  RVERS=$(R --version | grep "R version" | sed -e "s/R version \([[:digit:].]*\).*/\1/")
else
  RVERS=$(port list R-framework | sed -e "s/.*@//;s/[[:space:]].*//")
fi
# if we have to re-create the symlinks for binary installation
# this can be used to get the short version numer <major>.<minor>:
#RVSHORT=$(echo $RVERS | sed -e "s/\([[:digit:]]*\.\)\([[:digit:]]*\).*/\1\2/")

# make meta-package including dependencies
if [[ $MAKEMDMD ]] ; then
  if [[ $RPATHFIX ]] ; then
    # this is to fix some kind of a race condition: if RKWard gets installed before R-framework,
    # it will create a directory which must actually be a symlink in order for R to run! so we'll
    # move RKWard's own packages before bundling it
    RKWDSTROOT=${WORKDIR}/destroot
    RKWRFWPATH=${RKWDSTROOT}/${MPTINST}/Library/Frameworks/R.framework
    if ! [ -d ${RKWRFWPATH} ] ; then
      echo "cannot find R.framework, bogus path? ${RKWRFWPATH}"
      exit 1
    fi
    RFWPATH=${MPTINST}/var/macports/build/${BLDPRFX}math_R-framework/R-framework/work/destroot
    RVERSPATH=${RFWPATH}/${MPTINST}/Library/Frameworks/R.framework/Versions
    # this variable will hold the R version of the installed framework
    RFWVERS=$(cd ${RVERSPATH} && find . -type d -maxdepth 1 -mindepth 1 | sed -e "s#./##" || exit 1)
    if [[ $RFWVERS == "" ]] ; then
      echo "could not get R version! aborting..."
      exit 1
    fi
    # only do this if the Resources directory exists
    if [ -d ${RKWRFWPATH}/Resources ] ; then
      # now cd into RKWard's destroot and re-arrange the directory structure
      cd $RKWRFWPATH || exit 1
      sudo mkdir -p "Versions/${RFWVERS}/Resources" || exit 1
      sudo mv "Resources/library" "Versions/${RFWVERS}/Resources/" || exit 1
      sudo rm -rf "Resources" || exit 1
      cd $OLDWD || exit 1
    fi
  fi
  if [[ $DOEXCPCK ]] ; then
    # before we build the bundle package, replace the destroot folder of the packages
    # defined in the array EXCLPKG with empty ones, so their stuff is not included
    for i in ${EXCLPKG[@]} ; do
      THISPKG=${MPTINST}/var/macports/build/${BLDPRFX}${i}
      if [ -d ${THISPKG} ] ; then
        SUBFLDR=$(ls $THISPKG)
        if [ -d ${THISPKG}/${SUBFLDR}/work/destroot ] && [ ! -d ${THISPKG}/${SUBFLDR}/work/destroot_off ]; then
          sudo mv ${THISPKG}/${SUBFLDR}/work/destroot ${THISPKG}/${SUBFLDR}/work/destroot_off
          sudo mkdir ${THISPKG}/${SUBFLDR}/work/destroot
        fi
        unset SUBFLDR
      else
        echo "warning: can't find ${THISPKG}!"
      fi
      unset THISPKG
    done
  fi

#  # cleaning boost, the avahi port somehow gets installed in two varaints...
#  sudo port clean boost
  sudo port -v mdmg $PTARGET || exit 1

  if [[ $DOEXCPCK ]] ; then
    # restore original destroot directories
    for i in ${EXCLPKG[@]} ; do
      THISPKG=${MPTINST}/var/macports/build/${BLDPRFX}${i}
      if [ -d ${THISPKG} ] ; then
        SUBFLDR=$(ls $THISPKG)
        if [ -d ${THISPKG}/${SUBFLDR}/work/destroot_off ] ; then
          sudo rmdir ${THISPKG}/${SUBFLDR}/work/destroot
          sudo mv ${THISPKG}/${SUBFLDR}/work/destroot_off ${THISPKG}/${SUBFLDR}/work/destroot
        fi
        unset SUBFLDR
      fi
      unset THISPKG
    done
  fi
  if [[ $RPATHFIX ]] ; then
    if [ -d ${RKWRFWPATH}/Versions ] ; then
      cd $RKWRFWPATH || exit 1
      sudo mkdir -p "Resources" || exit 1
      sudo mv "Versions/${RFWVERS}/Resources/library" "Resources/" || exit 1
      sudo rm -rf "Versions" || exit 1
      cd $OLDWD || exit 1
    fi
  fi


  # copy the image file to a public directory
  if [[ $COPYMDMD ]] ; then
    MDMGFILE=${WORKDIR}/${PTARGET}-${PORTVERS}.dmg
    if [[ $BINARY ]] ; then
      TRGTFILE=${LPUBDIR}/RKWard${PNSUFFX}-${TARGETVERS}_KDE-${KDEVERS}_needs_CRAN_R-${RVERS}.dmg
    else
      TRGTFILE=${LPUBDIR}/RKWard${PNSUFFX}-${TARGETVERS}_R-${RVERS}_KDE-${KDEVERS}_MacOSX_bundle.dmg
    fi
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
    if [[ $BINARY ]] ; then
      TRGSFILE=${LPUBDIR}/RKWard${PNSUFFX}-${TARGETVERS}_KDE-${KDEVERS}_src.tar
    else
      TRGSFILE=${LPUBDIR}/RKWard${PNSUFFX}-${TARGETVERS}_R-${RVERS}_KDE-${KDEVERS}_src.tar
    fi
    echo "copying: $SRCFILE to $TRGSFILE ..."
    cp -av $SRCFILE $TRGSFILE
    echo "done."
  fi
fi

exit 0
