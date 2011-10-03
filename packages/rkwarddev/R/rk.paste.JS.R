#' Paste objects of class rk.JS.ite
#'
#' @param object An object of class \code{rk.JS.ite} or \code{rk.JS.arr}
#' @param level Integer, which indentation level to use, minimum is 1.
#' @param indent.by A character string defining the indentation string to use.
#' @param funct For \code{rk.JS.arr} objects only: Character string, name of the R function
#' 	to be called to combine the options, e.g. "list" for \code{list()}, or "c" for \code{c()}.
#' @return A character string.
#' @include rk.JS.ite-class.R
#' @include rk.JS.arr-class.R
#' @seealso
#'		\code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
#'		\code{\link[rkwarddev:ite]{ite}}
#' @export

rk.paste.JS <- function(object, level=1, indent.by="\t", funct=NULL){
	stopifnot(level > 0)

	if(inherits(object, "rk.JS.ite")){
		# done by an internal function, to ease handling of recursions
		result <- paste.JS.ite(object, level=level, indent.by=indent.by)
	} else if(inherits(object, "rk.JS.arr")){
		# done by an internal function, to ease handling of recursions
		result <- paste.JS.array(object, level=level, indent.by=indent.by, funct=funct)
	} else {
		result <- paste(object)
	}

	return(result)
}
