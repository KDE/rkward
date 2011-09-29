#' Create XML "attribute" node for RKWard plugins
#'
#' This function will create a attribute node for component sections in .pluginmap files.
#' 
#' @note NOT WORKING YET
#'
#' @param label A label.
#' @param value A value.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"}, an ID will be generated automatically from the tag names and
#'		IDs of the given nodes.
#' @return A list of objects of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.attributes]{rk.XML.attributes}}
# @examples
# # define a formula section with varselector and varslots
# test.formula <- rk.XML.vars("Variables", "Fixed", formula.dependent="Dependent")
# # define the attribute
# test.attribute <- rk.XML.attribute(test.formula)
# cat(pasteXMLNode(test.attribute))

rk.XML.attribute <- function(label, value, id.name="auto"){
	if(identical(id.name, "auto")){
		# try autogenerating some id
		id.name <- auto.ids(node.soup(nodes), prefix=ID.prefix("attribute"), chars=10)
	} else if(is.null(id.name)){
		stop(simpleError("Components need an ID!"))
	} else {}

	node <- new("XiMpLe.node",
			name="attribute",
			attributes=list(id=id.name),
			children=child.list(nodes)
		)

	return(node)
}
