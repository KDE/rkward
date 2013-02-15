#' Create XML node "optiondisplay" for RKWard plugins
#' 
#' This node is only allowed once insinde the \code{<content>} node of an \code{<optionset>}.
#'
#' @param index Logical, whether to show a column with a numeric index in the optiondisplay.
#' @param id.name Character string, a unique ID for this plugin element (optional).
#' @return An object of class \code{XiMpLe.node}.
#' @export
rk.XML.optiondisplay <- function(index=TRUE, id.name=NULL){

	if(is.null(id.name)){
		attr.list <- list()
	} else {
		attr.list <- list(id=id.name)
	}

	if(!isTRUE(index)){
		attr.list[["index"]] <- "false"
	} else {}

	node <- XMLNode("optiondisplay",
		attrs=attr.list)

	return(node)
}
