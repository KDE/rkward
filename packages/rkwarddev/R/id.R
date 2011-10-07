#' Replace XiMpLe.node objects with their ID value
#' 
#' This function is intended to be used for generating JavaScript code for
#' RKWard plugins. Its sole purpose is to replace objects of class \code{XiMpLe.node}
#' which hold an XML node of some plugin GUI definition, and objects of classes \code{rk.JS.arr} or \code{rk.JS.opt}
#' with their ID (or JS variable name), and combine these replacements with character strings.
#' 
#' @param ... One or several character strings and/or \code{XiMpLe.node} objects with plugin nodes,
#' 	and/or objects of classes \code{rk.JS.arr} or \code{rk.JS.opt}, simply separated by comma.
#' @param quote Logical, it the character strings sould be deparsed, so they come out "as-is" when
#'		written to files, e.g. by \code{cat}.
#' @param collapse Character string, defining if and how the individual elements should be glued together.
#' @return A character string.
#' @export
#' @seealso \code{\link[rkwarddev:rk.JS.vars]{rk.JS.vars}},
#'		\code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
#'		\code{\link[rkwarddev:rk.JS.options]{rk.JS.options}},
#'		\code{\link[rkwarddev:echo]{echo}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # an example checkbox XML node
#' cbox1 <- rk.XML.cbox(label="foo", value="foo1", id.name="CheckboxFoo.ID")
#' id("The variable name is: ", cbox1, "!", quote=TRUE, collapse=" + ")

id <- function(..., quote=FALSE, collapse=""){
	full.content <- list(...)
	ID.content <- sapply(full.content, function(this.part){
			if(inherits(this.part, "XiMpLe.node")){
				node.id <- camelCode(this.part@attributes$id)
				return(node.id)
			} else if(inherits(this.part, "rk.JS.arr") | inherits(this.part, "rk.JS.opt")){
				node.id <- this.part@opt.name
				return(node.id)
			}else {
				if(isTRUE(quote)){
					text.part <- deparse(this.part)
				} else {
					text.part <- this.part
				}
				return(text.part)
			}
		})
	result <- paste(ID.content, collapse=collapse)
	return(result)
}
