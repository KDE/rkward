#' Various label related utility functions
#' 
#' \code{rk.get.label} retrieves the rkward label (if any) of the given object.
#' 
#' \code{rk.set.label} sets the rkward label for the given object.
#' 
#' \code{rk.get.short.name} creates a short name for the given object.
#' 
#' \code{rk.get.description} creates descriptive string(s) for each of the
#' arguments in "\code{\dots{}}"; collapsing into a single string using
#' \code{paste.sep} (if not NULL). If \code{is.substitute=TRUE}, the arguments
#' will be deparsed, first, which can be useful when using
#' \code{rk.get.description} inside a function.
#' 
#' \code{rk.list.names} returns the names of the arguments passed as
#' \code{...}; when using \code{rk.list.names} inside a function, it may be
#' necessary to increase the \code{deparse.level} level.
#' 
#' \code{rk.list} returns a list of its arguments, with \code{names} set as
#' returned by \code{rk.get.description()}. This can be used as a drop-in
#' replacement for \code{\link{list}}.
#' 
#' @aliases rk.get.label rk.set.label rk.get.short.name rk.get.description
#'   rk.list.names rk.list
#' @param x any R object
#' @param label a string, to set the label attribute of an object
#' @param envir an environment, where the attribute is evaluated
#' @param paste.sep a string, used as the \code{collapse} argument for paste
#' @param is.substitute a logical (not NA). See Details.
#' @return \code{rk.set.label} returns the result of the evaluation of "setting
#'   the label" while the others return a character vector.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@lists.sourceforge.net}
#' @keywords utilities
#' @rdname rk.get.label
#' @examples
#' 
#' x <- data.frame(a=c(1:3), b=c(2:4))
#' rk.set.label(x[["a"]], "First column")
#' rk.get.short.name (x$a)                   # "x$a"
#' rk.get.label (x$a)                        # "First column"
#' rk.get.description (x$a)                  # "x$a (First column)"
#' rk.list.names (x, x$a, x$b)               # "x" "x$a" "x$b"
#' names (rk.list (x$a, x$b))                # "x$a (First column)" "x$b"
#' 
# retrieve the rkward label (if any) of the given object
"rk.get.label" <- function (x) {
	if (is.call (x) || is.name (x)) {
		ret <- attr (eval (x), ".rk.meta")[names (attr (eval (x), ".rk.meta")) == "label"]
	} else {
		ret <- attr (x, ".rk.meta")[names (attr (x, ".rk.meta")) == "label"]
	}
	as.character (as.vector (ret))
}

# set rkward label
"rk.set.label" <- function (x, label, envir=parent.frame()) {
	if (is.call (x) || is.name (x)) {
		meta <- attr (eval (x, envir=envir), ".rk.meta")
	} else {
		meta <- attr (x, ".rk.meta")
	}
	meta[["label"]] <- as.character (label)
	eval(substitute(attr(x, ".rk.meta") <- meta), envir = envir)
}

# get a short name for the given object
"rk.get.short.name" <- function (x) {
	if (is.call (x) || is.name (x)) {
		.rk.make.short.name (deparse (x))
	} else {
		.rk.make.short.name (deparse (substitute (x)))
	}
}

# make a short name from the given arg (a character string)
# e.g. return "b" for a[["b"]] (but 'a::"b"' for a::"b"
".rk.make.short.name" <- function (x) {
	splt <- strsplit (x, "[[\"", fixed=TRUE)[[1]]
	spltlen <- length (splt)
	if (spltlen == 1) {
		splt[1]
	} else {
		strsplit (splt[spltlen], "\"]]", fixed=TRUE)[[1]][1]
	}
}

# get descriptive strings for each of the arguments in ...
"rk.get.description" <- function (..., paste.sep=NULL, is.substitute=FALSE) {
	args <- list(...)
	if (is.substitute) {
		argnames <- list ()
		j <- 1
		for (symb in list (...)) {
			argnames[j] <- deparse (symb)
			j <- j + 1
		}
	} else {
		argnames <- rk.list.names (...)
	}
	descript <- c ()

	for (i in 1:length (args)) {
		lbl <- rk.get.label (args[[i]])
		if (is.substitute) {
			shortname <- .rk.make.short.name (as.character (argnames[i]))
		} else {
			shortname <- .rk.make.short.name (argnames[i])
		}

		if (is.null (lbl) || (length (lbl) < 1)) descript[i] <- shortname
		else descript[i] <- paste (shortname, " (", lbl, ")", sep="")
	}

	if (is.null (paste.sep)) {
		descript
	} else {
		paste (descript, collapse=paste.sep)
	}
}

# Drop-in replacement for list(). Returns a list of the given arguments, but with names set according to rk.get.description
"rk.list" <- function (...) {
	ret <- list (...)
	names (ret) <- rk.get.description (...)
	ret
}

