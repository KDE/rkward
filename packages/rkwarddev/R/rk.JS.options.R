#' Combine several options in one JavaScript variable
#' 
#' @param var Character string, name of the JavaScript variable to use in the script code.
#' @param ... A list of objects of class \code{rk.JS.ite} (see \code{\link[rkwarddev:ite]{ite}}).
#'		Use the \code{thenjs} element to define only the value to add to the option, without commas
#'		(e.g., \code{"paired=TRUE"} or \code{qp("conf.level=\"", conflevel, "\"")}.
#' @param collapse Character string, how all options should be concatenated on the R code level
#'		(if \code{array=FALSE}), or how \code{option} should be added to the generated R code. Hint:
#'		To place each option in a new line with tab indentation, set \code{collapse=",\\n\\t"}.
#' @param option A character string, naming, e.g., an option of an R function which should be
#'		constructed from several variables. Only used if \code{array=TRUE}.
#' @param funct Character string, name of the R function to be called to combine the options,
#'		e.g. "list" for \code{list()}, or "c" for \code{c()}. Set to \code{NULL} to drop.
#'		Only used if \code{array=TRUE}.
#' @param array Logical, if \code{TRUE} will generate the options as an array, otherwise in one
#'		concatenated character string (probably only useful for mandatory options).
#' @return An object of class \code{rk.JS.opt}, use \code{\link[rkwarddev:rk.paste.JS]{rk.paste.JS}}
#'		on that.
#' @seealso
#'		\code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export
#' @examples
#' # create two checkboxes for independent options
#' checkA <- rk.XML.cbox(label="Run Test A", value="A")
#' checkB <- rk.XML.cbox(label="Run it fast", value="true")
#' # combine them into one JavaScript options variable
#' rk.JS.options("optionsTestA",
#'   ite(checkA, qp("test=\"", checkA, "\"")),
#'   ite(checkB, "fast=TRUE")
#' )

rk.JS.options <- function(var, ..., collapse=", ", option=NULL, funct=NULL, array=TRUE){
	all.opts <- list(...)

	if(is.null(option)){
		option <- ""
	} else {}
	if(is.null(funct)){
		funct <- ""
	} else {}

	result <- new("rk.JS.opt",
		var.name=var,
		opt.name=option,
		collapse=collapse,
		ifs=all.opts,
		array=array,
		funct=funct)

	return(result)
}