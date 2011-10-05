#' Create XML copy node for RKWard plugins
#'
#' @param id Either a character string (the \code{id} of the property to be copied),
#'		or an object of class \code{XiMpLe.node} (whose \code{id} will be extracted and used).
#' @param as A character string resembling the \code{copy_element_tag_name} value. I.e., must be
#'		a valid tag name. Will cause a change of tag name of the \code{id} (e.g. "tab") to \code{as}
#'		(e.g. "page").
#' @return A list of objects of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.plugin]{rk.XML.plugin}},
#'		\code{\link[rkwarddev:rk.plugin.skeleton]{rk.plugin.skeleton}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # define a checkbox for the actual dialog
#' test.cbox1 <- rk.XML.cbox(label="More than 30 subjects", val="true")
#' # define the wizard
#' test.text <- rk.XML.text("Did you test more than 30 subjects?")
#' test.copy <- rk.XML.copy(id=test.cbox1)
#' test.wizard <- rk.XML.wizard(rk.XML.page(list(test.text, test.copy)))
#' cat(pasteXMLNode(test.wizard))

rk.XML.copy <- function(id, as=NULL){
	if(length(id) > 1 | length(as) > 1){
		stop(simpleError("'id' and 'as' must be of length 1!"))
	} else {}

	# let's see if we need to extract IDs first
	id.name <- check.ID(id)
	attr.list <- list(id=id.name)

	if(!is.null(as)){
		invalid.sets <- !as %in% c("browser", "checkbox", "column", "copy",
			"dropdown", "formula", "frame", "input", "page", "radio", "row", "saveobject",
			"spinbox", "stretch", "tabbook", "text", "varselector", "varslot")
		if(any(invalid.sets)){
			stop(simpleError(paste("Invalid XML nodes ('as') for wizard/dialog section: ", paste(as, collapse=", "), sep="")))
		} else {
				attr.list[["as"]] <- as.character(as)
		}
	} else {}

	node <- new("XiMpLe.node",
			name="copy",
			attributes=attr.list
		)

	return(node)
}
