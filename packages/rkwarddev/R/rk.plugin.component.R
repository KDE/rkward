#' Generate RKWard plugin components
#'
#' @param name Character sting, name of this plugin component.
#' @param about An object of class \code{XiMpLe.node} with descriptive information on the plugin, its authors and dependencies,
#'		see \code{link[XiMpLe:rk.XML.about]{rk.XML.about}} for details. Only useful for information that differs from the \code{<about>}
#'		section of the \code{.pluginmap} file. Skipped if \code{NULL}.
#' @param xml A named list of options to be forwarded to \code{\link[rkwarddev:rk.XML.plugin]{rk.XML.plugin}}, to generate the GUI XML file.
#'		Not all options are supported because some don't make sense in this context. Valid options are:
#'		\code{"dialog"}, \code{"wizard"}, \code{"logic"} and \code{"snippets"}.
#'		If not set, their default values are used. See \code{\link[rkwarddev:rk.XML.plugin]{rk.XML.plugin}} for details.
#' @param js A named list of options to be forwarded to \code{\link[rkwarddev:rk.JS.doc]{rk.JS.doc}}, to generate the JavaScript file.
#'		Not all options are supported because some don't make sense in this context. Valid options are:
#'		\code{"require"}, \code{"results.header"}, \code{"variables"}, \code{"preprocess"}, \code{"calculate"} and \code{"printout"}.
#'		If not set, their default values are used. See \code{\link[rkwarddev:rk.JS.doc]{rk.JS.doc}} for details.
#' @param rkh A named list of options to be forwarded to \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}}, to generate the help file.
#'		Not all options are supported because some don't make sense in this context. Valid options are:
#'		\code{"summary"}, \code{"usage"}, \code{"sections"}, \code{"settings"}, \code{"related"} and \code{"technical"}.
#'		If not set, their default values are used. See \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}} for details.
#' @param provides Character vector with possible entries of \code{"logic"}, \code{"dialog"} or \code{"wizard"}, defining what
#'		sections the GUI XML file should provide even if \code{dialog}, \code{wizard} and \code{logic} are \code{NULL}.
#'		These sections must be edited manually and some parts are therefore commented out.
#' @param scan A character vector to trigger various automatic scans of the generated GUI XML file. Valid enties are:
#'		\describe{
#'			\item{\code{"var"}}{Calls \code{\link{rk.JS.scan}} to define all needed variables in the \code{calculate()} function
#'				of the JavaScript file. These variables will be added to variables defined by the \code{js} option, if any (see below).}
#'			\item{\code{"saveobj"}}{Calls \code{\link{rk.JS.saveobj}} to generate code to save R results in the \code{printout()}
#'				function of the JavaScript file. This code will be added to the code defined by the \code{js} option, if any (see below).}
#'			\item{\code{"settings"}}{Calls \code{\link{rk.rkh.scan}} to generate \code{<setting>} sections for each relevant GUI element in
#'				the \code{<settings>} section of the help file. This option will be overruled if you provide that section manually
#'				by the \code{rkh} option (see below).}
#'		}
#' @param hierarchy A character vector with instructions where to place this component in the menu hierarchy, one list or string.
#'		Valid single values are \code{"file"}, \code{"edit"}, \code{"view"}, \code{"workspace"}, \code{"run"}, \code{"data"},
#'		\code{"analysis"}, \code{"plots"}, \code{"distributions"}, \code{"windows"}, \code{"settings"} and \code{"help"},
#'		anything else will place it in a "test" menu. If \code{hierarchy} is a list, each entry represents the label of a menu level.
#' @param pluginmap Character string, relative path to the pluginmap file, which will then be included in the head of the GUI XML document.
#' @param create A character vector with one or more of these possible entries:
#'		\describe{
#'			\item{\code{"xml"}}{Create the plugin \code{.xml} XML file skeleton.}
#'			\item{\code{"js"}}{Create the plugin \code{.js} JavaScript file skeleton.}
#'			\item{\code{"rkh"}}{Create the plugin \code{.rkh} help file skeleton.}
#'		}
#' @param indent.by A character string defining the indentation string to use.
#' @return An object of class \code{rk.plug.comp}.
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export
#' @examples
#' \dontrun{
#' test.dropdown <- rk.XML.dropdown("mydrop",
#'   opts=list("First Option"=c(val="val1"),
#'   "Second Option"=c(val="val2", chk=TRUE)))
#' test.checkboxes <- rk.XML.row(rk.XML.col(
#'   list(test.dropdown,
#'     rk.XML.cbox(label="foo", val="foo1", chk=TRUE),
#'     rk.XML.cbox(label="bar", val="bar2"))
#'   ))
#' test.vars <- rk.XML.vars("select some vars", "vars go here")
#' test.tabbook <- rk.XML.dialog(rk.XML.tabbook("My Tabbook", tab.labels=c("First Tab",
#'   "Second Tab"), children=list(test.checkboxes, test.vars)))
#' 
#' rk.plugin.component("Square the Circle",
#'   xml=list(dialog=test.tabbook))
#' }

