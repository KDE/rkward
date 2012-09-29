## README
# this R script can be used to let roxyPackage maintain the rkward package sources
# to use it
#  1. install the package "roxyPackage"
#  2. make a local copy of this file
#  3. checkout the lates rkward package sources via svn
#  4. cusomize the first part of the script (see below, you MUST set these paths correctly!)
#  5. run the script

## setup environment
# .libPaths("~/R")
require(roxyPackage)

local({
	#######################
	## these are the values you probably need to update
	#######################
	package.version <- "0.6.0"

	# this should point to the "rkward" root directory of the R package
	package.sources <- file.path("/PathTo","trunk","rkward","rkward","rbackend","rpackages","rkward")
	# path to R libs -- where should the package be installed to?
	install.path <- file.path("~", "R")
	# packages are copied to a "local repository", that is simply a folder
	# to create some files and directories. it can actually be used as a
	# R repository
	local.repository <- file.path("/PathTo", "repo_rkward")

	# what should be done? (un)comment actions as needed
	roxyPackage.actions <- c(
	## documentation:
		# "roxy" is needed also to update NAMESPACE; but beware that
		# it will overwrite the docs, so be careful with svn commits
		# until the docs are fully tagged!
				"roxy",			# roxygenize the docs
				"cite",			# update CITATION file
	# 			"doc",			# update pdf documentation
	# 			"cl2news",		# convert ChangeLog into NEWS.Rd
	## local repository:
	# 			"news2rss",		# convert NEWS.Rd into RSS feed
	# 			"html",			# update index.html
	# 			"win",			# update the windows binary package
	# 			"macosx",		# update the mac OS X binary package
	## build:
				"package" 		# build & install the package
	# 			"check"			# check package
	)

	#######################
	## from here on all should be left untouched
	#######################
	# package description
	package.description <-data.frame(
		Package="rkward",
		Type="Package",
		Title="Provides functions related to the RKWard GUI",
		Author="Thomas Friedrichsmeier <thomas.friedrichsmeier@ruhr-uni-bochum.de>, with contributions from the RKWard Team",
		AuthorsR="c(person(given=\"Thomas\", family=\"Friedrichsmeier\", email=\"thomas.friedrichsmeier@ruhr-uni-bochum.de\",
				role=c(\"aut\")),
			person(given=\"RKWard-devel\", family=\"mailing list\", email=\"rkward-devel@lists.sourceforge.net\",
				role=c(\"cre\",\"ctb\")))",
		Maintainer="RKWard-devel mailing list <rkward-devel@lists.sourceforge.net>",
		## TODO: check dependencies
		Depends="R (>= 2.9.0),methods",
		Description="This package contains functions which are useful in combination with the RKWard GUI. Many of these
			functions only needed for the internal communication between RKWard and R, but some are also useful in user scripts.",
		License="GPL (>= 2)",
		Encoding="UTF-8",
		LazyLoad="yes",
		URL="http://rkward.sourceforge.net",
		stringsAsFactors=FALSE)

	## here we go
	roxy.package(actions=roxyPackage.actions,
		pck.source.dir=package.sources,
		pck.version=package.version,
		pck.description=package.description,
		R.libs=install.path,
		repo.root=local.repository,
		cleanup=TRUE,
		URL="http://rkward.sourceforge.net")
})
