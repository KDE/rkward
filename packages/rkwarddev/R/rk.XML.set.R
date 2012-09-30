#' Create XML node "set" for RKWard plugins
#'
#' @param id Either a character string (the \code{id} of the property whose value should be set),
#'		or an object of class \code{XiMpLe.node} (whose \code{id} will be extracted and used).
#' @param set Character string, a valid modifier.
#' @param to Character string or logical, the value the property should be set to.
#' @param check.modifiers Logical, if \code{TRUE} the given modifiers will be checked for validity. Should only be
#'		turned off if you know what you're doing.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.convert]{rk.XML.convert}},
#'		\code{\link[rkwarddev:rk.XML.connect]{rk.XML.connect}},
#'		\code{\link[rkwarddev:rk.XML.external]{rk.XML.external}},
#'		\code{\link[rkwarddev:rk.XML.logic]{rk.XML.logic}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.set <- rk.XML.set(id="input_foo", set="required", to=TRUE)
#' cat(pasteXML(test.set))

rk.XML.set <- function(id, set=NULL, to, check.modifiers=TRUE){

	if(length(id) > 1 | length(to) > 1){
		stop(simpleError("'id' and 'to' must be of length 1!"))
	} else {}

	# check for container objects
	id <- stripXML(id)

	# let's see if we need to extract IDs first
	prop.id <- check.ID(id)

	if(!is.null(set)){
		if(isTRUE(check.modifiers)){
			modif.validity(id, modifier=set, ignore.empty=TRUE, warn.only=FALSE, bool=TRUE)
		} else {}
		prop.id <- paste(prop.id, set, sep=".")
	} else {}

	attr.list <- list(id=as.character(prop.id))

	if(is.logical(to)){
		attr.list[["to"]] <- ifelse(isTRUE(to), "true", "false")
	} else {
		attr.list[["to"]] <- as.character(to)
	}

	node <- XMLNode("set", attrs=attr.list)

	return(node)
}
