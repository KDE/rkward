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
# specify the prefix for build directories below ${MPTINST}/var/macports/build
BLDPRFX=_opt_rkward_var_macports_sources_rsync.macports.org_release_tarballs_ports_
# this array holds all packages who should not be included in the bundle
declare -a EXCLPKG=(audio_flac audio_jack audio_lame audio_libmodplug audio_libopus audio_libsamplerate \
 audio_libsndfile audio_libvorbis audio_phonon audio_speex databases_db46 databases_gdbm databases_sqlite3 devel_boost \
 devel_soprano devel_strigi devel_virtuoso gnome_gobject-introspection gnome_gtk2 gnome_hicolor-icon-theme gnome_libglade2 \
 multimedia_XviD multimedia_dirac multimedia_ffmpeg multimedia_libogg multimedia_libtheora multimedia_libvpx \ 
 multimedia_schroedinger multimedia_x264 net_avahi net_kerberos5 security_cyrus-sasl2 sysutils_e2fsprogs )

SVNREPO=http://svn.code.sf.net/p/rkward/code/trunk
OLDWD=$(pwd)

if [[ $1 == "" ]] ; then
 echo "Usage: update_bundle.sh OPTION
          OPTIONS:
           -D (build target rkward instead of rkward-devel)
           -X (completely!!! wipe ${MPTINST})
           -F <MacPorts version> (do an all fresh installation of <MacPorts version>)
           -f (list disk usage for all includable ports)
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
    f) LSDSKUSG=TRUE >&2 ;;
    l) RMSTLIBS=TRUE >&2 ;;
    p) UPMPORTS=TRUE >&2 ;;
    r) UPRKWARD=TRUE >&2 ;;
    m)
       RMSTLIBS=TRUE >&2
       MAKEMDMD=TRUE >&2 ;;
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

  sudo port -v mdmg $PTARGET || exit 1

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

## appendix ;-)
# this is the result of sudo port rdeps rkward-devel:
# The following ports are dependencies of rkward-devel @0.6.0-svn_0:
#   subversion
#     expat
#     neon
#       gettext
#         libiconv
#           gperf
#         ncurses
#       libcomerr
#         pkgconfig
#       openssl
#         zlib
#     apr
#     apr-util
#       db46
#       sqlite3
#         libedit
#     serf1
#     cyrus-sasl2
#       kerberos5
#         autoconf
#           xz
#           perl5
#             perl5.12
#               gdbm
#           m4
#           help2man
#             p5.12-locale-gettext
#         automake
#         libtool
#     file
#     curl-ca-bundle
#   cmake
#     libidn

##############
#   kdelibs4
##############
#   xz
#     libiconv
#       gperf
#     gettext
#       expat
#       ncurses
#   cmake
#     libidn
#     openssl
#       zlib
#   pkgconfig
#   automoc
#     qt4-mac
#       dbus
#       tiff
#         jpeg
#       libpng
#       libmng
#         autoconf
#           perl5
#             perl5.12
#               gdbm
#           m4
#           help2man
#             p5.12-locale-gettext
#         automake
#         libtool
#         lcms
#   flex
#   gmake
#   docbook-xsl-ns
#     unzip
#     xmlcatmgr
#   phonon
#   bzip2
#   soprano
#     strigi
#       clucene
#       exiv2
#       libxml2
#       ffmpeg
#         texi2html
#         yasm
#         lame
#         libvorbis
#           libogg
#         libopus
#         libtheora
#         libmodplug
#         jack
#           libxslt
#           libsndfile
#             flac
#           libsamplerate
#             fftw-3
#         dirac
#           cppunit
#         schroedinger
#           orc
#         openjpeg
#           lcms2
#           jbigkit
#         freetype
#         speex
#         libvpx
#         libsdl
#           xorg-libXext
#             xorg-util-macros
#             xorg-libX11
#               xorg-xtrans
#               xorg-bigreqsproto
#               xorg-xcmiscproto
#               xorg-xextproto
#               xorg-xf86bigfontproto
#               xorg-inputproto
#               xorg-libXdmcp
#                 xorg-xproto
#               xorg-libXau
#               xorg-libxcb
#                 xorg-xcb-proto
#                   python27
#                     sqlite3
#                       libedit
#                     db46
#                     python_select
#                 xorg-libpthread-stubs
#               xorg-kbproto
#           xorg-libXrandr
#             xrender
#               xorg-renderproto
#             xorg-randrproto
#         XviD
#         x264
#       boost
#         icu
#     raptor2
#       curl
#         curl-ca-bundle
#     redland
#       rasqal
#         mhash
#         mpfr
#           gmp
#     libiodbc
#       gtk2
#         atk
#           glib2
#             libffi
#           gobject-introspection
#             cairo
#               libpixman
#               fontconfig
#               lzo2
#               xorg-xcb-util
#         pango
#           harfbuzz
#             graphite2
#           Xft2
#         gdk-pixbuf2
#           jasper
#         xorg-libXi
#         xorg-libXcursor
#           xorg-fixesproto
#           xorg-libXfixes
#         xorg-libXinerama
#           xorg-xineramaproto
#         xorg-libXdamage
#           xorg-damageproto
#         xorg-libXcomposite
#           xorg-compositeproto
#         shared-mime-info
#           intltool
#             gnome-common
#             p5.12-xml-parser
#             p5.12-getopt-long
#             p5.12-pathtools
#             p5.12-scalar-list-utils
#         hicolor-icon-theme
#     virtuoso
#       gawk
#         readline
#   cyrus-sasl2
#     kerberos5
#       libcomerr
#   pcre
#   giflib
#     xorg-libsm
#       xorg-libice
#   openexr
#     ilmbase
#       gsed
#   libart_lgpl
#   enchant
#     aspell
#       texinfo
#     hunspell
#   aspell-dict-en
#   attica
#   avahi
#     libdaemon
#     libglade2
#     dbus-python27
#       dbus-glib
#         gtk-doc
#           gnome-doc-utils
#             py27-libxml2
#             docbook-xml
#               docbook-xml-4.1.2
#                 docbook-xml-4.2
#               docbook-xml-4.3
#               docbook-xml-4.4
#               docbook-xml-4.5
#               docbook-xml-5.0
#             docbook-xsl
#             rarian
#               getopt
#             iso-codes
#       py27-gobject
#     py27-gdbm
#     py27-pygtk
#       py27-cairo
#         py27-numpy
#           py27-nose
#             py27-distribute
#             nosetests_select
#   qca
#   dbusmenu-qt
#     qjson
#   grantlee
#   shared-desktop-ontologies

