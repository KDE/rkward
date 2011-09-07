#' Create XML node "text" for RKWard plugins
#'
#' @param id.name Character string, a unique ID for this plugin element.
#' @return An object of class \code{XiMpLe.node}.
#' @export

#<text id="TTtext">The TreeTagger folder is the one containing the bin, cmd and lib folders</text>
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
