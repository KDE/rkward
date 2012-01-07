#' RKWard file names
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
		.rk.cat.output (paste ("<html><head>\n<title>RKWard Output</title>\n", .rk.do.plain.call ("getCSSlink"), sep=""))
		# the next part defines a JavaScript function to add individual results to a global menu in the document
		.rk.cat.output (paste ("\t<script type=\"text/javascript\">
		function addToMenu(id){
			var fullHeader = document.getElementById(id);
			var resultsMenu = document.getElementById('RKWardResultsMenu');
			var headerName = fullHeader.getAttribute('name');
			var headerText = fullHeader.firstChild.data;
			// create new anchor for menu
			var newAnchor = document.createElement('a');
			var newLine = document.createElement('br');
			var anchorRef = document.createAttribute('href');
			var anchorText = document.createTextNode(headerText);
			anchorRef.nodeValue = '#' + headerName;
			newAnchor.setAttributeNode(anchorRef);
			newAnchor.appendChild(anchorText);
			resultsMenu.appendChild(newAnchor);
			resultsMenu.appendChild(newLine);
		}\n\t</script>\n", sep=""))
		.rk.cat.output (paste ("</head>\n<body>\n", sep=""))
		# This initial output mostly to indicate the output is really there, just empty for now
		.rk.cat.output (paste ("<a name=\"top\"></a>\n<pre>RKWard output initialized on", date (), "</pre>\n"))
		# an empty <div> where the menu gets added to dynamically
		.rk.cat.output (paste ("<div id=\"RKWardResultsMenu\"><!-- the menu goes here --></div>\n", sep=""))
	}

	# needs to come after initialization, so initialization alone does not trigger an update during startup
	.rk.do.plain.call ("set.output.file", x, synchronous=FALSE)
	invisible (NULL)
}
