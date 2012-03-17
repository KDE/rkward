#' Create XML "section" node for RKWard help pages
#'
#' This function will create a section node for settings sections in RKWard help files.
#'
#' @param title Character string, title to be displayed.
#' @param text Character string, the text to be displayed.
#' @param short Character string, short title for the menu for links to this section.
#' @param id.name Character string, a unique ID for this element.
#'		If \code{"auto"}, an ID will be generated automatically from the \code{title} value.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}}
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.section <- rk.rkh.section("General background", text="Some important notes...",
#' short="Background")
#' cat(pasteXML(test.section))

rk.rkh.section <- function(title, text=NULL, short=NULL, id.name="auto"){
	if(identical(id.name, "auto")){
		attr.list <- list(id=auto.ids(title, prefix=ID.prefix("section")),
			title=title)
	} else if(!is.null(id.name)){
		attr.list <- list(id=id.name, title=title)
	} else {
		stop(simpleError("'id.name' must have a value!"))
	}

	if(!is.null(short)){
		attr.list[["short_title"]] <- short
	} else {}

	if(is.null(text)){
		text <- ""
	} else {}

	node <- XMLNode("section", text, attrs=attr.list)

	return(node)
}
