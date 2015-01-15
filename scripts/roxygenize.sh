#!/bin/bash

cd `dirname $0`/..
BASEDIR=`pwd`

PACKAGES="'${BASEDIR}/rkward/rbackend/rpackages/rkward/'"
         # currently excluded due to missing support for slots in roxygen2:
         # '${BASEDIR}/rkward/rbackend/rpackages/rkwardtests/'
if [ "$1" = "--all" ]; then
PACKAGES="${PACKAGES}, \
          '${BASEDIR}/packages/XiMpLe/',
          '${BASEDIR}/packages/rkwarddev/'"
fi

echo "
	library (roxygen2)
	packages <- c ($PACKAGES)
	for (package in packages) {
		dummy <- roxygen2:::source_package (package) # See https://github.com/klutometis/roxygen/issues/167
		roxygenize (package)
		possibly_empty_dirs <- paste (package, c ('inst/doc', 'inst'), sep='/')
		for (dir in possibly_empty_dirs) {
			if (file.exists (dir)) suppressWarnings (try (file.remove (dir)))
		}
	}
	print (warnings())
" | /usr/bin/R --no-save