# this is basically copied from R-base table (). Returns the arguments passed to ... as a character vector
"rk.list.names" <- function(..., deparse.level=2) {
	l <- as.list(substitute(list(...)))[-1]
	nm <- names(l)
	fixup <- if (is.null(nm)) 
		seq(along = l)
	else nm == ""
	dep <- sapply(l[fixup], function(x) switch(deparse.level + 1, "", if (is.symbol(x)) as.character(x) else "", deparse(x)[1]))
	if (is.null(nm)) 
		dep
	else {
		nm[fixup] <- dep
		nm
	}
}

#' Sync R object(s)
#' 
#' RKWard keeps an internal representation of objects in the R workspace. For
#' objects in the \code{.GlobalEnv}, this representation is updated after each
#' top-level statement. For the rare cases where this is not enough,
#' \code{rk.sync} can be used to update the representation of a single object,
#' \code{x}, while \code{rk.sync.global} scans the \code{.GlobalEnv} for new
#' and removed objects, and updates as appropriate.
#' 
#' These functions are rarely needed outside automated testing. However,
#' rk.sync() can be useful, if an object outside the \code{.GlobalEnv} has
#' changed, since this will not be detected automatically. Also, by default
#' RKWard does not recurse into environments when updating its representation
#' of objects. rk.sync() can be used, here, to inspect the objects inside
#' environments (see examples).
#' 
#' @aliases rk.sync rk.sync.global
#' @param x any R object to sync
#' @return \code{NULL}, invisibly.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@lists.sourceforge.net}
#' @seealso \url{rkward://page/rkward_workspace_browser}
#' @keywords utilities misc
#' @rdname rk.sync
#' @examples
#' 
#' rk.sync (rkward::rk.record.plot)
#' 
# should this really be public?
"rk.sync" <- function (x) {
	object <- deparse (substitute (x))
	.rk.do.call ("sync", object)
}

# should this really be public?
"rk.sync.global" <- function () {
	.rk.do.call("syncglobal", ls (envir=globalenv (), all.names=TRUE))
}

#' Edit / show an object / file
#' 
#' \code{rk.edit} can be used to edit an object in the RKWard data editor.
#' Currently only \link{data.frame}s are supported. This is similar to
#' \link{edit.data.frame}, but the function returns immediately, and the object
#' is edit asynchronously.
#' 
#' \code{rk.edit.files}, \code{rk.show.files}, and \code{rk.show.html} are
#' equivalent to \link{file.edit}, \link{file.show}, and \link{browseURL},
#' respectively, but use RKWard as text/html editor/viewer. Generally it is
#' recommended to use \link{file.edit}, \link{file.show}, and \link{browseURL},
#' instead. These will call the respective RKWard functions by default, when
#' run inside an RKWard session.
#' 
#' @aliases rk.edit rk.edit.files rk.show.files rk.show.html
#' @param x an object to edit.
#' @param file character vector, filenames to show or edit.
#' @param title character vector, of the same length as \code{file}; This can
#'   be used to give descriptive titles to each file, which will be displayed
#'   to the user.
#' @param wtitle character vector, of length 1. This will be used as the window
#'   title.
#' @param prompt logical of length 1. If TRUE (the default) a prompt is dialog
#'   is shown along with the files to show / edit.
#' @param delete a logical (not NA), when \code{TRUE} the shown file(s) are
#'   deleted after closing.
#' @return All functions described on this page return \code{NULL},
#'   unconditionally.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@lists.sourceforge.net}
#' @seealso \code{\link{edit}}, \code{\link{file.edit}},
#'   \code{\link{file.show}}, \code{\link{browseURL}}
#' @keywords utilities IO
#' @rdname rk.edit
#' @examples
#' 
#' ## Not run
#' x <- data.frame (a=c(1:3), b=c(2:4))
#' rk.edit(x)
#' 
"rk.edit" <- function (x) {
	object <- deparse (substitute (x))
	.rk.do.call ("edit", object)
}

