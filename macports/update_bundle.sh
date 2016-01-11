#!/bin/bash
SRCDATE=$(date +%Y-%m-%d)
SRCPATH=/opt/ports
# specify git root path
GITROOT=/opt/git
# specify macports installation path
MPTINST=/opt/rkward
# specify the target port
PTARGET=rkward-devel
PNSUFFX="-devel"
RKUSER="${USER}"
USERBIN="${HOME}/bin"
OSXVERSION=$(sw_vers -productVersion | sed -e "s/.[[:digit:]]*$//")

# initalize varaibles
DEVEL=true
ANONGIT=true
BLDSETUP=false
SSHGEN=false
DEBUG=false
BINARY=false
FRESHMCP=false
LSDSKUSG=false
RMSTLIBS=false
DOEXCPCK=false
UPMPORTS=false
UPRKWARD=false
RPATHFIX=false
MAKEMDMD=false
MKSRCTAR=false
COPYMDMD=false
WIPEDSTF=false
WIPEINST=false

PVARIANT=""
GITBRANCH="master"
# specify work directory
WORKDIR="${SRCPATH}/kde/${PTARGET}/work"
# specify local public directory
LPUBDIR="${HOME}/Public/rkward"
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
declare -a EXCLPKG=(audio_lame audio_libmodplug audio_libopus \
  audio_libvorbis audio_speex audio_taglib databases_db46 databases_db48 databases_gdbm databases_openldap databases_sqlite3 devel_boost \
  gnome_at-spi2-atk gnome_at-spi2-core gnome_desktop-file-utils gnome_gnome-common gnome_gobject-introspection gnome_gtk-doc \
  gnome_gtk2 gnome_gtk3 gnome_hicolor-icon-theme gnome_libcroco gnome_libglade2 gnome_gobject-introspection \
  lang_llvm-3.7 \
  multimedia_ffmpeg multimedia_libass multimedia_libbluray multimedia_libogg multimedia_libtheora multimedia_libvpx \
  multimedia_schroedinger multimedia_x264 multimedia_XviD \
  net_avahi net_kerberos5 net_tcp_wrappers security_cyrus-sasl2 security_p11-kit sysutils_e2fsprogs \
  x11_mesa x11_pango x11_urw-fonts x11_Xft2 x11_xorg-bigreqsproto x11_xorg-compositeproto x11_xorg-damageproto \
  x11_xorg-fixesproto x11_xorg-inputproto x11_xorg-kbproto x11_xorg-libice x11_xorg-libpthread-stubs x11_xorg-libsm \
  x11_xorg-libX11 x11_xorg-libXau x11_xorg-libxcb x11_xorg-libXcomposite x11_xorg-libXcursor x11_xorg-libXdamage \
  x11_xorg-libXdmcp x11_xorg-libXext x11_xorg-libXfixes x11_xorg-libXi x11_xorg-libXinerama \
  x11_xorg-libXrandr x11_xorg-libXt x11_xorg-libXtst x11_xorg-randrproto x11_xorg-recordproto x11_xorg-renderproto \
  x11_xorg-util-macros x11_xorg-xcb-proto x11_xorg-xcb-util x11_xorg-xcmiscproto x11_xorg-xextproto \
  x11_xorg-xf86bigfontproto x11_xorg-xineramaproto x11_xorg-xproto x11_xorg-xtrans x11_xrender )

#LLVMFIX="configure.compiler=llvm-gcc-4.2"

# to see the dependency tree of ports, run
# sudo port rdeps rkward-devel

#SVNREPO=http://svn.code.sf.net/p/rkward/code/trunk
GITREPO=http://anongit.kde.org/rkward.git
OLDWD="$(pwd)"

