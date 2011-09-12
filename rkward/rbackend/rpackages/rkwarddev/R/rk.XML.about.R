#' Create XML node "about" for RKWard pluginmaps
#'
#' @param about A named list with these elements:
#'		\describe{
#'			\item{name}{Plugin name}
#'			\item{desc}{A short description}
#'			\item{version}{Plugin version}
#'			\item{date}{Release date}
#'			\item{url}{URL for the plugin}
#'			\item{license}{License the plugin is distributed under}
#'			\item{category}{An optional category}
#'		}
#' @param author A list of named character vectors with these elements:
#'		\describe{
#'			\item{name}{Full author name}
#'			\item{email}{Author mail address}
#'			\item{url}{Author homepage}
#'		}
#' @param dependencies A named list with these elements:
#'		\describe{
#'			\item{rkward.min}{Minimum RKWard version needed for this plugin}
#'			\item{rkward.max}{Maximum RKWard version needed for this plugin}
#'			\item{R.min}{Minimum R version needed for this plugin}
#'			\item{R.max}{Maximum R version needed for this plugin}
#'		}
#' @param package A list of named character vectors, each with these elements:
#'		\describe{
#'			\item{name}{Name of a package this plugin depends on}
#'			\item{min}{Minimum version of the package}
#'			\item{max}{Maximum version of the package}
#'			\item{repository}{Repository to download the package}
#'		}
#' @param pluginmap A named list with these elements:
#'		\describe{
#'			\item{name}{Identifier of a pluginmap this plugin depends on}
#'			\item{url}{URL to get the pluginmap}
#'		}
#' @export
#' @examples
#' about.node <- rk.XML.about(
#' 	about=list(
#' 		name="Square the circle",
#' 		desc="Squares the circle using Heisenberg compensation.",
#' 		version="0.1-3",
#' 		date=Sys.Date(),
#' 		url="http://eternalwondermaths.example.org/23/stc.html",
#' 		license="GPL",
#' 		category="Geometry"),
#' 	author=list(
#' 		c(name="E.A. Dölle", email="doelle@@eternalwondermaths.example.org",
#' 			url="http://eternalwondermaths.example.org"),
#' 		c(name="A. Assistant", email="alterego@@eternalwondermaths.example.org",
#' 			url="http://eternalwondermaths.example.org/staff/")
#' 		),
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
#' cat(pasteXMLNode(about.node))


rk.XML.about <- function(about, author, dependencies=NULL, package=NULL, pluginmap=NULL){
	# sanity checks
	stopifnot(all(c("name", "desc", "version", "date", "license") %in% names(about)))
	stopifnot(all(length(about[c("name", "desc", "version", "date", "license")]) > 0))

	## author
	# - name
	# - email
	# - url
	xml.authors <- unlist(sapply(author, function(this.author){
			author.name  <- this.author[["name"]]
			author.email <- this.author[["email"]]
			author.url   <- this.author[["url"]]
			result <- new("XiMpLe.node",
				name="author",
				attributes=list(name=author.name, email=author.email, url=author.url))
			return(result)
		}))

	## package
	# - name
	# - min="min_version",
	# - max="max_version",
	# - repository
	xml.package <- sapply(package, function(this.package){
			result <- new("XiMpLe.node",
				name="package",
				attributes=list(
					name=this.package[["name"]],
					"min_version"=this.package[["min"]],
					"max_version"=this.package[["max"]],
					repository=this.package[["repository"]]
				))
			return(result)
		})

	## pluginmap
	# - name,
	# - url
	xml.pluginmap <- sapply(pluginmap, function(this.pluginmap){
			result <- new("XiMpLe.node",
				name="pluginmap",
				attributes=list(
					name=this.pluginmap[["name"]],
					url=this.pluginmap[["url"]]
				))
			return(result)
		})

	## dependencies
	# - rkward.min="rkward_min_version",
	# - rkward.max="rkward_max_version",
	# - R.min="R_min_verion",
	# - R.max="R_max_verion"
	# + package
	# + pluginmap
	if(!is.null(xml.pluginmap)){
		for (pmap in xml.pluginmap){
			xml.package[[length(xml.package)+1]] <- pmap
		}
	} else {}
	if(is.null(xml.package)){
		xml.package <- list()
	} else {}
	xml.dependencies <-  new("XiMpLe.node",
				name="dependencies",
				attributes=list(
					"rkward_min_version"=dependencies[["rkward.min"]],
					"rkward_max_version"=dependencies[["rkward.max"]],
					"R_min_verion"=dependencies[["R.min"]],
					"R_max_verion"=dependencies[["R.max"]]
				),
				children=xml.package,
				value=""
		)
	# skip dependency listing if it has no entries
	if(all(sapply(xml.dependencies@attributes, is.null)) & length(xml.dependencies@children) == 0){
		xml.dependencies <- NULL
	} else {}

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
	if(!is.null(xml.dependencies)){
		xml.authors[[length(xml.authors)+1]] <- xml.dependencies
	} else {}
	if(is.null(xml.authors)){
		xml.authors <- list()
	} else {}
	xml.about <-  new("XiMpLe.node",
				name="about",
				attributes=list(
					name=about[["name"]],
					"shortinfo"=about[["desc"]],
					version=about[["version"]],
					"releasedate"=about[["date"]],
					url=about[["url"]],
					license=about[["license"]],
					category=about[["category"]]
				),
				children=xml.authors
		)

	return(xml.about)
}
