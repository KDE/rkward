## usage:
#  rk.write.about(about=about.plugins)
#
#  for sample data see below function definition
#
## function rk.write.about()
# this is the main function. it needs a list with the about information
# in the following format (fields with "!" are mandatory):
# about <- list(
# 	name="",               # ! name of the plugin
# 	desc="",               # ! a short description
# 	version="",            # ! version number
# 	date=Sys.Date(),       # ! release date
# 	url="",                #   some web address
# 	license="GPL",         # ! software license
# 	category="",           #   an optional category
# 	authors=list(          # ! one or more authors (at least one name and mail address)
# 			c(name="", email="", url=""),
# 			c(name="", email="", url="")
# 		),
# 	rkward.min="0.5.3",    #   minimum version number of RKWard
# 	rkward.max="",         #   maximum version number of RKWard
# 	R.min="2.10",          #   minimum version number of R
# 	R.max="",              #   maximum version number of R
# 	depends=list(          #   if the plugin needs other packages or pluginmaps (at least the names)
# 			c(package="", min="", max="", repository=""),
# 			c(pluginmap="", url="")
# 		)
# )
rk.write.about <- function(about, file=NULL, level=1){
	# sanity checks
	stopifnot(all(c("name", "desc", "version", "date", "license", "authors") %in% names(about)))
	stopifnot(all(length(about[c("name", "desc", "version", "date", "license", "authors")]) > 0))

	# a list with element names and their attribute names in XML
	about.e2t <- list(
		about=list(
			name="name",
			desc="shortinfo",
			version="version",
			date="releasedate",
			url="url",
			license="license",
			category="category"
		),
		author=list(
			name="name",
			email="email",
			url="url"
		),
		dependencies=list(
			rkward.min="rkward_min_version",
			rkward.max="rkward_max_version",
			R.min="R_min_verion",
			R.max="R_max_verion"
		),
		package=list(
			package="name",
			min="min_version",
			max="max_version",
			repository="repository"
		),
		pluginmap=list(
			pluginmap="name",
			url="url"
		)
	)

	# function lookupAttrName()
	# takes the original input element names and returns
	# the according XML attribute name
	lookupAttrName <- function(tag, attr){
		if(is.null(tag)){
			attr.name <- attr
		} else {
			attr.name <- about.e2t[[tag]][[attr]]
		}
		return(attr.name)
	} # end function lookupAttrName()

	## here the actual pasting starts
	# create children from all authors. (SCNR...)
	all.authors <- c()
	for (author in about[["authors"]]){
		all.authors <- paste(all.authors, pasteXMLTag("author", author, solo=TRUE, level=level+1), sep="")
	}
	# create children from all package and pluginmap dependencies
	all.package.deps <- c()
	if("depends" %in% names(about)){
		for (package in about[["depends"]]){
			if("package" %in% names(package)){
				all.package.deps <- paste(all.package.deps, pasteXMLTag("package", package, solo=TRUE, level=level+2), sep="")
			} else if("pluginmap" %in% names(package)){
				all.package.deps <- paste(all.package.deps, pasteXMLTag("pluginmap", package, solo=TRUE, level=level+2), sep="")
			}
		}
	} else {}
	# create dependencies
	dep.names <- names(about)[names(about) %in% c("rkward.min", "rkward.max", "R.min", "R.max")]
	if(length(dep.names) > 0){
		dependencies <- pasteXMLTag("dependencies", about[dep.names], child=all.package.deps, solo=FALSE, level=level+1)
	} else {
		dependencies <- ""
	}

	# combine all children for the about root tag
	all.children <- paste(all.authors, dependencies, sep="")

	# finally, put it all together
	about.names <- names(about)[names(about) %in% c("name", "desc", "version", "date", "url", "license", "category")]
	all.about <- pasteXMLTag("about", about[about.names], child=all.children, solo=FALSE, level=level)

	if(!is.null(file)){
		write(all.about, file=file)
		return(invisible(NULL))
	} else {
		return(all.about)
	}
} ## end function rk.write.about()

