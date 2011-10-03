#' Define variables in JavaScript code
#' 
#' @param variables A list with either character strings (the names of the variables to define),
#'		of objects of class \code{XiMpLe.node} with plugin XML nodes (whose ID will be extracted and used).
#' @param var.prefix A character string. If \code{def.vars=TRUE}, this string will be used as a prefix
#'		for the JS variable names.
#' @param level Integer, which indentation level to use, minimum is 1.
#' @param indent.by A character string defining how indentation should be done.
#' @return A character string.
#' @export
#' @seealso \code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
#'		\code{\link[rkwarddev:echo]{echo}},
#'		\code{\link[rkwarddev:id]{id}}
#' @examples
#' # create three checkboxes
#' checkA <- rk.XML.cbox(label="Run Test A", value="A")
#' checkB <- rk.XML.cbox(label="Run Test B", value="B")
#' checkC <- rk.XML.cbox(label="Run Test C", value="C")
#' # define them by their ID in JavaScript
#' cat(rk.JS.vars(list(checkA, checkB, checkC)))

rk.JS.vars <- function(variables, var.prefix=NULL, level=2, indent.by="\t"){
	indent.by <- indent(level, by=indent.by)

	JS.vars <- paste(unlist(sapply(child.list(variables), function(this.var){get.JS.vars(
						JS.var=this.var,
						XML.var=this.var,
						JS.prefix=var.prefix,
						indent.by=indent.by)
				})), collapse="")

	return(JS.vars)
}