if [[ $1 == "" ]] ; then
 echo "Usage: update_bundle.sh OPTIONS
          OPTIONS:

           the following must always be combined with r/m/s/c:
           -D  build target rkward instead of rkward-devel
           -d  build variant 'debug'
           -b  build subport 'binary', needs CRAN R

           these work on their own:
           
           system setup:
           -X  completely!!! wipe ${MPTINST}, ${GITROOT} & ${SRCPATH}
               there will only be a copy of this script left in ${USERBIN}/update_bundle.sh
           -S <comment>
               generate new ssh key pair to register with https://identity.kde.org
               comment could be \"<yourusername>@<yourmachine>\"
           -G  setup basic build environment: ${GITROOT} & ${SRCPATH}
               if -G is set without both -U and -E, it will fallback to setup
               ${GITROOT} anonymously -- build-only setup, no development
           -C  checkout a branch different than ${GITBRANCH}
           -U  set git user name (KDE account)
           -E  set git user e-mail (KDE account)
           -F <MacPorts version>
               do an all fresh installation of <MacPorts version>
           -f  list disk usage for all includable ports
           -x  completely!!! wipe ${MPTINST}/var/macports/distfiles

           building & bundling:
           -l  remove static port libraries
           -L  don't bundle probably superfluous ports
           -p  update macports, remove inactive
           -r  update port ${PTARGET}
           -m  create .mpkg of ${PTARGET}
           -s  create sources .tar
           -c  move .mpkg and src.tar to ${LPUBDIR}, if created"
exit 0
fi

# get the options
while getopts ":CDE:dbfGlLprmsS:cU:xXF:" OPT; do
  case $OPT in
    U) GITUSER=$OPTARG >&2 ;;
    E) GITMAIL=$OPTARG >&2 ;;
    G) BLDSETUP=true >&2
       if [[ $GITUSER == "" || $GITMAIL == "" ]] ; then
         ANONGIT=true >&2
       else
         ANONGIT=false >&2
       fi ;;
    C) GITBRANCH=$OPTARG >&2 ;;
    S) SSHGEN=true >&2
       SSHCOMMENT=$OPTARG >&2 ;;
    D) PTARGET=rkward >&2
       WORKDIR="${SRCPATH}/kde/${PTARGET}/work" >&2
       PNSUFFX="" >&2
       DEVEL=false >&2 ;;
    d) DEBUG=true >&2
       PVARIANT="+debug" >&2
       PNSUFFX="${PNSUFFX}-debug" >&2 ;;
    b) BINARY=true >&2
       PTARGET=${PTARGET}-binary >&2
       WORKDIR="${SRCPATH}/kde/rkward-devel/work" >&2
       PNSUFFX="${PNSUFFX}-binary" >&2 ;;
    F) FRESHMCP=true >&2
       MCPVERS=$OPTARG >&2 ;;
    f) LSDSKUSG=true >&2 ;;
    l) RMSTLIBS=true >&2 ;;
    L) DOEXCPCK=true >&2 ;;
    p) UPMPORTS=true >&2 ;;
    r) UPRKWARD=true >&2 ;;
    m) RPATHFIX=true >&2
       MAKEMDMD=true >&2 ;;
    s) MKSRCTAR=true >&2 ;;
    c) COPYMDMD=true >&2 ;;
    x) WIPEDSTF=true >&2 ;;
    X) WIPEDSTF=false >&2
       WIPEINST=true >&2 ;;
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

linkbuildscript () {
  # create a hardlink of the buildscript
  # $1: target directory (e.g., $HOME/bin)
  echo "updating hardlink $1/update_bundle.sh..."
  mkdir -p "$1" || exit 1
  ln -f "${GITROOT}/rkward/macports/update_bundle.sh" "$1/update_bundle.sh" || exit 1
}

rmdirv () {
  # remove directories recursively with info
  # $1: target directory to remove
  if [ -d "$1" ] ; then
    echo "removing $1..."
    sudo rm -rf "$1" || exit 1
  fi
}

