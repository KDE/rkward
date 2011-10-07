#' Create XML document for RKWard plugins
#'
#' @param name Character string, the name of the plugin. Will be used for the names of the JavaScript and
#'		help files to be included, following the scheme \emph{"<name>.<ext>"}.
#' @param dialog An object of class \code{XiMpLe.node} to be pasted as the \code{<dialog>} section. See
#'		\code{\link[rkwarddev:rk.XML.dialog]{rk.XML.dialog}} for details.
#' @param wizard An object of class \code{XiMpLe.node} to be pasted as the \code{<wizard>} section. See
#'		\code{\link[rkwarddev:rk.XML.wizard]{rk.XML.wizard}} for details.
#' @param logic An object of class \code{XiMpLe.node} to be pasted as the \code{<logic>} section. See
#'		\code{\link[rkwarddev:rk.XML.logic]{rk.XML.logic}} for details.
#' @param snippets An object of class \code{XiMpLe.node} to be pasted as the \code{<snippets>} section. See
#'		\code{\link[rkwarddev:rk.XML.snippets]{rk.XML.snippets}} for details.
#' @param provides Character vector with possible entries of \code{"logic"}, \code{"dialog"} or \code{"wizard"}, defining what
#'		sections the document should provide even if \code{dialog}, \code{wizard} and \code{logic} are \code{NULL}.
#'		These sections must be edited manually and some parts are therefore commented out.
#' @param help Logical, if \code{TRUE} an include tag for a help file named \emph{"<name>.rkh"} will be added to the header.
#' @param pluginmap Character string, relative path to the pluginmap file, which will then be included in the head of this document.
#' @param label Character string, a text label for the plugin's top level, i.e. the window title of the dialog.
#'		Will only be used if \code{dialog} or \code{wizard} are \code{NULL}.
#' @param clean.name Logical, if \code{TRUE}, all non-alphanumeric characters except the underscore (\code{"_"}) will be removed from \code{name}.
#' @return An object of class \code{XiMpLe.doc}.
#' @export
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
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
#'   "Second Tab"), dialog=list(test.checkboxes, test.dropdown))
#' # make a plugin with that tabbook
#' test.plugin <- rk.XML.plugin("My test", dialog=test.tabbook)
#' cat(pasteXMLTree(test.plugin))

rk.XML.plugin <- function(name, dialog=NULL, wizard=NULL, logic=NULL, snippets=NULL, provides=NULL, help=TRUE, pluginmap=NULL, label=NULL, clean.name=TRUE){
	if(isTRUE(clean.name)){
		name.orig <- name
		name <- clean.name(name)
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

	if(!is.null(snippets)){
		# check if this is *really* a snippets section, otherwise quit and go dancing
		if(inherits(snippets, "XiMpLe.node")){
			snippets.node.name <- snippets@name
		} else {
			snippets.node.name <- "yougottabekiddingme"
		}
		if(!identical(snippets.node.name, "snippets")){
			stop(simpleError("I don't know what this is, but 'snippets' is not a snippets section!"))
		} else {}
		all.children[[length(all.children)+1]] <- snippets
	} else {}

	if(is.null(logic)){
		if("logic" %in% provides){
			lgc.children <- list(
					new("XiMpLe.node",
						# add these as comments because they need editing
						name="!--",
						value="<convert id=\"!edit!\", mode=\"!edit!\", sources=\"!edit!\", standard=\"!edit!\" />"),
					new("XiMpLe.node",
						name="!--",
						value="<connect client=\"!edit!\", governor=\"!edit!\" />")
				)
			all.children[[length(all.children)+1]] <- new("XiMpLe.node",
				name="logic",
				children=lgc.children
			)
		} else {}
	} else {
		# check if this is *really* a logic section, otherwise quit and go dancing
		if(inherits(logic, "XiMpLe.node")){
			logic.node.name <- logic@name
		} else {
			logic.node.name <- "yougottabekiddingme"
		}
		if(!identical(logic.node.name, "logic")){
			stop(simpleError("I don't know what this is, but 'logic' is not a logic section!"))
		} else {}
		all.children[[length(all.children)+1]] <- logic
	}

	if(is.null(dialog)){
		if("dialog" %in% provides){
			all.children[[length(all.children)+1]] <- new("XiMpLe.node",
					name="dialog",
					attributes=list(label=label),
					value="")
		} else {}
	} else {
		# check if this is *really* a dialog section
		if(inherits(dialog, "XiMpLe.node")){
			dialog.node.name <- dialog@name
		} else {
			dialog.node.name <- "yougottabekiddingme"
		}
		if(!identical(dialog.node.name, "dialog")){
			stop(simpleError("I don't know what this is, but 'dialog' is not a dialog section!"))
		} else {}
		all.children[[length(all.children)+1]] <- dialog
	}

	if(is.null(wizard)){
		if("wizard" %in% provides){
			# create a first page for the wizard section
			plugin.wizard.page <- new("XiMpLe.node",
					name="page",
					attributes=list(label=label),
					value="")
			plugin.wizard <- new("XiMpLe.node",
					name="wizard",
					attributes=list(label=label),
					children=child.list(plugin.wizard.page),
					value="")
			all.children[[length(all.children)+1]] <- plugin.wizard
		} else {}
	} else {
		# check if this is *really* a wizard section
		if(inherits(wizard, "XiMpLe.node")){
			wizard.node.name <- wizard@name
		} else {
			wizard.node.name <- "yougottabekiddingme"
		}
		if(!identical(wizard.node.name, "wizard")){
			stop(simpleError("I don't know what this is, but 'wizard' is not a wizard section!"))
		} else {}
		all.children[[length(all.children)+1]] <- wizard
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
