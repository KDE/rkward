#' Create XML "menu" node for RKWard plugins
#'
#' This function will create a menu node for entry sections.
#' 
#' @note NOT WORKING YET
#'
#' @param label A label.
#' @param index An index.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"}, an ID will be generated automatically from the tag names and
#'		IDs of the given nodes.
#' @return A list of objects of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.menus]{rk.XML.menus}}
# @examples
# # define a formula section with varselector and varslots
# test.formula <- rk.XML.vars("Variables", "Fixed", formula.dependent="Dependent")
# # define the menu
# test.menu <- rk.XML.menu(test.formula)
# cat(pasteXMLNode(test.menu))

rk.XML.menu <- function(label, index=1, id.name="auto"){
	if(identical(id.name, "auto")){
		# try autogenerating some id
		id.name <- auto.ids(node.soup(nodes), prefix=ID.prefix("menu"), chars=10)
	} else if(is.null(id.name)){
		stop(simpleError("Components need an ID!"))
	} else {}

	node <- new("XiMpLe.node",
			name="menu",
			attributes=list(id=id.name),
			children=child.list(nodes)
		)

	return(node)
}
