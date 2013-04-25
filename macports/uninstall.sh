#!/bin/bash

if [[ $1 == "" ]] ; then
  echo "Usage: uninstall.sh OPTIONS
          OPTIONS:

           -l list all files and directories to be removed
           -R actually remove all files and directories (if empty)
"
  exit 0
fi

# get the options
while getopts ":lR" OPT; do
  case $OPT in
    l) LIST=TRUE >&2 ;;
    R) REMOVE=TRUE >&2 ;;
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


# fetch a list of all installed MacPorts packages
ALLINSTPORTS=$(pkgutil --pkgs=org.macports.*)

# treat only those who are associated with /opt/rkward
for i in ${ALLINSTPORTS} ; do
  if [[ $(pkgutil --only-dirs --files "$i" | grep "opt/rkward/") != "" ]] ; then
    if [ ${LIST} ] ; then
      pkgutil --files "$i" | sed -e "s#^#/#g"
    fi
    if [ ${REMOVE} ] ; then
      # remove files
      sudo rm $(pkgutil --only-files --files "$i" | sed -e "s#^#/#g") || exit 1
      # remove directories if empty
      sudo rmdir -p $(pkgutil --only-dirs --files "$i" | sed -e "s#^#/#g")
      # forget about the package
      sudo pkgutil --forget "$i"
    fi
  fi
done

exit 0
