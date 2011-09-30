#' Create XML "context" node for RKWard plugins
#'
#' This function will create a context node for .pluginmap files,
#' with mandatory child nodes "menu".
#' 
#' @param nodes A (list of) objects of class \code{XiMpLe.node}, must all be "menu".
#' @param id Character string, either "x11" or "import".
#' @return A list of objects of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.menu]{rk.XML.menu}},
#'		\code{\link[rkwarddev:rk.XML.entry]{rk.XML.entry}},
#'		\code{\link[rkwarddev:rk.XML.component]{rk.XML.component}},
#' @examples
#' test.component <- rk.XML.component("My GUI dialog", "plugins/MyGUIdialog.xml")
#' test.entry <- rk.XML.entry(test.component)
#' test.menu <- rk.XML.menu("Analysis", nodes=test.entry, id.name="analysis")
#' test.context <- rk.XML.context(test.menu)
#' cat(pasteXMLNode(test.context))

rk.XML.context <- function(nodes, id="x11"){
	# check the node names and allow only valid ones
	node.names <- sapply(child.list(nodes), function(this.node){
			this.node@name
		})

	if(!id %in% c("x11", "import")){
		stop(simpleError(paste("Invalid ID: ", id, sep="")))
	} else {}
		
	invalid.sets <- !node.names %in% c("menu")
	if(any(invalid.sets)){
		stop(simpleError(paste("Invalid XML nodes for context section: ",
			paste(node.names[invalid.sets], collapse=", "), sep="")))
	} else {}

	node <- new("XiMpLe.node",
			name="context",
			attributes=list(id=id),
			children=child.list(nodes)
		)

	return(node)
}