#' RKWard file names
#' 
#' 
#' In RKWard the output is saved as a html file which is located at "~/.rkward"
#' by default. (\bold{TODO}: make this platform free). The name of this html
#' file can be retrieved and set using \code{rk.get.output.html.file} and
#' \code{rk.set.output.html.file}.
#' 
#' \code{rk.get.tempfile.name} returns a non-existing filename inside the
#' directory of the output file. It is mainly used by \link{rk.graph.on} to
#' create filenames suitable for storing images in the output. The filenames of
#' the temporary files are of the form
#' "\code{prefix}\emph{xyz}.\code{extension}". \code{rk.get.tempfile.name} is
#' somewhat misnamed. For truly temporary files, \link{tempfile} is generally
#' more suitable.
#' 
#' \code{rk.get.workspace.url} returns the url of workspace file which has been
#' loaded in RKWard, or NULL, if no workspace has been loaded. NOTE: This value
#' is note affected by running \code{load} in R, only by loading R workspaces
#' via the RKWard GUI.
#' 
#' @aliases rk.get.tempfile.name rk.get.workspace.url rk.get.output.html.file
#'   rk.set.output.html.file
#' @param prefix a string, used as a filename prefix when saving images to the
#'   output file
#' @param extension a string, used as a filename extension when saving images
#'   to the output file
#' @param x a string, giving the filename of the of the output file
#' @return \code{rk.get.tempfile.name}, \code{rk.get.output.html.file}, and
#'   \code{rk.get.workspace.url} return a string while
#'   \code{rk.set.output.html.file} returns \code{NULL}.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@lists.sourceforge.net}
#' @seealso \url{rkward://page/rkward_output}, \link{tempfile}, \link{file},
#'   \link{rk.print}
#' @keywords utilities IO
#' @rdname rk.get.tempfile.name
#' @examples
#' 
#' testfile.name <- rk.get.tempfile.name(prefix="test", extension=".txt")
#' testfile <- file(testfile.name)
#' cat("This is a test\n", file=testfile)
#' close(testfile)
#' unlink(testfile.name)
#' 
#' outfile <- rk.get.output.html.file()
#' 
#' ## Not run
#' rk.set.output.html.file("~/.rkward/another_file.html")
#' rk.header("Output on a different output file")
#' rk.show.html(rk.get.output.html.file())
#' rk.set.output.html.file(outfile)
#' 
"rk.get.tempfile.name" <- function (prefix="image", extension=".jpg") {
	return (.rk.do.plain.call ("get.tempfile.name", c (prefix, extension)))
}

"rk.get.workspace.url" <- function () {
	res <- .rk.do.plain.call ("getWorkspaceUrl")
	if (length (res)) res
	else NULL
}

"rk.get.output.html.file" <- function () {
	return (.rk.variables$.rk.output.html.file)
}

"rk.set.output.html.file" <- function (x) {
	stopifnot (is.character (x))
	assign (".rk.output.html.file", x, .rk.variables)

	if (!file.exists (x)) {
		.rk.cat.output (paste ("<?xml version=\"1.0\" encoding=\"", .Call ("rk.locale.name"), "\"?>\n", sep=""))
		.rk.cat.output (paste ("<html><head>\n<title>RKWard Output</title>\n", .rk.do.plain.call ("getCSSlink"), "</head>\n<body>\n", sep=""))
		# This initial output mostly to indicate the output is really there, just empty for now
		.rk.cat.output (paste ("<pre>RKWard output initialized on", date (), "</pre>\n"))
	}

	# needs to come after initialization, so initialization alone does not trigger an update during startup
	.rk.do.plain.call ("set.output.file", x, synchronous=FALSE)
	invisible (NULL)
}

#' Save or restore RKWard workplace
#' 

#' 
#' \code{rk.save.workplace} can be used to save a representation of the RKWard
#' workplace (i.e. which scripts, data edtiors and other windows are shown) to
#' a file. \code{rk.restore.workplace} restores an RKWard workplace as saved by
#' \code{rk.save.workplace}.
#' 
#' If the \code{file} parameter is omitted (or \code{NULL}), a suitable
#' filename is selected automatically. If a workspace has been loaded, this is
#' the URL of the workspace with an appended \code{.rkworkplace}. Otherwise a
#' filename in the RKWard directory, as generated by
#' \link{rk.get.tempfile.name}.
#' 
#' NOTE: Not all types of windows can be saved and restored. Esp. graphics
#' device windows will not be restored (but WILL be closed by
#' \code{rk.restore.workplace()}, if \code{close.windows} is TRUE).
#' 
#' @aliases rk.save.workplace rk.restore.workplace
#' @param file a character string giving the url of the file to save to, or
#'   NULL for automatic selection of a suitable file (see Details).
#' @param description For internal use, only. A character string describing the
#'   workplace status to save. Generally, you should leave this as the default
#'   value (\code{NULL}).
#' @param close.windows a logical; whether current windows should be closed
#'   before restoring.
#' @return Both functions return \code{NULL}.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@lists.sourceforge.net}
#' @seealso \url{rkward://page/rkward_for_r_users}, \link{rk.get.workspace.url}
#' @keywords utilities
#' @rdname rk.workplace
#' @examples
#' 
#' ## Not run
#' rk.save.workplace ()
#' rk.restore.workplace ()
#' ## End not run
#' 
"rk.save.workplace" <- function (file=NULL, description=NULL) {
	if (is.null (file)) {
		file <- rk.get.workspace.url ()
		if (is.null (file)) file <- rk.get.tempfile.name (prefix="unsaved", extension=".RData")
		file <- paste (file, "rkworkplace", sep=".")
	}
	if (is.null (description)) lines <- .rk.do.plain.call ("workplace.layout", "get")
	else lines <- description
	writeLines (lines, file)
}