# correct setting of RPATHFIX workaround, it's not needed
# for binary subports since they don't include R.framework
if $BINARY ; then
  RPATHFIX=false
fi

# remove MacPorts completely
if $WIPEINST ; then
  rmdirv "${MPTINST}"
  rmdirv "${APPLDIR}"
  rmdirv "/Applications/MacPorts"
  # these leftovers would conflict with port installation
  for libsymlink in \
    /Library/LaunchDaemons/org.freedesktop.dbus-system.plist \
    /Library/LaunchAgents/org.freedesktop.dbus-session.plist \
    /Library/LaunchDaemons/org.freedesktop.avahi-daemon.plist \
    /Library/LaunchDaemons/org.freedesktop.avahi-dnsconfd.plist \
    /Library/LaunchAgents/org.macports.kdecache.plist \
    /Library/LaunchDaemons/org.macports.mysql5.plist \
    /Library/LaunchDaemons/org.macports.rsyncd.plist \
    /Library/LaunchDaemons/org.macports.slapd.plist
  do
    if [ -L "${libsymlink}" ] ; then
      echo "removing symbolic link ${libsymlink}..."
      sudo rm "${libsymlink}"
    fi
  done
  if [ -f "${SRCPATH}" ] ; then
    echo "removing symlink ${SRCPATH}..."
    sudo rm "${SRCPATH}" || exit 1
  fi
  # ensure ${USERBIN}/update_bundle.sh
  linkbuildscript "${USERBIN}"
  rmdirv "${GITROOT}"
  echo "successfully wiped RKWard from this machine!"
  exit 0
fi

# prepare for a clean installation, remove all cached sources
if $WIPEDSTF ; then
  echo "rm -rf ${MPTINST}/var/macports/distfiles/*"
  sudo rm -rf "${MPTINST}/var/macports/distfiles/*"
fi

if $SSHGEN ; then
  mkdir -p "${HOME}/.ssh" || exit 1
  chmod 700 "${HOME}/.ssh" || exit 1
  ssh-keygen -t rsa -b 4096 -C "${SSHCOMMENT}" || exit 1
  echo "next step: upload ~/.ssh/id_rsa.pub to https://identity.kde.org"
  exit 0
fi

# prepare build environment
if $BLDSETUP ; then
  # check for Xcode.app
  if ! [ -d "/Applications/Xcode.app" ] ; then
    echo "you must install Xcode first!"
    exit 1
  fi
  sudo mkdir -p "${GITROOT}" || exit 1
  sudo chown "${USER}" "${GITROOT}" || exit 1
  cd "${GITROOT}" || exit 1
  if $ANONGIT ; then
    git clone git://anongit.kde.org/rkward.git || exit 1
    cd rkward || exit 1
  else
    # should this fail, try https:// instead of git@
    git clone git@git.kde.org:rkward.git || exit 1
    cd rkward || exit 1
    echo "set git user to \"${GITUSER}\"..."
    git config user.name "${GITUSER}" || exit 1
    echo "set git e-mail to \"${GITMAIL}\"..."
    git config user.email "${GITMAIL}" || exit 1
    git config --global push.default simple || exit 1
  fi
  if [[ ! "${GITBRANCH}" == "master" ]] ; then
    git checkout "${GITBRANCH}" || exit 1
  fi
  if ! [ -d ${SRCPATH} ] ; then
    echo "sudo ln -s ${GITROOT}/rkward/macports/ ${SRCPATH}"
    sudo ln -s "${GITROOT}/rkward/macports/" "${SRCPATH}" || exit 1
  fi
  linkbuildscript "${USERBIN}"
  if [ -f "${HOME}/.bash_profile" ] ; then
    BPFPATH=$(grep "^PATH" "${HOME}/.bash_profile")
    if ! $(echo "${BPFPATH}" | grep -q "${USERBIN}/:${MPTINST}/bin/:") ; then
      echo "PATH=${USERBIN}/:${MPTINST}/bin/:\$PATH" >> "${HOME}/.bash_profile"
    fi
  else
    echo "PATH=${USERBIN}/:${MPTINST}/bin/:\$PATH" > "${HOME}/.bash_profile"
  fi
  . "${HOME}/.bash_profile"
  cd "${OLDWD}" || exit 1
  echo "successfully completed reincarnation of ${GITROOT} -- you can now invoke the \"-F\" option!"
  echo "but you should call the following first:"
  echo ". \"${HOME}/.bash_profile\""
  exit 0
