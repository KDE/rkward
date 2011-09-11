#' Paste an XML node from a XiMpLe.node object
#'
#' @param node An object of class \code{XiMpLe.node}.
#' @param level Indentation level.
#' @param shine Integer, controlling if the output should be formatted for better readability. Possible values:
#'		\describe{
#'			\item{0}{No formatting.}
#'			\item{1}{Nodes will be indented.}
#'			\item{2}{Nodes will be indented and each attribute gets a new line.}
#'		}
#' @param indent.by A charachter string defining how indentation should be done. Defaults to tab.
#' @include XiMpLe.node-class.R
#' @export
pasteXMLNode <- function(node, level=1, shine=2, indent.by="\t"){
	if(!inherits(node, "XiMpLe.node")){
		stop(simpleError("'node' must be of class XiMpLe.node!"))
	} else {}

	new.indent <- ifelse(shine > 0, indent(level+1, by=indent.by), "")
	new.node   <- ifelse(shine > 0, "\n", "")

	# get the slot contents
	node.name <- slot(node, "name")
	node.attr <- slot(node, "attributes")
	node.chld <- slot(node, "children")
	node.val  <- slot(node, "value")

	if(!length(node.attr) > 0){
		node.attr <- NULL
	} else {}

	if(length(node.chld) > 0){
		node.chld <- paste(unlist(sapply(node.chld, function(this.node){
			return(pasteXMLNode(this.node, level=(level + 1), shine=shine, indent.by=indent.by))})), collapse="", sep="")
		node.empty <- FALSE
	} else {
		node.chld <- NULL
		node.empty <- TRUE
	}

	if(length(node.val) > 0){
		node.empty <- FALSE
		if(nchar(node.val) > 0){
			node.chld <- node.val
		} else {}
	} else {}

	pasted.node <- pasteXMLTag(node.name, attr=node.attr, child=node.chld, empty=node.empty, level=level, allow.empty=TRUE, rename=NULL, shine=shine, indent.by=indent.by)
	
	return(pasted.node)
}