## sample input data:
# about.plugins <- list(
# 	name="Square the circle",
# 	desc="Squares the circle using Heisenberg compensation.",
# 	version="0.1-3",
# 	date=Sys.Date(),
# 	url="http://eternalwondermaths.example.org/23/stc.html",
# 	license="GPL",
# 	category="Geometry",
# 	authors=list(
# 			c(name="E.A. Dölle", email="doelle@eternalwondermaths.example.org", url="http://eternalwondermaths.example.org"),
# 			c(name="A. Assistant", email="alterego@eternalwondermaths.example.org", url="http://eternalwondermaths.example.org/staff/")
# 		),
# 	rkward.min="0.5.3",
# 	rkward.max="",
# 	R.min="2.10",
# 	R.max="",
# 	depends=list(
# 			c(package="heisenberg", min="0.11-2", max="", repository="http://rforge.r-project.org"),
# 			c(package="DreamsOfPi", min="0.2", max="", repository=""),
# 			c(pluginmap="", url="")
# 		)
# )
## a sample with less data
# about.plugins2 <- list(
# 	name="Square the circle",
# 	desc="Squares the circle using Heisenberg compensation.",
# 	version="0.1-3",
# 	date=Sys.Date(),
# 	license="GPL",
# 	authors=list(c(name="E.A. Dölle"))
# )

## additional functions called by the main function:

## function indent()
# will create tabs to format the output
indent <- function(level){
	paste(rep("\t", level-1), collapse="")
} ## end function indent()

## function pasteXMLAttr()
# pastes all attributes in a nicely readable way
pasteXMLAttr <- function(attr=NULL, tag=NULL, level=1){
	if(is.null(attr)){
		return("")
	} else {}
	# only use formatting if more than one attribute
	if(length(attr) > 1){
		full.attr <- c()
		for (this.attr in names(attr)){
			# skip empty elements
			if(is.null(attr[[this.attr]])){next}
			if(nchar(attr[[this.attr]]) > 0){
				# look up attribute name to paste
				attr.name <- lookupAttrName(tag, this.attr)
				full.attr <- paste(full.attr, "\n", indent(level+1), attr.name, "=\"", attr[[this.attr]], "\"", sep="")
			} else {}
		}
	} else {
		# look up attribute name to paste
		attr.name <- lookupAttrName(tag, names(attr))
		full.attr <- paste(attr.name, "=\"", attr[[1]], "\"", sep="")
	}
	return(full.attr)
} ## end function pasteXMLAttr()

## function pasteXMLTag()
# creates a whole XML tag with attributes and, if it is a pair of start and end tags,
# also one object as child.
# - tag: name of the tag
# - attr: a list of attributes for the tag
# - child: if 'solo=FALSE', a character string to be pasted als a child node between start and end tag
# - solo: <true /> or <false></false>
# - level: indentation level
# - allow.empty: if FALSE, tags without attributes will not be returned
pasteXMLTag <- function(tag, attr=NULL, child=NULL, solo=TRUE, level=1, allow.empty=FALSE){
	# what attributes do we have?
	all.attributes <- pasteXMLAttr(attr, tag=tag, level=level)
	# probaly don't produce empty tags
	if(!isTRUE(allow.empty) & is.null(all.attributes)){
		return("")
	} else {}
	# solo decides whether this is a solo tag or a pair of start and end tags
	if(isTRUE(solo)){
		full.tag <- paste(indent(level), "<", tag, " ", pasteXMLAttr(attr, tag=tag, level=level), "\n", indent(level), "/>\n", sep="")
	} else {
		full.tag <- paste(
			indent(level), "<", tag, " ", pasteXMLAttr(attr, tag=tag, level=level), ">\n",
			if(!is.null(child)){child},
			indent(level), "</", tag, ">\n", sep="")
	}
	return(full.tag)
} ## end function pasteXMLTag()
