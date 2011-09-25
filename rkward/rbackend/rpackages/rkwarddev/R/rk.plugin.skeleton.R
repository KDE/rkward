#' Create skeleton for RKWard plugins
#'
#' @param name Character sting, name of the plugin package.
#' @param about A list with descriptive information on the plugin, its authors and dependencies.
#'		At the very least you must specify \code{name} and \code{author}.
#'		See \code{\link[rkwarddev:rk.XML.about]{rk.XML.about}} for details and a full list of elements!
#' @param path Character sting, path to the main directory where the skeleton should be created.
#' @param dialog A list of objects of class XiMpLe.node. If provided, will be included in the
#'		created plugin XML file as the dialog.
#' @param wiz.dialog Like \code{dialog}, but will be included as the wizard dialog code (and only if \code{wizard=TRUE} as well).
#' @param dial.require A character vector with names of R packages that the dialog requires.
#' @param overwrite Logical, whether existing files should be replaced. Defaults to \code{FALSE}.
#' @param wizard Logical, whether a \code{<wizard>} section should be added to the \code{<dialog>} section.
#'		Defaults to \code{FALSE}.
#' @param tests Logical, whether directories and files for plugin tests should be created.
#'		Defaults to \code{TRUE}.
#' @param lazyLoad Logical, whether the package should be prepared for lazy loading or not. Should be left \code{TRUE},
#'		unless you have very good reasons not to.
#' @param logic An object of class \code{XiMpLe.node} to be pasted as the \code{<logic>} section (see
#'		\code{\link[rkwarddev:rk.XML.logic]{rk.XML.logic}}.
#' @param JS.prep A character string with JavaScript code to be included in the \code{preprocess()} function. This string will be
#'		pasted as-is, see \code{\link[rkwarddev:rk.JS.doc]{rk.JS.doc}}.
#' @param JS.calc A character string with JavaScript code to be included in the \code{calculate()} function. This string will be
#'		pasted as-is, see \code{\link[rkwarddev:rk.JS.doc]{rk.JS.doc}}.
#' @param JS.prnt A character string with JavaScript code to be included in the \code{printout()} function. This string will be
#'		pasted as-is, see \code{\link[rkwarddev:rk.JS.doc]{rk.JS.doc}}.
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
#' test.tabbook <- rk.XML.tabbook("My Tabbook", tab.labels=c("First Tab",
#'   "Second Tab"), children=list(test.checkboxes, test.vars))
#' test.plugin <- rk.XML.plugin("My test", label="Check this out",
#'   children=test.tabbook)
#' 
#' rk.plugin.skeleton("Square the Circle", about=about.info,
#'   dialog=test.tabbook, overwrite=TRUE)
#' }