fi

# do a full clean installation
if $FRESHMCP ; then
  if ! [ -d ${SRCPATH} ] ; then
    echo "can't find ${SRCPATH} -- you should call the script with \"-G\" before setting up MacPorts!"
  fi
  echo "creating ${MPTINST}..."
  sudo mkdir -p "${MPTINST}" || exit 1
  mkdir /tmp/MP && cd /tmp/MP
  curl "https://distfiles.macports.org/MacPorts/MacPorts-${MCPVERS}.tar.bz2" -o "MacPorts-${MCPVERS}.tar.bz2" || exit 1
  tar xjvf "MacPorts-${MCPVERS}.tar.bz2" || exit 1
  cd "MacPorts-${MCPVERS}" || exit 1
  ./configure --prefix="${MPTINST}"  || exit 1
  make || exit 1
  sudo make install || exit 1
  cd "${OLDWD}" || exit 1
  rm -rf /tmp/MP || exit 1
  echo "update MacPorts configuration"
  sudo sed -i -e "s+#\(portautoclean[[:space:]]*\)yes+\1no+" "${MPTINST}/etc/macports/macports.conf"
  sudo sed -i -e "s+\(applications_dir[[:space:]]*\)/Applications/MacPorts+\1${APPLDIR}+" "${MPTINST}/etc/macports/macports.conf"
  sudo "${MPTINST}/bin/port" -v selfupdate || exit 1
  echo "adding local portfiles to ${MPTINST}/etc/macports/sources.conf..."
  sudo sed -i -e "s+rsync://rsync.macports.org.*\[default\]+file://${SRCPATH}/\\`echo -e '\n\r'`&+" "${MPTINST}/etc/macports/sources.conf" || exit 1
  # install a needed gcc/clang first?
  if [[ $CMPLR ]] ; then
    sudo "${MPTINST}/bin/port" -v install "${CMPLR}" "${LLVMFIX}" || exit 1
  fi
  if [[ $CLANG ]] ; then
    sudo "${MPTINST}/bin/port" -v install "${CLANG}" "${LLVMFIX}" || exit 1
  fi
  # (re-)generate portindex
  cd "${SRCPATH}" || exit 1
  "${MPTINST}/bin/portindex" || exit 1
  cd "${OLDWD}" || exit 1
  sudo "${MPTINST}/bin/port" -v selfupdate || exit 1
  echo "successfully completed reincarnation of ${MPTINST}!"
  exit 0
fi

# update installed ports
if $UPMPORTS ; then
  echo "sudo ${MPTINST}/bin/port selfupdate"
  sudo "${MPTINST}/bin/port" selfupdate
  echo "sudo ${MPTINST}/bin/port -v upgrade outdated"
  sudo "${MPTINST}/bin/port" -v upgrade outdated
  # get rid of inactive stuff
  echo "sudo ${MPTINST}/bin/port clean inactive"
  sudo "${MPTINST}/bin/port" clean inactive
  echo "sudo ${MPTINST}/bin/port -f uninstall inactive"
  sudo "${MPTINST}/bin/port" -f uninstall inactive
fi

