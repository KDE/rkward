#' Create XML hierarchy section for RKWard plugins
#'
#' This function will create a hierarchy section with optional child nodes "menu".
#'
#' @note NOT WORKING YET
#'
#' @param nodes A (list of) objects of class \code{XiMpLe.node}. 
#' @return A list of objects of class \code{XiMpLe.node}.
#' @export
# @examples
# # define an input field and two checkboxes
# test.input <- rk.XML.input("Type some text")
# test.cbox1 <- rk.XML.cbox(label="Want to type?", val="true")
# test.cbox2 <- rk.XML.cbox(label="Are you shure?", val="true")
# test.hierarchy <- rk.XML.hierarchy(rk.XML.col(list(test.input, test.cbox1, test.cbox2)))
# cat(pasteXMLNode(test.hierarchy))

rk.XML.hierarchy <- function(nodes){
	# check the node names and allow only valid ones
	node.names <- sapply(child.list(nodes), function(this.node){
			this.node@name
		})

	invalid.sets <- !node.names %in% c("menu")
	if(any(invalid.sets)){
		stop(simpleError(paste("Invalid XML nodes for hierarchy section: ", paste(node.names[invalid.sets], collapse=", "), sep="")))
	} else {}

	node <- new("XiMpLe.node",
			name="hierarchy",
			children=child.list(nodes)
		)

	return(node)
}
