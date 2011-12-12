#' Create XML node "checkbox" for RKWard plugins
#'
#' @param label Character string, a text label for this plugin element.
#' @param value Character string, the value to submit if the element is checked.
#' @param un.value Character string, an optional value for the unchecked option.
#' @param chk Logical, whether this element should be checked by default.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"}, an ID will be generated automatically from the label.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.checkboxes <- rk.XML.row(rk.XML.col(
#'   list(
#'     rk.XML.cbox(label="foo", value="foo1", chk=TRUE),
#'     rk.XML.cbox(label="bar", value="bar2"))))
#' cat(pasteXMLNode(test.checkboxes))

rk.XML.cbox <- function(label, value="true", un.value=NULL, chk=FALSE, id.name="auto"){
	if(identical(id.name, "auto")){
		id <- auto.ids(label, prefix=ID.prefix("checkbox"))
	} else {
		id <- id.name
	}

	attr.list <- list(id=id, label=label, value=value)
	if(!is.null(un.value)){
		attr.list[["value_unchecked"]] <- un.value
	} else {}
	if(isTRUE(chk)){
		attr.list[["checked"]] <- "true"
	} else {}

	checkbox <- new("XiMpLe.node",
		name="checkbox",
		attributes=attr.list)

	return(checkbox)
}
