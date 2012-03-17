#' Create XML node "input" for RKWard plugins
#'
#' @param label Character string, a text label for this plugin element.
#' @param initial Character string, if not \code{NULL} will be used as the initial value of the input field.
#' @param size One value of either "small", "medium" or "large".
#' @param required Logical, whether an entry is mandatory or not.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"}, an ID will be generated automatically from the label.
#' @return An object of class \code{XiMpLe.node}.
#' @seealso
#'		\href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export
#' @examples
#' test.input <- rk.XML.input("Type some text")
#' cat(pasteXML(test.input))

rk.XML.input <- function(label, initial=NULL, size="medium", required=FALSE, id.name="auto"){
	attr.list <- list(label=label)

	if(identical(id.name, "auto")){
		attr.list[["id"]] <- auto.ids(label, prefix=ID.prefix("input"))
	} else if(!is.null(id.name)){
		attr.list[["id"]] <- id.name
	} else {}

	if(!is.null(initial)){
		attr.list[["initial"]] <- initial
	} else {}
	if(identical(size, "small") | identical(size, "large")){
		attr.list[["size"]] <- size
	} else {}
	if(isTRUE(required)){
		attr.list[["required"]] <- "true"
	} else {}

	node <- new("XiMpLe.node",
			name="input",
			attributes=attr.list)

	return(node)
}
