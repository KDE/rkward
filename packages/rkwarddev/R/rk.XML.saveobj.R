#' Create XML node "saveobject" for RKWard plugins
#'
#' @param label Character string, a text label for this plugin element.
#' @param chk Logical, if \code{TRUE} and \code{checkable=TRUE} the option is checkable and active by default.
#' @param checkable Logical, if \code{TRUE} the option can be switched on and off.
#' @param intitial Character string, the default name for the object should be saved to.
#'		If \code{"auto"} and a label was provided, an name will be generated automatically from the label.
#' @param required Logical, whether an entry is mandatory or not.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"} and a label was provided, an ID will be generated automatically from the label.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @examples
#' test.saveobj <- rk.XML.saveobj("Save the results")
#' cat(pasteXMLNode(test.saveobj))

rk.XML.saveobj <- function(label, chk=FALSE, checkable=TRUE, initial="auto", required=FALSE, id.name="auto"){
	attr.list <- list(label=label)

	if(isTRUE(checkable)){
		attr.list[["checkable"]] <- "true"
		if(isTRUE(chk)){
			attr.list[["checked"]] <- "true"
		} else {}
	} else {
		attr.list[["checkable"]] <- "false"
	}

	if(identical(initial, "auto")){
		attr.list[["initial"]] <- auto.ids(label, suffix=".obj")
	} else if(!is.null(id.name)){
		attr.list[["initial"]] <- initial
	} else {}
	if(isTRUE(required)){
		attr.list[["required"]] <- "true"
	} else {}


	if(identical(id.name, "auto")){
		attr.list[["id"]] <- list(id=auto.ids(label, prefix=ID.prefix("saveobject")))
	} else if(!is.null(id.name)){
		attr.list[["id"]] <- id.name
	} else {}

	node <- new("XiMpLe.node",
			name="saveobject",
			attributes=attr.list)

	return(node)
}
