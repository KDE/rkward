#' Create XML node "tabbook" for RKWard plugins
#'
#' @param label Character string, a text label for this plugin element.
#' @param tab.labels Character vector, where each string defines the name of one tab.
#'		The number of \code{tab.labels} also defines the number of tabs.
#' @param children An optional list with objects of class \code{XiMpLe.node} (or a list of these objects).
#'		You must provide one element for each tab. Use \code{NULL} for tabs without predefined children.
#' @param id.name Character vector, unique IDs for the tabbook (first entry) and all tabs.
#'		If \code{"auto"}, IDs will be generated automatically from the labels.
#'		If \code{NULL}, no IDs will be given.
#' @return An object of class \code{XiMpLe.node}.
#' @export

rk.XML.tabbook <- function(label, tab.labels, children=list(), id.name="auto"){
	num.tabs <-  length(tab.labels)
	# check if number of children fits
	if(length(children) > 0){
		if(!identical(length(children), num.tabs)){
			stop(simpleError("If you provide children, you must do so for each tab (use NULL for tabs without children)!"))
		} else {}
	} else {
		children <- NULL
	}

	if(identical(id.name, "auto")){
		tab.ids <- auto.ids(tab.labels, prefix="tab.")
	} else {}
	tabs <- sapply(1:num.tabs, function(this.num){
			this.tab <- tab.labels[[this.num]]
			attr.list <- list(label=this.tab)
			if(identical(id.name, "auto")){
				attr.list[["id"]] <- tab.ids[[this.num]]
			} else if(!is.null(id.name)){
				attr.list[["id"]] <- id.name[[this.num + 1]]
			} else {}
			if(!is.null(children[[this.num]])){
				child <- children[[this.num]]
			} else {
				child <- list()
			}
			new("XiMpLe.node",
				name="tab",
				attributes=attr.list,
				children=child.list(child),
				value="")
		})

	if(identical(id.name, "auto")){
		tb.id <- auto.ids(label, prefix="tbk.")
	} else if(!is.null(id.name)){
		tb.id <- id.name[[1]]
	} else {}
	tbk.attr.list <- list(id=tb.id, label=label)

	tabbook <- new("XiMpLe.node",
			name="tabbook",
			attributes=tbk.attr.list,
			children=child.list(tabs)
		)

	return(tabbook)
}
