#' Create XML "matrix" node for RKWard plugins
#' 
#' @param label Character string, a label for the matrix.
#' @param mode Character string, one of "integer", "real" or "string". The type of data that will
#'		be accepted in the table (required)
#' @param rows Number of rows in the matrix. Has no effect if \code{allow_user_resize_rows=TRUE}.
#' @param columns Number of columns in the matrix. Has no effect if \code{allow_user_resize_columns=TRUE}.
#' @param min Minimum acceptable value (if \code{type} is "integer" or "real"). Defaults to the
#'		smallest representable value.
#' @param max Maximum acceptable value (if \code{type} is "integer" or "real"). Defaults to the
#'		largest representable value.
#' @param allow_missings Logical, whether missing (empty) values are allowed in the matrix
#'		(if \code{type} is "string").
#' @param allow_user_resize_columns Logical, if \code{TRUE}, the user can add columns by typing
#'		on the rightmost (inactive) cells.
#' @param allow_user_resize_rows Logical, if \code{TRUE}, the user can add rows by typing on the
#'		bottommost (inactive) cells.
#' @param fixed_width Logical, force the GUI element to stay at its initial width. Do not use in
#'		combindation with matrices, where the number of columns may change in any way.
#'		Useful, esp. when creating a vector input element (rows="1").
#' @param fixed_height Logical, force the GUI element to stay at its initial height. Do not use in
#'		combindation with matrices, where the number of rows may change in any way.
#'		Useful, esp. when creating a vector input element (columns="1").
#' @param horiz_headers Character vector to use for the horiztonal header. Defaults to column number.
#' @param vert_headers Character vector to use for the vertical header. Defaults to row number.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"}, an ID will be generated automatically from the label.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.matrix <- rk.XML.matrix("A matrix")

rk.XML.matrix <- function(label, mode="real", rows=2, columns=2, min=NULL, max=NULL,
	allow_missings=FALSE, allow_user_resize_columns=TRUE,
	allow_user_resize_rows=TRUE, fixed_width=FALSE, fixed_height=FALSE,
	horiz_headers=NULL, vert_headers=NULL, id.name="auto"){
	if(identical(id.name, "auto")){
		# try autogenerating some id
		id.name <- auto.ids(label, prefix=ID.prefix("matrix"), chars=10)
	} else if(is.null(id.name)){
		stop(simpleError("Matrices need an ID!"))
	} else {}
	attr.list <- list(id=check.ID(id.name), label=label)

	if(!mode %in% c("integer", "real", "string")){
		stop(simpleError(paste("Invalid mode: ", mode, sep="")))
	} else {
		attr.list[["mode"]] <- mode
	}

	if(mode %in% c("string")){
		if(isTRUE(allow_missings)){
			attr.list[["allow_missings"]] <- "true"
		} else {}
	} else {}

	if(mode %in% c("integer", "real")){
		if(!is.null(min)){
			attr.list[["min"]] <- min
		} else {}
		if(!is.null(max)){
			attr.list[["max"]] <- max
		} else {}
	} else {}

	if(!isTRUE(allow_user_resize_rows)){
		attr.list[["allow_user_resize_rows"]] <- "false"
		if(rows != 2){
			attr.list[["rows"]] <- rows
		} else {}
	} else {}

	if(!isTRUE(allow_user_resize_columns)){
		attr.list[["allow_user_resize_columns"]] <- "false"
		if(columns != 2){
			attr.list[["columns"]] <- columns
		} else {}
	} else {}

	if(isTRUE(fixed_width)){
		attr.list[["fixed_width"]] <- "true"
	} else {}
	if(isTRUE(fixed_height)){
		attr.list[["fixed_height"]] <- "true"
	} else {}
	
	if(!is.null(horiz_headers)){
		if(is.character(horiz_headers)){
			attr.list[["horiz_headers"]] <- paste(horiz_headers, sep=";")
		} else {
			stop(simpleError("'horiz_headers' must be a character vector!"))
		}
	} else {}

	if(!is.null(vert_headers)){
		if(is.character(vert_headers)){
			attr.list[["vert_headers"]] <- paste(vert_headers, sep=";")
		} else {
			stop(simpleError("'vert_headers' must be a character vector!"))
		}
	} else {}

	node <- XMLNode("matrix", attrs=attr.list)

	return(node)
}
