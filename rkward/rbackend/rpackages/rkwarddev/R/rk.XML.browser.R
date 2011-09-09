#' Create XML node "browser" for RKWard plugins
#'
#' @param label Character string, a text label for this plugin element.
#' @param dir Logical, if \code{TRUE} type of object browser defaults to "dir", otherwise "file".
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"} and a label was provided, an ID will be generated automatically from the label.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @examples
#' test.browser <- rk.XML.browser("Browse here:")
#' cat(pasteXMLNode(test.browser, shine=1))


rk.XML.browser <- function(label, dir=TRUE, id.name="auto"){
	attr.list <- list(label=label)

	if(isTRUE(dir)){
		attr.list[["type"]] <- "dir"
	} else {
		attr.list[["type"]] <- "file"
	}

	if(identical(id.name, "auto")){
		attr.list[["id"]] <- list(id=auto.ids(label, prefix=ID.prefix("browser")))
	} else if(!is.null(id.name)){
		attr.list[["id"]] <- id.name
	} else {}

	node <- new("XiMpLe.node",
			name="browser",
			attributes=attr.list)

	return(node)
}