"rk.restore.workplace" <- function (file=NULL, close.windows=TRUE) {
	if (is.null (file)) {
		if (exists (".rk.workplace.save", envir=globalenv (), inherits=FALSE)) {
			# For backwards compatibility with workspaces saved by RKWard 0.5.4 and earlier.
			# TODO: remove in time.
			lines <- as.character (.GlobalEnv$.rk.workplace.save)
			rm (list = c (".rk.workplace.save"), envir=globalenv ())
		} else {
			file <- rk.get.workspace.url ()
			if (is.null (file)) file <- rk.get.tempfile.name (prefix="unsaved", extension=".RData")
			file <- paste (file, "rkworkplace", sep=".")
		}
	}

	close <- "close"
	if (!isTRUE (close.windows)) close <- "noclose"
	if (!exists ("lines", inherits=FALSE)) lines <- readLines (file)
	.rk.do.plain.call ("workplace.layout", c ("set", close, lines), synchronous=FALSE)
	invisible (NULL)
}

#' Miscellaneous utility functions
#' 
#' \code{rk.rename.in.container} renames a named object (column/element) in a
#' data.frame/list without changing its position.
#' 
#' \code{rk.make.repos.string} just creates a R statement for \code{repos}. A
#' typical user should not need to use this function.
#' 
#' \code{rk.select.CRAN.mirror} is an in-house replacement for
#' \code{\link{chooseCRANmirror}} without changing \code{options ("repos")},
#' permanently. It uses native KDE gui and provides more information on each
#' mirror.
#' 
#' @aliases rk.misc rk.rename.in.container rk.make.repos.string
#'   rk.select.CRAN.mirror
#' @param x a data.frame or list.
#' @param old_name a string, the name of the column or element to be renamed.
#' @param new_name a string, the new name.
#' @param envir an environment where \code{x} is available.
#' @return \code{rk.rename.in.container} returns \code{NULL} on successfule
#'   renaming, otherwise an error.
#' 
#' \code{rk.make.repos.string} returns a valid R expression as a character
#'   string which can then be parsed and evaluated.
#' 
#' \code{rk.select.CRAN.mirror} returns the URL of the chosen mirror, as a
#'   string.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@lists.sourceforge.net}
#' @keywords attribute misc utilities
#' @rdname rk.misc
#' @examples
#' 
#' ## rk.rename.in.container
#' ir <- iris
#' str (ir)
#' rk.rename.in.container(ir, "Species", "Taxonomic.Group")
#' str (ir)
#' 
# renames a named object in a data.frame/list without changing it's position
# TODO: create a generic function instead, that can handle all kinds of renames
"rk.rename.in.container" <- function (x, old_name, new_name, envir=parent.frame()) {
	temp <- (names (x) == old_name)
	i = 1;
	for (val in temp) {
		if (val) {
			eval (substitute (names (x)[i] <- new_name), envir=envir)
			return ()
		}
		i = i+1;
	}
	error ("Could not find column with given name")
}

