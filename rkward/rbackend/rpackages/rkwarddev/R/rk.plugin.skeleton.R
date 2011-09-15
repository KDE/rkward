#' Create skeleton for RKWard plugins
#'
#' @param name Character sting, name of the plugin package.
#' @param about A list with descriptive information on the plugin, its authors and dependencies.
#'		At the very least you must specify \code{name} and \code{author}.
#'		See \code{\link[XiMpLe:rk.XML.about]{rk.XML.about}} for details and a full list of elements!
#' @param path Character sting, path to the main directory where the skeleton should be created.
#' @param dialog A list of objects of class XiMpLe.node. If provided, will be included in the
#'		created plugin XML file as the dialog.
#' @param dial.require A character vector with names of R packages that the dialog requires.
#' @param overwrite Logical, whether existing files should be replaced. Defaults to \code{FALSE}.
#' @param tests Logical, whether directories and files for plugin tests should be created.
#'		Defaults to \code{TRUE}.
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

rk.plugin.skeleton <- function(name, about, path=tempdir(), dialog=list(), dial.require=c(), overwrite=FALSE, tests=TRUE, lazyLoad=TRUE){
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
	plugin.pluginmap <- file.path(rkward.dir, paste(name, ".pluginmap", sep=""))
	plugin.xml <- file.path(plugin.dir, paste(name, ".xml", sep=""))
	plugin.js <- file.path(plugin.dir, paste(name, ".js", sep=""))
	plugin.rkh <- file.path(plugin.dir, paste(name, ".rkh", sep=""))
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
	if(isTRUE(checkCreateFiles(plugin.xml))){
		XML.plugin <- rk.XML.plugin(
			name=name,
			label=name.orig,
			children=dialog,
			pluginmap=paste("../", name, ".pluginmap", sep=""))
		cat(pasteXMLTree(XML.plugin, shine=1), file=plugin.xml)
	} else {}

	## create plugin.js
	if(isTRUE(checkCreateFiles(plugin.js))){
		JS.code <- rk.JS.doc(
			require=dial.require,
			variables=rk.JS.scan(XML.plugin),
			results.header=paste(name.orig, " results", sep=""))
		cat(JS.code, file=plugin.js)
	} else {}

	## create plugin.rkh
	if(isTRUE(checkCreateFiles(plugin.rkh))){
		rkh.doc <- rk.rkh.doc(settings=rk.rkh.scan(XML.plugin))
		cat(pasteXMLTree(rkh.doc, shine=1), file=plugin.rkh)
	} else {}

	## create plugin.pluginmap
	if(isTRUE(checkCreateFiles(plugin.pluginmap))){
		XML.pluginmap <- rk.XML.pluginmap(
			name=name.orig,
			about=about,
			components=paste(name, ".xml", sep=""),
			plugin.dir="plugins",
			hierarchy="analysis")
		cat(pasteXMLTree(XML.pluginmap), file=plugin.pluginmap)
	} else {}

	## create testsuite.R
	if(isTRUE(tests) & isTRUE(checkCreateFiles(testsuite.file))){
		testsuite.doc <- rk.testsuite.doc(name=name)
		cat(testsuite.doc, file=testsuite.file)
	} else {}

	# create DESCRIPTION file
	if(isTRUE(checkCreateFiles(description.file))){
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
			AuthorsR=paste(deparse(about.info[["author"]]), collapse=" "),
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
	} else {}

	return(invisible(NULL))
}
