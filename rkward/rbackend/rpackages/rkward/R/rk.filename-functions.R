#' RKWard file names
#' 
#' In RKWard the output is saved as a html file which is located at "~/.rkward"
#' by default. (\bold{TODO}: make this platform free). The name of this html
#' file can be retrieved and set using \code{rk.get.output.html.file} and
#' \code{rk.set.output.html.file}. \code{rk.flush.output.html.file} will delete 
#' the current (or specified) html file, and re-initialize it.
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
#' @param additional.header.contents NULL or an additional string to add to the HTML header section.
#'        This could be scripts or additional CSS definitions, for example. Note that
#'        @em nothing will be added to the header, if the file already exists.
#' @param flush.images. If true, any images used in the output file will be deleted as well.
#' @param ask Logical: Whether to ask before flushing the output file.
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
#' rk.flush.output()
#' rk.set.output.html.file(outfile)
#' 
#' @export
"rk.get.tempfile.name" <- function (prefix="image", extension=".jpg") {
	return (.rk.do.plain.call ("get.tempfile.name", c (prefix, extension)))
}

#' @export
#' @rdname rk.get.tempfile.name
"rk.get.workspace.url" <- function () {
	res <- .rk.do.plain.call ("getWorkspaceUrl")
	if (length (res)) res
	else NULL
}

#' @export
#' @rdname rk.get.tempfile.name
"rk.get.output.html.file" <- function () {
	return (.rk.variables$.rk.output.html.file)
}

#' @export
#' @rdname rk.get.tempfile.name
"rk.set.output.html.file" <- function (x, additional.header.contents = getOption ("rk.html.header.additions")) {
	stopifnot (is.character (x))
	assign (".rk.output.html.file", x, .rk.variables)

	if (!file.exists (x)) {
		.rk.cat.output (paste ("<?xml version=\"1.0\" encoding=\"", .Call ("rk.locale.name"), "\"?>\n", sep=""))
		.rk.cat.output (paste ("<html><head>\n<title>RKWard Output</title>\n", .rk.do.plain.call ("getCSSlink"), sep=""))
		# the next part defines a JavaScript function to add individual results to a global table of contents menu in the document
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

		if (!is.null (additional.header.contents)) .rk.cat.output (as.character (additional.header.contents))
		.rk.cat.output ("</head>\n<body>\n")
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

	# needs to come after initialization, so initialization alone does not trigger an update during startup
	.rk.do.plain.call ("set.output.file", x, synchronous=FALSE)
	invisible (NULL)
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
			sline <- toupper (sline)
			if (substring (sline, 0, 3) == "IMG") {
				parts <- strsplit (sline, "SRC")[[1]]
				if (length (parts) < 2) next
				parts <- strsplit (parts[2], "\"")[[1]]
				if (length (parts) < 2) next
				files <- c (files, parts[2])
			} else if (substring (sline, 0, 6) == "OBJECT") {
				parts <- strsplit (sline, "DATA")[[1]]
				if (length (parts) < 2) next
				parts <- strsplit (parts[2], "\"")[[1]]
				if (length (parts) < 2) next
				files <- c (files, parts[2])
			}
		}
	}
	files
}

#' @export
#' @rdname rk.get.tempfile.name
"rk.flush.output" <- function (x=rk.get.output.html.file (), flush.images=TRUE, ask=TRUE) {
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

	rk.set.output.html.file (x)
}
