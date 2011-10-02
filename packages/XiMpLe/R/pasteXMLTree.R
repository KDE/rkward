#' Paste an XML tree structure from a XiMpLe.doc object
#'
#' @param obj An object of class \code{XiMpLe.doc}.
#' @param shine Integer, controlling if the output should be formatted for better readability. Possible values:
#'		\describe{
#'			\item{0}{No formatting.}
#'			\item{1}{Nodes will be indented.}
#'			\item{2}{Nodes will be indented and each attribute gets a new line.}
#'		}
#' @param indent.by A charachter string defining how indentation should be done. Defaults to tab.
#' @include XiMpLe.doc-class.R
#' @export
pasteXMLTree <- function(obj, shine=1, indent.by="\t"){
	if(!inherits(obj, "XiMpLe.doc")){
		if(inherits(obj, "XiMpLe.node")){
			# hand over to pasteXMLNode()
			warning("'node' is of class XiMpLe.node, called pasteXMLNode() instead.")
			return(pasteXMLNode(node=obj, shine=shine, indent.by=indent.by))
		} else {
			stop(simpleError("'obj' must be of class XiMpLe.doc!"))
		}
	} else {}

	filename <- slot(obj, "file")
	tree.xml <- slot(obj, "xml")
	tree.doctype <- slot(obj, "dtd")
	tree.nodes <- slot(obj, "children")

	if(any(nchar(unlist(tree.xml)) > 0)) {
		doc.xml <- pasteXMLTag("?xml", attr=tree.xml, child=NULL, empty=TRUE, level=1, allow.empty=FALSE, rename=NULL, shine=min(1, shine), indent.by=indent.by)
		doc.xml <- gsub("/>", "\\?>", doc.xml)
	} else {
		doc.xml <- ""
	}

	if(any(nchar(unlist(tree.doctype)) > 0)) {
		new.node   <- ifelse(shine > 0, "\n", "")
		doc.doctype <- paste("<!DOCTYPE ", paste(tree.doctype[["doctype"]], tree.doctype[["id"]], sep=" "), sep="")
		if(length(tree.doctype[["refer"]]) > 0) {
			if(nchar(tree.doctype[["refer"]]) > 0){
				doc.doctype <- paste(doc.doctype, " \"",tree.doctype[["refer"]], "\"", sep="")
			} else {}
		} else {}
		doc.doctype <- paste(doc.doctype, ">", new.node, sep="")
	} else {
		doc.doctype <- ""
	}

	if(length(tree.nodes) > 0) {
		doc.nodes <- paste(unlist(sapply(tree.nodes, function(this.node){
			return(pasteXMLNode(this.node, level=1, shine=shine, indent.by=indent.by))})), collapse="", sep="")
	} else {
		doc.nodes <- ""
	}

	doc.all <- paste(doc.xml, doc.doctype, doc.nodes, collapse="", sep="")

	return(doc.all)
}
