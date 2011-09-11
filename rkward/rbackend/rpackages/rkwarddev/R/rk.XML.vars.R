#' Create a variable selector for RKWard plugins
#'
#' This function will create a <frame> node including a <varselector> and a <varslot> node.
#'
#' @param label Character string, a text label for the whole frame.
#' @param slot.text Character string, a text label for the variable selection slot.
#' @param required Logical, whether the selection of variables is mandatory or not.
#' @param multi Logical, whether the varslot holds only one or several objects.
#' @param min If \code{multi=TRUE} defines how many objects must be selected.
#' @param any If \code{multi=TRUE} defines how many objects must be selected at least if any
#'		are selected at all.
#' @param max If \code{multi=TRUE} defines how many objects can be selected in total
#'		(0 means any number).
#' @param dim The number of dimensions, an object needs to have. If \code{dim=0} any number
#'		of dimensions is acceptable.
#' @param min.len The minimum length, an object needs to have.
#' @param max.len The maximum length, an object needs to have. If \code{NULL}, defaults to the largest
#'		integer number representable on the system.
#' @param classes An optional character vector, defining class names to which the selection must be limited.
#' @param types If you specify one or more variables types here, the varslot will only accept objects of those
#'		types. Valid types are "unknown", "number", "string", "factor", "invalid". Optional, use with great care,
#'		the user should not be prevented from making valid choices, and rkward does not always know the type
#'		of a variable!
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

rk.XML.vars <- function(label, slot.text=NULL, required=FALSE, multi=FALSE, min=1, any=1, max=0,
	dim=0, min.len=0, max.len=NULL, classes=NULL, types=NULL, horiz=TRUE, id.name="auto"){
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
	if(!is.null(types)){
		valid.types <- types[types %in% c("unknown", "number", "string", "factor", "invalid")]
		var.slot.attr[["types"]] <- paste(valid.types, collapse=" ")
	} else {}
	if(isTRUE(required)){
		var.slot.attr[["required"]] <- "true"
	} else {}
	if(isTRUE(multi)){
		var.slot.attr[["multi"]] <- "true"
		if(min > 1){
			var.slot.attr[["min_vars"]] <- min
		} else {}
		if(any > 1){
			var.slot.attr[["min_vars_if_any"]] <- any
		} else {}
		if(max > 0){
			var.slot.attr[["max_vars"]] <- max
		} else {}
	} else {}

	if(dim > 0){
		var.slot.attr[["num_dimensions"]] <- dim
	} else {}
	if(min.len > 0){
		var.slot.attr[["min_length"]] <- min.len
	} else {}
	if(!is.null(max.len)){
		var.slot.attr[["max_length"]] <- max.len
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
