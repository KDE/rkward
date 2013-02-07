#' Create XML node "about" for RKWard pluginmaps
#'
#' @param name A character string with the plugin name.
#' @param author A vector of objects of class \code{person} with these elements (mandatory):
#'		\describe{
#'			\item{given}{Author given name}
#'			\item{family}{Author family name}
#'			\item{email}{Author mail address}
#'			\item{role}{This person's specific role, e.g. \code{"aut"} for actual author, \code{"cre"} for maintainer or \code{"ctb"} for contributor.}
#'		}
#'		See \code{\link[utils:person]{person}} for more details on this, especially for valid roles.
#' @param about A named list with these elements:
#'		\describe{
#'			\item{desc}{A short description (mandatory)}
#'			\item{version}{Plugin version (mandatory)}
#'			\item{date}{Release date (mandatory)}
#'			\item{url}{URL for the plugin (optional)}
#'			\item{license}{License the plugin is distributed under (mandatory)}
#'			\item{category}{A category for this plugin (optional)}
#'			\item{long.desc}{A long description (optional, defaults to \code{desc})}
#'		}
#' @param dependencies A named list with these elements:
#'		\describe{
#'			\item{rkward.min}{Minimum RKWard version needed for this plugin (optional)}
#'			\item{rkward.max}{Maximum RKWard version needed for this plugin (optional)}
#'			\item{R.min}{Minimum R version needed for this plugin (optional)}
#'			\item{R.max}{Maximum R version needed for this plugin (optional)}
#'		}
#' @param package A list of named character vectors, each with these elements:
#'		\describe{
#'			\item{name}{Name of a package this plugin depends on (optional)}
#'			\item{min}{Minimum version of the package (optional)}
#'			\item{max}{Maximum version of the package (optional)}
#'			\item{repository}{Repository to download the package (optional)}
#'		}
#' @param pluginmap A named list with these elements:
#'		\describe{
#'			\item{name}{Identifier of a pluginmap this plugin depends on (optional)}
#'			\item{url}{URL to get the pluginmap (optional)}
#'		}
#' @export
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' about.node <- rk.XML.about(
#' 	name="Square the circle",
#' 	author=c(
#' 		person(given="E.A.", family="Dölle",
#' 			email="doelle@@eternalwondermaths.example.org", role="aut"),
#' 		person(given="A.", family="Assistant",
#' 			email="alterego@@eternalwondermaths.example.org", role=c("cre","ctb"))
#' 		),
#' 	about=list(
#' 		desc="Squares the circle using Heisenberg compensation.",
#' 		version="0.1-3",
#' 		date=Sys.Date(),
#' 		url="http://eternalwondermaths.example.org/23/stc.html",
#' 		license="GPL",
#' 		category="Geometry"),
#' 	dependencies=list(
#' 		rkward.min="0.5.3",
#' 		rkward.max="",
#' 		R.min="2.10",
#' 		R.max=""),
#' 	package=list(
#' 		c(name="heisenberg", min="0.11-2", max="",
#' 			repository="http://rforge.r-project.org"),
#' 		c(name="DreamsOfPi", min="0.2", max="", repository="")),
#' 	pluginmap=list(
#' 		c(name="heisenberg.pluginmap", url="http://eternalwondermaths.example.org/hsb"))
#' )
#' 
#' cat(pasteXML(about.node, shine=2))


