#' Create XML node "code" for RKWard plugins
#'
#' @param file A character string, the JavaScript file name to be included.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.code <- rk.XML.code("some_file.js")
#' cat(pasteXMLNode(test.code))

rk.XML.code <- function(file){
	node <- new("XiMpLe.node",
			name="code",
			attributes=list(file=as.character(file))
		)

	return(node)
}
