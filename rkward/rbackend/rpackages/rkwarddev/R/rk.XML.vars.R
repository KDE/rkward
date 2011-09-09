#' Create a variable selector for RKWard plugins
#'
#' This function will create a <frame> node including a <varselector> and a <varslot> node.
#'
#' @param label Character string, a text label for the whole frame.
#' @param slot.text Character string, a text label for the variable selection slot.
#' @param classes An optional character vector, defining class names to which the selection must be limited.
#' @param horiz Logical. If \code{TRUE}, the varslot will be placed next to the selector,
#'		if \code{FALSE} below it.
#' @param id.name Character vector, unique IDs for the frame (first entry), the varselector (second entry)
#'		and varslot (third entry).
#'		If \code{"auto"}, IDs will be generated automatically from \code{label} and \code{slot.text}.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @examples
#' test.vars <- rk.XML.vars("Select some vars", "Vars go here")
#' cat(pasteXMLNode(test.vars, shine=1))

rk.XML.vars <- function(label, slot.text=NULL, classes=NULL, horiz=TRUE, id.name="auto"){
	if(identical(id.name, "auto")){
		var.sel.attr <- list(id=auto.ids(label, prefix=ID.prefix("varselector", length=3)))
		var.slot.attr <- list(id=auto.ids(label, prefix=ID.prefix("varslot", length=4)))
	} else if(!is.null(id.name)){
		var.sel.attr <- list(id=id.name[[2]])
		var.slot.attr <- list(id=id.name[[3]])
	} else {}
	
	v.selector <- new("XiMpLe.node",
		name="varselector",
		attributes=var.sel.attr)

	if(!is.null(slot.text)){
		var.slot.attr[["label"]] <- slot.text
	} else {}
	
	var.slot.attr[["sources"]] <- var.sel.attr[["id"]]
	if(!is.null(classes)){
		var.slot.attr[["classes"]] <- paste(classes, collapse=" ")
	} else {}

	v.slot <- new("XiMpLe.node",
		name="varslot",
		attributes=var.slot.attr)

	if(isTRUE(horiz)){
		aligned.chld <- rk.XML.row(list(rk.XML.col(v.selector), rk.XML.col(v.slot)))
	} else {
		aligned.chld <- list(v.selector, v.slot)
	}
	vars.frame <- rk.XML.frame(
		children=child.list(aligned.chld),
		label=label,
		id.name=id.name[[1]])

	return(vars.frame)
}
