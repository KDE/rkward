#' Define variables in JavaScript code
#' 
#' @param ... Either one or more character strings (the names of the variables to define),
#'		or objects of class \code{XiMpLe.node} with plugin XML nodes (whose ID will be extracted and used).
#' @param var.prefix A character string. will be used as a prefix for the JS variable names.
#' @param properties A character vector with properties you'd like to get of the XML node.
#' @param default Logical, if \code{TRUE} the default value (no special property) of the node will
#'		also be defined. Does nothing if \code{properties=NULL}.
#' @param join A character string, useful for GUI elements which accept multiple objects (i.e., multi-varslots).
#'		If \code{join} is something other than \code{""}, these objects will be collapsed into one string when pasted,
#'		joined by this string.
#' @return An object of class \code{rk.JS.var}.
#' @export
#' @seealso \code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
#'		\code{\link[rkwarddev:echo]{echo}},
#'		\code{\link[rkwarddev:id]{id}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # create three checkboxes
#' checkA <- rk.XML.cbox(label="Run Test A", value="A")
#' checkB <- rk.XML.cbox(label="Run Test B", value="B")
#' checkC <- rk.XML.cbox(label="Run Test C", value="C")
#' # define them by their ID in JavaScript
#' cat(rk.paste.JS(rk.JS.vars(list(checkA, checkB, checkC))))

rk.JS.vars <- function(..., var.prefix=NULL, properties=NULL, default=FALSE, join=""){
	variables <- list(...)

	JS.vars <- new("rk.JS.var",
				vars=sapply(child.list(variables), function(this.var){get.JS.vars(
						JS.var=this.var,
						XML.var=this.var,
						JS.prefix=var.prefix,
						properties=properties,
						default=default,
						join=join)
				}))

	return(JS.vars)
}
