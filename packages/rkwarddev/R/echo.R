#' Generate JavaScript echo command call
#' 
#' This function will take several elements, either character strings, or objects of class \code{XiMpLe.node}
#' which hold an XML node of some plugin GUI definition, or objects of classes \code{rk.JS.arr} or \code{rk.JS.opt}.
#' From those, it will generate a ready-to-run JavaScript \code{echo();} call from it.
#' 
#' @param ... One or several character strings and/or \code{XiMpLe.node} objects with plugin nodes,
#'		and/or objects of classes \code{rk.JS.arr} or \code{rk.JS.opt}, simply separated by comma.
#' @param newline Character string, can be set to e.g. \code{"\n"} to force a newline after the call.
#' @return A character string.
#' @seealso \code{\link[rkwarddev:rk.JS.vars]{rk.JS.vars}},
#'		\code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
#'		\code{\link[rkwarddev:rk.JS.options]{rk.JS.options}},
#'		\code{\link[rkwarddev:ite]{ite}},
#'		\code{\link[rkwarddev:id]{id}},
#'		\code{\link[rkwarddev:id]{qp}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export
#' @examples
#' cbox1 <- rk.XML.cbox(label="foo", value="foo1", id.name="CheckboxFoo.ID")
#' echo("bar <- \"", cbox1, "\"")

echo <- function(..., newline=""){
	ID.content <- qp(...)
	result <- paste("echo(", ID.content, ");", newline, sep="")
	return(result)
}

## internal class rk.JS.echo
# this is a quick fix to be able to add values into echo() without quotes
setClass("rk.JS.echo",
	representation=representation(
		value="character"
	),
	prototype(
		value=character()
	)
)
