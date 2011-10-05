#' Generate JavaScript if/then/else constructs
#' 
#' @param ifjs A character string, to be placed in the brackets if an \code{if()} statement.
#' @param thenjs A character string, the code to be executed in case the \code{if()} statement is true.
#' @param elsejs A character string, the code to be executed in case the \code{if()} statement is not true.
#' @return An object of class \code{rk.JS.ite}
#' @include rk.JS.ite-class.R
#' @seealso \code{\link[rkwarddev:rk.paste.JS]{rk.paste.JS}},
#'		\code{\link[rkwarddev:rk.JS.vars]{rk.JS.vars}},
#'		\code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
#'		\code{\link[rkwarddev:echo]{echo}},
#'		\code{\link[rkwarddev:id]{id}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export
#' @examples
#' # first create an example checkbox XML node
#' cbox1 <- rk.XML.cbox(label="foo", value="foo1", id.name="CheckboxFoo.ID")
#' # now some JavaScript generation
#' ite(cbox1, echo("bar <- \"", cbox1, "\""), echo("bar <- NULL"))

ite <- function(ifjs, thenjs, elsejs=NULL){
	#check for recursion
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
		ifJS=id(ifjs),
		thenJS=thenjs,
		elseJS=elsejs,
		elifJS=elifJS
	)
	return(result)
}
