#' Create XML "related" node for RKWard help pages
#'
#' @param ... Objects of class \code{XiMpLe.node}. They must all have the name "link".
#' @param text Character string, the text to be displayed.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}}
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' package.link <- rk.rkh.link("Spice")
#' plugin.related <- rk.rkh.related(package.link)
#' cat(pasteXML(plugin.related))

rk.rkh.related <- function(..., text=NULL){
	links <- list(...)

	# check the node names and allow only valid ones
	li.elements <- sapply(child.list(links), function(this.node){
			if(!identical(slot(this.node, "name"), "link")){
				stop(simpleError(paste("Invalid XML nodes for links section: ", this.node@name, sep="")))
			} else {
				li.element <- XMLNode("li", .children=child.list(this.node, empty=FALSE))
			}
			return(li.element)
		})

	ul <- XMLNode("ul", .children=child.list(li.elements, empty=FALSE))

	node <- XMLNode("related", text, ul)

	return(node)
}
