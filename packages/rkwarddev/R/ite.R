#' Generate JavaScript if/then/else constructs
#' 
#' @param ifjs Either a character string to be placed in the brackets if an \code{if()} statement,
#'		or an object of class \code{XiMpLe.node}. \code{rk.JS.arr} or \code{rk.JS.opt} (whose identifier will be used).
#' @param thenjs Either a character string, the code to be executed in case the \code{if()} statement is true,
#'		or an object of class \code{XiMpLe.node}. \code{rk.JS.arr} or \code{rk.JS.opt} (whose identifier will be used).
#'		The latter is especially useful in combination with \code{\link[rkwarddev:rk.JS.options]{rk.JS.options}}.
#'		You can also give another object of class \code{rk.JS.ite}.
#' @param elsejs Like \code{thenjs}, the code to be executed in case the \code{if()} statement is not true.
#' @return An object of class \code{rk.JS.ite}
#' @include rk.JS.ite-class.R
#' @seealso \code{\link[rkwarddev:rk.paste.JS]{rk.paste.JS}},
#'		\code{\link[rkwarddev:rk.JS.vars]{rk.JS.vars}},
#'		\code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
#'		\code{\link[rkwarddev:rk.JS.options]{rk.JS.options}},
#'		\code{\link[rkwarddev:echo]{echo}},
#'		\code{\link[rkwarddev:id]{id}},
#'		\code{\link[rkwarddev:qp]{qp}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export
#' @examples
#' # first create an example checkbox XML node
#' cbox1 <- rk.XML.cbox(label="foo", value="foo1", id.name="CheckboxFoo.ID")
#' # now some JavaScript generation
#' ite(cbox1, echo("bar <- \"", cbox1, "\""), echo("bar <- NULL"))

ite <- function(ifjs, thenjs, elsejs=NULL){
	#check for recursion
	if(inherits(thenjs, "rk.JS.ite")){
		thenifJS <- list(thenjs)
		thenjs <- ""
	} else {
		thenifJS <- list()
	}
	if(inherits(elsejs, "rk.JS.ite")){
		elifJS <- list(elsejs)
		elsejs <- ""
	} else {
		elifJS <- list()
		if(is.null(elsejs)){
			elsejs <- ""
		} else {}
	}
	result <- new("rk.JS.ite",
		ifJS=id(ifjs, js=TRUE),
		thenJS=id(thenjs, js=TRUE),
		thenifJS=thenifJS,
		elseJS=elsejs,
		elifJS=elifJS
	)
	return(result)
}
