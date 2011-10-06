#' Create XML "snippet" node for RKWard plugins
#'
#' This function will create a snippet node for snippets sections.
#'
#' @param nodes A (list of) objects of class \code{XiMpLe.node}.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"}, an ID will be generated automatically from the tag names and
#'		IDs of the given nodes.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.snippets]{rk.XML.snippets}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # define a formula section with varselector and varslots
#' test.formula <- rk.XML.vars("Variables", "Fixed", formula.dependent="Dependent")
#' # define the snippet
#' test.snippet <- rk.XML.snippet(test.formula)
#' cat(pasteXMLNode(test.snippet))

rk.XML.snippet <- function(nodes, id.name="auto"){
	if(identical(id.name, "auto")){
		# try autogenerating some id
		id.name <- auto.ids(node.soup(nodes), prefix=ID.prefix("snippet"), chars=10)
	} else if(is.null(id.name)){
		stop(simpleError("Snippets need an ID!"))
	} else {}

	node <- new("XiMpLe.node",
			name="snippet",
			attributes=list(id=id.name),
			children=child.list(nodes)
		)

	return(node)
}
