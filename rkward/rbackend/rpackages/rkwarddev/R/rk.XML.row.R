#' Create XML node "row" for RKWard plugins
#'
#' @param children An optional list with objects of class \code{XiMpLe.node}.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{NULL}, no ID will be given.
#' @return An object of class \code{XiMpLe.node}.
#' @export

rk.XML.row <- function(children=list(), id.name=NULL){
	if(!is.null(id.name)){
		attr.list <- list(id=id.name)
	} else {
		attr.list <- list()
	}

	row <- new("XiMpLe.node",
		name="row",
		attributes=attr.list,
		children=child.list(children),
		value="")

	return(row)
}
