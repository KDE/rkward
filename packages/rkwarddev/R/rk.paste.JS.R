#' Paste objects of class rk.JS.ite
#'
#' @param object An object of class \code{rk.JS.ite}
#' @param level Integer, which indentation level to use, minimum is 1.
#' @param indent.by A character string defining the indentation string to use.
#' @return A character string.
#' @include rk.JS.ite-class.R
#' @export

rk.paste.JS <- function(object, level=1, indent.by="\t"){
	stopifnot(level > 0)
	# check indentation
	main.indent <- paste(rep(indent.by, level-1), collapse="")
	scnd.indent <- paste(rep(indent.by, level), collapse="")

	if(inherits(object, "rk.JS.ite")){
		ifJS <- paste(main.indent, "if(", object@ifJS, ") {\n", sep="")
		thenJS <- paste(scnd.indent, object@thenJS, "\n", main.indent, "}", sep="")
		if(!is.null(object@elseJS)) {
			elseJS <- paste(" else {\n", scnd.indent, object@elseJS, "\n", main.indent, "}", sep="")
		} else {
			elseJS <- NULL
		}
		result <- paste(ifJS, thenJS, elseJS, collapse="", sep="")
	} else {
		result <- paste(object)
	}

	return(result)
}
