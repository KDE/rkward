#' Create XML node "embed" for RKWard plugins
#'
#' @param component A character string, registered name (\code{id} in pluginmap file) of the component to be embedded.
#' @param button Logical, whether the plugin should be embedded as a button and appear if it's pressed.
#' @param label A character string, text label for the button (only used if \code{button=TRUE}).
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.embed <- rk.XML.embed("someComponent")
#' cat(pasteXMLNode(test.embed))

rk.XML.embed <- function(component, button=FALSE, label="Options"){
	attr.list <- list(component=component)

	if(isTRUE(button)){
		attr.list[["as_button"]] <- "true"
	} else {}
	
	if(!identical(label, "Options")){
		attr.list[["label"]] <- label
	} else {}

	node <- new("XiMpLe.node",
			name="embed",
			attributes=attr.list
		)

	return(node)
}
