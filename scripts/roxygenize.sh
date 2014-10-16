#!/bin/bash

cd `dirname $0`/..
BASEDIR=`pwd`

echo "
	library (roxygen2)
	packages <- c ( '${BASEDIR}/rkward/rbackend/rpackages/rkward/',
					# '${BASEDIR}/rkward/rbackend/rpackages/rkwardtests/', # currently excluded due to missing support for slots in roxygen2
					'${BASEDIR}/packages/XiMpLe/',
					'${BASEDIR}/packages/rkwarddev/'
					)
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
