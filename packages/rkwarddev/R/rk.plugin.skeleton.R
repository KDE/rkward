#' Generate skeletons for RKWard plugins
#'
#' @param name Character sting, name of the plugin package.
#' @param about An object of class \code{XiMpLe.node} with descriptive information on the plugin, its authors and dependencies,
#'		see \code{link[XiMpLe:rk.XML.about]{rk.XML.about}} for details. Skipped if \code{NULL}.
#'		If \code{NULL}, no \code{DESCRIPTION} file will be created either.
#' @param path Character sting, path to the main directory where the skeleton should be created.
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
#' @param xml A named list of options to be forwarded to \code{\link[rkwarddev:rk.XML.plugin]{rk.XML.plugin}}, to generate the GUI XML file.
#'		Not all options are supported because some don't make sense in this context. Valid options are:
#'		\code{"dialog"}, \code{"wizard"}, \code{"logic"} and \code{"snippets"}.
#'		If not set, their default values are used. See \code{\link[rkwarddev:rk.XML.plugin]{rk.XML.plugin}} for details.
#' @param js A named list of options to be forwarded to \code{\link[rkwarddev:rk.JS.doc]{rk.JS.doc}}, to generate the JavaScript file.
#'		Not all options are supported because some don't make sense in this context. Valid options are:
#'		\code{"require"}, \code{"results.header"}, \code{"variables"}, \code{"preprocess"}, \code{"calculate"} and \code{"printout"}.
#'		If not set, their default values are used. See \code{\link[rkwarddev:rk.JS.doc]{rk.JS.doc}} for details.
#' @param pluginmap A named list of options to be forwarded to \code{\link[rkwarddev:rk.XML.pluginmap]{rk.XML.pluginmap}}, to generate the pluginmap file.
#'		Not all options are supported because some don't make sense in this context. Valid options are:
#'		\code{"hierarchy"} and \code{"require"}.
#'		If not set, their default values are used. See \code{\link[rkwarddev:rk.XML.pluginmap]{rk.XML.pluginmap}} for details.
#' @param rkh A named list of options to be forwarded to \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}}, to generate the help file.
#'		Not all options are supported because some don't make sense in this context. Valid options are:
#'		\code{"summary"}, \code{"usage"}, \code{"sections"}, \code{"settings"}, \code{"related"} and \code{"technical"}.
#'		If not set, their default values are used. See \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}} for details.
#' @param overwrite Logical, whether existing files should be replaced. Defaults to \code{FALSE}.
#' @param tests Logical, whether directories and files for plugin tests should be created.
#'		Defaults to \code{TRUE}.
#' @param lazyLoad Logical, whether the package should be prepared for lazy loading or not. Should be left \code{TRUE},
#'		unless you have very good reasons not to.
#' @param create A character vector with one or more of these possible entries:
#'		\describe{
#'			\item{\code{"pmap"}}{Create the \code{.pluginmap} file.}
#'			\item{\code{"xml"}}{Create the plugin \code{.xml} XML file skeleton.}
#'			\item{\code{"js"}}{Create the plugin \code{.js} JavaScript file skeleton.}
#'			\item{\code{"rkh"}}{Create the plugin \code{.rkh} help file skeleton.}
#'			\item{\code{"desc"}}{Create the \code{DESCRIPTION} file.}
#'		}
#'		Default is to create all of these files. Existing files will only be overwritten if \code{overwrite=TRUE}.
#' @param edit Logical, if \code{TRUE} RKWard will automatically open the created files for editing, by calling \code{rk.edit.files}.
#'		This applies to all files defined in \code{create}.
#' @param load Logical, if \code{TRUE} and \code{"pmap"} in \code{create}, RKWard will automatically add the created .pluginmap file
#'		to its menu structure by calling \code{rk.load.pluginmaps}. You can then try the plugin immediately.
#' @param show Logical, if \code{TRUE} and \code{"pmap"} in \code{create}, RKWard will automatically call the created plugin after
#'		it was loaded (i.e., this implies and also sets \code{load=TRUE}).
#' @param indent.by A character string defining the indentation string to use.
#' @return Character string with the path to the plugin root directory.
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export
#' @examples
#' \dontrun{
#' # a simple example with only basic information
#' about.info <- rk.XML.about(
#' 	name="Square the circle",
#' 	author=c(
#' 		person(given="E.A.", family="Dölle",
#' 			email="doelle@@eternalwondermaths.example.org", role="aut"),
#' 		person(given="A.", family="Assistant",
#' 			email="alterego@@eternalwondermaths.example.org", role=c("cre","ctb"))
#' 		))
#' 
#' rk.plugin.skeleton("Square the Circle", about=about.info)
#' 
#' # a more complex example, already including some dialog elements
#' about.info <- rk.XML.about(
#' 	name="Square the circle",
#' 	author=c(
#' 		person(given="E.A.", family="Dölle",
#' 			email="doelle@@eternalwondermaths.example.org", role="aut"),
#' 		person(given="A.", family="Assistant",
#' 			email="alterego@@eternalwondermaths.example.org", role=c("cre","ctb"))
#' 		),
#' 	about=list(
#' 		desc="Squares the circle using Heisenberg compensation.",
#' 		version="0.1-3",
#' 		date=Sys.Date(),
#' 		url="http://eternalwondermaths.example.org/23/stc.html",
#' 		license="GPL",
#' 		category="Geometry"),
#' 	dependencies=list(
#' 		rkward.min="0.5.3",
#' 		rkward.max="",
#' 		R.min="2.10",
#' 		R.max=""),
#' 	package=list(
#' 		c(name="heisenberg", min="0.11-2", max="",
#' 			repository="http://rforge.r-project.org"),
#' 		c(name="DreamsOfPi", min="0.2", max="", repository="")),
#' 	pluginmap=list(
#' 		c(name="heisenberg.pluginmap", url="http://eternalwondermaths.example.org/hsb"))
#' )
#'
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
#' rk.plugin.skeleton("Square the Circle", about=about.info,
#'   xml=list(dialog=test.tabbook), overwrite=TRUE)
#' }

