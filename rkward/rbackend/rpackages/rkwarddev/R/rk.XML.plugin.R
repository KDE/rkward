#' Create XML document for RKWard plugins
#'
#' @param name Character string, the name of the plugin.
#' @param label Character string, a text label for the plugin's top level, i.e. the window title of the dialog.
#' @param children An optional list with objects of class \code{XiMpLe.node}. Will be included inside the top level node as the dialog code.
#' @param wiz.children An optional list with objects of class \code{XiMpLe.node}. Will be included inside the top level node as the wizard code
#'		(but only if \code{provides} includes \code{"wizard"} as well).
#' @param help Logical, if \code{TRUE} an include tag for a help file named \emph{"<name>.rkh"} will be added to the header.
#' @param logic Logical, if \code{TRUE} a logic section will be added to the document.
#'		This must be edited manually, it is therefore commented out.
#' @param provides Character vector with at least one entry of \code{"dialog"} or \code{"wizard"}, defining what the document provides.
#' @param pluginmap Character string, relative path to the pluginmap file, which will then be included in the head of this document.
#' @return An object of class \code{XiMpLe.doc}.
#' @export
#' @examples
#' test.checkboxes <- rk.XML.row(rk.XML.col(
#'   list(
#'     rk.XML.cbox(label="foo", val="foo1", chk=TRUE),
#'     rk.XML.cbox(label="bar", val="bar2"))))
#' test.dropdown <- rk.XML.dropdown("mydrop",
#'   opts=list("First Option"=c(val="val1"),
#'   "Second Option"=c(val="val2", chk=TRUE)))
#' # combine the above into a tabbook
#' test.tabbook <- rk.XML.tabbook("My Tabbook", tab.labels=c("First Tab",
#'   "Second Tab"), children=list(test.checkboxes, test.dropdown))
#' # make a plugin with that tabbook
#' test.plugin <- rk.XML.plugin("My test", label="Check this out", children=test.tabbook)
#' cat(pasteXMLTree(test.plugin, shine=1))

rk.XML.plugin <- function(name, label, children=list(), wiz.children=list(), help=TRUE, logic=TRUE, provides=c("dialog"), pluginmap=NULL){
	name.orig <- name
	name <- gsub("[[:space:]]*[^[:alnum:]]*", "", name)
	if(!identical(name.orig, name)){
		message(paste("For filenames ", sQuote(name.orig), " was renamed to ", sQuote(name), ".", sep=""))
	} else {}

	all.children <- list(new("XiMpLe.node",
		name="code",
		attributes=list(file=paste(name, ".js", sep=""))
	))

	if(isTRUE(help)){
		all.children[[length(all.children)+1]] <- new("XiMpLe.node",
			name="help",
			attributes=list(file=paste(name, ".rkh", sep=""))
		)
	} else {}

	if(!is.null(pluginmap)){
		all.children[[length(all.children)+1]] <- new("XiMpLe.node",
			name="include",
			attributes=list(file=pluginmap)
		)
	} else {}

	if(isTRUE(logic)){
		all.children[[length(all.children)+1]] <- new("XiMpLe.node",
			name="logic",
			children=list(
				new("XiMpLe.node",
					# add these as comments because they need editing
					name="!--",
					value="<convert id=\"!edit!\", mode=\"!edit!\", sources=\"!edit!\", standard=\"!edit!\" />"),
				new("XiMpLe.node",
					name="!--",
					value="<connect client=\"!edit!\", governor=\"!edit!\" />")
			)
		)
	} else {}

	if("dialog" %in% provides){
		plugin.dialog <- new("XiMpLe.node",
				name="dialog",
				attributes=list(label=label),
				value="")
		if(length(children) > 0){
			plugin.dialog@children <- child.list(children)
		} else {}
		all.children[[length(all.children)+1]]	<- plugin.dialog
	} else {}

	if("wizard" %in% provides){
		## TODO: wizard code
		# create a first page for the wizard section
		plugin.wizard.page <- new("XiMpLe.node",
				name="page",
				attributes=list(label=label),
				value="")
		if(length(wiz.children) > 0){
			plugin.wizard.page@children <- child.list(wiz.children)
		} else {}
		plugin.wizard <- new("XiMpLe.node",
				name="wizard",
				attributes=list(label=label),
				children=child.list(plugin.wizard.page),
				value="")
		all.children[[length(all.children)+1]]	<- plugin.wizard
	}

	top.doc <- new("XiMpLe.node",
		name="document",
		children=child.list(all.children)
	)

	plugin <- new("XiMpLe.doc",
			dtd=list(doctype="rkplugin"),
			children=child.list(top.doc)
	)

	return(plugin)
}
