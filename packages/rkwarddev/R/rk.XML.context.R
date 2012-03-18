#' Create XML "context" node for RKWard plugins
#'
#' This function will create a context node for .pluginmap files,
#' with mandatory child nodes "menu".
#' 
#' @param ... Objects of class \code{XiMpLe.node}, must all be "menu".
#' @param id Character string, either "x11" or "import".
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.menu]{rk.XML.menu}},
#'		\code{\link[rkwarddev:rk.XML.entry]{rk.XML.entry}},
#'		\code{\link[rkwarddev:rk.XML.component]{rk.XML.component}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.component <- rk.XML.component("My GUI dialog", "plugins/MyGUIdialog.xml")
#' test.entry <- rk.XML.entry(test.component)
#' test.menu <- rk.XML.menu("Analysis", nodes=test.entry, id.name="analysis")
#' test.context <- rk.XML.context(test.menu)
#' cat(pasteXML(test.context))

rk.XML.context <- function(..., id="x11"){
	nodes <- list(...)

	# check the node names and allow only valid ones
	valid.child("context", children=nodes)

	if(!id %in% c("x11", "import")){
		stop(simpleError(paste("Invalid ID: ", id, sep="")))
	} else {}
		
	node <- XMLNode("context",
			attrs=list(id=id),
			.children=child.list(nodes, empty=FALSE)
		)

	return(node)
}
