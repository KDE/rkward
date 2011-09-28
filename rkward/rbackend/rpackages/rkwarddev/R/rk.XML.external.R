#' Create XML node "external" for RKWard plugins
#'
#' @param id Character string, the ID of the new property.
#' @param default Character string, initial value of the property if not connected.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.convert]{rk.XML.convert}},
#'		\code{\link[rkwarddev:rk.XML.connect]{rk.XML.connect}},
#'		\code{\link[rkwarddev:rk.XML.logic]{rk.XML.logic}}
#'		\code{\link[rkwarddev:rk.XML.set]{rk.XML.set}}
#' @examples
#' test.external <- rk.XML.external(id="ext_property", default="none")
#' cat(pasteXMLNode(test.external))

rk.XML.external <- function(id, default=NULL){
	attr.list <- list(id=id)

	if(!is.null(default)){
		attr.list[["default"]] <- as.character(default)
	} else {}

	node <- new("XiMpLe.node",
			name="external",
			attributes=attr.list)

	return(node)
}
