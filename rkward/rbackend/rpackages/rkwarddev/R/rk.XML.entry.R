#' Create XML "entry" node for RKWard plugins
#'
#' This function will create a entry node for .pluginmap files.
#' 
#' @note NOT WORKING YET
#'
#' @param component An ID.
#' @param index An index.
#' @return A list of objects of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.entrys]{rk.XML.entrys}}
# @examples
# # define a formula section with varselector and varslots
# test.formula <- rk.XML.vars("Variables", "Fixed", formula.dependent="Dependent")
# # define the entry
# test.entry <- rk.XML.entry(test.formula)
# cat(pasteXMLNode(test.entry))

rk.XML.entry <- function(component, index=1){
	if(identical(id.name, "auto")){
		# try autogenerating some id
		id.name <- auto.ids(node.soup(nodes), prefix=ID.prefix("entry"), chars=10)
	} else if(is.null(id.name)){
		stop(simpleError("Components need an ID!"))
	} else {}

	node <- new("XiMpLe.node",
			name="entry",
			attributes=list(id=id.name),
			children=child.list(nodes)
		)

	return(node)
}