#' Print objects and results to output
#' 
#' Various utilty functions which can be used to print or export R objects to
#' the (html) output file. The output file can be accessed from Windows -> Show
#' Output. Basically, these functions along with the ones described in
#' \code{\link{rk.get.label}}, \code{\link{rk.get.tempfile.name}}, and
#' \code{\link{rk.graph.on}} can be used to create a HTML report.
#' 
#' \code{rk.print} prints/exports the given object to the output (html) file
#' using the \code{\link{HTML}} function. This requires the \code{R2HTML}
#' package. Additional arguments in \code{...} are passed on to
#' \code{\link{HTML}}.
#' 
#' \code{rk.print.literal} prints/exports the given object using a
#' \code{paste(x, collapse="\n")} construct to the output (html) file.
#' 
#' \code{rk.print.code} applies syntax highlighting to the given code string,
#' and writes it to the output (html) file.
#' 
#' \code{rk.header} prints a header / caption, possibly with parameters, to the
#' output file. See example.
#' 
#' \code{rk.results} is similar to \code{rk.print} but prints in a more
#' tabulated fashion. This has been implemented only for certain types of
#' \code{x}: tables, lists (or data.frames), and vectors. See example.
#' 
#' \code{rk.describe.alternatives} describes the alternative (H1) hypothesis of
#' a \code{htest}. This is similar to \code{stats:::print.htext} and makes
#' sense only when \code{x$alternatives} exists.
#' 
#' @aliases rk.print rk.print.code rk.print.literal rk.header rk.results
#'   rk.describe.alternative
#' @param x any R object to be printed/exported. A suitable list in case of
#'   \code{rk.describe.alternative}.
#' @param code a character vector (single string) of R code
#' @param title a string, used as a header for the html output
#' @param level an integer, header level. For example, \code{level=2} creates
#'   the header with \code{<h2></h>} tag.
#' @param parameters a list, preferably named, giving a list of "parameters" to
#'   be printed to the output
#' @param titles a character vector, giving the column headers for a html
#'   table.
#' @param print.rownames controls printing of rownames. TRUE to force printing,
#'   FALSE to suppress printing, omitted (default) to print rownames, unless
#'   they are plain row numbers.
#' @return \code{rk.describe.alternatives} returns a string while all other
#'   functions return \code{NULL}, invisibly.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@lists.sourceforge.net}
#' @seealso \code{\link{HTML}}, \code{\link{rk.get.output.html.file}},
#'   \code{\link{rk.get.description}}, \code{\link{rk.call.plugin}},
#'   \url{rkward://page/rkward_output}
#' @keywords utilities
#' @rdname rk.results
#' @examples
#' 
#' require (rkward)
#' require (R2HTML)
#' 
#' ## see the output: Windows->Show Output
#' ## stolen from the two-sample t-test plugin ;)
#' local({
#' x1 <- rnorm (100)
#' x2 <- rnorm (100, 2)
#' nm <- rk.get.description (x1,x2)
#' 
#' result <- t.test (x1, x2, alternative="less")
#' rk.print.code ("result <- t.test (x1, x2, alternative=\"less\")")
#' 
#' rk.header (result$method,
#'   parameters=list ("Comparing", paste (nm[1], "against", nm[2]),
#'   "H1", rk.describe.alternative (result),
#'   "Equal variances", "not assumed"))
#' 
#' rk.print.literal ("Raw data (first few rows):")
#' rk.print (head (cbind (x1,x2)), align = "left")
#' 
#' rk.print.literal ("Test results:")
#' rk.results (list (
#'   'Variable Name'=nm,
#'   'estimated mean'=result$estimate,
#'   'degrees of freedom'=result$parameter,
#'   t=result$statistic,
#'   p=result$p.value,
#'   'confidence interval percent'=(100 * attr(result$conf.int, "conf.level")),
#'   'confidence interval of difference'=result$conf.int ))
#' })
#' 
"rk.print" <- function(x,...) {
	htmlfile <- rk.get.output.html.file()
	if(require("R2HTML")==TRUE) {
		HTML(x, file=htmlfile,...)
	}
}

"rk.print.code" <- function(code) {
	.rk.cat.output (.rk.do.plain.call ("highlightRCode", as.character (code)))
}

"rk.header" <- function (title, parameters=list (), level=1) {
	sink (rk.get.output.html.file(), append=TRUE)
	on.exit (sink ())

	cat ("<h", level, ">", title, "</h", level, ">\n", sep="")
	# legacy handling: parameter=value used to be passed as parameter, value
	if (!is.null (names (parameters))) {
		pnames <- names (parameters)
		p <- list ()
		for (i in 1:length (parameters)) {
			p[i*2-1] <- pnames[i]
			p[i*2] <- parameters[i]
		}
		parameters <- p
	}
	if (length (parameters)) {
		cat ("<h", level + 1, ">Parameters</h", level + 1, ">\n<ul>", sep="")
		len <- length (parameters)
		i <- 2
		while (i <= len) {
			cat ("<li>", parameters[[i-1]], ": ", parameters[[i]], "</li>\n", sep="")
			i <- i + 2
		}
		cat ("</ul>\n")
	}
	if (level==1) cat (date ())
	cat ("<br />\n")
}

"rk.results" <- function (x, titles=NULL, print.rownames) {
	sink (rk.get.output.html.file(), append=TRUE)
	on.exit (sink ())

	# convert 2d tables to data.frames with values labelled
	if (is.table(x) && (length(dim(x)) == 2)) {
		rows = dim(x)[1]
		cols = dim(x)[2]
		if (is.null(titles)) {
			titles <- names(dimnames(x))
		}
		rn <- c ()   # row names
		for (row in 1:rows) rn[row] <- paste (titles[1], "=", dimnames(x)[[1]][row])
		x <- data.frame (cbind (x))
		rownames (x) <- as.character (rn)
		titles <- c ("", paste (titles[2], "=", names (internal)))
	}

	if (is.list (x)) {	# or a data.frame
		if (is.data.frame (x)) {
			# by default, print rownames, unless they are just plain row numbering
			if (missing (print.rownames)) print.rownames <- !isTRUE (all.equal (rownames (x), as.character (1:dim(x)[1])))
			if (isTRUE (print.rownames)) {
				x <- cbind (rownames (x), x)
				names (x)[1] <- '';
			}
		}
		if (is.null (titles)) {
			titles <- names (x)
		}

		cat ("<table border=\"1\">\n<tr>")
		try ({	# if anything fails, make sure the "</table>" is still printed
			for (i in 1:length (x)) {
				cat ("<td>", titles[i], "</td>", sep="")
			}
			cat ("</tr>\n")
	
			if (is.data.frame (x)) {
				for (row in 1:dim (x)[1]) {
					cat ("<tr>")
					for (col in 1:dim (x)[2]) {
						cat ("<td>", x[row, col], "</td>", sep="")
					}
					cat ("</tr>\n")
				}
			} else {		# generic list
				cat ("<tr>")
				for (col in x) {
					col <- as.vector (col)
					cat ("<td>")
					for (row in 1:length (col)) {
						if (row != 1) cat ("\n<br/>")
						cat (col[row])
					}
					cat ("</td>")
				}
				cat ("</tr>\n")
			}
		})
		cat ("</table>\n")
	} else if (is.vector (x)) {
		cat ("<h3>", titles[1], ": ", sep="")
		cat (x)
		cat ("</h3>")
	} else {
		stop ("uninmplemented")
	}
}

