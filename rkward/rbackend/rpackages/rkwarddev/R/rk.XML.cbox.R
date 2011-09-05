#' Create XML node "checkbox" for RKWard plugins
#'
#' @param label Character string, a text label for this plugin element.
#' @param val Character string, the value to submit if the element is checked.
#' @param chk Logical, whether this element should be checked by default.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"}, an ID will be generated automatically from the label.
#' @return An object of class \code{XiMpLe.node}.
#' @export

rk.XML.cbox <- function(label, val, chk=FALSE, id.name="auto"){
	if(identical(id.name, "auto")){
		id <- auto.ids(label, prefix="chk.")
	} else {
		id <- id.name
	}

	attr.list <- list(id=id, label=label, value=val)
	if(isTRUE(chk)){
		attr.list[["checked"]] <- "true"
	} else {}

	checkbox <- new("XiMpLe.node",
		name="checkbox",
		attributes=attr.list)

	return(checkbox)
}
