#' Create XML hierarchy section for RKWard plugins
#'
#' This function will create a hierarchy section for .pluginmap files,
#' with mandatory child nodes "menu".
#'
#' @param ... Objects of class \code{XiMpLe.node}, must all be "menu".
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.menu]{rk.XML.menu}},
#'		\code{\link[rkwarddev:rk.XML.entry]{rk.XML.entry}},
#'		\code{\link[rkwarddev:rk.XML.component]{rk.XML.component}},
#'		\code{\link[rkwarddev:rk.XML.components]{rk.XML.components}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.component <- rk.XML.component("My GUI dialog", "plugins/MyGUIdialog.xml")
#' test.entry <- rk.XML.entry(test.component)
#' test.menu <- rk.XML.menu("Analysis", nodes=test.entry, id.name="analysis")
#' test.hierarchy <- rk.XML.hierarchy(test.menu)
#' cat(pasteXML(test.hierarchy))

rk.XML.hierarchy <- function(...){
	nodes <- list(...)

	# check the node names and allow only valid ones
	valid.child("hierarchy", children=nodes)

	node <- XMLNode("hierarchy", .children=child.list(nodes, empty=FALSE))

	return(node)
}
