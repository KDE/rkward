#' Create XML node "dropdown" for RKWard plugins
#'
#' @param label Character string, a text label for this plugin element.
#' @param options A named list with options to choose from. The names of the list elements will become
#'		labels of the options, \code{val} defines the value to submit if the option is checked, and
#'		\code{chk=TRUE} should be set in the one option which is checked by default.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"} and a label was provided, an ID will be generated automatically from the label.
#' @return An object of class \code{XiMpLe.node}.
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export
#' @examples
#' test.dropdown <- rk.XML.dropdown("mydrop",
#'   options=list("First Option"=c(val="val1"),
#'   "Second Option"=c(val="val2", chk=TRUE)))
#' cat(pasteXML(test.dropdown))

rk.XML.dropdown <- function(label, options=list(label=c(val=NULL, chk=FALSE)), id.name="auto"){
	num.opt <- length(options)
	dd.options <- sapply(1:num.opt, function(this.num){
			this.element <- names(options)[[this.num]]
			this.value <- options[[this.num]][["val"]]
			attr.list <- list(label=this.element, value=this.value)
			if("chk" %in% names(options[[this.num]])){
				if(isTRUE(as.logical(options[[this.num]][["chk"]]))){
					attr.list[["checked"]] <- "true"
				} else {}
			} else {}
			new("XiMpLe.node",
				name="option",
				attributes=attr.list)
		})

	if(identical(id.name, "auto")){
		id <- auto.ids(label, prefix=ID.prefix("dropdown"))
	} else {
		id <- id.name
	}
	drp.attr.list <- list(id=id, label=label)

	dropdown <- new("XiMpLe.node",
			name="dropdown",
			attributes=drp.attr.list,
			children=dd.options
		)

	return(dropdown)
}
