#' Create XML "setting" node for RKWard help pages
#'
#' This function will create a setting node for settings sections in RKWard help files.
#'
#' @param id Either a character string (the \code{id} of the XML element to explain),
#'		or an object of class \code{XiMpLe.node} (whose \code{id} will be extracted and used).
#' @param text Character string, the text to be displayed.
#' @param title Character string, title to be displayed. If \code{NULL}, the \code{label}
#'		of the element will be shown.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}},
#'		\code{\link[rkwarddev:rk.rkh.settings]{rk.rkh.settings}}
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # define a formula section with varselector and varslots
#' test.checkbox <- rk.XML.cbox(label="foo", value="foo1", chk=TRUE)
#' # explain the option
#' test.setting <- rk.rkh.setting(test.checkbox, text="Check this to do Foo.")
#' cat(pasteXML(test.setting))

rk.rkh.setting <- function(id, text=NULL, title=NULL){
	# let's see if we need to extract IDs first
	attr.list <- list(id=check.ID(id))

	if(!is.null(title)){
		attr.list[["title"]] <- title
	} else {}

	if(is.null(text)){
		text <- ""
	} else {}

	node <- XMLNode("setting", text, attrs=attr.list)

	return(node)
}
