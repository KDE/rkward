#' Create XML node "include" for RKWard plugins
#'
#' @param file A character string, the file name to be included.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.include <- rk.XML.include("../some_file.xml")
#' cat(pasteXMLNode(test.include))

rk.XML.include <- function(file){
	node <- new("XiMpLe.node",
			name="include",
			attributes=list(file=as.character(file))
		)

	return(node)
}
