#' Create XML empty node "stretch" for RKWard plugins
#'
#' The simplest way to use \code{rk.XML.stretch} is to call it without arguments.
#' If you provide \code{before} and/or \code{after}, a "<stretch />" will be put between
#' the XML elements defined there.
#'
#' @param before A list of objects of class \code{XiMpLe.node}.
#' @param after A list of objects of class \code{XiMpLe.node}.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @examples
#' cat(pasteXMLNode(rk.XML.stretch()))

#<stretch />
rk.XML.stretch <- function(before=NULL, after=NULL){
	strch <- new("XiMpLe.node", name="stretch")

	# if called without furter objects, just return the node
	if(is.null(c(before, after))){
		return(strch)
	} else {}

	if(!is.null(before)){
		strch.lst <- child.list(before)
		strch.lst[[length(strch.lst)+1]] <- strch
	} else {
		strch.lst <- list(strch)
	}

	if(!is.null(after)){
		for(this.element in child.list(after)){
				strch.lst[[length(strch.lst)+1]] <- this.element
			}
	} else {}

	return(strch.lst)
}
