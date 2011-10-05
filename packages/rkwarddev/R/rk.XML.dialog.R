#' Create XML dialog section for RKWard plugins
#'
#' This function will create a dialog section with optional child nodes "browser", "checkbox",
#' "column", "copy", "dropdown", "embed", "formula", "frame", "include", "input", "insert",
#' "preview", "radio", "row", "saveobject", "spinbox", "stretch", "tabbook", "text", "varselector" and "varslot".
#'
#' @param nodes A (list of) objects of class \code{XiMpLe.node}. 
#' @param label Character string, a text label for this plugin element.
#' @return A list of objects of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.plugin]{rk.XML.plugin}},
#'		\code{\link[rkwarddev:rk.plugin.skeleton]{rk.plugin.skeleton}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # define an input field and two checkboxes
#' test.input <- rk.XML.input("Type some text")
#' test.cbox1 <- rk.XML.cbox(label="Want to type?", val="true")
#' test.cbox2 <- rk.XML.cbox(label="Are you shure?", val="true")
#' test.dialog <- rk.XML.dialog(rk.XML.col(list(test.input, test.cbox1, test.cbox2)))
#' cat(pasteXMLNode(test.dialog))

rk.XML.dialog <- function(nodes, label=NULL){
	# check the node names and allow only valid ones
	node.names <- sapply(child.list(nodes), function(this.node){
			this.node@name
		})

	invalid.sets <- !node.names %in% c("browser", "checkbox", "column", "copy",
		"dropdown", "embed", "formula", "frame", "include", "input", "insert",
		"preview", "radio", "row", "saveobject", "spinbox", "stretch", "tabbook",
		"text", "varselector", "varslot")
	if(any(invalid.sets)){
		stop(simpleError(paste("Invalid XML nodes for dialog section: ", paste(node.names[invalid.sets], collapse=", "), sep="")))
	} else {}

	node <- new("XiMpLe.node",
			name="dialog",
			attributes=list(label=label),
			children=child.list(nodes)
		)

	return(node)
}
