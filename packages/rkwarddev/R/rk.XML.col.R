#' Create XML node "column" for RKWard plugins
#'
#' @param ... Objects of class \code{XiMpLe.node}.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"}, an ID will be generated automatically from the objects in \code{...}.
#'		If \code{NULL}, no ID will be given.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.checkboxes <- rk.XML.row(rk.XML.col(
#'     rk.XML.cbox(label="foo", val="foo1", chk=TRUE),
#'     rk.XML.cbox(label="bar", val="bar2")))
#' cat(pasteXMLNode(test.checkboxes))

rk.XML.col <- function(..., id.name="auto"){
	nodes <- list(...)

	if(identical(id.name, "auto")){
		# try autogenerating some id
		attr.list <- list(id=auto.ids(node.soup(nodes), prefix=ID.prefix("column"), chars=10))
	} else if(is.null(id.name)){
		attr.list <- list()
	} else {
		attr.list <- list(id=id.name)
	}

	col <- new("XiMpLe.node",
		name="column",
		attributes=attr.list,
		children=child.list(nodes),
		value="")

	return(col)
}
