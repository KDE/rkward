#' Constructor function for XiMpLe.node objects
#' 
#' Can be used to create XML nodes.
#' 
#' To generate a CDATA node, set \code{name="![CDATA["}, to create a comment, set \code{name="!--"}.
#' 
#' @param name Character string, the tag name.
#' @param ... Optional children for the tag. Must be either objects of class XiMpLe.node or character strings,
#'		which are treated as simple text values. If this is empty, the tag will be treated as an empty tag. To
#'		force a closing tag, supply an empty string, i.e. \code{""}.
#' @param attrs An optional named list of attributes.
#' @param namespace Currently ignored.
#' @param namespaceDefinitions Currently ignored.
#' @param .children Alternative way of specifying children, if you have them already as a list.
#' @return An object of class XiMpLe.node
#' @export

XMLNode <- function(name, ..., attrs=NULL, namespace="", namespaceDefinitions=NULL, .children=list(...)){

	all.children <- list()

	# text node?
	if(identical(name, "") &
			(all(unlist(lapply(.children, is.character)))) |
			all(unlist(lapply(.children, is.numeric)))){
		value <- paste(..., sep=" ")
	} else if(identical(.children, list(""))){
		value <- ""
	} else {
		# remove NULLs
		.children <- .children[unlist(lapply(.children, length) != 0)]
		# check for text values
		all.children <- sapply(child.list(.children), function(this.child){
			if(is.character(this.child) | is.numeric(this.child)){
				this.child <- new("XiMpLe.node",
						name="",
						value=as.character(this.child)
					)
			} else {}
			return(this.child)
		})
		value <- character()
	}

	newNode <- new("XiMpLe.node",
		name=name,
		attributes=as.list(attrs),
		children=all.children,
		value=value)

	return(newNode)
}
