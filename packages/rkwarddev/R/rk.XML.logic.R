#' Create XML logic section for RKWard plugins
#'
#' This function will create a logic section with "convert", "connect", "include", "insert", "external" and "set" nodes.
#' You can also include JavaScript code to use the locig scripting features of RKWard, if you place it in a comment
#' with \code{\link[rkwarddev:rk.comment]{rk.comment}}: Its contents will automatically be placed inside a
#' \code{<script><![CDATA[ ]]></script>} node.
#'
#' @param ... Objects of class \code{XiMpLe.node}.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.convert]{rk.XML.convert}},
#'		\code{\link[rkwarddev:rk.XML.connect]{rk.XML.connect}},
#'		\code{\link[rkwarddev:rk.XML.external]{rk.XML.external}},
#'		\code{\link[rkwarddev:rk.XML.set]{rk.XML.set}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # define an input field and two checkboxes
#' test.input <- rk.XML.input("Type some text")
#' test.cbox1 <- rk.XML.cbox(label="Want to type?", value="true")
#' test.cbox2 <- rk.XML.cbox(label="Are you shure?", value="true")
#' # now create some logic so that the input field is only enabled when both boxes are checked
#' test.convert <- rk.XML.convert(c(state=test.cbox1,state=test.cbox2), mode=c(and=""))
#' test.connect <- rk.XML.connect(governor=test.convert, client=test.input, set="enabled")
#' test.logic <- rk.XML.logic(test.convert, test.connect)
#' cat(pasteXMLNode(test.logic))
#' 
#' # with only one checkbox, you can directly query if it's checked
#' test.connect2 <- rk.XML.connect(governor=test.cbox1, client=test.input, set="enabled")
#' test.logic2 <- rk.XML.logic(test.connect2)
#' cat(pasteXMLNode(test.logic2))

rk.XML.logic <- function(...){
	nodes <- list(...)

	# transform "!--" comment nodes into "![CDATA[" for scripting logic
	nodes <- sapply(child.list(nodes), function(this.node){
			if(identical(this.node@name, "!--")){
				this.node@name <- "![CDATA["
				this.node <- new("XiMpLe.node",
						name="script",
						children=child.list(this.node),
						value=""
					)
			} else {}
			return(this.node)
		})

	# check the node names and allow only valid ones
	node.names <- sapply(child.list(nodes), function(this.node){
			this.node@name
		})

	invalid.sets <- !node.names %in% c("connect", "convert","include","insert","external","set","script")
	if(any(invalid.sets)){
		stop(simpleError(paste("Invalid XML nodes for logic section: ", paste(node.names[invalid.sets], collapse=", "), sep="")))
	} else {}

	node <- new("XiMpLe.node",
			name="logic",
			children=child.list(nodes),
			value=""
		)

	return(node)
}
