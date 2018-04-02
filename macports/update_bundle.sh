#!/bin/bash

TXT_DGRAY="\033[1;30m"
TXT_LRED="\033[1;31m"
TXT_RED="\033[0;31m"
TXT_BLUE="\033[0;34m"
TXT_GREEN="\033[0;32m"
TXT_BOLD="\033[1m"
TXT_ITALIC="\033[3m"
TXT_UNDERSCORE="\033[4m"
OFF="\033[0m"

# poor man's configuration
CONFIGDIR="${HOME}/.config/bash_scripts_${USER}"
CONFIGFILE="${CONFIGDIR}/update_bundle.conf"
if ! [ -f "${CONFIGFILE}" ] ; then
 mkdir -p "${CONFIGDIR}"
 cat <<EOF > "${CONFIGFILE}"
SRCDATE=\$(date +%Y-%m-%d)
SRCPATH="/opt/ports"
# specify git root path
GITROOT="/opt/git"
# specify macports installation path
MPTINST="/opt/rkward"
# specify the target port
PTARGET="kf5-rkward"
BINARY=true
DEVEL=true
DEBUG=false
RKUSER="${USER}"
USERBIN="${HOME}/bin"
OSXVERSION=\$(sw_vers -productVersion | sed -e "s/.[[:digit:]]*\$//")
PVARIANT=""
GITBRANCH="master"
# specify work directory
WORKDIR="\${SRCPATH}/kf5/\${PTARGET}/work"
# specify local public directory
LPUBDIR="${HOME}/Public/rkward"
# specify application dir used
APPLDIR="/Applications/RKWard"
# specify the prefix for build directories below \${MPTINST}/var/macports/build
BLDPRFX=_opt_rkward_var_macports_sources_rsync.macports.org_release_tarballs_ports_
GITREPO="git://anongit.kde.org/rkward.git"
GITREPOKDE="git@git.kde.org:rkward.git"
RJVBREPO="https://github.com/mkae/macstrop.git"
EOF
fi

. "${CONFIGFILE}"

# mainly for the usage menu
BINMENU=0
DEVMENU=0
DBGMENU=0
if $BINARY ; then
  BINMENU=1
fi
if $DEVEL ; then
  DEVMENU=1
fi
if $DEBUG ; then
  DBGMENU=1
fi

# initalize variables
ANONGIT=true
BLDSETUP=false
BUILDQT=false
SSHGEN=false
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
GETTARGVERS=true
PORTGROUPS=false

