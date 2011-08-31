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

"rk.edit" <- function (x) {
	object <- deparse (substitute (x))
	.rk.do.call ("edit", object)
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