"rk.print.literal" <- function (x) {
	cat ("<pre>", paste (x, collapse="\n"), "</pre>\n", sep="", file=rk.get.output.html.file(), append=TRUE);
}

# Describe the alternative (H1) of an htest.
# This code adapted from stats:::print.htest
"rk.describe.alternative" <- function (x) {
	res <- ""
	if (!is.null(x$alternative)) {
		if (!is.null(x$null.value)) {
 			if (length(x$null.value) == 1) {
 				alt.char <- switch(x$alternative, two.sided = "not equal to", less = "less than", greater = "greater than")
 				res <- paste ("true", names(x$null.value), "is", alt.char, x$null.value)
 			} else {
 				res <- paste (x$alternative, "\nnull values:\n", x$null.value)
 			}
		} else {
			res <-  (x$alternative)
		}
	}
	res
}

"rk.make.repos.string" <- function () {
	x <- getOption ("repos")
	len <- length (x)
	ret <- sprintf ("c (")
	first <- TRUE
	for (i in 1:len) {
		if (first) {
			first <- FALSE
		} else {
			ret <- sprintf ("%s, ", ret)
		}
		if (!(is.null (names (x)) || (names (x)[i] == ""))) {
			ret <- sprintf ("%s%s=\"%s\"", ret, names (x)[i], x[i])
		} else {
			ret <- sprintf ("%s\"%s\"", ret, x[i])
		}
	}
	ret <- sprintf ("%s)", ret)
	ret
}

# utility to convert the rkward meta data of objects created in rkward < 0.4.0
# keep for a few months after 0.4.0 is out
"rk.convert.pre040" <- function (x) {
	if (missing (x)) {
		x <- list ()
		lst <- ls (all.names=TRUE, envir=globalenv ())
		for (childname in lst) {
	        	assign (childname, rk.convert.pre040 (get (childname, envir = globalenv ())), envir=globalenv ())
		}
		return (invisible ())
	}

	if (is.list (x)) {
		oa <- attributes (x)
		x <- lapply (x, function (X) rk.convert.pre040 (X))
		attributes (x) <- oa
	}

	a <- attr (x, ".rk.meta")
	if (is.null (a)) return (x)
	if (!is.data.frame (a)) return (x)
	if (length (a) < 1) return (x)
	an <- as.character (a[[1]])
	names (an) <- row.names (a)
	attr (x, ".rk.meta") <- an

	x
}

"rk.edit.files" <- function (file = file, title = file, name = NULL, prompt = TRUE)
{
	if (!is.character (file)) {
		nfile = tempfile()
		env = environment (file)
		dput (file, file=nfile, control=c ("useSource", "keepNA", "keepInteger", "showAttributes"))
		.Call("rk.edit.files", nfile, title, name, prompt)
		x <- dget (nfile)
		environment (x) <- env
		return (x)
	}
	invisible (.Call ("rk.edit.files",  as.character (file),  as.character (title),  as.character (name), isTRUE (prompt)))
}

"rk.show.files" <- function (file = file, title = file, wtitle = NULL, delete=FALSE, prompt = TRUE)
{
	invisible (.Call ("rk.show.files", as.character (file), as.character (title), as.character (wtitle), delete, isTRUE (prompt)))
}

"rk.show.html" <- function (url) {
	invisible (.rk.do.plain.call ("showHTML", as.character (url), synchronous=FALSE));
}

"rk.call.plugin" <- function (plugin, ..., submit.mode = c ("manual", "auto", "submit")) {
	# prepare arguments
	settings <- list (...)
	callstrings <- list ()
	callstrings[1] <- plugin
	callstrings[2] <- match.arg (submit.mode)
	if (length (settings) > 0) {
		for (i in 1:length(settings)) {
			# properly passing on escaped characters is a pain. This seems to work.
			deparsed <- deparse (settings[[i]])
			deparsed_unquoted <- substr (deparsed, 2, nchar (deparsed) - 1)
			callstrings[i + 2] <- paste(names(settings)[i], deparsed_unquoted, 
			sep = "=")
		}
	}

	# do call
	res <- .rk.do.call ("doPlugin", callstrings)

	# handle result
	if (!is.null (res)) {
		if (res$type == "warning") {
			warning (res$message)
		} else {
			stop (res$message)
		}
	}

	invisible (TRUE)
}

