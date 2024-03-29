#!/bin/bash
# This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#
# roxygenize rkward package _or_ the packages specified on the command line

cd `dirname $0`/..
BASEDIR=`pwd`
PACKAGESDIR="${BASEDIR}/rkward/rbackend/rpackages"


if [ "$#" = 0 ]; then
	PACKAGES="'${PACKAGESDIR}/rkward/', '${PACKAGESDIR}/rkwardtests/'"
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
	library (devtools)
	packages <- c ($PACKAGES)
	for (package in packages) {
		#dummy <- roxygen2:::source_package (package) # See https://github.com/klutometis/roxygen/issues/167
		document (package)
#		roxygenize (package, load_code=load_source)
		possibly_empty_dirs <- paste (package, c ('inst/doc', 'inst'), sep='/')
		for (dir in possibly_empty_dirs) {
			if (file.exists (dir)) suppressWarnings (try (file.remove (dir)))
		}
	}
	print (warnings())
" | /usr/bin/R --no-save
