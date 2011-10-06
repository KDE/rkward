#' Create skeleton for RKWard plugins
#'
#' @param name Character sting, name of the plugin package.
#' @param about A list with descriptive information on the plugin, its authors and dependencies.
#'		At the very least you must specify \code{name} and \code{author}.
#'		See \code{\link[rkwarddev:rk.XML.about]{rk.XML.about}} for details and a full list of elements!
#'		If \code{NULL}, no \code{DESCRIPTION} file will be created either.
#' @param path Character sting, path to the main directory where the skeleton should be created.
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
#' @param dial.require A character vector with names of R packages that the dialog requires.
#' @param overwrite Logical, whether existing files should be replaced. Defaults to \code{FALSE}.
#' @param tests Logical, whether directories and files for plugin tests should be created.
#'		Defaults to \code{TRUE}.
#' @param lazyLoad Logical, whether the package should be prepared for lazy loading or not. Should be left \code{TRUE},
#'		unless you have very good reasons not to.
#' @param menu A character string with instructions where to place the plugin in the menu hierarchy, Valid values are
#'		\code{"file"}, \code{"edit"}, \code{"view"}, \code{"workspace"}, \code{"run"}, \code{"data"},
#'		\code{"analysis"}, \code{"plots"}, \code{"distributions"}, \code{"windows"}, \code{"settings"} and \code{"help"}.
#'		Anything else will place it in a "test" menu.
#' @param results.header A character string to headline the printed results.
#' @param JS.prep A character string with JavaScript code to be included in the \code{preprocess()} function of the .js file. This string will be
#'		pasted as-is, see \code{\link[rkwarddev:rk.JS.doc]{rk.JS.doc}}.
#' @param JS.calc Either a character string with JavaScript code to be included in the \code{calculate()} function of the .js file. This string will be
#'		pasted as-is, see \code{\link[rkwarddev:rk.JS.doc]{rk.JS.doc}}.
#' @param JS.prnt A character string with JavaScript code to be included in the \code{printout()} function of the .js file. This string will be
#'		pasted as-is, see \code{\link[rkwarddev:rk.JS.doc]{rk.JS.doc}}.
#' @param var.scan Logical, if \code{TRUE} \code{\link{rk.JS.scan}} will be called automatically to  to define the needed variables
#'		which will be added to the code in the \code{calculate()} function.
#' @param saveobj.scan Logical, if \code{TRUE} \code{\link{rk.JS.saveobj}} will be called automatically to generate code to save R results
#'		which will be appended to the code in the \code{printout()} function.
#' @param summary An object of class \code{XiMpLe.node} to be pasted as the \code{<summary>} section of the help file. See
#'		\code{\link[rkwarddev:rk.rkh.summary]{rk.rkh.summary}} for details.
#' @param usage An object of class \code{XiMpLe.node} to be pasted as the \code{<usage>} section of the help file. See
#'		\code{\link[rkwarddev:rk.rkh.usage]{rk.rkh.usage}} for details.
#' @param sections A (list of) objects of class \code{XiMpLe.node} to be pasted as \code{<section>} sections of the help file. See
#'		\code{\link[rkwarddev:rk.rkh.section]{rk.rkh.section}} for details.
#' @param settings Either an object of class \code{XiMpLe.node} to be pasted as the \code{<settings>} section of the help file (see
#'		\code{\link[rkwarddev:rk.rkh.settings]{rk.rkh.settings}} for details), or the special value "scan", in which case
#'		\code{\link{rk.rkh.scan}} will be called automatically to prepare empty \code{<setting>} nodes according to the plugin structure.
#' @param related An object of class \code{XiMpLe.node} to be pasted as the \code{<related>} section. See
#'		\code{\link[rkwarddev:rk.rkh.related]{rk.rkh.related}} for details.
#' @param technical An object of class \code{XiMpLe.node} to be pasted as the \code{<technical>} section. See
#'		\code{\link[rkwarddev:rk.rkh.technical]{rk.rkh.technical}} for details.
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
#' @return Character string with the path to the plugin root directory.
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export
#' @examples
#' \dontrun{
#' # a simple example with only basic information
#' about.info <- list(
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
#' about.info <- list(
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
#'   dialog=test.tabbook, overwrite=TRUE)
#' }

