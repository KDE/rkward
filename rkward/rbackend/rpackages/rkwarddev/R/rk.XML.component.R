#' Create XML "component" node for RKWard plugins
#'
#' This function will create a component node for components sections.
#' 
#' @note NOT WORKING YET
#'
#' @param label A label.
#' @param file A file name.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"}, an ID will be generated automatically from the tag names and
#'		IDs of the given nodes.
#' @param type Character string, type of component. As of now, only "standard" is supported. The option is
#'		just implemented for completeness.
#' @return A list of objects of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.components]{rk.XML.components}}
# @examples
# # define a formula section with varselector and varslots
# test.formula <- rk.XML.vars("Variables", "Fixed", formula.dependent="Dependent")
# # define the component
# test.component <- rk.XML.component(test.formula)
# cat(pasteXMLNode(test.component))

rk.XML.component <- function(label, file, id.name="auto", type="standard"){
	if(identical(id.name, "auto")){
		# try autogenerating some id
		id.name <- auto.ids(node.soup(nodes), prefix=ID.prefix("component"), chars=10)
	} else if(is.null(id.name)){
		stop(simpleError("Components need an ID!"))
	} else {}

	node <- new("XiMpLe.node",
			name="component",
			attributes=list(id=id.name),
			children=child.list(nodes)
		)

	return(node)
}
