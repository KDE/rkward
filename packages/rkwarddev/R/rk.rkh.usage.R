#' Create XML "usage" node for RKWard help pages
#'
#' @param text Character string, the text to be displayed.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}}
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' plugin.usage <- rk.rkh.usage("First do this, then do that ...")
#' cat(pasteXMLNode(plugin.usage))

rk.rkh.usage <- function(text=NULL){
	if(is.null(text)){
		text <- ""
	} else {}

	node <- new("XiMpLe.node",
			name="usage",
			value=text)

	return(node)
}
