# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#' Edit / show an object / file
#' 
#' \code{rk.edit} can be used to edit an object in the RKWard data editor.
#' Currently only \link{data.frame}s are supported. This is similar to
#' \link{edit.data.frame}, but the function returns immediately, and the object
#' is edit asynchronously.
#' 
#' \code{rk.edit.files}, \code{rk.show.files}, and \code{rk.show.html} are
#' equivalent to \link{edit}, \link{file.show}, and \link{browseURL},
#' respectively, but use RKWard as text/html editor/viewer. Generally it is
#' recommended to use \link{edit}, \link{file.edit}, \link{file.show},
#' and \link{browseURL}, instead. These will call the respective RKWard functions
#' by default, when run inside an RKWard session. (via \code{getOption("editor")},
#' and \code{getOption("browser")}.
#'
#' \code{rk.show.html} with \code{content}-argument, insed of \code{url}-argument, will
#' show the given HTML string. This is mostly useful for plugins, where it may be
#' desirable to show an error-message at the place where usually an HTML-file would
#' be shown.
#'
#' \code{rk.show.pdf} opens a PDF (or postscript) document in a viewer window inside RKWard.
#'
#' @param x an object to edit.
#' @aliases rk.edit rk.edit.files rk.show.files rk.show.html
#' @return All functions described on this page return \code{NULL},
#'   unconditionally.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@kde.org}
#' @seealso \code{\link{edit}}, \code{\link{file.edit}},
#'   \code{\link{file.show}}, \code{\link{browseURL}}
#' @keywords utilities IO
#' @rdname rk.edit
#' @export
#' @examples
#' \dontrun{
#' x <- data.frame (a=c(1:3), b=c(2:4))
#' rk.edit(x)
#' }
"rk.edit" <- function (x) {
	object <- deparse (substitute (x))
	.rk.call.nested("edit", object)
}

#' @param name name of the environment to use (optional).
#' @param file character vector, filenames to show or edit.
#' @param title character vector, of the same length as \code{file}; This can
#'   be used to give descriptive titles to each file, which will be displayed
#'   to the user.
#' @param prompt logical of length 1. If TRUE (the default) a prompt is dialog
#'   is shown along with the files to show / edit.
#' @export
#' @rdname rk.edit
"rk.edit.files" <- function (name, file="", title, prompt = TRUE) {
	# for the time being, translate NULL into missingness and throw a warning
	if(!missing(name)){
		if (is.null (name)) {
			warning("Deprecated: name = NULL, leave missing if unused!")
			name <- substitute()
		}
	}
	if (missing (name)) {
		name <- character(0)
	} else {
		if (missing (title)) title = deparse (substitute (name))
		if (file == "") file = tempfile()
		env = environment(name)
		dput (name, file = file, control = c("useSource", "keepNA", "keepInteger", "showAttributes"))
		.Call ("rk.edit.files", file, title, "", prompt, PACKAGE = "(embedding)")
		x <- dget(file)
		environment(x) <- env
		return(x)
	}
	invisible (.Call ("rk.edit.files", as.character (file), as.character (title), as.character (name), isTRUE (prompt), PACKAGE="(embedding)"))
}

#' @param header character, header to show.
#' @param delete.file a logical (not NA), when \code{TRUE} the shown file(s) are
#'   deleted after closing.
#' @param delete logical, for compatibility with earlier versions of R.
#' @export
#' @rdname rk.edit
"rk.show.files" <- function (file = file, header = file, title, delete.file=FALSE, prompt = TRUE,
	delete = delete.file  # For compatibility with earlier versions of R
)
{
	# for the time being, translate NULL into missingness and throw a warning
	if(!missing(title)){
		if (is.null (title)) {
			warning("Deprecated: title = NULL, leave missing if unused!")
			title <- substitute()
		}
	}
	if(missing (title)) title <- character(0)
	invisible (.Call ("rk.show.files", as.character (file), as.character (header), as.character (title), delete, isTRUE (prompt), PACKAGE="(embedding)"))
}

#' @param url a URL to show.
#' @export
#' @rdname rk.edit
"rk.show.html" <- function(url, content) {
	if (missing(url) == missing(content)) stop("Exactly one of 'url' or 'content' must be specified.")
	if (missing(url)) .rk.call.async("showHTML", c("", as.character(content)))
	else .rk.call.async("showHTML", as.character(url))
}

#' @param url a URL to show.
#' @export
#' @rdname rk.edit
"rk.show.pdf" <- function(url) {
	.rk.call.async("showPDF", as.character(url));
}
