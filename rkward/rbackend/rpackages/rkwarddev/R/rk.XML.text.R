#' Create XML node "text" for RKWard plugins
#'
#' @param id.name Character string, a unique ID for this plugin element.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @examples
#' test.text <- rk.XML.text("Added this text.")
#' cat(pasteXMLNode(test.text, shine=1))

rk.XML.text <- function(text, id.name=NULL){
	if(!is.null(id.name)){
		attr.list <- list(id=id.name)
	} else {
		attr.list <- list()
	}

	node <- new("XiMpLe.node",
			name="text",
			attributes=attr.list,
			value=text)

	return(node)
}
