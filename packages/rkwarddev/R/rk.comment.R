#' Create comment for RKWard plugin code
#'
#' @param text Character string, the text to be displayed.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @examples
#' test.comment <- rk.comment("Added this text.")
#' cat(pasteXMLNode(test.comment))

rk.comment <- function(text){
	node <- new("XiMpLe.node",
			name="!--",
			value=text)

	return(node)
}
