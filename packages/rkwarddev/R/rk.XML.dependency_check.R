#' Create XML node "dependency_check" for RKWard pluginmaps
#'
#' @note The \code{<dependency_check>} node was introduced with RKWard 0.6.1, please set the dependencies
#'    of your component/plugin accordingly.
#'
#' @param id.name Character string, a unique ID for this plugin element.
#' @param dependencies A named list with these elements:
#'    \describe{
#'      \item{rkward.min}{Minimum RKWard version needed for this plugin (optional)}
#'      \item{rkward.max}{Maximum RKWard version needed for this plugin (optional)}
#'      \item{R.min}{Minimum R version needed for this plugin (optional)}
#'      \item{R.max}{Maximum R version needed for this plugin (optional)}
#'    }
#' @param package A list of named character vectors, each with these elements:
#'    \describe{
#'      \item{name}{Name of a package this plugin depends on (optional)}
#'      \item{min}{Minimum version of the package (optional)}
#'      \item{max}{Maximum version of the package (optional)}
#'      \item{repository}{Repository to download the package (optional)}
#'    }
#' @param pluginmap A named list with these elements:
#'    \describe{
#'      \item{name}{Identifier of a pluginmap this plugin depends on (optional)}
#'      \item{url}{URL to get the pluginmap (optional)}
#'    }
#' @param hints Logical, if \code{TRUE}, \code{NULL} values will be replaced with example text.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.dependencies]{rk.XML.dependencies}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' dependency_check.node <- rk.XML.dependency_check(
#'   id.name="dep_check",
#'   dependencies=list(
#'     rkward.min="0.5.3",
#'     rkward.max="",
#'     R.min="2.10",
#'     R.max=""),
#'   package=list(
#'     c(name="heisenberg", min="0.11-2", max="",
#'       repository="http://rforge.r-project.org"),
#'     c(name="DreamsOfPi", min="0.2", max="", repository="")),
#'   pluginmap=list(
#'     c(name="heisenberg.pluginmap", url="http://eternalwondermaths.example.org/hsb"))
#' )

rk.XML.dependency_check <- function(id.name, dependencies=NULL, package=NULL, pluginmap=NULL, hints=FALSE){
  # this is basically rk.XML.dependencies() with a different node name and additional ID
  xml.dependency_check <- rk.XML.dependencies(
    dependencies=dependencies,
    package=package,
    pluginmap=pluginmap,
    hints=hints)

  XMLName(xml.dependency_check) <- "dependency_check"
  XMLAttrs(xml.dependency_check)[["id"]] <- id.name

  return(xml.dependency_check)
}