rk.XML.about <- function(name, author, about=list(desc="SHORT_DESCRIPTION", version="0.01-0", date=Sys.Date(), url="http://EXAMPLE.com", license="GPL (>= 3)", long.desc=NULL), dependencies=NULL, package=NULL, pluginmap=NULL){
	# sanity checks
	stopifnot(all(length(name), length(author)) > 0)
	if(is.null(about)){
		about <- list()
	} else {}
	if(!"desc" %in% names(about)){
		about[["desc"]] <- "SHORT_DESCRIPTION"
	} else {}
	if(!"version" %in% names(about)){
		about[["version"]] <- "0.01-0"
	} else {}
	if(!"date" %in% names(about)){
		about[["date"]] <- Sys.Date()
	} else {}
	if(!"url" %in% names(about)){
		about[["url"]] <- "http://EXAMPLE.com"
	} else {}
	if(!"license" %in% names(about)){
		about[["license"]] <- "GPL (>= 3)"
	} else {}
	if(!"long.desc" %in% names(about) | is.null(about[["long.desc"]])){
		about[["long.desc"]] <- about[["desc"]]
	} else {}

	## author
	# - given
	# - family
	# - email
	# - role
	xml.authors <- unlist(sapply(author, function(this.author){
			stopifnot(all(c("given", "family", "email") %in% names(unlist(this.author))))
			author.given  <- format(this.author, include="given")
			author.family <- format(this.author, include="family")
			author.email  <- format(this.author, include="email", braces=list(email=""))
			author.role   <- format(this.author, include="role", braces=list(role=""), collapse=list(role=", "))
			result <- XMLNode("author",
				attrs=list(
					given=author.given,
					family=author.family,
					email=author.email,
					role=author.role
				))
			return(result)
		}))

	###################
	## the 'package' code is about to be removed in a future release
	###################
		## package
		# - name
		# - min="min_version",
		# - max="max_version",
		# - repository
		# create example, if empty
		if(is.null(package)){
			xml.package <- list()
		} else {
			warning("<package> inside <about> is deprecated, use rk.XML.dependencies() instead!")
			xml.package <- sapply(package, function(this.package){
					pck.options <- names(this.package)
					pck.attributes <- list(name=this.package[["name"]])
					for (this.option in c("min", "max","repository" )){
						if(this.option %in% pck.options){
							pck.attributes[[this.option]] <- this.package[[this.option]]
						} else {}
					}
					result <- XMLNode("package", attrs=pck.attributes)
					return(result)
				})
		}

	###################
	## the 'pluginmap' code is about to be removed in a future release
	###################
		## pluginmap
		# - name,
		# - url
		# create example, if empty
		if(is.null(pluginmap)){
			xml.pluginmap <- list()
		} else {
			warning("<pluginmap> inside <about> is deprecated, use rk.XML.dependencies() instead!")
			xml.pluginmap <- sapply(pluginmap, function(this.pluginmap){
					result <- XMLNode("pluginmap",
						attrs=list(
							name=this.pluginmap[["name"]],
							url=this.pluginmap[["url"]]
						))
					return(result)
				})
		}

	###################
	## the 'dependencies' code is about to be removed in a future release
	###################
		## dependencies
		# - rkward.min="rkward_min_version",
		# - rkward.max="rkward_max_version",
		# - R.min="R_min_version",
		# - R.max="R_max_version"
		# + package
		# + pluginmap
		for (pmap in xml.pluginmap){
			xml.package[[length(xml.package)+1]] <- pmap
		}
		if(is.null(dependencies)){
			xml.dependencies <- list()
		} else {
			warning("<dependencies> inside <about> is deprecated, use rk.XML.dependencies() instead!")
			dep.options <- names(dependencies)
			dep.attributes <- list()
			if("rkward.min" %in% dep.options){
				dep.attributes[["rkward_min_version"]] <- dependencies[["rkward.min"]]
			} else {}
			if("rkward.max" %in% dep.options){
				dep.attributes[["rkward_max_version"]] <- dependencies[["rkward.max"]]
			} else {}
			if("R.min" %in% dep.options){
				dep.attributes[["R_min_version"]] <- dependencies[["R.min"]]
			} else {}
			if("R.max" %in% dep.options){
				dep.attributes[["R_max_version"]] <- dependencies[["R.max"]]
			} else {}
			xml.dependencies <- XMLNode("dependencies",
				attrs=dep.attributes,
				.children=child.list(xml.package, empty=FALSE))
		}

	## about
	# - name
	# - desc="shortinfo",
	# - version
	# - date="releasedate",
	# - url
	# - license
	# - category
	# + authors
	# + dependencies
	xml.authors[[length(xml.authors)+1]] <- xml.dependencies
	if(is.null(xml.authors)){
		xml.authors <- list()
	} else {}
	xml.about <-  XMLNode("about",
		attrs=list(
			name=name,
			"shortinfo"=about[["desc"]],
			"longinfo"=about[["long.desc"]],
			version=about[["version"]],
			"releasedate"=about[["date"]],
			url=about[["url"]],
			license=about[["license"]],
			category=about[["category"]]
		),
		.children=xml.authors)

	return(xml.about)
}
