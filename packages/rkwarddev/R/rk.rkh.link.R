#' Create XML "link" node for RKWard help pages
#'
#' @param href Character string, either the URL to link to, name of an R package or ID of
#'		another plugin (see \code{type}).
#' @param text Character string, optional link text.
#' @param type Character string, one of the following valid entries:
#'		\itemize{
#'			\item{\code{"url"}}{\code{href} is assumend to be the actual URL.}
#'			\item{\code{"R"}}{\code{href} is assumend to be the name of an R package, i.e.,
#'				the link generated will look like \code{rkward://rhelp/<href>}.}
#'			\item{\code{"RK"}}{\code{href} is assumend to be the ID of another RKWard plugin, i.e.,
#'				the link generated will look like \code{rkward://component/<href>}.}
#'		}
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}}
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' package.link <- rk.rkh.link("Spice")
#' cat(pasteXML(package.link))

rk.rkh.link <- function(href, text=NULL, type="R"){
	if(identical(type, "url")){
		text <- ifelse(is.null(text), href, text)
		link <- href
	} else if(identical(type, "R")){
		link <- paste0("rkward://rhelp/", href)
	} else if(identical(type, "RK")){
		link <- paste0("rkward://component/", href)
	} else {
		stop(simpleError(paste0("Invalid type for link node:", type)))
	}

	if(is.null(text)){
		text <- character()
	} else {}

	node <- XMLNode(name="link", text, attrs=list(href=link))

	return(node)
}
