#' Create XML page node for RKWard plugins
#'
#' This function will create a page node for wizard sections, with optional child nodes "browser", "checkbox",
#' "column", "copy", "dropdown", "formula", "frame", "input", "page", "radio", "row", "saveobject",
#' "spinbox", "stretch", "tabbook", "text", "varselector" and "varslot".
#'
#' @param nodes A (list of) objects of class \code{XiMpLe.node}.
#' @param id.name Character string, a unique ID for this plugin element.
#' @return A list of objects of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.wizard]{rk.XML.wizard}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # define a checkbox for the actual dialog
#' test.cbox1 <- rk.XML.cbox(label="More than 30 subjects", val="true")
#' # define the wizard
#' test.text <- rk.XML.text("Did you test more than 30 subjects?")
#' test.copy <- rk.XML.copy(id.name=test.cbox1)
#' test.wizard <- rk.XML.wizard(rk.XML.page(list(test.text, test.copy)))
#' cat(pasteXMLNode(test.wizard))

rk.XML.page <- function(nodes, id.name=NULL){
	# check the node names and allow only valid ones
	node.names <- sapply(child.list(nodes), function(this.node){
			this.node@name
		})

	invalid.sets <- !node.names %in% c("browser", "checkbox", "column", "copy",
		"dropdown", "formula", "frame", "input", "page", "radio", "row", "saveobject",
		"spinbox", "stretch", "tabbook", "text", "varselector", "varslot")
	if(any(invalid.sets)){
		stop(simpleError(paste("Invalid XML nodes for page/wizard section: ", paste(node.names[invalid.sets], collapse=", "), sep="")))
	} else {}

	node <- new("XiMpLe.node",
			name="page",
			attributes=list(id=id.name),
			children=child.list(nodes)
		)

	return(node)
}