# this array holds all packages who should not be included in the bundle
declare -a EXCLPKG=(audio_lame audio_libmodplug audio_libopus \
  audio_libvorbis audio_speex audio_taglib databases_db46 databases_db48 databases_gdbm databases_openldap databases_sqlite3 devel_boost \
  gnome_at-spi2-atk gnome_at-spi2-core gnome_desktop-file-utils gnome_gnome-common gnome_gobject-introspection gnome_gtk-doc \
  gnome_gtk2 gnome_gtk3 gnome_hicolor-icon-theme gnome_libcroco gnome_libglade2 gnome_gobject-introspection \
  lang_llvm-3.7 \
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

OLDWD="$(pwd)"

if [[ $1 == "" ]] ; then
 echo -e "Usage:
  ${TXT_BOLD}update_bundle.sh${OFF} ${TXT_DGRAY}${TXT_ITALIC}[OPTIONS]${OFF}
  
  typically, you might want to run the following steps in this order:
    ${TXT_DGRAY}0.${OFF} remove a previous installation (${TXT_BOLD}-X${OFF})
    ${TXT_DGRAY}1.${OFF} setup basic build environment (${TXT_BOLD}-G${OFF})
    ${TXT_DGRAY}2.${OFF} install MacPorts (${TXT_BOLD}-F${OFF} ${TXT_ITALIC}<version>${OFF})
    ${TXT_DGRAY}3.${OFF} build Qt (${TXT_BOLD}-Q${OFF})
    ${TXT_DGRAY}4.${OFF} build RKWard using CRAN R (${TXT_BOLD}-r${OFF})
    ${TXT_DGRAY}5.${OFF} remove static libs & create binary bundle (${TXT_BOLD}-lm${OFF})

  ${TXT_UNDERSCORE}OPTIONS${OFF}:

       ${TXT_DGRAY}cleaning up:${OFF}
           ${TXT_BOLD}-X${OFF}  ${TXT_LRED}completely!!! wipe${OFF} ${TXT_BLUE}${MPTINST}${OFF}, ${TXT_BLUE}${GITROOT}${OFF} & ${TXT_BLUE}${SRCPATH}${OFF}
               there will only be a copy of this script left in ${TXT_BLUE}${USERBIN}/update_bundle.sh${OFF}
           ${TXT_BOLD}-x${OFF}  ${TXT_LRED}completely!!! wipe${OFF} ${TXT_BLUE}${MPTINST}/var/macports/distfiles${OFF}

       ${TXT_DGRAY}system setup:${OFF}
           ${TXT_BOLD}-G${OFF}  setup basic build environment: ${TXT_BLUE}${GITROOT}${OFF} & ${TXT_BLUE}${SRCPATH}${OFF}
               if ${TXT_BOLD}-G${OFF} is set without both ${TXT_BOLD}-U${OFF} and ${TXT_BOLD}-E${OFF}, it will fallback to setup
               ${TXT_BLUE}${GITROOT}${OFF} anonymously -- build-only setup, no development
           ${TXT_BOLD}-F${OFF} ${TXT_LRED}${TXT_ITALIC}<MacPorts version>${OFF}
               do an all fresh installation of ${TXT_LRED}${TXT_ITALIC}<MacPorts version>${OFF}
           ${TXT_BOLD}-Q${OFF}  build and install ports ${TXT_BLUE}qt5-kde${OFF} and ${TXT_BLUE}kf5-osx-integration${OFF} from RJVB repo

           ${TXT_BOLD}-q${OFF}  manually update/fix PortGroups from RJVB repo
           ${TXT_BOLD}-f${OFF}  list disk usage for all includable ports
           ${TXT_BOLD}-S${OFF} ${TXT_LRED}${TXT_ITALIC}<comment>${OFF}
               generate new ssh key pair to register with ${TXT_BLUE}https://identity.kde.org${OFF}
               comment could be ${TXT_BLUE}${TXT_ITALIC}\"<yourusername>@<yourmachine>\"${OFF}

       ${TXT_DGRAY}optional git configuration (${OFF}${TXT_BOLD}-G${OFF}${TXT_DGRAY}):${OFF}
           ${TXT_BOLD}-U${OFF}  set git user name (KDE account)
           ${TXT_BOLD}-E${OFF}  set git user e-mail (KDE account)
           ${TXT_BOLD}-C${OFF}  checkout a certain git branch of the RKWard repo
               default: ${TXT_BLUE}${GITBRANCH}${OFF}

       ${TXT_DGRAY}building & bundling (can be combined with${OFF} ${TXT_BOLD}-D${OFF}${TXT_DGRAY}/${OFF}${TXT_BOLD}-d${OFF}${TXT_DGRAY}/${OFF}${TXT_BOLD}-b${OFF}${TXT_DGRAY}):${OFF}
           ${TXT_BOLD}-l${OFF}  remove static port libraries
           ${TXT_BOLD}-p${OFF}  update macports, remove inactive
           ${TXT_BOLD}-r${OFF}  update port ${TXT_BLUE}${PTARGET}${OFF}
           ${TXT_BOLD}-m${OFF}  create .mpkg of ${TXT_BLUE}${PTARGET}${OFF}
           ${TXT_BOLD}-s${OFF}  create sources .tar
           ${TXT_BOLD}-c${OFF}  rename and move .mpkg and src.tar to ${TXT_BLUE}${LPUBDIR}${OFF}, if existing
           ${TXT_BOLD}-t${OFF}  set target version for ${TXT_BOLD}-c${OFF} manually

       ${TXT_DGRAY}the following must always be combined with${OFF} ${TXT_BOLD}-r${OFF}${TXT_DGRAY}/${OFF}${TXT_BOLD}-m${OFF}${TXT_DGRAY}/${OFF}${TXT_BOLD}-s${OFF}${TXT_DGRAY}/${OFF}${TXT_BOLD}-c${OFF}${TXT_DGRAY}:${OFF}
           ${TXT_BOLD}-D${OFF} ${TXT_LRED}${TXT_ITALIC}<0|1>${OFF}  1 will build target ${TXT_BLUE}${PTARGET}-devel${OFF} instead of ${TXT_BLUE}${PTARGET}${OFF}
               default: ${TXT_BLUE}${DEVMENU}${OFF}
           ${TXT_BOLD}-d${OFF} ${TXT_LRED}${TXT_ITALIC}<0|1>${OFF}  1 will build variant ${TXT_BLUE}debug${OFF}
               default: ${TXT_BLUE}${DBGMENU}${OFF}
           ${TXT_BOLD}-b${OFF} ${TXT_LRED}${TXT_ITALIC}<0|1>${OFF}  1 build subport ${TXT_BLUE}binary${OFF}, needs CRAN R
               default: ${TXT_BLUE}${BINMENU}${OFF}
"
# off for the moment:
#            ${TXT_BOLD}-L${OFF}  don't bundle probably superfluous ports
exit 0
fi

# get the options
while getopts ":CD:E:d:b:fGlLprQqmsS:cU:xXF:t:" OPT; do
  case $OPT in
    U) GITUSER=$OPTARG >&2 ;;
    E) GITMAIL=$OPTARG >&2 ;;
    G) BLDSETUP=true >&2
       if [[ $GITUSER == "" || $GITMAIL == "" ]] ; then
         ANONGIT=true >&2
       else
         ANONGIT=false >&2
       fi ;;
    Q) BUILDQT=true >&2 ;;
    q) PORTGROUPS=true >&2 ;;
    C) GITBRANCH=$OPTARG >&2 ;;
    S) SSHGEN=true >&2
       SSHCOMMENT=$OPTARG >&2 ;;
    D) if [ $OPTARG -eq 1 ] ; then
         DEVEL=true >&2
       else
         DEVEL=false >&2
       fi ;;
    d) if [ $OPTARG -eq 1 ] ; then
         DEBUG=true >&2
       else
         DEBUG=false >&2
       fi ;;
    b) if [ $OPTARG -eq 1 ] ; then
         BINARY=true >&2
       else
         BINARY=false >&2
       fi ;;
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
    t) GETTARGVERS=false >&2
       TARGETVERS=$OPTARG >&2 ;;
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

