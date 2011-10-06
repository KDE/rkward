#' Create XML "title" node for RKWard help pages
#'
#' @param text Character string, the text to be displayed.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}}
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' plugin.title <- rk.rkh.title("Spice")
#' cat(pasteXMLNode(plugin.title))

rk.rkh.title <- function(text=NULL){

	node <- new("XiMpLe.node",
			name="title",
			value=text)

	return(node)
}
