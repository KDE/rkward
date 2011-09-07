#' Create XML node "input" for RKWard plugins
#'
#' @param label Character string, a text label for this plugin element.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"}, an ID will be generated automatically from the label.
#' @return An object of class \code{XiMpLe.node}.
#' @export

#<input id="inpCelexRunWd" label="Number of running words" />
rk.XML.input <- function(label, id.name="auto"){
	attr.list <- list(label=label)

	if(identical(id.name, "auto")){
		attr.list[["id"]] <- list(id=auto.ids(label, prefix=ID.prefix("input")))
	} else if(!is.null(id.name)){
		attr.list[["id"]] <- id.name
	} else {}

	node <- new("XiMpLe.node",
			name="input",
			attributes=attr.list)

	return(node)
}
