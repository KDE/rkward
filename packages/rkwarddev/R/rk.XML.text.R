#' Create XML node "text" for RKWard plugins
#'
#' @param text Character string, the text to be displayed.
#' @param type One value of either "normal", "warning" or "error".
#' @param id.name Character string, a unique ID for this plugin element.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.text <- rk.XML.text("Added this text.")
#' cat(pasteXMLNode(test.text))

rk.XML.text <- function(text, type="normal", id.name=NULL){
	if(!is.null(id.name)){
		attr.list <- list(id=id.name)
	} else {
		attr.list <- list()
	}

	if(identical(type, "warning") | identical(type, "error")){
		attr.list[["type"]] <- type
	} else {}

	node <- new("XiMpLe.node",
			name="text",
			attributes=attr.list,
			value=text)

	return(node)
}
