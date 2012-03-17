#' Create XML "page" node for RKWard plugins
#'
#' This function will create a page node for wizard sections, with optional child nodes "browser", "checkbox",
#' "column", "copy", "dropdown", "formula", "frame", "input", "page", "radio", "row", "saveobject",
#' "spinbox", "stretch", "tabbook", "text", "varselector" and "varslot".
#'
#' @param ... Objects of class \code{XiMpLe.node}.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"}, an ID will be generated automatically from the objects in \code{...}.
#'		If \code{NULL}, no ID will be given.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.wizard]{rk.XML.wizard}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # define a checkbox for the actual dialog
#' test.cbox1 <- rk.XML.cbox(label="More than 30 subjects", val="true")
#' # define the wizard
#' test.text <- rk.XML.text("Did you test more than 30 subjects?")
#' test.copy <- rk.XML.copy(id=test.cbox1)
#' test.wizard <- rk.XML.wizard(rk.XML.page(test.text, test.copy))
#' cat(pasteXML(test.wizard))

rk.XML.page <- function(..., id.name="auto"){
	nodes <- list(...)

	# check the node names and allow only valid ones
	node.names <- sapply(child.list(nodes), function(this.node){
			this.node@name
		})

	invalid.sets <- !node.names %in% c("browser", "checkbox", "column", "copy",
		"dropdown", "formula", "frame", "input", "page", "radio", "row", "saveobject",
		"spinbox", "stretch", "tabbook", "text", "varselector", "varslot", "!--")
	if(any(invalid.sets)){
		stop(simpleError(paste("Invalid XML nodes for page/wizard section: ", paste(node.names[invalid.sets], collapse=", "), sep="")))
	} else {}

	if(identical(id.name, "auto")){
		# try autogenerating some id
		attr.list <- list(id=auto.ids(node.soup(nodes), prefix=ID.prefix("page"), chars=10))
	} else if(is.null(id.name)){
		attr.list <- list()
	} else {
		attr.list <- list(id=id.name)
	}

	node <- new("XiMpLe.node",
			name="page",
			attributes=attr.list,
			children=child.list(nodes),
			value=""
		)

	return(node)
}