rk.plugin.skeleton <- function(name, about=NULL, path=tempdir(),
	provides=c("logic", "dialog"),
	scan=c("var", "saveobj", "settings"),
	xml=list(), js=list(), pluginmap=list(), rkh=list(),
	overwrite=FALSE, tests=TRUE, lazyLoad=TRUE,
	create=c("pmap", "xml", "js", "rkh", "desc"),
	edit=FALSE, load=FALSE, show=FALSE, indent.by="\t"){
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
		# also stop creation of DESCRIPTION file
		create <- create[!create %in% "desc"]
	}

	# define paths an file names
	main.dir <- file.path(path, name)
	description.file <- file.path(main.dir, "DESCRIPTION")
	rkward.dir <- file.path(main.dir, "inst", "rkward")
	plugin.dir <- file.path(rkward.dir, "plugins")
	# the basic file names
	plugin.fname.pluginmap <- paste(name, ".pluginmap", sep="")
	plugin.fname.xml <- paste(name, ".xml", sep="")
	plugin.fname.js <- paste(name, ".js", sep="")
	plugin.fname.rkh <- paste(name, ".rkh", sep="")
	# file names with paths
	plugin.pluginmap <- file.path(rkward.dir, plugin.fname.pluginmap)
	plugin.xml <- file.path(plugin.dir, plugin.fname.xml)
	plugin.js <- file.path(plugin.dir, plugin.fname.js)
	plugin.rkh <- file.path(plugin.dir, plugin.fname.rkh)
	tests.main.dir <- file.path(rkward.dir, "tests")
	tests.dir <- file.path(rkward.dir, "tests", name)
	testsuite.file <- file.path(tests.main.dir, "testsuite.R")

	checkCreateFiles <- function(file.name, ow=overwrite){
		if(all(file.exists(file.name), as.logical(ow)) | !file.exists(file.name)){
			return(TRUE)
		} else {
			warning(paste("Skipping existing file ", file.name, ".", sep=""))
			return(FALSE)
		}
	}

	# check if we can access the given root directory
	# create it, if necessary
	if(!file_test("-d", main.dir)){
		stopifnot(dir.create(main.dir, recursive=TRUE))
		message(paste("Created directory ", main.dir, ".", sep=""))
	} else {}

	# create directory structure
	if(!file_test("-d", plugin.dir)){
		stopifnot(dir.create(plugin.dir, recursive=TRUE))
		message(paste("Created directory ", plugin.dir, ".", sep=""))
	} else {}
	if(isTRUE(tests) & !file_test("-d", tests.dir)){
		stopifnot(dir.create(tests.dir, recursive=TRUE))
		message(paste("Created directory ", tests.dir, ".", sep=""))
	} else {}

	## create plugin.xml
	if("xml" %in% create){
		if(isTRUE(checkCreateFiles(plugin.xml))){
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
				pluginmap=paste("../", name, ".pluginmap", sep=""))
			cat(pasteXMLTree(XML.plugin, shine=1, indent.by=indent.by), file=plugin.xml)
		} else {}
		if(isTRUE(edit)){
			rk.edit.files(plugin.xml, title=plugin.fname.xml, prompt=FALSE)
		} else {}
	} else {}

	## create plugin.js
	if("js" %in% create){
		if(isTRUE(checkCreateFiles(plugin.js))){
			# require=c(), variables=NULL, preprocess=NULL, calculate=NULL, printout=NULL, results.header=NULL
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
				js[["variables"]] <- paste(js[["variables"]], rk.JS.scan(XML.plugin), sep="\n")
			} else {}
			if("saveobj" %in% scan){
				js[["printout"]] <- paste(js[["printout"]], rk.JS.saveobj(XML.plugin), sep="\n")
			} else {}
			JS.code <- rk.JS.doc(
				require=js[["require"]],
				variables=js[["variables"]],
				results.header=js[["results.header"]],
				preprocess=js[["preprocess"]],
				calculate=js[["calculate"]],
				printout=js[["printout"]],
				indent.by=indent.by)
			cat(JS.code, file=plugin.js)
		} else {}
		if(isTRUE(edit)){
			rk.edit.files(plugin.js, title=plugin.fname.js, prompt=FALSE)
		} else {}
	} else {}

	## create plugin.rkh
	if("rkh" %in% create){
		if(isTRUE(checkCreateFiles(plugin.rkh))){
			# summary=NULL, usage=NULL, sections=NULL, settings="scan", related=NULL, technical=NULL
			got.rkh.options <- names(js)
			# if settings were defined manually, this overwrites the scan
			if("settings" %in% scan & !"settings" %in% got.rkh.options){
				rkh[["settings"]] <- rk.rkh.settings(rk.rkh.scan(XML.plugin))
			} else {}
			for (this.opt in c("summary", "usage", "sections", "settings", "related", "technical")){
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
			cat(pasteXMLTree(rkh.doc, shine=1, indent.by=indent.by), file=plugin.rkh)
		} else {}
		if(isTRUE(edit)){
			rk.edit.files(plugin.rkh, title=plugin.fname.rkh, prompt=FALSE)
		} else {}
	} else {}

	## create plugin.pluginmap
	if("pmap" %in% create){
		if(isTRUE(checkCreateFiles(plugin.pluginmap))){
			got.pm.options <- names(pluginmap)
			for (this.opt in c("hierarchy", "require")){
				if(!this.opt %in% got.pm.options) {
					pluginmap[[this.opt]] <- eval(formals(rk.XML.pluginmap)[[this.opt]])
				} else {}
			}
			XML.pluginmap <- rk.XML.pluginmap(
				name=name.orig,
				about=about,
				components=paste("plugins/", name, ".xml", sep=""),
				hierarchy=pluginmap[["hierarchy"]],
				require=pluginmap[["require"]],
				hints=TRUE)
			cat(pasteXMLTree(XML.pluginmap, shine=2, indent.by=indent.by), file=plugin.pluginmap)
		} else {}
		if(isTRUE(edit)){
			rk.edit.files(plugin.pluginmap, title=plugin.fname.pluginmap, prompt=FALSE)
		} else {}
		if(isTRUE(load) | isTRUE(show)){
			rk.load.pluginmaps(plugin.pluginmap)
			if(isTRUE(show)){
				# call the plugin; reconstructed the ID generation from rk.XML.pluginmap()
				plugin.ID <- auto.ids(paste(name, name, sep=""), prefix=ID.prefix("component"), chars=25)
				rk.call.plugin(paste("rkward::", plugin.ID, sep=""))
			} else {}
		} else {}
	} else {}

	## create testsuite.R
	if(isTRUE(tests) & isTRUE(checkCreateFiles(testsuite.file))){
		testsuite.doc <- rk.testsuite.doc(name=name)
		cat(testsuite.doc, file=testsuite.file)
	} else {}

	## create DESCRIPTION file
	if("desc" %in% create){
		if(isTRUE(checkCreateFiles(description.file))){
			authors <- XML2person(about.node, eval=TRUE)
			all.authors <- format(get.by.role(authors, role="aut"),
				include=c("given", "family", "email"), braces=list(email=c("<", ">")))
			all.maintainers <- format(get.by.role(authors, role="cre"),
				include=c("given", "family", "email"), braces=list(email=c("<", ">")))

## TODO: check and add the commented values here:
## especially dependencies must be created from 'about'
			desc <- data.frame(
				Package=name,
				Type="Package",
				Title=about.node@attributes[["name"]],
				Version=about.node@attributes[["version"]],
				Date=about.node@attributes[["releasedate"]],
				Author=all.authors,
				AuthorsR=XML2person(about.node, eval=FALSE),
				Maintainer=all.maintainers,
#				Depends="R (>= 2.9.0)",
				Enhances="rkward",
				Description=about.node@attributes[["shortinfo"]],
				License=about.node@attributes[["license"]],
#				Encoding="UTF-8",
				LazyLoad=ifelse(isTRUE(lazyLoad), "yes", "no"),
				URL=about.node@attributes[["url"]],
				stringsAsFactors=FALSE)

			# i have no clue how to circumvent this workaround:
			desc$`Authors@R` <- desc[["AuthorsR"]]
			desc <- subset(desc, select=-AuthorsR)

			write.dcf(desc, file=description.file)
		} else {}
		if(isTRUE(edit)){
			rk.edit.files(description.file, title="DESCRIPTION", prompt=FALSE)
		} else {}
	} else {}

	return(main.dir)
}