if $BINARY ; then
  WORKDIR="${SRCPATH}/kf5/kf5-rkward-binary/work"
  BINSTRING="-binary"
else
  BINARY=false
  BINSTRING=""
fi
if $DEVEL ; then
  DEVSTRING="-devel"
else
  WORKDIR="${SRCPATH}/kf5/kf5-rkward/work"
  DEVSTRING=""
fi
if $DEBUG ; then
  PVARIANT="+debug"
  DBGSTRING="-debug"
else
  DBGSTRING=""
fi

PNSUFFX="${BINSTRING}${DBGSTRING}${DEVSTRING}"
PTARGET="kf5-rkward${PNSUFFX}"

error() {
  # $1: message to print
  echo -e "${TXT_RED}error:${OFF} $1"
  exit 1
}

alldone() {
  echo -e " ${TXT_GREEN}done! ${OFF}"
}

linkbuildscript () {
  # create a hardlink of the buildscript
  # $1: target directory (e.g., $HOME/bin)
  echo -en "updating hardlink ${TXT_BLUE}$1/update_bundle.sh${OFF}..."
  mkdir -p "$1" || exit 1
  ln -f "${GITROOT}/rkward/macports/update_bundle.sh" "$1/update_bundle.sh" || exit 1
  alldone
}

rmdirv () {
  # remove directories recursively with info
  # $1: target directory to remove
  if [ -d "$1" ] ; then
    echo -en "removing ${TXT_BLUE}$1${OFF}..."
    sudo rm -rf "$1" || exit 1
    alldone
  fi
}

mkmissingdir() {
  # $1: path to check
  if [ ! -d "${1}" ] ; then
    echo -en "create missing directory ${TXT_BLUE}$1${OFF}..."
    mkdir -p "${1}" || exit 1
    alldone
  fi
}

