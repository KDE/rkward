#' Create XML node "preview" for RKWard plugins
#'
#' @param label A character string, text label for the preview checkbox.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.preview <- rk.XML.preview("See a preview?")
#' cat(pasteXML(test.preview))

rk.XML.preview <- function(label="Preview"){
	if(!identical(label, "Preview")){
		attr.list <- list(label=label)
	} else {
		attr.list <- list()
	}

	node <- XMLNode("preview", attrs=attr.list)

	return(node)
}
