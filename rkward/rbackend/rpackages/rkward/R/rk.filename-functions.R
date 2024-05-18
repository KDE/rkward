# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#' RKWard file names
#' 
#' In RKWard the output is saved as a html file which is located at "~/.rkward"
#' by default. (\bold{TODO}: make this platform free). The name of this html
#' file can be retrieved and set using \code{rk.get.output.html.file} and
#' \code{rk.set.output.html.file}. \code{rk.flush.output.html.file} will delete 
#' the current (or specified) html file, and re-initialize it.
#' 
#' \code{rk.get.tempfile.name} returns a non-existing filename inside the
#' specified directory (or the directory of the current output file, if the parameter is
#' omitted). The filename is returned as an absolute path,
#' but the relative path with respect to the base directory can be obtained via
#' \code{names()}. It is mainly used by \link{rk.graph.on} to
#' create filenames suitable for storing images in the output. The filenames of
#' the temporary files are of the form
#' "\code{prefix}\emph{xyz}.\code{extension}". \code{rk.get.tempfile.name} is
#' somewhat misnamed. For truly temporary files, \link{tempfile} is generally
#' more suitable.
#'
#' \code{rk.tempdir} returns a directory suitable for storing rkward related
#'   temporary files. Almost the same as \code{tempdir()}, but the directory
#'   returned will be inside the configured RKWard data path  ("$HOME/.rkward", by default).
#' 
#' \code{rk.get.workspace.url} returns the url of workspace file which has been
#' loaded in RKWard, or NULL, if no workspace has been loaded. NOTE: This value
#' is note affected by running \code{load} in R, only by loading R workspaces
#' via the RKWard GUI.
#' 
#' \code{rk.home} returns the filename of the specified component similar to
#' \link{R.home}.
#'
#' @aliases rk.get.tempfile.name rk.get.workspace.url rk.get.output.html.file
#'   rk.set.output.html.file rk.tempdir rk.home
#' @param prefix a string, used as a filename prefix when saving images to the
#'   output file. This is usually just a plain file name, but can also be a relative or absolute
#'   path. Relative paths are resolved with the default output directory as base, absolute paths
#'   are kept as is.
#' @param extension a string, used as a filename extension when saving images
#'   to the output file
#' @param directory a string, The base directory for the file. If left empty, this will default to the
#'   write directory of the current output file (usually "~.rkward)
#' @param component a string specifying the desired path. "home" (the default value) means to
#'        return the generic data dir used by RKWard. "lib" means to return the directory where
#'        the rkward R library is installed.
#' @param x a string, giving the filename of the of the output file
#' @param additional.header.contents NULL or an additional string to add to the HTML header section.
#'        This could be scripts or additional CSS definitions, for example. Note that
#'        \emph{nothing} will be added to the header, if the file already exists.
#' @param style Currently either "regular" or "preview". The latter omits table of contents and date.
#' @param css Local file name of CSS file to use, or NULL for no CSS file. The CSS file will be
#'            placed next to x, with file name extension ".css". Only effective when initializing a
#'            (non-existing) output file.
#' @param silent Set to true to avoid the output window being raised in the frontend.
#' @param flush.images If true, any images used in the output file will be deleted as well.
#' @param ask Logical: Whether to ask before flushing the output file.
#' @param ... Further parameters passed to rk.set.output.html.file()
#' @return \code{rk.get.tempfile.name}, \code{rk.get.output.html.file}, \code{rk.get.workspace.url}, and
#'   \code{rk.home} return a string while
#'   \code{rk.set.output.html.file} returns the \bold{previous} output html file.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@kde.org}
#' @seealso \url{rkward://page/rkward_output}, \link{tempfile}, \link{file},
#'   \link{rk.print}
#' @keywords utilities IO
#' @rdname rk.get.tempfile.name
#' @export
#' @examples
#' \dontrun{
#' testfile.name <- rk.get.tempfile.name(prefix="test", extension=".txt")
#' testfile <- file(testfile.name)
#' cat("This is a test\n", file=testfile)
#' close(testfile)
#' unlink(testfile.name)
#' 
#' outfile <- rk.get.output.html.file()
#' 
#' rk.set.output.html.file("~/.rkward/another_file.html")
#' rk.header("Output on a different output file")
#' rk.show.html(rk.get.output.html.file())
#' rk.flush.output()
#' rk.set.output.html.file(outfile)
#' }
"rk.get.tempfile.name" <- function (prefix="image", extension=".jpg", directory=dirname (rk.get.output.html.file ())) {
	x <- .rk.call.backend("unused.filename", c(prefix, extension, directory))
	ret <- x[2]
	names(ret) <- x[1]
	ret
}

