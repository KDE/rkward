#' Create XML logic section for RKWard plugins
#'
#' This function will create a logic section with "convert", "connect" ,"external" and "set" nodes.
#'
#' @param nodes A list of objects of class \code{XiMpLe.node}. 
#' @return A list of objects of class \code{XiMpLe.node}.
#' @export
#' @examples
#' test.convert <- rk.XML.convert(c(string="foo"), mode=c(notequals="bar"), id.name="lgc_foobar")
#' test.connect <- rk.XML.connect(governor="lgc_foobar", client="frame_bar")
#' test.external <- rk.XML.external(id="ext_property", default="none")
#' test.logic <- rk.XML.logic(nodes=list(test.convert, test.connect, test.external))
#' cat(pasteXMLNode(test.logic, shine=1))

rk.XML.logic <- function(nodes){
	# check the node names and allow only valid ones
	node.names <- sapply(nodes, function(this.node){
			this.node@name
		})

	invalid.sets <- !node.names %in% c("connect", "convert","external","set")
	if(any(invalid.sets)){
		stop(simpleError(paste("Invalid XML nodes for logic section: ", paste(invalid.sets, collapse=", "), sep="")))
	} else {}

	node <- new("XiMpLe.node",
			name="logic",
			children=child.list(nodes)
		)

	return(node)
}
