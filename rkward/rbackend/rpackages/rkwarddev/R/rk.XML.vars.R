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
#' @param add.nodes A list of objects of class \code{XiMpLe.node} to be placed after the varslot.
#' @param id.name Character vector, unique IDs for the frame (first entry), the varselector (second entry)
#'		and varslot (third entry).
#'		If \code{"auto"}, IDs will be generated automatically from \code{label} and \code{slot.text}.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @examples
#' test.vars <- rk.XML.vars("Select some vars", "Vars go here")
#' cat(pasteXMLNode(test.vars, shine=1))

rk.XML.vars <- function(label, slot.text, required=FALSE, multi=FALSE, min=1, any=1, max=0,
	dim=0, min.len=0, max.len=NULL, classes=NULL, types=NULL, horiz=TRUE, add.nodes=NULL, id.name="auto"){
	if(identical(id.name, "auto")){
		## if this ID generation get's changed, change it in rk.XML.varslot(), too!
		var.sel.attr <- list(id=auto.ids(label, prefix=ID.prefix("varselector", length=3)))
		var.slot.id <- auto.ids(label, prefix=ID.prefix("varslot", length=4))
	} else if(!is.null(id.name)){
		var.sel.attr <- list(id=id.name[[2]])
		var.slot.id <- id.name[[3]]
	} else {}
	
	v.selector <- new("XiMpLe.node",
		name="varselector",
		attributes=var.sel.attr)

	v.slot <- rk.XML.varslot(
		label=slot.text,
		source=var.sel.attr[["id"]],
		required=required,
		multi=multi,
		min=min,
		any=any,
		max=max,
		dim=dim,
		min.len=min.len,
		max.len=max.len,
		classes=classes,
		types=types,
		id.name=var.slot.id)

	# do we need to add extra nodes to the varslot?
	slot.content <- list(v.slot)
	if(!is.null(add.nodes)){
		for (this.node in add.nodes) {
			slot.content[[length(slot.content)+1]] <- this.node
		}
	} else {}

	if(isTRUE(horiz)){
		aligned.chld <- rk.XML.row(list(rk.XML.col(v.selector), rk.XML.col(slot.content)))
	} else {
		aligned.chld <- list(v.selector, unlist(slot.content))
	}

	vars.frame <- rk.XML.frame(
		children=child.list(aligned.chld),
		label=label,
		id.name=id.name[[1]])

	return(vars.frame)
}
