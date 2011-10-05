#' Combine several options in one JavaScript variable
#' 
#' @param opt.name Character string, name of the JavaScript variable to use in the script code.
#' @param ... A list of objects of class \code{rk.JS.ite} (see \code{\link[rkwarddev:ite]{ite}}).
#'		Use the \code{thenjs} element only to define the value to add to the option, without commas
#'		or similar (e.g., \code{"paired=TRUE"} or \code{id("conf.level=", conflevel)}.
#' @return An object of class \code{rk.JS.opt}, use \code{\link[rkwarddev:rk.paste.JS]{rk.paste.JS}}
#'		on that.
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export

rk.JS.options <- function(opt.name, ...){
	all.opts <- list(...)
	result <- new("rk.JS.opt",
		option=opt.name,
		ifs=all.opts)

	return(result)
}