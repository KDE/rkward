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
#' cat(pasteXMLNode(plugin.related))

rk.rkh.related <- function(..., text=NULL){
	links <- list(...)

	# check the node names and allow only valid ones
	li.elements <- sapply(child.list(links), function(this.node){
			if(!identical(this.node@name, "link")){
				stop(simpleError(paste("Invalid XML nodes for links section: ", this.node@name, sep="")))
			} else {
				li.element <- new("XiMpLe.node",
						name="li",
						children=child.list(this.node),
						value="")
			}
			return(li.element)
		})

	ul <- new("XiMpLe.node",
			name="ul",
			children=child.list(li.elements),
			value="")

	if(is.null(text)){
		text <- ""
	} else {}

	node <- new("XiMpLe.node",
			name="related",
			children=child.list(ul),
			value=text)

	return(node)
}
