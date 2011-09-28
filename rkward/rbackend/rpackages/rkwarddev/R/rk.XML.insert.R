#' Create XML node "insert" for RKWard plugins
#'
#' This function creates an insert node to use snippets.
#'
#' @param snippet Either a character string (the \code{id} of the snippet to be inserted),
#'		or an object of class \code{XiMpLe.node} (whose \code{id} will be extracted and used; must be a snippet!).
#' @return A list of objects of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.snippets]{rk.XML.snippets}}
#'		\code{\link[rkwarddev:rk.XML.snippet]{rk.XML.snippet}}
#' @examples
#' # define a formula section with varselector and varslots
#' test.formula <- rk.XML.vars("Variables", "Fixed", formula.dependent="Dependent")
#' # define the snippet
#' test.snippet <- rk.XML.snippet(test.formula)
#' # now to insert the snippet
#' test.insert <- rk.XML.insert(test.snippet)
#' cat(pasteXMLNode(test.insert))

rk.XML.insert <- function(snippet){
	if(length(snippet) > 1){
		stop(simpleError("'snippet' must be of length 1!"))
	} else {}

	if(inherits(snippet, "XiMpLe.node")){
		node.name <- snippet@name
		if(!identical(node.name, "snippet")){
			stop(simpleError(paste("Invalid XML node, must be a snippet: ", node.name, sep="")))
		} else {}
	} else {}

	# let's see if we need to extract IDs first
	attr.list <- list(snippet=check.ID(snippet))

	node <- new("XiMpLe.node",
			name="insert",
			attributes=attr.list
		)

	return(node)
}
