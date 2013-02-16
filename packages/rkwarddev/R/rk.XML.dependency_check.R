#' Create XML node "dependency_check" for RKWard pluginmaps
#'
#' @param id.name Character string, a unique ID for this plugin element.
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
#' @param hints Logical, if \code{TRUE}, \code{NULL} values will be replaced with example text.
#' @export
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' dependency_check.node <- rk.XML.dependency_check(
#' 	id.name="dep_check",
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

rk.XML.dependency_check <- function(id.name, dependencies=NULL, package=NULL, pluginmap=NULL, hints=FALSE){
	## package
	# - name
	# - min="min_version",
	# - max="max_version",
	# - repository
	# create example, if empty
	if(is.null(package)){
		if(isTRUE(hints)){
			xml.package.example <- XMLNode("package",
				attrs=list(
					name="CHANGE_ME_OR_DELETE_ME",
					"min_version"="CHANGE_ME_OR_DELETE_ME",
					"max_version"="CHANGE_ME_OR_DELETE_ME",
					repository="CHANGE_ME_OR_DELETE_ME"
				))
			xml.package <- list(XMLNode("!--", xml.package.example))
		} else {
			xml.package <- list()
		}
	} else {
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

	## pluginmap
	# - name,
	# - url
	# create example, if empty
	if(is.null(pluginmap)){
		if(isTRUE(hints)){
			xml.pluginmap.text <- XMLNode("", "If this plugin depends on other pluginmaps, edit this part to your needs:")
			xml.pluginmap.example <- XMLNode("pluginmap",
				attrs=list(
					name="CHANGE_ME_OR_DELETE_ME",
					url="CHANGE_ME_OR_DELETE_ME"
				))
			xml.pluginmap <- list(XMLNode("!--", xml.pluginmap.text, xml.pluginmap.example))
		} else {
			xml.pluginmap <- list()
		}
	} else {
		xml.pluginmap <- sapply(pluginmap, function(this.pluginmap){
				result <- XMLNode("pluginmap",
					attrs=list(
						name=this.pluginmap[["name"]],
						url=this.pluginmap[["url"]]
					))
				return(result)
			})
	}

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
	# comment out an example dependency listing if it has no entries
	if(is.null(dependencies)){
		if(isTRUE(hints)){
			R.v <- R.Version()
			xml.dependencies <- XMLNode("dependencies",
				attrs=list(
					"rkward_min_version"=.rk.app.version,
					"rkward_max_version"="CHANGE_ME_OR_DELETE_ME",
					"R_min_version"=paste(R.v$major, R.v$minor, sep="."),
					"R_max_version"="CHANGE_ME_OR_DELETE_ME"
				),
				.children=child.list(xml.package, empty=FALSE))
		} else {
			xml.dependencies <- XMLNode("dependencies", .children=child.list(xml.package, empty=FALSE))
		}
	} else {
		dep.options <- names(dependencies)
		dep.attributes <- list(id=id.name)
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
		xml.dependency_check <- XMLNode("dependency_check",
			attrs=dep.attributes,
			.children=child.list(xml.package, empty=FALSE))
	}

	return(xml.dependency_check)
}
