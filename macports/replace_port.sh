#!/bin/bash

# specify macports installation path
MPTINST=/opt/rkward
# specify application dir used
APPLDIR=/Applications/RKWard
# specify deactivation suffix
DCTSFFX="_works"

DEACT=false
REACT=false
RMDCT=false

TXT_DGRAY="\033[1;30m"
TXT_LRED="\033[1;31m"
TXT_RED="\033[0;31m"
TXT_BLUE="\033[0;34m"
TXT_GREEN="\033[0;32m"
TXT_BOLD="\033[1m"
TXT_ITALIC="\033[3m"
TXT_UNDERSCORE="\033[4m"
TXT_ORANGE_ON_GREY="\033[48;5;240;38;5;202m"
OFF="\033[0m"

if [[ $1 == "" ]] ; then
  echo -e "Usage: replace_port.sh OPTION
          OPTIONS:
           -d (deactivate running RKWard installation)
           -a (re-activate previous RKWard installation)
           -x (!remove! the deactivated RKWard installation)\n"
  exit 0
fi

# get the options
while getopts ":dax" OPT; do
  case $OPT in
    d) DEACT=true >&2 ;;
    a) REACT=true >&2 ;;
    x) RMDCT=true >&2 ;;
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

error() {
  # $1: message to print
  echo -e "${TXT_RED}ouch:${OFF} $1"
  exit 1
}

warning() {
  # $1: message to print
  echo -e "${TXT_ORANGE_ON_GREY}warning:${OFF} $1"
}

alldone() {
  echo -e " ${TXT_GREEN}done! ${OFF}"
}

if $DEACT ; then
  if [ -d "${MPTINST}${DCTSFFX}" ] ; then
    error "${TXT_BLUE}\"${MPTINST}${DCTSFFX}\"${OFF} already exists, aborting!"
  fi
  if [ -d "${APPLDIR}${DCTSFFX}" ] ; then
    error "${TXT_BLUE}\"${APPLDIR}${DCTSFFX}\"${OFF} already exists, aborting!"
  fi
  echo -en "renaming ${TXT_BLUE}${MPTINST}${OFF} into ${TXT_BLUE}${MPTINST}${DCTSFFX}${OFF}..."
  sudo mv "${MPTINST}" "${MPTINST}${DCTSFFX}" || error "failed!"
  alldone
  echo -en "renaming ${TXT_BLUE}${APPLDIR}${OFF} into ${TXT_BLUE}${APPLDIR}${DCTSFFX}${OFF}..."
  sudo mv "${APPLDIR}" "${APPLDIR}${DCTSFFX}" || error "failed!"
  alldone
fi

if $REACT ; then
  if ! [ -d "${MPTINST}${DCTSFFX}" ] ; then
    error "${TXT_BLUE}\"${MPTINST}${DCTSFFX}\"${OFF} doesn't exist, aborting!"
  else
    echo -en "removing ${TXT_BLUE}${MPTINST}${OFF}..."
    sudo rm -rf "${MPTINST}" || error "failed!"
    alldone
    echo -en "renaming ${TXT_BLUE}${MPTINST}${DCTSFFX}${OFF} into ${TXT_BLUE}${MPTINST}${OFF}..."
    sudo mv "${MPTINST}${DCTSFFX}" "${MPTINST}" || error "failed!"
    alldone
  fi
  if ! [ -d "${APPLDIR}${DCTSFFX}" ] ; then
    error "${TXT_BLUE}\"${APPLDIR}${DCTSFFX}\"${OFF} doesn't exist, aborting!"
  else
    echo -en "removing ${TXT_BLUE}${APPLDIR}${OFF}..."
    sudo rm -rf "${APPLDIR}" || error "failed!"
    alldone
    echo -en "renaming ${TXT_BLUE}${APPLDIR}${DCTSFFX}${OFF} into ${TXT_BLUE}${APPLDIR}${OFF}..."
    sudo mv "${APPLDIR}${DCTSFFX}" "${APPLDIR}" || error "failed!"
    alldone
  fi
fi

exit 0
