#' Create XML node "preview" for RKWard plugins
#'
#' @param label A character string, text label for the preview checkbox.
#' @return A list of objects of class \code{XiMpLe.node}.
#' @export
#' @examples
#' test.preview <- rk.XML.preview("See a preview?")
#' cat(pasteXMLNode(test.preview))

rk.XML.preview <- function(label="Preview"){
	if(!identical(label, "Preview")){
		attr.list <- list(label=label)
	} else {
		attr.list <- list()
	}

	node <- new("XiMpLe.node",
			name="preview",
			attributes=attr.list
		)

	return(node)
}
