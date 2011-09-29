#' Create XML "context" node for RKWard plugins
#'
#' This function will create a context node for .pluginmap files.
#' 
#' @note NOT WORKING YET
#'
#' @param id Either "x11" or "import".
#' @return A list of objects of class \code{XiMpLe.node}.
#' @export
# @examples
# # define a formula section with varselector and varslots
# test.formula <- rk.XML.vars("Variables", "Fixed", formula.dependent="Dependent")
# # define the context
# test.context <- rk.XML.context(test.formula)
# cat(pasteXMLNode(test.context))

rk.XML.context <- function(id="x11"){

	node <- new("XiMpLe.node",
			name="context",
			attributes=list(id=id.name)
		)

	return(node)
}
