#' Create XML node "radio" for RKWard plugins
#'
#' @param label Character string, a text label for this plugin element.
#' @param options A named list with options to choose from. The names of the list elements will become
#'		labels of the options, \code{val} defines the value to submit if the option is checked, and
#'		\code{chk=TRUE} should be set in the one option which is checked by default.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"} and a label was provided, an ID will be generated automatically from the label.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.radio <- rk.XML.radio("Chose one",
#'   options=list("First Option"=c(val="val1"),
#'   "Second Option"=c(val="val2", chk=TRUE)))
#' cat(pasteXML(test.radio))

rk.XML.radio <- function(label, options=list(label=c(val=NULL, chk=FALSE)), id.name="auto"){
	num.opt <- length(options)
	rd.options <- sapply(1:num.opt, function(this.num){
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
		id <- auto.ids(label, prefix=ID.prefix("radio"))
	} else {
		id <- id.name
	}
	rd.attr.list <- list(id=id, label=label)

	radio <- new("XiMpLe.node",
			name="radio",
			attributes=rd.attr.list,
			children=child.list(rd.options)
		)

	return(radio)
}
