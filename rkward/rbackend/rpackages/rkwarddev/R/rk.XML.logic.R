#' Create XML logic section for RKWard plugins
#'
#' This function will create a logic section with "convert", "connect", "include", "insert", "external" and "set" nodes.
#'
#' @param nodes A list of objects of class \code{XiMpLe.node}. 
#' @return A list of objects of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.convert]{rk.XML.convert}},
#'		\code{\link[rkwarddev:rk.XML.connect]{rk.XML.connect}},
#'		\code{\link[rkwarddev:rk.XML.external]{rk.XML.external}},
#'		\code{\link[rkwarddev:rk.XML.set]{rk.XML.set}}
#' @examples
#' # define an input field and two checkboxes
#' test.input <- rk.XML.input("Type some text")
#' test.cbox1 <- rk.XML.cbox(label="Want to type?", value="true")
#' test.cbox2 <- rk.XML.cbox(label="Are you shure?", value="true")
#' # now create some logic so that the input field is only enabled when both boxes are checked
#' test.convert <- rk.XML.convert(c(state=test.cbox1,state=test.cbox2), mode=c(and=""))
#' test.connect <- rk.XML.connect(governor=test.convert, client=test.input, set="enabled")
#' test.logic <- rk.XML.logic(nodes=list(test.convert, test.connect))
#' cat(pasteXMLNode(test.logic))
#' 
#' # with only one checkbox, you can directly query if it's checked
#' test.connect2 <- rk.XML.connect(governor=test.cbox1, client=test.input, set="enabled")
#' test.logic2 <- rk.XML.logic(nodes=list(test.connect2))
#' cat(pasteXMLNode(test.logic2))

rk.XML.logic <- function(nodes){
	# check the node names and allow only valid ones
	node.names <- sapply(child.list(nodes), function(this.node){
			this.node@name
		})

	invalid.sets <- !node.names %in% c("connect", "convert","include","insert","external","set")
	if(any(invalid.sets)){
		stop(simpleError(paste("Invalid XML nodes for logic section: ", paste(node.names[invalid.sets], collapse=", "), sep="")))
	} else {}

	node <- new("XiMpLe.node",
			name="logic",
			children=child.list(nodes)
		)

	return(node)
}
