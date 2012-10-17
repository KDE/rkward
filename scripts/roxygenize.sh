#!/bin/bash

cd `dirname $0`/..
BASEDIR=`pwd`

echo "
	library (roxygen2)
	packages <- c ( '${BASEDIR}/rkward/rbackend/rpackages/rkward/',
					# '${BASEDIR}/rkward/rbackend/rpackages/rkwardtests/', # currently excluded due to missing support for slots in roxygen2
					'${BASEDIR}/packages/rkwarddev/',
					'${BASEDIR}/packages/XiMpLe/')
	for (package in packages) {
		roxygenize (package)
		possibly_empty_dirs <- paste (package, c ('inst/doc', 'inst'), sep='/')
		for (dir in possibly_empty_dirs) {
			if (file.exists (dir)) try (file.remove (dir))
		}
	}
	print (warnings())
" | /usr/bin/R --no-save
