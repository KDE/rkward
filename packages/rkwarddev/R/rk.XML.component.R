#' Create XML "component" node for RKWard plugins
#'
#' This function will create a component node for components sections of .pluginmap files.
#' 
#' @param label Character string, a label for the component.
#' @param file Character string, file name of a plugin XML file defining the GUI.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"}, an ID will be generated automatically from the label.
#' @param type Character string, type of component. As of now, only "standard" is supported. The option is
#'		just implemented for completeness.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'		\code{\link[rkwarddev:rk.XML.components]{rk.XML.components}},
#'		and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.component <- rk.XML.component("My GUI dialog", "plugins/MyGUIdialog.xml")
#' cat(pasteXMLNode(test.component))

rk.XML.component <- function(label, file, id.name="auto", type="standard"){
	if(identical(id.name, "auto")){
		# try autogenerating some id
		id.name <- auto.ids(label, prefix=ID.prefix("component"), chars=10)
	} else if(is.null(id.name)){
		stop(simpleError("Components need an ID!"))
	} else {}
	attr.list <- list(id=check.ID(id.name), label=label)

	# once there are more types supported, this will make much more sense...
	if(!type %in% c("standard")){
		stop(simpleError(paste("Invalid type: ", type, sep="")))
	} else {
		attr.list[["type"]] <- type
	}
	if(!is.null(file)){
		attr.list[["file"]] <- file
	} else {}

	node <- new("XiMpLe.node",
			name="component",
			attributes=attr.list
		)

	return(node)
}