#' Call or list built-in RKWard plugin(s)
#' 
#' \code{rk.call.plugin} provides a high level wrapper to call any plugin
#' available in RKWard. The exact string to be used as \code{plugin}, and the
#' list of arguments available for a particular plugin, are generally not
#' transparent to the user.\code{rk.list.plugins} can be used to obtain a list
#' of current plugins. For plugin arguments, it is recommended to run the
#' plugin, and inspect the "Run again" link that is generated on the output.
#' 
#' \bold{Warning}: Using \code{rk.call.plugin}, especially with submit.modes
#' \code{"auto"} or \code{"submit"} to program a sequence of analyses has
#' important drawbacks. First, the semantics of plugins are not guaranteed to
#' remain unchanged across different versions of RKWard, thus your code may
#' stop working after an upgrade. Second, your code will not be usable outside
#' of an RKWard session. Consider copying the generated code for each plugin,
#' instead. The primary use-cases for \code{rk.call.plugin} are automated
#' tests, cross-references, and scripted tutorials.
#' 
#' \bold{Note}: Even when using \code{"submit.mode=submit"}, the plugin code is
#' run in the global context. Any local variables of the calling context are
#' not available to the plugin.
#' 
#' \code{rk.list.plugins} returns the list of the names of all currently
#' registered plugins.
#' 
#' @aliases rk.call.plugin rk.list.plugins
#' @param plugin character string, giving the name of the plugin to call. See
#'   Details.
#' @param \dots arguments passed to the \code{plugin}
#' @param submit.mode character string, specifying the submission mode:
#'   \code{"manual"} will open the plugin GUI and leave it to the user to
#'   submit it manually, \code{"auto"} will try to submit the plugin, if it can
#'   be submitted with the current settings (i.e. if the "Submit"-button is
#'   enabled after applying all specified parameters). If the plugin cannot be
#'   submitted, with the current settings, it will behave like \code{"manual"}.
#'   \code{"submit"} is like \code{"auot"}, but will close the plugin, and
#'   generate an error, if it cannot be submitted. \code{"manual"} will always
#'   return immediately, \code{"auto"} may or may not return immediately, and
#'   \code{"submit"} will always wait until the plugin has been run, or produce
#'   an error.
#' @return \code{rk.call.plugin} returns \code{TRUE} invisibly.
#' 
#' \code{rk.list.plugins} returns a character vector of plugin names. If none
#'   found, \code{NULL} is returned.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@lists.sourceforge.net}
#' @seealso \code{\link{rk.results}}, \url{rkward://page/rkward_output}
#' @keywords utilities
#' @rdname rk.call.plugin
#' @examples
#' 
#' ## list all current plugins
#' rk.list.plugins ()
#' 
#' ## "t_test_two_vars" plugin:
#' ## see the output: Windows->Show Output
#' local({
#' x1 <- rnorm (100)
#' x2 <- rnorm (100, 2)
#' 
#' rk.call.plugin ("rkward::t_test_two_vars", 
#'   confint.state="1", conflevel.real="0.95", hypothesis.string="greater", paired.state="0", varequal.state="0", 
#'   x.available="x1", y.available="x2", 
#'   submit.mode="submit")
#' })
#'
# list all available plugins in RKWard; this is a companion function for rk.call.plugin:
# the output provides possible strings for "plugin" argument in rk.call.plugin
rk.list.plugins <- function () {
	.rk.do.plain.call ("listPlugins")
}

# a wrapper around chooseCRANmirror() without changing options ("repos"), permanently
"rk.select.CRAN.mirror" <- function () {
	old_repos <- getOption("repos")
	on.exit (options (repos=old_repos))

	if (!interactive())
		stop("cannot choose a CRAN mirror non-interactively")
	m <- getCRANmirrors(all = FALSE, local.only = FALSE)
	res <- menu (paste(m[, 1L], m[, 5L], sep = " - "), getOption("menu.graphics"), "CRAN mirror")
	if (res > 0L) {
		URL <- m[res, "URL"]
		repos <- getOption("repos")
		repos["CRAN"] <- gsub("/$", "", URL[1L])
		options(repos = repos)
	}

	return (as.character (getOption ("repos")["CRAN"]))
}

