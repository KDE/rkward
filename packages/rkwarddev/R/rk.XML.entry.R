#' Create XML "entry" node for RKWard plugins
#'
#' This function will create a entry node for menu sections in .pluginmap files.
#' 
#' @param component An ID.
#' @param index Integer number to influence the level of menu placement.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.menu]{rk.XML.menu}},
#'		\code{\link[rkwarddev:rk.XML.hierarchy]{rk.XML.hierarchy}},
#'		\code{\link[rkwarddev:rk.XML.component]{rk.XML.component}},
#'		\code{\link[rkwarddev:rk.XML.components]{rk.XML.components}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.component <- rk.XML.component("My GUI dialog", "plugins/MyGUIdialog.xml")
#' test.entry <- rk.XML.entry(test.component)
#' cat(pasteXMLNode(test.entry))

rk.XML.entry <- function(component, index=-1){
	if(length(component) > 1){
		stop(simpleError("'component' must be of length 1!"))
	} else {}

	# check the node names and allow only valid ones
	if(inherits(component, "XiMpLe.node")){
		node.name <- component@name
		if(!identical(node.name, "component")){
			stop(simpleError(paste("Invalid XML node for 'entry': ", node.name, sep="")))
		} else {}
	} else {}

	attr.list <- list(component=check.ID(component))

	if(!identical(index, -1)){
		attr.list[["index"]] <- index
	} else {}

	node <- new("XiMpLe.node",
			name="entry",
			attributes=attr.list
		)

	return(node)
}