rk.plugin.component <- function(name, about=NULL, xml=list(), js=list(), rkh=list(),
	provides=c("logic", "dialog"), scan=c("var", "saveobj", "settings"), hierarchy="test",
	pluginmap=NULL, create=c("xml", "js", "rkh"), indent.by="\t"){
	# to besure, remove all non-character symbols from name
	name.orig <- name
	name <- clean.name(name)

	if(!is.null(about)){
		if(inherits(about, "XiMpLe.node")){
			about.node.name <- about@name
			# check if this is *really* a about section, otherwise quit and go dancing
			if(!identical(about.node.name, "about")){
				stop(simpleError("I don't know what this is, but 'about' is not an about section!"))
			} else {
				about.node <- about
			}
		} else {
			stop(simpleError("'about' must be a XiMpLe.node, see ?rk.XML.about()!"))
		}
	} else {
		about.node <- NULL
	}

	# check hierarchy
	if(is.null(hierarchy)){
		hierarchy <- list()
	} else {
		hierarchy <- as.list(hierarchy)
	}

	## create the full component
	this.component <- new("rk.plug.comp",
		name=name.orig,
		create=create,
		hierarchy=hierarchy
	)

	## create plugin.xml
	if("xml" %in% create & length(xml) > 0){
		got.XML.options <- names(xml)
		for (this.opt in c("dialog", "wizard", "logic", "snippets")){
			if(!this.opt %in% got.XML.options) {
				xml[[this.opt]] <- eval(formals(rk.XML.plugin)[[this.opt]])
			} else {}
		}
		XML.plugin <- rk.XML.plugin(
			name=name,
			label=name.orig,
			dialog=xml[["dialog"]],
			wizard=xml[["wizard"]],
			logic=xml[["logic"]],
			snippets=xml[["snippets"]],
			provides=provides,
			pluginmap=pluginmap,
			about=about.node)
		this.component@xml <- XML.plugin
	} else {
		this.component@xml <- rk.XML.plugin("")
	}

	## create plugin.js
	if("js" %in% create & length(js) > 0){
		got.JS.options <- names(js)
		for (this.opt in c("require", "variables", "preprocess", "calculate", "printout")){
			if(!this.opt %in% got.JS.options) {
				js[[this.opt]] <- eval(formals(rk.JS.doc)[[this.opt]])
			} else {}
		}
		if(!"results.header" %in% got.JS.options) {
			js[["results.header"]] <- paste(name.orig, " results", sep="")
		} else {}
		if("var" %in% scan){
			var.scanned <- rk.JS.scan(XML.plugin)
			if(!is.null(var.scanned)){
				js[["variables"]] <- paste(
					ifelse(is.null(js[["variables"]]), "", paste(js[["variables"]], "\n", sep="")),
					var.scanned,
					sep="")
			} else {}
		} else {}
		if("saveobj" %in% scan){
			saveobj.scanned <- rk.JS.saveobj(XML.plugin)
			if(!is.null(saveobj.scanned)){
				js[["printout"]] <- paste(js[["printout"]], saveobj.scanned, sep="\n")
			} else {}
		} else {}
		JS.code <- rk.JS.doc(
			require=js[["require"]],
			variables=js[["variables"]],
			results.header=js[["results.header"]],
			preprocess=js[["preprocess"]],
			calculate=js[["calculate"]],
			printout=js[["printout"]],
			indent.by=indent.by)
		this.component@js <- JS.code
	} else {
		this.component@js <- rk.JS.doc()
	}

	## create plugin.rkh
	if("rkh" %in% create & length(rkh) > 0){
		got.rkh.options <- names(rkh)
		# if settings were defined manually, this overwrites the scan
		if(!"settings" %in% got.rkh.options){
			if("settings" %in% scan){
				rkh[["settings"]] <- rk.rkh.settings(rk.rkh.scan(XML.plugin))
			} else {
				rkh[["settings"]] <- eval(formals(rk.rkh.doc)[["settings"]])
			}
		} else {}
		for (this.opt in c("summary", "usage", "sections", "related", "technical")){
			if(!this.opt %in% got.rkh.options) {
				rkh[[this.opt]] <- eval(formals(rk.rkh.doc)[[this.opt]])
			} else {}
		}
		rkh.doc <- rk.rkh.doc(
			summary=rkh[["summary"]],
			usage=rkh[["usage"]],
			sections=rkh[["sections"]],
			settings=rkh[["settings"]],
			related=rkh[["related"]],
			technical=rkh[["technical"]],
			title=rk.rkh.title(name.orig))
		this.component@rkh <- rkh.doc
	} else {
		this.component@rkh <- rk.rkh.doc()
	}

	return(this.component)
}
