#' Generate JavaScript echo command call
#' 
#' This function will take several elements, either character strings or objects of class \code{XiMpLe.node}
#' which hold an XML node of some plugin GUI definition, and generate a ready-to-run JavaScript \code{echo();}
#' call from it.
#' 
#' @param ... One or several character strings and/or \code{XiMpLe.node} objects with plugin nodes,
#'		simply separated by comma.
#' @param newline Character string, can be set to e.g. \code{"\n"} to force a newline after the call.
#' @return A character string.
#' @export

echo <- function(..., newline=""){
	ID.content <- id(..., quote=TRUE,  collapse=" + ")
	result <- paste("echo(", ID.content, ");", newline, sep="")
	return(result)
}
