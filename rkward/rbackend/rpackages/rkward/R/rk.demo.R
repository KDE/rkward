#' Opens an R demo script for editing
#' 
#' \code{rk.demo} behaves similar to \code{\link{demo}}, but opens the demo
#' script for editing, instead of sourcing it. Contrary to \code{\link{demo}},
#' the specification of a topic is mandatory.
#' 
#' @param topic topic of the example
#' @param package package(s) to search for the demo. If NULL (the default), all
#'   currently loaded packages are searched.
#' @param lib.loc Library locations.
#' @return Return \code{NULL}, unconditionally.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@lists.sourceforge.net}
#' @seealso \code{\link{rk.edit.files}}, \code{\link{rk.show.files}},
#'   \code{\link{demo}}
#' @keywords utilities IO
#' @rdname rk.demo
#' @examples
#' 
#' ## Not run
#' rk.demo("graphics")

"rk.demo" <- function (topic, package=NULL, lib.loc=NULL) {
	if (is.null (package)) {
		package <- .packages (lib.loc=lib.loc)
	}

	loc <- ""
	for (p in package) {
		loc <- system.file ("demo", paste (topic, ".R", sep=""), package=p, lib.loc=lib.loc)
		if (loc != "") break
	}

	if (loc == "") stop ("No demo found for topic'", topic, "'")
	rk.edit.files (loc, prompt=FALSE)
}