# drop-in-replacement for tk_select.list()
"rk.select.list" <- function (list, preselect = NULL, multiple = FALSE, title = NULL) {
	preselect <- as.character (preselect)
	preselect.len = length (preselect)
	list <- as.character (list)
	list.len <- length (list)
	params <- list ()

	# serialize all parameters
	params[1] <- as.character (title)
	if (multiple) params[2] <- "multi"
	else params[2] <- "single"
	params[3] <- as.character (preselect.len)
	if (preselect.len) {
		for (i in 1:preselect.len) {
			params[3+i] <- preselect[i]
		}
	}
	if (list.len) {	# we should hope, the list is not empty...
		for (i in 1:list.len) {
			params[3+preselect.len+i] <- list[i]
		}
	}

	.rk.do.plain.call ("select.list", params)
}

#' Message boxes and selection list using native KDE GUI
#' 
#' Multi-purpose pop-up message boxes and selection list using native KDE GUI
#' elements. The message boxes can be used either to show some information or
#' ask some question. The selection list can be used to get a vector of
#' selected items.
#' 
#' For \code{rk.show.question}, the R interpreter always waits for the user's
#' choice.
#' 
#' \code{rk.select.list} replaces \code{utils::select.list} for the running
#' session acting as a drop-in replacement for \code{tk_select.list}. Use
#' \code{.rk.backups$select.list} for the original \code{utils::select.list}
#' function (see Examples).
#' 
#' @aliases rk.show.message rk.show.question rk.select.list
#' @param message a string for the content of the message box.
#' @param caption a string for title of the message box.
#' @param button.yes a string for the text label of the \bold{Yes} button. Can
#'   be an empty string (\code{""}), in which case the button is not displayed
#'   at all.
#' @param button.no a string used for the text label of the \bold{No} button,
#'   similar to \code{button.yes}.
#' @param button.canel a string used for the text label of the \bold{Cancel}
#'   button, similar to \code{button.yes}.
#' @param wait a logical (not NA) indicating whether the R interpreter should
#'   wait for the user's action, or run it asynchronously.
#' @param list a vector, coerced into a character vector.
#' @param preselct a vector, coerced into a character vector, items to be
#'   preselected.
#' @param multiple a logical (not NA), when \code{TRUE} multiple selection
#'   selection is allowed.
#' @param title a string, for the window title of the displayed list
#' @return \code{rk.show.message} always returns \code{TRUE}, invisibly.
#' 
#' \code{rk.show.question} returns \code{TRUE} for \bold{Yes}, \code{FALSE} for
#'   \bold{No}, and \code{NULL} for \bold{Cancel} actions.
#' 
#' \code{rk.select.list} returns the value of \code{\link{select.list}}.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@lists.sourceforge.net}
#' @seealso \code{\link{system}}, \code{\link{select.list}}
#' @keywords utilities
#' @rdname rk.show.messages
#' @examples
#' 
#' require (rkward)
#' 
#' ## Message boxes
#' if (rk.show.question ("Question:\nDo you want to know about RKWard?", 
#'     button.yes = "Yes, I do!", button.no = "No, I don't care!", button.cancel = "")) {
#'   rk.show.message ("Message:\nRKWard is a KDE GUI for R.", "RKWard Info")
#' } else {
#'   rk.show.message ("You must be joking!", "RKWard Info", wait = FALSE) ## Run asynchronously
#' }
#' 
#' ## Selection lists:
#' rk.select.list (LETTERS, preselect = c("A", "E", "I", "O", "U"), 
#'   multiple = TRUE, title = "vowels")
#' .rk.backups$select.list (LETTERS, preselect = c("A", "E", "I", "O", "U"), 
#'   multiple = TRUE, title = "vowels")
#' 
"rk.show.message" <- function (message, caption = "Information", wait=TRUE) {
	.Call ("rk.dialog", caption, message, "ok", "", "", isTRUE (wait))
	invisible (TRUE)
}

# to disable a button, set it to ""
"rk.show.question" <- function (message, caption = "Question", button.yes = "yes", button.no = "no", button.cancel = "cancel") {
	res <- .Call ("rk.dialog", caption, message, button.yes, button.no, button.cancel, TRUE)
	if (res > 0) return (TRUE)
	else if (res < 0) return (FALSE)
	else return (NULL)	# cancelled
}

#' Print information on the RKWard session
#' 
#' Gathers and prints information on the setup of the current RKWard session.
#' In general, you should always include this information when reporting a bug
#' in RKWard.
#' 
#' Typically, when reporting a bug, you should use \code{Help->Report Bug...}
#' from the menu. Internally, this will call \code{rk.sessionInfo()}.
#' 
#' @return Returns the object created by \code{sessionInfo()}, invisibly. Note
#'   that this includes only the information on the R portion of the session.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@lists.sourceforge.net}
#' @seealso \code{\link{sessionInfo}}
#' @keywords utilities misc
#' @rdname rk.sessionInf
#' @examples
#' 
#' rk.sessionInfo()
#' 
"rk.sessionInfo" <- function () {
	cat (.rk.do.plain.call ("getSessionInfo"), sep="\n")
	cat ("R runtime session info:\n")
	print (sessionInfo())
}
