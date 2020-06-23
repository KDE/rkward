#!/bin/bash
# roxygenize rkward package _or_ the packages specified on the command line

cd `dirname $0`/..
BASEDIR=`pwd`
PACKAGESDIR="${BASEDIR}/rkward/rbackend/rpackages"


if [ "$#" = 0 ]; then
	PACKAGES="'${PACKAGESDIR}/rkward/'"
	# currently excluded due to missing support for slots in roxygen2:
	# '${PACKAGESDIR}/rkward/tests/'
else
	PACKAGES="'$1'"
	shift
fi

if [ ! -f "${PACKAGESDIR}/rkward/R/ver.R" ]; then
	echo "ver.R does not exist in rkward package. Run cmake, first."
	exit 1
fi

while (( "$#" )); do
	PACKAGES="${PACKAGES}, '$1'"
	shift
done

echo "
	library (roxygen2)
	packages <- c ($PACKAGES)
	for (package in packages) {
#		dummy <- roxygen2:::source_package (package) # See https://github.com/klutometis/roxygen/issues/167
		roxygenize (package)
		possibly_empty_dirs <- paste (package, c ('inst/doc', 'inst'), sep='/')
		for (dir in possibly_empty_dirs) {
			if (file.exists (dir)) suppressWarnings (try (file.remove (dir)))
		}
	}
	print (warnings())
" | /usr/bin/R --no-save
