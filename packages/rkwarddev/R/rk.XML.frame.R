#' Create XML node "column" for RKWard plugins
#'
#' @param ... Objects of class \code{XiMpLe.node}.
#' @param label Character string, a text label for this plugin element.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"} and a label was provided, an ID will be generated automatically from the label
#'		if presen, otherwise from the objects in the frame.
#'		If \code{NULL}, no ID will be given.
#' @return An object of class \code{XiMpLe.node}.
#' @seealso
#'		\href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export
#' @examples
#' test.dropdown <- rk.XML.dropdown("mydrop",
#'   options=list("First Option"=c(val="val1"),
#'   "Second Option"=c(val="val2", chk=TRUE)))
#' cat(pasteXMLNode(rk.XML.frame(test.dropdown, label="Some options")))

rk.XML.frame <- function(..., label=NULL, id.name="auto"){
	nodes <- list(...)

	if(!is.null(label)){
		attr.list <- list(label=label)
	} else {
		attr.list <- list()
	}

	if(identical(id.name, "auto")){
		if(!is.null(label)){
			attr.list[["id"]] <- auto.ids(label, prefix=ID.prefix("frame"))
		} else {
			# try autogenerating some id
			attr.list[["id"]] <- auto.ids(node.soup(nodes), prefix=ID.prefix("frame"), chars=10)
		}
	} else if(!is.null(id.name)){
		attr.list[["id"]] <- id.name
	} else {}

	frame <- new("XiMpLe.node",
		name="frame",
		attributes=attr.list,
		children=child.list(nodes),
		value="")

	return(frame)
}
