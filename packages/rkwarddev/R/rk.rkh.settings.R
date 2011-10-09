#' Create XML "settings" node for RKWard help pages
#'
#' This function will create a settings node for the document section, with optional child nodes "setting" and "caption".
#'
#' @param ... Objects of class \code{XiMpLe.node}. They must all have the name "setting" or "caption".
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}},
#'		\code{\link[rkwarddev:rk.rkh.setting]{rk.rkh.setting}},
#'		\code{\link[rkwarddev:rk.rkh.caption]{rk.rkh.caption}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # define a sample frame
#' test.dropdown <- rk.XML.dropdown("mydrop",
#'   options=list("First Option"=c(val="val1"),
#'   "Second Option"=c(val="val2", chk=TRUE)))
#' test.frame <- rk.XML.frame(test.dropdown, label="Some options")
#' # create the caption
#' test.caption <- rk.rkh.caption(test.frame)
#' test.setting <- rk.rkh.setting(test.dropdown, text="Chose one of the options.")
#' test.settings <- rk.rkh.settings(list(test.caption, test.setting))
# cat(pasteXMLNode(test.settings))

rk.rkh.settings <- function(...){
	nodes <- list(...)

	# check the node names and allow only valid ones
	node.names <- sapply(child.list(nodes), function(this.node){
			this.node@name
		})

	invalid.sets <- !node.names %in% c("setting", "caption")
	if(any(invalid.sets)){
		stop(simpleError(paste("Invalid XML nodes for settings section: ", paste(node.names[invalid.sets], collapse=", "), sep="")))
	} else {}

	node <- new("XiMpLe.node",
			name="settings",
			children=child.list(nodes),
			value=""
		)

	return(node)
}
