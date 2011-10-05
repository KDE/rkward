#' Create XML node "column" for RKWard plugins
#'
#' @param children An optional list with objects of class \code{XiMpLe.node}.
#' @param label Character string, a text label for this plugin element.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"} and a label was provided, an ID will be generated automatically from the label.
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

rk.XML.frame <- function(children=list(), label=NULL, id.name="auto"){
	if(!is.null(label)){
		attr.list <- list(label=label)
		if(identical(id.name, "auto")){
			attr.list[["id"]] <- list(id=auto.ids(label, prefix=ID.prefix("frame")))
		} else if(!is.null(id.name)){
			attr.list[["id"]] <- id.name
		} else {}
	} else {
		attr.list <- list()
	}

	frame <- new("XiMpLe.node",
		name="frame",
		attributes=attr.list,
		children=child.list(children),
		value="")

	return(frame)
}
