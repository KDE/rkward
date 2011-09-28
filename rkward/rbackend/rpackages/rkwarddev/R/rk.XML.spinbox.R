#' Create XML node "spinbox" for RKWard plugins
#'
#' @param label Character string, a text label for this plugin element.
#' @param min Numeric, the lowest value allowed. Defaults to the lowest value technically representable in the spinbox.
#' @param max Numeric, the largest value allowed. Defaults to the highest value technically representable in the spinbox.
#' @param initial Numeric, will be used as the initial value.
#' @param real Logical, whether values should be real or integer numbers.
#' @param precision Numeric, if \code{real=TRUE} defines the default number of decimal places shown in the spinbox.
#' @param max.precision Numeric, maximum number of digits that can be meaningfully represented.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"} and a label was provided, an ID will be generated automatically from the label.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @examples
#' test.spinbox <- rk.XML.spinbox("Spin this:")
#' cat(pasteXMLNode(test.spinbox))


rk.XML.spinbox <- function(label, min=NULL, max=NULL, initial=0, real=TRUE, precision=2, max.precision=8, id.name="auto"){
	attr.list <- list(label=label)

	if(identical(id.name, "auto")){
		attr.list[["id"]] <- list(id=auto.ids(label, prefix=ID.prefix("spinbox")))
	} else if(!is.null(id.name)){
		attr.list[["id"]] <- id.name
	} else {}

	if(initial != 0){
		check.type(initial, "numeric", "initial")
		attr.list[["initial"]] <- as.numeric(initial)
	} else {}
	if(!is.null(min)){
		check.type(min, "numeric", "min")
		attr.list[["min"]] <- as.numeric(min)
	} else {}
	if(!is.null(max)){
		check.type(max, "numeric", "max")
		attr.list[["max"]] <- as.numeric(max)
	} else {}
	if(!isTRUE(real)){
		attr.list[["type"]] <- "integer"
	} else {}
	if(precision != 2){
		check.type(precision, "numeric", "precision")
		attr.list[["precision"]] <- as.numeric(precision)
	} else {}
	if(max.precision != 8){
		check.type(max.precision, "numeric", "max.precision")
		attr.list[["max.precision"]] <- as.numeric(max.precision)
	} else {}

	node <- new("XiMpLe.node",
			name="spinbox",
			attributes=attr.list)

	return(node)
}