# remove previous installation and its build left-overs
if $UPRKWARD ; then
  INSTALLEDPORTS=$("${MPTINST}/bin/port" installed)
  # make sure each instance of previous RKWard installations is removed first
  for i in rkward rkward-devel rkward-binary rkward-devel-binary rkward-debug rkward-devel-debug ; do
    if [[ $(echo "$INSTALLEDPORTS" | grep "[[:space:]]${i}[[:space:]]" 2> /dev/null ) ]] ; then
      echo "sudo ${MPTINST}/bin/port uninstall ${i}"
      sudo "${MPTINST}/bin/port" uninstall "${i}"
      echo "sudo ${MPTINST}/bin/port clean ${i}"
      sudo "${MPTINST}/bin/port" clean "${i}"
    fi
  done
  # build and install recent version
  echo "sudo ${MPTINST}/bin/port -v install ${PTARGET} ${PVARIANT}"
  sudo "${MPTINST}/bin/port" -v install ${PTARGET} ${PVARIANT} || exit 1
fi

# remove static libraries, they're a waste of disk space
if $RMSTLIBS ; then
  echo "deleting all static libs in ${MPTINST}/lib/..."
  sudo rm ${MPTINST}/lib/*.a
  echo "deleting all static libs in ${MPTINST}/var/macports/build..."
  #find "${MPTINST}/var/macports/build" -name "*.a" -exec sudo rm \{\} \;
  # only remove libs in destroot/libs/
  find -E "${MPTINST}/var/macports/build" -type f -regex '.*/destroot'${MPTINST}'/lib/[^/]*\.a' -exec sudo rm \{\} \;
fi

# list disk usage of ports
if $LSDSKUSG ; then
  cd "${MPTINST}/var/macports/build/"
  SBFLDRS=$(ls)
  for i in ${SBFLDRS} ; do
    echo $(du -sh ${i}/$(ls ${i}/)/work/destroot | sed -e "s+\(${BLDPRFX}\)\(.*\)\(/work/destroot\)+\2+")
  done
fi

