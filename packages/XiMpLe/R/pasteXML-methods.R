#' Paste methods for XiMpLe XML objects
#' 
#' These methods can be used to paste objects if class \code{\link[XiMpLe:XiMpLe.node-class]{XiMpLe.node}}
#' or \code{\link[XiMpLe:XiMpLe.doc-class]{XiMpLe.doc}}.
#'
#' @note The functions pasteXMLNode() and pasteXMLTree() have been replaced by the pasteXML methods.
#'		They should no longer be used.
#'
#' @param obj An object of class \code{XiMpLe.node} or \code{XiMpLe.doc}.
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
#' @aliases
#'		pasteXML,-methods
#'		pasteXML,XiMpLe.doc-method
#'		pasteXMLNode
#'		pasteXMLTree
#' @seealso \code{\link[XiMpLe:XiMpLe.node-class]{XiMpLe.node}}
#'		\code{\link[XiMpLe:XiMpLe.doc-class]{XiMpLe.doc}}
#' @keywords methods
#' @rdname pasteXML-methods
#' @include XiMpLe.node-class.R
#' @include XiMpLe.doc-class.R
#' @exportMethod pasteXML
setGeneric("pasteXML", function(obj, ...) standardGeneric("pasteXML"))

#' @usage pasteXML(obj, level=1, shine=1, indent.by="\t", tidy=TRUE)
#' @rdname pasteXML-methods
#' @aliases
#'		pasteXML,XiMpLe.node-method
#' @export
setMethod("pasteXML",
	signature=signature(obj="XiMpLe.node"),
	function(obj, level=1, shine=1, indent.by="\t", tidy=TRUE){

		new.indent <- ifelse(shine > 0, indent(level+1, by=indent.by), "")
		new.node   <- ifelse(shine > 0, "\n", "")

		# get the slot contents
		node.name <- slot(obj, "name")
		node.attr <- slot(obj, "attributes")
		node.chld <- slot(obj, "children")
		node.val  <- slot(obj, "value")

		if(!length(node.attr) > 0){
			node.attr <- NULL
		} else {}

		if(length(node.chld) > 0){
			node.chld <- paste(unlist(sapply(node.chld, function(this.node){
				if(slot(this.node, "name") == ""){
					this.node.pasted <- paste(new.indent, pasteXML(this.node, level=level, shine=shine, indent.by=indent.by, tidy=tidy), sep="")
				} else {
					this.node.pasted <- pasteXML(this.node, level=(level + 1), shine=shine, indent.by=indent.by, tidy=tidy)
				}
				return(this.node.pasted)})), collapse="", sep="")
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
				node.chld <- paste(node.chld, paste(node.val, new.node, collapse=" "), sep="")
			} else {}
		} else {}

		pasted.node <- pasteXMLTag(node.name, attr=node.attr, child=node.chld, empty=node.empty, level=level, allow.empty=TRUE, rename=NULL, shine=shine, indent.by=indent.by, tidy=tidy)
		
		return(pasted.node)
	}
)
#' @usage
#' 	# S4 method for objects of class XiMpLe.doc
#'		pasteXML(obj, shine=1, indent.by="\t", tidy=TRUE)
#' @rdname pasteXML-methods
#' @export
setMethod("pasteXML",
	signature=signature(obj="XiMpLe.doc"),
	function(obj, shine=1, indent.by="\t", tidy=TRUE){

		filename <- slot(obj, "file")
		tree.xml <- slot(obj, "xml")
		tree.doctype <- slot(obj, "dtd")
		tree.nodes <- slot(obj, "children")

		if(any(nchar(unlist(tree.xml)) > 0)) {
			doc.xml <- pasteXMLTag("?xml", attr=tree.xml, child=NULL, empty=TRUE, level=1, allow.empty=FALSE, rename=NULL, shine=min(1, shine), indent.by=indent.by, tidy=tidy)
			doc.xml <- gsub("/>", "\\?>", doc.xml)
		} else {
			doc.xml <- ""
		}

		if(any(nchar(unlist(tree.doctype)) > 0)) {
			new.node   <- ifelse(shine > 0, "\n", "")
			doc.doctype <- paste("<!DOCTYPE ", tree.doctype[["doctype"]], sep="")
			for (elmt in c("id", "refer")){
				if(length(tree.doctype[[elmt]]) > 0) {
					if(nchar(tree.doctype[[elmt]]) > 0){
						doc.doctype <- paste(doc.doctype, " \"",tree.doctype[[elmt]], "\"", sep="")
					} else {}
				} else {}
			}
			doc.doctype <- paste(doc.doctype, ">", new.node, sep="")
		} else {
			doc.doctype <- ""
		}

		if(length(tree.nodes) > 0) {
			doc.nodes <- paste(unlist(sapply(tree.nodes, function(this.node){
				return(pasteXML(this.node, level=1, shine=shine, indent.by=indent.by, tidy=tidy))})), collapse="", sep="")
		} else {
			doc.nodes <- ""
		}

		doc.all <- paste(doc.xml, doc.doctype, doc.nodes, collapse="", sep="")

		return(doc.all)
	}
)

# for compatibility reasons, deploy wrapper functions
#' @export
pasteXMLNode <- function(node, level=1, shine=1, indent.by="\t", tidy=TRUE){
	pasteXML(node, level=level, shine=shine, indent.by=indent.by, tidy=tidy)
}
#' @export
pasteXMLTree <- function(obj, shine=1, indent.by="\t", tidy=TRUE){
	pasteXML(obj, shine=shine, indent.by=indent.by, tidy=tidy)
}
