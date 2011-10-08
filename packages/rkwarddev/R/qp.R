#' Replace XiMpLe.node objects with their ID value
#' 
#' This function is a shortcut for \code{\link[rkwarddev:id]{id}} which sets some useful defaults
#' (\code{quote=TRUE, collapse=" + ", js=TRUE}). The abbreviation stands for "quote + plus".
#' 
#' @param ... One or several character strings and/or \code{XiMpLe.node} objects with plugin nodes,
#' 	and/or objects of classes \code{rk.JS.arr} or \code{rk.JS.opt}, simply separated by comma.
#' @return A character string.
#' @export
#' @seealso \code{\link[rkwarddev:rk.JS.vars]{rk.JS.vars}},
#'		\code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
#'		\code{\link[rkwarddev:rk.JS.options]{rk.JS.options}},
#'		\code{\link[rkwarddev:echo]{echo}},
#'		\code{\link[rkwarddev:id]{id}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # an example checkbox XML node
#' cbox1 <- rk.XML.cbox(label="foo", value="foo1", id.name="CheckboxFoo.ID")
#' qp("The variable name is: ", cbox1, "!")

qp <- function(...){
	result <- id(..., quote=TRUE, collapse=" + ", js=TRUE)
	return(result)
}
