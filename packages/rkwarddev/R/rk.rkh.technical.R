#' Create XML "technical" node for RKWard help pages
#'
#' @param text Character string, the text to be displayed.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}}
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' plugin.technical <- rk.rkh.technical("<b>TODO</b>: Implement sandworm detector.")
#' cat(pasteXML(plugin.technical))

rk.rkh.technical <- function(text=NULL){
	if(is.null(text)){
		text <- ""
	} else {}

	node <- XMLNode("technical", text)

	return(node)
}