rk.plugin.skeleton <- function(name, about=NULL, path=tempdir(), dialog=NULL, wizard=NULL, logic=NULL, snippets=NULL,
	provides=c("logic", "dialog"), dial.require=c(), overwrite=FALSE, tests=TRUE, lazyLoad=TRUE, menu="test", results.header=NULL,
	JS.prep=NULL, JS.calc=NULL, JS.prnt=NULL, var.scan=TRUE, saveobj.scan=TRUE,
	summary=NULL, usage=NULL, sections=NULL, settings="scan", related=NULL, technical=NULL,
	create=c("pmap", "xml", "js", "rkh", "desc"), edit=FALSE, load=FALSE, show=FALSE){
	# to besure, remove all non-character symbols from name
	name.orig <- name
	name <- clean.name(name)

	if(!is.null(about)){
		# create an about.node, which probably has some default values
		about.node <- rk.XML.about(
			name=about[["name"]],
			author=about[["author"]],
			about=about[["about"]],
			dependencies=about[["dependencies"]],
			package=about[["package"]],
			pluginmap=about[["pluginmap"]]
		)
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
			XML.plugin <- rk.XML.plugin(
				name=name,
				label=name.orig,
				dialog=dialog,
				wizard=wizard,
				logic=logic,
				snippets=snippets,
				provides=provides,
				pluginmap=paste("../", name, ".pluginmap", sep=""))
			cat(pasteXMLTree(XML.plugin, shine=1), file=plugin.xml)
		} else {}
		if(isTRUE(edit)){
			rk.edit.files(plugin.xml, title=plugin.fname.xml, prompt=FALSE)
		} else {}
	} else {}

	## create plugin.js
	if("js" %in% create){
		if(isTRUE(checkCreateFiles(plugin.js))){
			if(is.null(results.header)){
				results.header <- paste(name.orig, " results", sep="")
			} else {}
			if(isTRUE(var.scan)){
				variables <- rk.JS.scan(XML.plugin)
			} else {
				variables <- NULL
			}
			if(isTRUE(saveobj.scan)){
				JS.prnt <- paste(JS.prnt, rk.JS.saveobj(XML.plugin), sep="\n")
			} else {}
			JS.code <- rk.JS.doc(
				require=dial.require,
				variables=variables,
				results.header=results.header,
				preprocess=JS.prep,
				calculate=JS.calc,
				printout=JS.prnt)
			cat(JS.code, file=plugin.js)
		} else {}
		if(isTRUE(edit)){
			rk.edit.files(plugin.js, title=plugin.fname.js, prompt=FALSE)
		} else {}
	} else {}

	## create plugin.rkh
	if("rkh" %in% create){
		if(isTRUE(checkCreateFiles(plugin.rkh))){
			if(identical(settings, "scan")){
				settings <- rk.rkh.settings(rk.rkh.scan(XML.plugin))
			} else {}
			rkh.doc <- rk.rkh.doc(
				summary=summary,
				usage=usage,
				sections=sections,
				settings=settings,
				related=related,
				technical=technical,
				title=rk.rkh.title(name.orig))
			cat(pasteXMLTree(rkh.doc, shine=1), file=plugin.rkh)
		} else {}
		if(isTRUE(edit)){
			rk.edit.files(plugin.rkh, title=plugin.fname.rkh, prompt=FALSE)
		} else {}
	} else {}

	## create plugin.pluginmap
	if("pmap" %in% create){
		if(isTRUE(checkCreateFiles(plugin.pluginmap))){
			XML.pluginmap <- rk.XML.pluginmap(
				name=name.orig,
				about=about,
				components=paste("plugins/", name, ".xml", sep=""),
				hierarchy=as.character(menu),
				hints=TRUE)
			cat(pasteXMLTree(XML.pluginmap, shine=2), file=plugin.pluginmap)
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
			all.authors <- format(get.by.role(about[["author"]], role="aut"),
				include=c("given", "family", "email"), braces=list(email=c("<", ">")))
			all.maintainers <- format(get.by.role(about[["author"]], role="cre"),
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
				AuthorsR=paste(deparse(about[["author"]]), collapse=" "),
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