#########
# kate
#########
#       ilmbase
#         gsed
#     libart_lgpl
#     enchant
#       aspell
#         texinfo
#       hunspell
#     aspell-dict-en
#     attica
#     avahi
#       libdaemon
#       libglade2
#       dbus-python27
#         dbus-glib
#           gtk-doc
#             gnome-doc-utils
#               py27-libxml2
#               docbook-xml
#                 docbook-xml-4.1.2
#                   docbook-xml-4.2
#                 docbook-xml-4.3
#                 docbook-xml-4.4
#                 docbook-xml-4.5
#                 docbook-xml-5.0
#               docbook-xsl
#               rarian
#                 getopt
#               iso-codes
#         py27-gobject
#       py27-gdbm
#       py27-pygtk
#         py27-cairo
#           py27-numpy
#             py27-nose
#               py27-distribute
#               nosetests_select
#     qca
#     dbusmenu-qt
#       qjson
#     grantlee
#     shared-desktop-ontologies
#   kactivities
#   oxygen-icons

################
#   R-framework
################
#   pkgconfig
#     libiconv
#       gperf
#   readline
#     ncurses
#   icu
#   xorg-libsm
#     xorg-xtrans
#     xorg-libice
#       xorg-xproto
#   xorg-libX11
#     xorg-bigreqsproto
#     xorg-xcmiscproto
#     xorg-xextproto
#     xorg-xf86bigfontproto
#     xorg-inputproto
#     xorg-libXdmcp
#     xorg-libXau
#     xorg-libxcb
#       xorg-xcb-proto
#         libxml2
#           zlib
#           xz
#             gettext
#               expat
#         python27
#           openssl
#           sqlite3
#             libedit
#           db46
#           bzip2
#           python_select
#       xorg-libpthread-stubs
#     xorg-kbproto
#   xorg-libXt
#   tiff
#     jpeg
#   libpng
#   cairo
#     libpixman
#     glib2
#       libffi
#       perl5
#         perl5.12
#           gdbm
#     fontconfig
#       freetype
#     lzo2
#     xrender
#       xorg-renderproto
#     xorg-libXext
#       xorg-util-macros
#       autoconf
#         m4
#         help2man
#           p5.12-locale-gettext
#       automake
#       libtool
#     xorg-xcb-util
#   pango
#     gobject-introspection
#     harfbuzz
#       graphite2
#         cmake
#           libidn
#     Xft2
#   gcc45
#     gmp
#     mpfr
#     libmpc
#     ppl
#       glpk
#     gcc_select
#     ld64
#       libunwind-headers
#       dyld-headers
#       cctools-headers
#       llvm-3.2
#         llvm_select
#     cctools
#     libstdcxx
#       cloog
#         isl
