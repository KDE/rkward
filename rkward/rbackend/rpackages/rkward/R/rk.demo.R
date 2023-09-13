# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#' Opens an R demo script for editing
#' 
#' \code{rk.demo} behaves similar to \code{\link{demo}}, but opens the demo
#' script for editing, instead of sourcing it. Contrary to \code{\link{demo}},
#' the specification of a topic is mandatory.
#' 
#' @param topic topic of the example
#' @param package package(s) to search for the demo. If missing (the default), all
#'   currently loaded packages are searched.
#' @param lib.loc Library locations, passed on to \code{\link[base:system.file]{system.file}}.
#' @return Return \code{NULL}, unconditionally.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@kde.org}
#' @seealso \code{\link{rk.edit.files}}, \code{\link{rk.show.files}},
#'   \code{\link{demo}}
#' @keywords utilities IO
#' @rdname rk.demo
#' @export
#' @examples
#' \dontrun{
#' rk.demo("graphics")
#' }
"rk.demo" <- function (topic, package, lib.loc=NULL) {
	# for the time being, translate NULL into missingness and throw a warning
	if(!missing(package)){
		if (is.null (package)) {
			warning("Deprecated: package = NULL, leave missing if unused!")
			package <- substitute()
		}
	}
	if (missing (package)) {
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