updatePortGrous() {
  echo "syncing PortGroup files..."
  sudo rsync -av "${GITROOT}/macstrop/_resources/port1.0/"  "${MPTINST}/var/macports/sources/rsync.macports.org/macports/release/tarballs/ports/_resources/port1.0/" || exit 1
  alldone
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
    /Library/LaunchAgents/org.freedesktop.dbus-session.plist \
    /Library/LaunchAgents/org.macports.kdecache.plist \
    /Library/LaunchAgents/org.macports.kdecache5.plist \
    /Library/LaunchAgents/org.macports.kwalletd5.plist \
    /Library/LaunchDaemons/org.freedesktop.dbus-system.plist \
    /Library/LaunchDaemons/org.freedesktop.avahi-daemon.plist \
    /Library/LaunchDaemons/org.freedesktop.avahi-dnsconfd.plist \
    /Library/LaunchDaemons/org.macports.mysql5.plist \
    /Library/LaunchDaemons/org.macports.rsyncd.plist \
    /Library/LaunchDaemons/org.macports.slapd.plist
  do
    if [ -L "${libsymlink}" ] ; then
      echo -en "removing symbolic link ${TXT_BLUE}${libsymlink}${OFF}..."
      sudo rm "${libsymlink}"
      alldone
    fi
  done
  if [ -f "${SRCPATH}" ] ; then
    echo -en "removing symlink ${TXT_BLUE}${SRCPATH}${OFF}..."
    sudo rm "${SRCPATH}" || exit 1
    alldone
  fi
  # ensure ${USERBIN}/update_bundle.sh
  linkbuildscript "${USERBIN}"
  rmdirv "${GITROOT}"
  echo -e "${TXT_GREEN}successfully wiped RKWard from this machine!${OFF}"
  exit 0
fi

# prepare for a clean installation, remove all cached sources
if $WIPEDSTF ; then
  echo -en "rm -rf ${TXT_BLUE}${MPTINST}/var/macports/distfiles/*${OFF}..."
  sudo rm -rf "${MPTINST}/var/macports/distfiles/*"
  alldone
fi

if $SSHGEN ; then
  mkdir -p "${HOME}/.ssh" || exit 1
  chmod 700 "${HOME}/.ssh" || exit 1
  ssh-keygen -t rsa -b 4096 -C "${SSHCOMMENT}" || exit 1
  echo -e "next step: upload ${TXT_BLUE}~/.ssh/id_rsa.pub${OFF} to ${TXT_BLUE}https://identity.kde.org${OFF}"
  exit 0
fi

# prepare build environment
if $BLDSETUP ; then
  # check for Xcode.app
  if ! [ -d "/Applications/Xcode.app" ] ; then
    error "you must install Xcode first!"
  fi
  if ! [ -d "${GITROOT}" ] ; then
    echo -en "create missing directory ${TXT_BLUE}${GITROOT}${OFF}..."
    sudo mkdir -p "${GITROOT}" || exit 1
    sudo chown "${USER}" "${GITROOT}" || exit 1
    alldone
  fi
  cd "${GITROOT}" || exit 1
  if $ANONGIT ; then
    git clone "${GITREPO}" || exit 1
    cd rkward || exit 1
  else
    # should this fail, try https:// instead of git@
    git clone "${GITREPOKDE}" || exit 1
    cd rkward || exit 1
    echo -e "set git user to ${TXT_BLUE}\"${GITUSER}\"${OFF}..."
    git config user.name "${GITUSER}" || exit 1
    echo -e "set git e-mail to ${TXT_BLUE}\"${GITMAIL}\"${OFF}..."
    git config user.email "${GITMAIL}" || exit 1
    git config --global push.default simple || exit 1
  fi
  if [[ ! "${GITBRANCH}" == "master" ]] ; then
    git checkout "${GITBRANCH}" || exit 1
  fi
  echo "cloning RJVB local repository (patched Qt5)"
  cd "${GITROOT}" || exit 1
  git clone "${RJVBREPO}" || exit 1
  if ! [ -d ${SRCPATH} ] ; then
    echo -en "sudo ln -s ${GITROOT}/rkward/macports/ ${SRCPATH}${OFF}..."
    sudo ln -s "${GITROOT}/rkward/macports/" "${SRCPATH}" || exit 1
    alldone
  fi
  linkbuildscript "${USERBIN}"
  if [ -f "${HOME}/.bash_profile" ] ; then
    BPFPATH=$(grep "^PATH" "${HOME}/.bash_profile")
    if ! $(echo "${BPFPATH}" | grep -q "${USERBIN}/:${MPTINST}/bin/:") ; then
      echo "PATH=${USERBIN}/:${MPTINST}/bin/:\$PATH" >> "${HOME}/.bash_profile"
    fi
    if ! $(echo "${BPFPATH}" | grep -q "KDE_SESSION_VERSION") ; then
      echo "export KDE_SESSION_VERSION=5" >> "${HOME}/.bash_profile"
    fi
  else
    echo "PATH=${USERBIN}/:${MPTINST}/bin/:\$PATH" > "${HOME}/.bash_profile"
    echo "export KDE_SESSION_VERSION=5" >> "${HOME}/.bash_profile"
  fi
  . "${HOME}/.bash_profile"
  cd "${OLDWD}" || exit 1
  echo -e "${TXT_GREEN}successfully completed reincarnation of${OFF} ${TXT_BLUE}${GITROOT}${OFF} -- you can now invoke the \"-F\" option!"
  echo "but you should call the following first:"
  echo -e ". ${TXT_BLUE}\"${HOME}/.bash_profile\"${OFF}"
  exit 0
fi

# do a full clean installation
if $FRESHMCP ; then
  if ! [ -d ${SRCPATH} ] ; then
    error "can't find ${TXT_BLUE}${SRCPATH}${OFF} -- you should call the script with \"-G\" before setting up MacPorts!"
  fi
  if ! [ -d "${MPTINST}" ] ; then
    echo -en "create missing directory ${TXT_BLUE}${MPTINST}${OFF}..."
    sudo mkdir -p "${MPTINST}" || exit 1
    alldone
  fi
  mkdir /tmp/MP || error "can't create ${TXT_BLUE}/tmp/MP${OFF}"
  cd /tmp/MP || exit 1
  curl "https://distfiles.macports.org/MacPorts/MacPorts-${MCPVERS}.tar.bz2" -o "MacPorts-${MCPVERS}.tar.bz2" || exit 1
  tar xjvf "MacPorts-${MCPVERS}.tar.bz2" || exit 1
  cd "MacPorts-${MCPVERS}" || exit 1
  ./configure --prefix="${MPTINST}" || exit 1
  make || exit 1
  sudo make install || exit 1
  cd "${OLDWD}" || exit 1
  rm -rf /tmp/MP || exit 1
  echo "update MacPorts configuration"
  sudo sed -i -e "s+#\(portautoclean[[:space:]]*\)yes+\1no+" "${MPTINST}/etc/macports/macports.conf"
  sudo sed -i -e "s+\(applications_dir[[:space:]]*\)/Applications/MacPorts+\1${APPLDIR}+" "${MPTINST}/etc/macports/macports.conf"
  sudo "${MPTINST}/bin/port" -v selfupdate || exit 1
  echo -e "adding local portfiles to ${TXT_BLUE}${MPTINST}/etc/macports/sources.conf${OFF}..."
  # sudo sed -i -e "s+rsync://rsync.macports.org.*\[default\]+file://${SRCPATH}/\\`echo -e '\n\r'`&+" "${MPTINST}/etc/macports/sources.conf" || exit 1
  # adding newlines with sed in macOS is totally f**ked up, here's an ugly workaround in three steps
  sudo sed -i -e $'s+rsync://rsync.macports.org.*\[default\]+file://_GITROOT_/macstrop/\\\nfile://_SRCPATH_/\\\n&+' "${MPTINST}/etc/macports/sources.conf" || exit 1
  sudo sed -i -e "s+file://_SRCPATH_+file://${SRCPATH}+" "${MPTINST}/etc/macports/sources.conf" || exit 1
  sudo sed -i -e "s+file://_GITROOT_+file://${GITROOT}+" "${MPTINST}/etc/macports/sources.conf" || exit 1
  # install a needed gcc/clang first?
  if [[ $CMPLR ]] ; then
    sudo "${MPTINST}/bin/port" -v install "${CMPLR}" "${LLVMFIX}" || exit 1
  fi
  if [[ $CLANG ]] ; then
    sudo "${MPTINST}/bin/port" -v install "${CLANG}" "${LLVMFIX}" || exit 1
  fi
  updatePortGrous
  # (re-)generate portindex
  cd "${SRCPATH}" || exit 1
  "${MPTINST}/bin/portindex" || exit 1
  cd "${GITROOT}/macstrop" || exit 1
  "${MPTINST}/bin/portindex" || exit 1
  cd "${OLDWD}" || exit 1
  sudo "${MPTINST}/bin/port" -v selfupdate || exit 1
  echo -e "${TXT_GREEN}successfully completed reincarnation of${OFF} ${TXT_BLUE}${MPTINST}${OFF}${TXT_GREEN}!${OFF}"
  exit 0
fi

if $PORTGROUPS ; then
  updatePortGrous
fi

if $BUILDQT ; then
  echo -e "sudo ${TXT_BLUE}${MPTINST}/bin/port${OFF} -v install qt5-kde"
  sudo "${MPTINST}/bin/port" -v install qt5-kde || exit 1
  echo -e "sudo ${TXT_BLUE}${MPTINST}/bin/port${OFF} -v install kf5-osx-integration"
  sudo "${MPTINST}/bin/port" -v install kf5-osx-integration || exit 1
  alldone
fi

# update installed ports
if $UPMPORTS ; then
  echo "updating RJVB local repository (patched Qt5)"
  cd "${GITROOT}/macstrop" || exit 1
  git pull --rebase origin || exit 1
  updatePortGrous
  "${MPTINST}/bin/portindex" || exit 1
  cd "${OLDWD}" || exit 1
  echo -e "sudo ${TXT_BLUE}${MPTINST}/bin/port${OFF} selfupdate"
  sudo "${MPTINST}/bin/port" selfupdate
  echo -e "sudo ${TXT_BLUE}${MPTINST}/bin/port${OFF} -v upgrade outdated"
  sudo "${MPTINST}/bin/port" -v upgrade outdated
  # get rid of inactive stuff
  echo -e "sudo ${TXT_BLUE}${MPTINST}/bin/port${OFF} clean inactive"
  sudo "${MPTINST}/bin/port" clean inactive
  echo -e "sudo ${TXT_BLUE}${MPTINST}/bin/port${OFF} -f uninstall inactive"
  sudo "${MPTINST}/bin/port" -f uninstall inactive
  echo -e "sudo ${TXT_BLUE}${MPTINST}/bin/port${OFF} reclaim ${TXT_DGRAY}# remove unused distfiles${OFF}"
  sudo "${MPTINST}/bin/port" reclaim
  alldone
fi

# remove previous installation and its build left-overs
if $UPRKWARD ; then
  INSTALLEDPORTS=$("${MPTINST}/bin/port" installed)
  # make sure each instance of previous RKWard installations is removed first
  for i in kf5-rkward kf5-rkward-devel kf5-rkward-binary kf5-rkward-binary-devel kf5-rkward-debug kf5-rkward-debug-devel ; do
    if [[ $(echo "$INSTALLEDPORTS" | grep "[[:space:]]${i}[[:space:]]" 2> /dev/null ) ]] ; then
      echo -e "sudo ${TXT_BLUE}${MPTINST}/bin/port${OFF} uninstall ${i}"
      sudo "${MPTINST}/bin/port" uninstall "${i}"
      echo -e "sudo ${TXT_BLUE}${MPTINST}/bin/port${OFF} clean ${i}"
      sudo "${MPTINST}/bin/port" clean "${i}"
    fi
  done
  # build and install recent version
  echo -e "sudo ${TXT_BLUE}${MPTINST}/bin/port${OFF} -v install ${PTARGET} ${PVARIANT}"
  sudo "${MPTINST}/bin/port" -v install ${PTARGET} ${PVARIANT} || exit 1
  alldone
fi

# remove static libraries, they're a waste of disk space
if $RMSTLIBS ; then
  echo -e "deleting all static libs in ${TXT_BLUE}${MPTINST}/lib/${OFF}..."
  sudo rm ${MPTINST}/lib/*.a
  echo -e "deleting all static libs in ${TXT_BLUE}${MPTINST}/var/macports/build${OFF}..."
  #find "${MPTINST}/var/macports/build" -name "*.a" -exec sudo rm \{\} \;
  # only remove libs in destroot/libs/
  find -E "${MPTINST}/var/macports/build" -type f -regex '.*/destroot'${MPTINST}'/lib/[^/]*\.a' -exec sudo rm \{\} \;
  alldone
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
  if $GETTARGVERS ; then
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
        echo -e "${TXT_RED}neither wget nor curl found, only commit can be used!${OFF}"
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
  fi
  KDEVERS=$("${MPTINST}/bin/port" list kf5-kparts | sed -e "s/.*@//;s/[[:space:]].*//")
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
  if $RPATHFIX ; then
    # this is to fix some kind of a race condition: if RKWard gets installed before R-framework,
    # it will create a directory which must actually be a symlink in order for R to run! so we'll
    # move RKWard's own packages before bundling it
    RKWDSTROOT="${WORKDIR}/destroot"
    RKWRFWPATH="${RKWDSTROOT}/${MPTINST}/Library/Frameworks/R.framework"
    if ! [ -d "${RKWRFWPATH}" ] ; then
      error "cannot find R.framework, bogus path? ${TXT_BLUE}${RKWRFWPATH}${OFF}"
    fi
    RFWPATH="${MPTINST}/var/macports/build/${BLDPRFX}math_R-framework/R-framework/work/destroot"
    RVERSPATH="${RFWPATH}/${MPTINST}/Library/Frameworks/R.framework/Versions"
    # this variable will hold the R version of the installed framework
    RFWVERS=$(cd "${RVERSPATH}" && find . -type d -maxdepth 1 -mindepth 1 | sed -e "s#./##" || exit 1)
    if [[ $RFWVERS == "" ]] ; then
      error "could not get R version! aborting..."
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
        echo -e "${TXT_RED}warning: can't find ${THISPKG}!${OFF}"
      fi
      unset THISPKG
    done
  fi

#  # cleaning boost, the avahi port somehow gets installed in two varaints...
#  sudo port clean boost
  echo -e "sudo ${TXT_BLUE}${MPTINST}/bin/port${OFF} -v mpkg ${PTARGET}"
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
      TRGTFILE="${LPUBDIR}/RKWard${PNSUFFX}-${TARGETVERS}_OSX${OSXVERSION}_KF5-${KDEVERS}_needs_CRAN_R-${RVERS}.pkg"
    else
      TRGTFILE="${LPUBDIR}/RKWard${PNSUFFX}-${TARGETVERS}_R-${RVERS}_KF5-${KDEVERS}_MacOSX${OSXVERSION}_bundle.pkg"
    fi
    mkmissingdir "${LPUBDIR}"
    echo -en "moving: ${TXT_BLUE}${MPKGFILE}${OFF} to ${TXT_BLUE}${TRGTFILE}${OFF} ..."
    sudo mv "${MPKGFILE}" "${TRGTFILE}" || exit 1
    sudo chown ${RKUSER} "${TRGTFILE}" || exit 1
    alldone
  fi
fi

# archive sources
if $MKSRCTAR ; then
  echo -e "sudo ${TXT_BLUE}${MPTINST}/bin/port${OFF} reclaim ${TXT_DGRAY}# remove unused distfiles${OFF}"
  sudo "${MPTINST}/bin/port" reclaim
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
      TRGSFILE="${LPUBDIR}/RKWard${PNSUFFX}-${TARGETVERS}_OSX${OSXVERSION}_KF5-${KDEVERS}_src.tar"
    else
      TRGSFILE="${LPUBDIR}/RKWard${PNSUFFX}-${TARGETVERS}_OSX${OSXVERSION}_R-${RVERS}_KF5-${KDEVERS}_src.tar"
    fi
    mkmissingdir "${LPUBDIR}"
    echo -en "moving: ${TXT_BLUE}${SRCFILE}${OFF} to ${TXT_BLUE}${TRGSFILE}${OFF} ..."
    sudo mv "${SRCFILE}" "${TRGSFILE}" || exit 1
    sudo chown ${RKUSER} "${TRGSFILE}" || exit 1
    alldone
  fi
fi

exit 0