rk.plugin.skeleton <- function(name, about, path=tempdir(), dialog=list(), wiz.dialog=list(),
	dial.require=c(), overwrite=FALSE, wizard=FALSE, tests=TRUE, lazyLoad=TRUE, logic=NULL,
	JS.prep=NULL, JS.calc=NULL, JS.prnt=NULL, create=c("pmap", "xml", "js", "rkh", "desc"), edit=FALSE){
	# to besure, remove all non-character symbols from name
	name.orig <- name
	name <- gsub("[[:space:]]*[^[:alnum:]]*", "", name)
	if(!identical(name.orig, name)){
		message(paste("For filenames ", sQuote(name.orig), " was renamed to ", sQuote(name), ".", sep=""))
	} else {}
	# create an about.node, which probably has some default values
	about.node <- rk.XML.about(
		name=about[["name"]],
		author=about[["author"]],
		about=about[["about"]],
		dependencies=about[["dependencies"]],
		package=about[["package"]],
		pluginmap=about[["pluginmap"]]
	)
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
	if("xml" %in% create & isTRUE(checkCreateFiles(plugin.xml))){
		if(isTRUE(wizard))
			plugin.provides <- c("logic","dialog","wizard")
		else {
			plugin.provides <- c("logic","dialog")
		}
		XML.plugin <- rk.XML.plugin(
			name=name,
			label=name.orig,
			children=dialog,
			wiz.children=wiz.dialog,
			logic=logic,
			provides=plugin.provides,
			pluginmap=paste("../", name, ".pluginmap", sep=""))
		cat(pasteXMLTree(XML.plugin, shine=1), file=plugin.xml)
		if(isTRUE(edit)){
			rk.edit.files(plugin.xml, title=plugin.fname.xml, prompt=FALSE)
		} else {}
	} else {}

	## create plugin.js
	if("js" %in% create & isTRUE(checkCreateFiles(plugin.js))){
		JS.code <- rk.JS.doc(
			require=dial.require,
			variables=rk.JS.scan(XML.plugin),
			results.header=paste(name.orig, " results", sep=""),
			preprocess=JS.prep,
			calculate=JS.calc,
			printout=JS.prnt)
		cat(JS.code, file=plugin.js)
		if(isTRUE(edit)){
			rk.edit.files(plugin.js, title=plugin.fname.js, prompt=FALSE)
		} else {}
	} else {}

	## create plugin.rkh
	if("rkh" %in% create & isTRUE(checkCreateFiles(plugin.rkh))){
		rkh.doc <- rk.rkh.doc(settings=rk.rkh.scan(XML.plugin))
		cat(pasteXMLTree(rkh.doc, shine=1), file=plugin.rkh)
		if(isTRUE(edit)){
			rk.edit.files(plugin.rkh, title=plugin.fname.rkh, prompt=FALSE)
		} else {}
	} else {}

	## create plugin.pluginmap
	if("pmap" %in% create & isTRUE(checkCreateFiles(plugin.pluginmap))){
		XML.pluginmap <- rk.XML.pluginmap(
			name=name.orig,
			about=about,
			components=paste(name, ".xml", sep=""),
			plugin.dir="plugins",
			hierarchy="analysis")
		cat(pasteXMLTree(XML.pluginmap), file=plugin.pluginmap)
		if(isTRUE(edit)){
			rk.edit.files(plugin.pluginmap, title=plugin.fname.pluginmap, prompt=FALSE)
		} else {}
	} else {}

	## create testsuite.R
	if(isTRUE(tests) & isTRUE(checkCreateFiles(testsuite.file))){
		testsuite.doc <- rk.testsuite.doc(name=name)
		cat(testsuite.doc, file=testsuite.file)
	} else {}

	# create DESCRIPTION file
	if("desc" %in% create & isTRUE(checkCreateFiles(description.file))){
		all.authors <- format(get.by.role(about[["author"]], role="aut"),
			include=c("given", "family", "email"), braces=list(email=c("<", ">")))
		all.maintainers <- format(get.by.role(about[["author"]], role="cre"),
			include=c("given", "family", "email"), braces=list(email=c("<", ">")))

## TODO: check and add the commented values here:
		desc <- data.frame(
			Package=name,
			Type="Package",
			Title=about.node@attributes[["name"]],
			Version=about.node@attributes[["version"]],
			Date=about.node@attributes[["releasedate"]],
			Author=all.authors,
			AuthorsR=paste(deparse(about[["author"]]), collapse=" "),
			Maintainer=all.maintainers,
#			Depends="R (>= 2.9.0)",
			Enhances="rkward",
			Description=about.node@attributes[["shortinfo"]],
			License=about.node@attributes[["license"]],
#			Encoding="UTF-8",
			LazyLoad=ifelse(isTRUE(lazyLoad), "yes", "no"),
			URL=about.node@attributes[["url"]],
			stringsAsFactors=FALSE)

		# i have no clue how to circumvent this workaround:
		desc$`Authors@R` <- desc[["AuthorsR"]]
		desc <- subset(desc, select=-AuthorsR)

		write.dcf(desc, file=description.file)
		if(isTRUE(edit)){
			rk.edit.files(description.file, title="DESCRIPTION", prompt=FALSE)
		} else {}
	} else {}

	return(invisible(NULL))
}
