#' Create XML "menu" node for RKWard plugins
#'
#' This function will create a menu node for hierarchy sections.
#' Use same \code{id} values to place entries in the same menu.
#' 
#' @param label Character string, a label for the menu.
#' @param nodes A (list of) objects of class \code{XiMpLe.node}, must be either
#'		"menu" or "entry".
#' @param index Integer number to influence the level of menu placement.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"}, an ID will be generated automatically from the label.
#'		Used to place the menu in the global menu hierarchy.
#' @return A list of objects of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.hierarchy]{rk.XML.hierarchy}},
#'		\code{\link[rkwarddev:rk.XML.entry]{rk.XML.entry}},
#'		\code{\link[rkwarddev:rk.XML.component]{rk.XML.component}},
#'		\code{\link[rkwarddev:rk.XML.components]{rk.XML.components}}
#' @examples
#' test.component <- rk.XML.component("My GUI dialog", "plugins/MyGUIdialog.xml")
#' test.entry <- rk.XML.entry(test.component)
#' test.menu <- rk.XML.menu("Analysis", nodes=test.entry, id.name="analysis")
#' cat(pasteXMLNode(test.menu))

rk.XML.menu <- function(label, nodes, index=-1, id.name="auto"){
	# check the node names and allow only valid ones
	sapply(child.list(nodes), function(this.node){
			stopifnot(inherits(this.node, "XiMpLe.node"))
			node.name <- this.node@name
			if(!node.name %in% c("entry", "menu")){
				stop(simpleError(paste("Invalid XML nodes for menu section: ", node.name, sep="")))
			} else {}
		})

	if(identical(id.name, "auto")){
		# try autogenerating some id
		id.name <- auto.ids(label, prefix=ID.prefix("menu"), chars=10)
	} else if(is.null(id.name)){
		stop(simpleError("Menu needs an ID!"))
	} else {}

	attr.list <- list(id=id.name, label=label)

	if(!identical(index, -1)){
		attr.list[["index"]] <- index
	} else {}

	node <- new("XiMpLe.node",
			name="menu",
			attributes=attr.list,
			children=child.list(nodes)
		)

	return(node)
}
