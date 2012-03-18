#' Create comment for RKWard plugin code
#'
#' @param text Character string, the text to be displayed.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @examples
#' test.comment <- rk.comment("Added this text.")
#' cat(pasteXML(test.comment))

rk.comment <- function(text){
	return(XMLNode(name="!--", text))
}
