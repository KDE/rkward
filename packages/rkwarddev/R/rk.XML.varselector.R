#' Create node "varselector" for RKWard plugins
#'
#' @param label Character string, a text label for the variable selection slot.
#'		Must be set if \code{id.name="auto"}.
#' @param id.name Character vector, unique ID for this element.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.varslot]{rk.XML.varslot}},
#'		\code{\link[rkwarddev:rk.XML.vars]{rk.XML.vars}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.varselector <- rk.XML.varselector("Select some vars")
#' cat(pasteXML(test.varselector))

rk.XML.varselector <- function(label=NULL, id.name="auto"){
	if(identical(id.name, "auto")){
		## if this ID generation get's changed, change it in rk.XML.vars(), too!
		attr.list <- list(id=auto.ids(label, prefix=ID.prefix("varselector", length=3)))
	} else if(!is.null(id.name)){
		attr.list <- list(id=id.name)
	} else {}

	if(!is.null(label)){
		attr.list[["label"]] <- label
	} else {
		if(identical(id.name, "auto")){
			stop(simpleError("If id.name=\"auto\", then 'label' must have a value!"))
		} else {}
	}

	node <- XMLNode("varselector", attrs=attr.list)

	return(node)
}
