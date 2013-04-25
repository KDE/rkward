#!/bin/bash

# specify macports installation path
MPTINST=/opt/rkward
# specify application dir used
APPLDIR=/Applications/RKWard
# specify path to make
MAKEPATH=/usr/bin/make
# specify deactivation suffix
DCTSFFX="_works"

if [[ $1 == "" ]] ; then
 echo "Usage: replace_port.sh OPTION
          OPTIONS:
           -d (deactivate running RKWard installation)
           -a (re-activate previous RKWard installation)
           -m (also include ${MAKEPATH} in the process)
           -x (!remove! the deactivated RKWard installation)
"
fi

# get the options
while getopts ":damx" OPT; do
  case $OPT in
    d) DEACT=TRUE >&2 ;;
    a) REACT=TRUE >&2 ;;
    m) IMAKE=TRUE >&2 ;;
    x) RMDCT=TRUE >&2 ;;
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

if [[ $DEACT ]] ; then
	if [ -d "${MPTINST}${DCTSFFX}" ] ; then
		echo "ouch: \"${MPTINST}${DCTSFFX}\" already exists, aborting!"
		exit 1
	fi
	if [ -d "${APPLDIR}${DCTSFFX}" ] ; then
		echo "ouch: \"${APPLDIR}${DCTSFFX}\" already exists, aborting!"
		exit 1
	fi
	sudo mv "${MPTINST}" "${MPTINST}${DCTSFFX}"
	sudo mv "${APPLDIR}" "${APPLDIR}${DCTSFFX}"
	if [[ $IMAKE ]] && [ -f "${MAKEPATH}${DCTSFFX}" ] ; then
		echo "ouch: \"${MAKEPATH}${DCTSFFX}\" already exists, skipping!"
	else
		sudo mv "${MAKEPATH}" "${MAKEPATH}${DCTSFFX}"
	fi
fi

if [[ $REACT ]] ; then
	if [[ $IMAKE ]] && ! [ -f "${MAKEPATH}${DCTSFFX}" ] ; then
		echo "ouch: \"${MAKEPATH}${DCTSFFX}\" doesn't exist, skipping!"
	else
		sudo mv "${MAKEPATH}${DCTSFFX}" "${MAKEPATH}"
	fi
	if ! [ -d "${MPTINST}${DCTSFFX}" ] ; then
		echo "ouch: \"${MPTINST}${DCTSFFX}\" doesn't exist, aborting!"
		exit 1
	else
		sudo rm -rf "${MPTINST}"
		sudo mv "${MPTINST}${DCTSFFX}" "${MPTINST}"
	fi
	if ! [ -d "${APPLDIR}${DCTSFFX}" ] ; then
		echo "ouch: \"${APPLDIR}${DCTSFFX}\" doesn't exist, aborting!"
		exit 1
	else
		sudo rm -rf "${APPLDIR}"
		sudo mv "${APPLDIR}${DCTSFFX}" "${APPLDIR}"
	fi
fi

exit 0
