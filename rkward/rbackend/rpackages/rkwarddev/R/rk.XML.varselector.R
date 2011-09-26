#' Create node "varselector" for RKWard plugins
#'
#' @param label Character string, a text label for the variable selection slot.
#' @param id.name Character vector, unique ID for this element.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.varslot]{rk.XML.varslot}},
#'		\code{\link[rkwarddev:rk.XML.vars]{rk.XML.vars}}
#' @examples
#' test.varselector <- rk.XML.varselector("Select some vars")
#' cat(pasteXMLNode(test.varselector, shine=1))

rk.XML.varselector <- function(label, id.name="auto"){
	if(identical(id.name, "auto")){
		## if this ID generation get's changed, change it in rk.XML.vars(), too!
		attr.list <- list(id=auto.ids(label, prefix=ID.prefix("varselector", length=3)))
	} else if(!is.null(id.name)){
		attr.list <- list(id=id.name)
	} else {}

	attr.list[["label"]] <- label

	node <- new("XiMpLe.node",
		name="varselector",
		attributes=attr.list)

	return(node)
}