#' @export
"rk.tempdir" <- function() {
	.rk.call.backend("tempdir")
}

#' @export
#' @rdname rk.get.tempfile.name
"rk.get.workspace.url" <- function () {
	res <- .rk.call("getWorkspaceUrl")
	if (length (res)) res
	else NULL
}

#' @export
#' @rdname rk.get.tempfile.name
"rk.home" <- function (component="home") {
	if(component %in% c("home", "lib")) {
		normalizePath(.rk.call.backend("home", component))
	} else {
		stop("Unknown component type");
	}
}

#' @export
#' @rdname rk.get.tempfile.name
"rk.get.output.html.file" <- function () {
	return (.rk.variables$.rk.output.html.file)
}

#' @export
#' @rdname rk.get.tempfile.name
"rk.set.output.html.file" <- function (x, additional.header.contents = getOption ("rk.html.header.additions"), style=c ("regular", "preview"), css = getOption ("rk.output.css.file"), silent=FALSE) {
	stopifnot (is.character (x))
	style <- match.arg (style)
	oldfile <- rk.get.output.html.file ()
	dir.create (dirname (x), showWarnings=FALSE, recursive=TRUE)
	stopifnot (dir.exists (dirname (x)))
	assign (".rk.output.html.file", x, .rk.variables)

	if (!file.exists (x)) {
		encoding.name <- function() {
			li <- l10n_info();
			if(isTRUE(li$'UTF-8')) return("UTF-8")  # hopefully the most common case, these days
			if(!is.null(li$codeset)) return(li$codeset)
			if(!is.null(li$codepage)) return(paste0("windows-", li$codepage))
			return(tail(strsplit(Sys.getlocale("LC_CTYPE"), ".") ,1))
		}
		.rk.cat.output (paste ("<?xml version=\"1.0\" encoding=\"", encoding.name(), "\"?>\n", sep=""))
		.rk.cat.output ("<html><head>\n<title>RKWard Output</title>\n")
		if (!is.null (css)) {
			cssfilename <- paste (sub ("\\.[^.]*$", "", basename (x)), ".css", sep="")
			.rk.cat.output (paste ("<link rel=\"StyleSheet\" type=\"text/css\" href=\"", cssfilename, "\"/>\n", sep=""))
			cssfile <- file.path (dirname (x), cssfilename)
			if (!file.copy (css, cssfile, overwrite=TRUE)) {
				warning ("Failed to copy CSS file ", css, " to ", cssfile)
			}
		}
		# the next part defines a JavaScript function to add individual results to a global table of contents menu in the document
		if (style != "preview") {
			.rk.cat.output (paste ("\t<script type=\"text/javascript\">
			<!--
				function addToTOC(id, level){
					var fullHeader = document.getElementById(id);
					var resultsTOC = document.getElementById('RKWardResultsTOCShown');
					var headerName = fullHeader.getAttribute('name');
					var headerText = fullHeader.firstChild.data;
					var headerTitle = fullHeader.getAttribute('title');
					var newDiv = document.createElement('div');
					// create new anchor for TOC
					var newAnchor = '<a href=\"#' + headerName + '\" title=\"' + headerTitle + '\"';
					// indent anchor depending on header level
					if(level > 1){
						newDiv.style.textIndent = level-1 + 'em';
						newDiv.className = 'level' + level;
						newAnchor = '&bull; ' + newAnchor + '>' + headerText + '</a>';
					} else {
						newAnchor = newAnchor + '>' + headerText + '</a>';
					}
					newDiv.innerHTML = newAnchor;
					resultsTOC.appendChild(newDiv);
				}
				function switchVisible(show, hide) {
					document.getElementById(show).style.display = 'inline';
					document.getElementById(hide).style.display = 'none';
				}
				function showMLevel(nodes){
					for(var i=0; i < nodes.length; i++) {
						nodes[i].style.display = 'block';
					}
				}
				function hideMLevel(nodes){
					for(var i=0; i < nodes.length; i++) {
						nodes[i].style.display = 'none';
					}
				}
				function maxLevel(level){
					if(level > 5){
						return false;
					}
					for(var i=1; i < 6; i++) {
						if(i <= level){
							showMLevel(document.getElementsByClassName('level' + i));
						} else {
							hideMLevel(document.getElementsByClassName('level' + i));
						}
					}
				}
			// -->\n\t</script>\n", sep=""))
			# positioning of the TOC is done by CSS, default state is hidden
			# see $SRC/rkward/pages/rkward_output.css
		}

		if (!is.null (additional.header.contents)) .rk.cat.output (as.character (additional.header.contents))
		.rk.cat.output ("</head>\n<body>\n")
		if (style != "preview") {
			# This initial output mostly to indicate the output is really there, just empty for now
			.rk.cat.output (paste ("<a name=\"top\"></a>\n<pre>RKWard output initialized on", .rk.date (), "</pre>\n"))
			# an empty <div> where the TOC menu gets added to dynamically, and a second one to toggle show/hide
			.rk.cat.output (paste (
				"<div id=\"RKWardResultsTOCShown\" class=\"RKTOC\">\n",
				"\t<a onclick=\"javascript:switchVisible('RKWardResultsTOCHidden','RKWardResultsTOCShown'); return false;\" href=\"\" class=\"toggleTOC\">Hide TOC</a>\n",
				"\t<span class=\"right\"><a href=\"#top\" class=\"toggleTOC\">Go to top</a></span>\n<br />",
				"\t\t<span class=\"center\">\n\t\t\t<a onclick=\"javascript:maxLevel('1'); return false;\" href=\"\" title=\"TOC level 1\">1</a> &bull;\n",
				"\t\t\t<a onclick=\"javascript:maxLevel('2'); return false;\" href=\"\" title=\"TOC level 2\">2</a> &bull;\n",
				"\t\t\t<a onclick=\"javascript:maxLevel('3'); return false;\" href=\"\" title=\"TOC level 3\">3</a> &bull;\n",
				"\t\t\t<a onclick=\"javascript:maxLevel('4'); return false;\" href=\"\" title=\"TOC level 4\">4</a>\n\t\t</span>\n",
				"\t<!-- the TOC menu goes here -->\n</div>\n",
				"<div id=\"RKWardResultsTOCHidden\" class=\"RKTOC RKTOChidden\">\n",
				"\t<a onclick=\"javascript:switchVisible('RKWardResultsTOCShown','RKWardResultsTOCHidden'); return false;\" href=\"\" class=\"toggleTOC\">Show TOC</a>\n",
				"\t<span class=\"right\"><a href=\"#top\" class=\"toggleTOC\">Go to top</a></span>\n",
				"</div>\n", sep=""))
		}
	}

	# needs to come after initialization, so initialization alone does not trigger an update during startup
	if (!isTRUE(silent)) .rk.call.async("set.output.file", x)
	invisible (oldfile)
}

# Internal helper function to extract file names of images used in html files.
# Almost definitely, this could be simplified, but I'll leave that as an exercise to the reader ;-)
# Note that this uses heuristics, rather than real parsing
".rk.get.images.in.html.file" <- function (file) {
	lines <- readLines (file)
	lines <- grep ("<(img|object)", lines, ignore.case=TRUE, value=TRUE)
	files <- character (0)
	for (line in lines) {
		slines <- strsplit (line, "<")[[1]]
		for (sline in slines) {
			if (substring (toupper(sline), 0, 3) == "IMG") {
				parts <- strsplit (sline, "[Ss][Rr][Cc]")[[1]]
				if (length (parts) < 2) next
			} else if (substring (toupper(sline), 0, 6) == "OBJECT") {
				parts <- strsplit (sline, "[Dd][Aa][Tt][Aa]")[[1]]
				if (length (parts) < 2) next
			} else {
				next
			}
			parts <- strsplit (parts[2], "\"")[[1]]
			if (length (parts) < 2) next
			files <- c (files, sub("^file://", "", parts[2]))
		}
	}
	files
}

#' @export
#' @rdname rk.get.tempfile.name
"rk.flush.output" <- function (x=rk.get.output.html.file (), flush.images=TRUE, ask=TRUE, ...) {
	images <- character (0)
	if (flush.images) images <- .rk.get.images.in.html.file (x)

	desc <- x
	if (length (images)) {
		desc <- paste (x, ", along with ", length (images), " image files", sep="")
	}

	if (isTRUE (ask)) {
		if (!rk.show.question (paste ("Do you really want to flush the output file (", desc, ")?\nIt will not be possible to restore it.", sep=""))) stop ("Aborted by user")
	}

	unlink (x)
	try (
		for (image in images) {
			unlink (image)
		}
	)

	rk.set.output.html.file (x, ...)
}
