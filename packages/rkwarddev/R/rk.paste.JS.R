#' Paste JavaScript objects and character strings
#'
#' @param ... Objects of class \code{rk.JS.ite}, \code{rk.JS.arr}, \code{rk.JS.opt} or character.
#' @param level Integer, which indentation level to use, minimum is 1.
#' @param indent.by A character string defining the indentation string to use.
#' @param funct For \code{rk.JS.arr} objects only: Character string, name of the R function
#' 	to be called to combine the options, e.g. "list" for \code{list()}, or "c" for \code{c()}.
#' @return A character string.
#' @include rk.JS.ite-class.R
#' @include rk.JS.arr-class.R
#' @include rk.JS.opt-class.R
#' @seealso
#'		\code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
#'		\code{\link[rkwarddev:rk.JS.options]{rk.JS.options}},
#'		\code{\link[rkwarddev:ite]{ite}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export

rk.paste.JS <- function(..., level=1, indent.by="\t", funct=NULL){
	stopifnot(level > 0)
	all.objects <- list(...)
	
	paste.results <- paste(sapply(all.objects, function(this.object){
		if(inherits(this.object, "rk.JS.ite")){
			# done by an internal function, to ease handling of recursions
			result <- paste.JS.ite(this.object, level=level, indent.by=indent.by)
		} else if(inherits(this.object, "rk.JS.arr")){
			# done by an internal function, to ease handling of recursions
			result <- paste.JS.array(this.object, level=level, indent.by=indent.by, funct=funct)
		} else if(inherits(this.object, "rk.JS.opt")){
			result <- paste.JS.options(this.object, level=level, indent.by=indent.by)
		} else {
			result <- paste(indent(level, by=indent.by), this.object, sep="")
		}
		return(result)
	}), collapse="\n")

	return(paste.results)
}