# set some variables
if $COPYMDMD ; then
  # get version information of installed ports
  PORTVERS=$("${MPTINST}/bin/port" list $PTARGET | sed -e "s/.*@//;s/[[:space:]].*//")
  if $DEVEL ; then
    # we moved to git
    # TARGETVERS=${PORTVERS}$(svn info "$SVNREPO" | grep "^Revision:" | sed "s/[^[:digit:]]*//")
    # 
    # this one-liner would give us the latest commit hash, but no date -- bad for humans and sorting:
    # TARGETVERS=${PORTVERS}$(git ls-remote http://anongit.kde.org/rkward master | cut -c 1-7)
    # 
    # so here's something a little more elaborate...
    TEMPFILE=$(mktemp /tmp/git_rev.XXXXXX || exit 1)
    if ! [[ $(which wget) == "" ]] ; then
      wget -q -O "${TEMPFILE}" "http://quickgit.kde.org/?p=rkward.git" || exit 1
      GOTQUICKGIT=true
    elif ! [[ $(which curl) == "" ]] ; then
      curl -s -o "${TEMPFILE}" "http://quickgit.kde.org/?p=rkward.git" || exit 1
      GOTQUICKGIT=true
    else
      echo "neither wget nor curl found, only commit can be used!"
      TARGETVERS=${PORTVERS}-git$(git ls-remote http://anongit.kde.org/rkward master | cut -c 1-7)
      GOTQUICKGIT=false
    fi
    if ${GOTQUICKGIT} ; then
      CHANGEDATE=$(grep "last change.*<time datetime=" "${TEMPFILE}" | sed "s/.*datetime=\"\(.*\)+00:00.*/\1/g" | sed "s/[^[:digit:]]//g")
      LASTCOMMIT=$(grep -m1 "<td class=\"monospace\">[[:alnum:]]\{7\}" "${TEMPFILE}" | sed "s#.*<td class=\"monospace\">\(.*\)</td>.*#\1#")
      TARGETVERS="${PORTVERS}-git${CHANGEDATE}~${LASTCOMMIT}"
    fi
    rm "${TEMPFILE}"
  else
    TARGETVERS="$PORTVERS"
  fi
  KDEVERS=$("${MPTINST}/bin/port" list kdelibs4 | sed -e "s/.*@//;s/[[:space:]].*//")
fi

# get R version, long and short
if $BINARY ; then
  RVERS=$(R --version | grep "R version" | sed -e "s/R version \([[:digit:].]*\).*/\1/")
else
  RVERS=$("${MPTINST}/bin/port" list R-framework | sed -e "s/.*@//;s/[[:space:]].*//")
fi
# if we have to re-create the symlinks for binary installation
# this can be used to get the short version numer <major>.<minor>:
#RVSHORT=$(echo $RVERS | sed -e "s/\([[:digit:]]*\.\)\([[:digit:]]*\).*/\1\2/")

# make meta-package including dependencies
if $MAKEMDMD ; then
  # check for PackageMaker.app
  if ! [ -d /Applications/PackageMaker.app ] ; then
    # this is an anchient app, but MacPorts still relies on it for packaging
    echo "unable to find /Applications/PackageMaker.app!"
    echo "probably check whether MacPorts really still needs it."
    exit 1
  fi
  if $RPATHFIX ; then
    # this is to fix some kind of a race condition: if RKWard gets installed before R-framework,
    # it will create a directory which must actually be a symlink in order for R to run! so we'll
    # move RKWard's own packages before bundling it
    RKWDSTROOT="${WORKDIR}/destroot"
    RKWRFWPATH="${RKWDSTROOT}/${MPTINST}/Library/Frameworks/R.framework"
    if ! [ -d "${RKWRFWPATH}" ] ; then
      echo "cannot find R.framework, bogus path? ${RKWRFWPATH}"
      exit 1
    fi
    RFWPATH="${MPTINST}/var/macports/build/${BLDPRFX}math_R-framework/R-framework/work/destroot"
    RVERSPATH="${RFWPATH}/${MPTINST}/Library/Frameworks/R.framework/Versions"
    # this variable will hold the R version of the installed framework
    RFWVERS=$(cd "${RVERSPATH}" && find . -type d -maxdepth 1 -mindepth 1 | sed -e "s#./##" || exit 1)
    if [[ $RFWVERS == "" ]] ; then
      echo "could not get R version! aborting..."
      exit 1
    fi
    # only do this if the Resources directory exists
    if [ -d "${RKWRFWPATH}/Resources" ] ; then
      # now cd into RKWard's destroot and re-arrange the directory structure
      cd "${RKWRFWPATH}" || exit 1
      sudo mkdir -p "Versions/${RFWVERS}/Resources" || exit 1
      sudo mv "Resources/library" "Versions/${RFWVERS}/Resources/" || exit 1
      sudo rm -rf "Resources" || exit 1
      cd "${OLDWD}" || exit 1
    fi
  fi
  if $DOEXCPCK ; then
    # before we build the bundle package, replace the destroot folder of the packages
    # defined in the array EXCLPKG with empty ones, so their stuff is not included
    for i in ${EXCLPKG[@]} ; do
      THISPKG=${MPTINST}/var/macports/build/${BLDPRFX}${i}
      if [ -d "${THISPKG}" ] ; then
        SUBFLDR=$(ls $THISPKG)
        if [ -d "${THISPKG}/${SUBFLDR}/work/destroot" ] && [ ! -d "${THISPKG}/${SUBFLDR}/work/destroot_off" ]; then
          sudo mv "${THISPKG}/${SUBFLDR}/work/destroot" "${THISPKG}/${SUBFLDR}/work/destroot_off"
          sudo mkdir "${THISPKG}/${SUBFLDR}/work/destroot"
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
  echo "sudo ${MPTINST}/bin/port -v mpkg ${PTARGET}"
  sudo "${MPTINST}/bin/port" -v mpkg ${PTARGET} || exit 1

  if $DOEXCPCK ; then
    # restore original destroot directories
    for i in ${EXCLPKG[@]} ; do
      THISPKG="${MPTINST}/var/macports/build/${BLDPRFX}${i}"
      if [ -d "${THISPKG}" ] ; then
        SUBFLDR="$(ls $THISPKG)"
        if [ -d "${THISPKG}/${SUBFLDR}/work/destroot_off" ] ; then
          sudo rmdir "${THISPKG}/${SUBFLDR}/work/destroot"
          sudo mv "${THISPKG}/${SUBFLDR}/work/destroot_off" "${THISPKG}/${SUBFLDR}/work/destroot"
        fi
        unset SUBFLDR
      fi
      unset THISPKG
    done
  fi
  if $RPATHFIX ; then
    if [ -d "${RKWRFWPATH}/Versions" ] ; then
      cd "${RKWRFWPATH}" || exit 1
      sudo mkdir -p "Resources" || exit 1
      sudo mv "Versions/${RFWVERS}/Resources/library" "Resources/" || exit 1
      sudo rm -rf "Versions" || exit 1
      cd "${OLDWD}" || exit 1
    fi
  fi


  # copy the image file to a public directory
  if $COPYMDMD ; then
    MPKGFILE="${WORKDIR}/${PTARGET}-${PORTVERS}.mpkg"
    if $BINARY ; then
      TRGTFILE="${LPUBDIR}/RKWard${PNSUFFX}-${TARGETVERS}_OSX${OSXVERSION}_KDE-${KDEVERS}_needs_CRAN_R-${RVERS}.pkg"
    else
      TRGTFILE="${LPUBDIR}/RKWard${PNSUFFX}-${TARGETVERS}_R-${RVERS}_KDE-${KDEVERS}_MacOSX${OSXVERSION}_bundle.pkg"
    fi
    if ! [ -d "${LPUBDIR}" ] ; then
      echo "creating directory: ${LPUBDIR}"
      mkdir -p "${LPUBDIR}" || exit 1
    fi
    echo "moving: $MPKGFILE to $TRGTFILE ..."
    sudo mv "${MPKGFILE}" "${TRGTFILE}" || exit 1
    sudo chown ${RKUSER} "${TRGTFILE}" || exit 1
    echo "done."
  fi
fi

# archive sources
if $MKSRCTAR ; then
  if ! $COPYMDMD ; then
    # get version information of installed ports
    PORTVERS=$("${MPTINST}/bin/port" list ${PTARGET} | sed -e "s/.*@//;s/[[:space:]].*//")
  fi
  SRCFILE="${SRCPATH}/sources_bundle_RKWard-${PORTVERS}_${SRCDATE}.tar"
  if [ -f "${SRCFILE}" ] ; then
    rm "${SRCFILE}" || exit 1
  fi
  tar cvf "${SRCFILE}" "${MPTINST}/var/macports/distfiles" || exit 1
  # copy the source archive to a public directory
  if $COPYMDMD ; then
    if $BINARY ; then
      TRGSFILE="${LPUBDIR}/RKWard${PNSUFFX}-${TARGETVERS}_OSX${OSXVERSION}_KDE-${KDEVERS}_src.tar"
    else
      TRGSFILE="${LPUBDIR}/RKWard${PNSUFFX}-${TARGETVERS}_OSX${OSXVERSION}_R-${RVERS}_KDE-${KDEVERS}_src.tar"
    fi
    if ! [ -d "${LPUBDIR}" ] ; then
      echo "creating directory: ${LPUBDIR}"
      mkdir -p "${LPUBDIR}" || exit 1
    fi
    echo "moving: $SRCFILE to $TRGSFILE ..."
    sudo mv "${SRCFILE}" "${TRGSFILE}" || exit 1
    sudo chown ${RKUSER} "${TRGSFILE}" || exit 1
    echo "done."
  fi
fi

exit 0
