#' Create XML "snippets" node for RKWard plugins
#'
#' This function will create a snippets node for the document section, with optional child nodes "snippet".
#'
#' @param ... Objects of class \code{XiMpLe.node}. They must all have the name "snippet".
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.plugin]{rk.XML.plugin}}
#'		\code{\link[rkwarddev:rk.XML.snippet]{rk.XML.snippet}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # define a formula section with varselector and varslots
#' test.formula <- rk.XML.vars("Variables", "Fixed", formula.dependent="Dependent")
#' # define the snippets section
#' test.snippet <- rk.XML.snippet(test.formula)
#' test.snippets <- rk.XML.snippets(test.snippet)
#' cat(pasteXML(test.snippets))

rk.XML.snippets <- function(...){
	nodes <- list(...)

	# check the node names and allow only valid ones
	sapply(child.list(nodes), function(this.node){
			stopifnot(inherits(this.node, "XiMpLe.node"))
			node.name <- slot(this.node, "name")
			if(!node.name %in% c("snippet", "!--")){
				stop(simpleError(paste0("Invalid XML nodes for snippets section: ", node.name)))
			} else {}
		})

	node <- XMLNode("snippets", .children=child.list(nodes, empty=FALSE))

	return(node)
}
