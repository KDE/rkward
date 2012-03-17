#' Constructor function for XiMpLe.doc objects
#'
#' Can be used to create full XML trees.
#'
#' @param ... Optional children for the XML tree. Must be either objects of class XiMpLe.node or character strings,
#'		which are treated as simple text values.
#' @param xml A named list, XML declaration of the XML tree. Currently just pasted, no checking is done.
#' @param dtd A named list, doctype definition of the XML tree. Valid elements are \code{doctype}, \code{id} and \code{refer}.
#'		Currently just pasted, no checking is done.
#' @param .children Alternative way of specifying children, if you have them already as a list.
#' @return An object of class XiMpLe.doc
#' @export

XMLTree <- function(..., xml=NULL, dtd=NULL, .children=list(...)){

	# remove NULLs
	.children <- .children[unlist(lapply(.children, length) != 0)]

	# check for text values
	all.children <- sapply(child.list(.children), function(this.child){
		if(is.character(this.child)){
			this.child <- new("XiMpLe.node",
					name="",
					value=this.child
				)
		} else {}
		return(this.child)
	})

	if(is.null(xml)){
		xml <- list()
	} else {}
	if(is.null(dtd)){
		dtd <- list()
	} else {}
	
	newTree <- new("XiMpLe.doc",
		xml=xml,
		dtd=dtd,
		children=all.children
	)

	return(newTree)
}