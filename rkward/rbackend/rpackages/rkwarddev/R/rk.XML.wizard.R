#' Create XML wizard section for RKWard plugins
#'
#' This function will create a wizard section with optional child nodes "browser", "checkbox",
#' "column", "copy", "dropdown", "embed", "formula", "frame", "include", "input", "insert",
#' "page", "preview", "radio", "row", "saveobject", "spinbox", "stretch", "tabbook", "text", "varselector" and "varslot".
#'
#' @param nodes A (list of) objects of class \code{XiMpLe.node}
#' @param label Character string, a text label for this plugin element.
#' @return A list of objects of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.plugin]{rk.XML.plugin}},
#'		\code{\link[rkwarddev:rk.plugin.skeleton]{rk.plugin.skeleton}}
#' @examples
#' # define a checkbox for the actual dialog
#' test.cbox1 <- rk.XML.cbox(label="More than 30 subjects", val="true")
#' # define the wizard
#' test.text <- rk.XML.text("Did you test more than 30 subjects?")
#' test.copy <- rk.XML.copy(id=test.cbox1)
#' test.wizard <- rk.XML.wizard(rk.XML.page(list(test.text, test.copy)))
#' cat(pasteXMLNode(test.wizard))

rk.XML.wizard <- function(nodes, label=NULL){
	# check the node names and allow only valid ones
	node.names <- sapply(child.list(nodes), function(this.node){
			this.node@name
		})

	invalid.sets <- !node.names %in% c("browser", "checkbox", "column", "copy",
		"dropdown", "embed", "formula", "frame", "include", "input", "insert",
		"page", "preview", "radio", "row", "saveobject", "spinbox", "stretch",
		"tabbook", "text", "varselector", "varslot")
	if(any(invalid.sets)){
		stop(simpleError(paste("Invalid XML nodes for wizard section: ", paste(node.names[invalid.sets], collapse=", "), sep="")))
	} else {}

	node <- new("XiMpLe.node",
			name="wizard",
			attributes=list(label=label),
			children=child.list(nodes)
		)

	return(node)
}
