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
#' @param tidy Logical, if \code{TRUE} the special characters "<" and ">" will be replaced with the entities
#'		"&lt;" and "gt;" in attributes and text values.
#' @include XiMpLe.node-class.R
#' @export
pasteXMLNode <- function(node, level=1, shine=1, indent.by="\t", tidy=TRUE){
	if(!inherits(node, "XiMpLe.node")){
		if(inherits(node, "XiMpLe.doc")){
			# hand over to pasteXMLTree()
			warning("'node' is of class XiMpLe.doc, called pasteXMLTree() instead.")
			return(pasteXMLTree(obj=node, shine=shine, indent.by=indent.by, tidy=tidy))
		} else {
			stop(simpleError("'node' must be of class XiMpLe.node!"))
		}
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
			return(pasteXMLNode(this.node, level=(level + 1), shine=shine, indent.by=indent.by, tidy=tidy))})), collapse="", sep="")
		node.empty <- FALSE
	} else {
		node.chld <- NULL
		node.empty <- TRUE
	}

	# take care of text value
	if(length(node.val) > 0){
		node.empty <- FALSE
		if(nchar(node.val) > 0){
			if(isTRUE(tidy)){
				node.val <- sapply(node.val, xml.tidy)
			} else {}
			node.chld <- paste(new.indent, paste(node.val, collapse=" "), new.node, sep="")
		} else {}
	} else {}

	pasted.node <- pasteXMLTag(node.name, attr=node.attr, child=node.chld, empty=node.empty, level=level, allow.empty=TRUE, rename=NULL, shine=shine, indent.by=indent.by, tidy=tidy)
	
	return(pasted.node)
}
