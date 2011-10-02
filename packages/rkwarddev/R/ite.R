#' Generate JavaScript if/then/else constructs
#' 
#' @param ifjs A character string, to be placed in the brackets if an \code{if()} statement.
#' @param thenjs A character string, the code to be executed in case the \code{if()} statement is true.
#' @param elsejs A character string, the code to be executed in case the \code{if()} statement is not true.
#' @return An object of class \code{rk.JS.ite}
#' @include rk.JS.ite-class.R
#' @export

ite <- function(ifjs, thenjs, elsejs=NULL){
	result <- new("rk.JS.ite",
		ifJS=ifjs,
		thenJS=thenjs,
		elseJS=elsejs
	)
	return(result)
}
