#' Create XML node "browser" for RKWard plugins
#'
#' @param label Character string, a text label for this plugin element.
#' @param type Character string, valid values are "dir", "file" and "savefile" (i.e., an non-existing file).
#' @param initial Character string, if not \code{NULL} will be used as the initial value of the browser.
#' @param urls Logical, whether non-local URLs are permitted or not.
#' @param filter Character vector, file type filter, e.g. \code{filter=c("*.txt", "*.csv")} for .txt and .csv files.
#'		Try not to induce limits unless absolutely needed, though.
#' @param required Logical, whether an entry is mandatory or not.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"} and a label was provided, an ID will be generated automatically from the label.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @examples
#' test.browser <- rk.XML.browser("Browse here:")
#' cat(pasteXMLNode(test.browser))


rk.XML.browser <- function(label, type="file", initial=NULL, urls=FALSE, filter=NULL, required=FALSE, id.name="auto"){
	attr.list <- list(label=label)

	if(length(type) == 1 & type %in% c("dir", "file", "savefile")){
		attr.list[["type"]] <- type
	} else {
		stop(simpleError(paste("Unknown browser type: ", type, sep="")))
	}

	if(identical(id.name, "auto")){
		attr.list[["id"]] <- auto.ids(label, prefix=ID.prefix("browser"))
	} else if(!is.null(id.name)){
		attr.list[["id"]] <- id.name
	} else {}
	if(!is.null(initial)){
		attr.list[["initial"]] <- initial
	} else {}
	if(isTRUE(urls)){
		attr.list[["allow_urls"]] <- "true"
	} else {}
	if(!is.null(filter)){
		attr.list[["filter"]] <- paste(filter, collapse=" ")
	} else {}
	if(isTRUE(required)){
		attr.list[["required"]] <- "true"
	} else {}

	node <- new("XiMpLe.node",
			name="browser",
			attributes=attr.list)

	return(node)
}
