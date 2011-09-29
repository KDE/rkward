#' Create XML "components" node for RKWard plugins
#'
#' This function will create a components node for a .pluginmap file, with optional child nodes "component".
#'
#' @note NOT WORKING YET
#'
#' @param nodes A (list of) objects of class \code{XiMpLe.node}. They must all have the name "component".
#' @return A list of objects of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.pluginmap]{rk.XML.pluginmap}}
#'		\code{\link[rkwarddev:rk.XML.component]{rk.XML.component}}
# @examples
# # define a formula section with varselector and varslots
# test.formula <- rk.XML.vars("Variables", "Fixed", formula.dependent="Dependent")
# # define the components section
# test.component <- rk.XML.component(test.formula)
# test.components <- rk.XML.components(test.component)
# cat(pasteXMLNode(test.components))

rk.XML.components <- function(nodes=NULL){
	if(!is.null(nodes)){
		# check the node names and allow only valid ones
		sapply(child.list(nodes), function(this.node){
				node.name <- this.node@name
				if(!identical(node.name, "component")){
					stop(simpleError(paste("Invalid XML nodes for components section: ", node.name, sep="")))
				} else {}
			})
	} else {
		nodes <- list()
	}

	node <- new("XiMpLe.node",
			name="components",
			children=child.list(nodes),
			value=""
		)

	return(node)
}
