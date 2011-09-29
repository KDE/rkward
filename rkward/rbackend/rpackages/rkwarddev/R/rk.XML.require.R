#' Create XML "require" node for RKWard plugins
#'
#' This function will create a require node for .pluginmap files.
#' 
#' @note NOT WORKING YET
#'
#' @param file A file name.
#' @return A list of objects of class \code{XiMpLe.node}.
#' @export
# @examples
# # define a formula section with varselector and varslots
# test.formula <- rk.XML.vars("Variables", "Fixed", formula.dependent="Dependent")
# # define the require
# test.require <- rk.XML.require(test.formula)
# cat(pasteXMLNode(test.require))

rk.XML.require <- function(file){

	node <- new("XiMpLe.node",
			name="require",
			attributes=list(file=file)
		)

	return(node)
}
