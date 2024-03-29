# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
# gather debug information.
# Note: subsequent browser() calls should be suppressed while inside this function!
#' @importFrom utils capture.output
#' @export
.rk.callstack.info <- function () {
	nframes <- sys.nframe() - 1	# strip this function call
	calls <- character (0)
	funs <- character (0)
	envs <- character (0)
	locals <- character (0)
	relsrclines <- integer (0)

	if (nframes > 0) {
		for (i in 1:nframes) {
			calls[i] <- as.character (try (paste (deparse (sys.call (i)), collapse="\n")), silent=TRUE)
			funs[i] <- as.character (try (paste (deparse (sys.function (i), control="all"), collapse="\n"), silent=TRUE))
			envs[i] <- as.character (try (capture.output (print (environment (sys.function (i)))), silent=TRUE))
			locals[i] <- as.character (try (paste (ls (sys.frame (i), all.names=TRUE), collapse="\n"), silent=TRUE))
			relsrclines[i] <- as.character (try (rk.relative.src.line (sys.call (i+1), sys.function (i)), silent=TRUE))
		}
	}
	list ("calls"=calls, "functions"=funs, "environments"=envs, "locals"=locals, "relsrclines"=relsrclines)
}

#' Get relative source location
#'
#' @param inner either an object containing source references, or an object of class
#'   \code{\link[base:srcref]{srcref}}.
#' @param outer see \code{inner}.
#' @return An integer or \code{NA}.
#' @seealso \code{\link[utils:getSrcref]{getSrcref}},
#'   \code{\link[utils:getSrcFilename]{getSrcFilename}},
#'   \code{\link[utils:getSrcLocation]{getSrcLocation}}
#' @importFrom utils getSrcref getSrcFilename getSrcLocation
#' @export
# NOTE: this requires R >= 2.13.0
rk.relative.src.line <- function (inner, outer) {
	if (!inherits (inner, "srcref")) inner <- getSrcref (inner)
	if (!inherits (outer, "srcref")) outer <- getSrcref (outer)
	if (is.null (inner) || is.null (outer)) return (NA)
	if (getSrcFilename (inner) != getSrcFilename (outer)) return (NA)
	outer_first <- getSrcLocation (outer, "line")
	outer_last <- getSrcLocation (outer, "line", first=FALSE)
	inner_first <- getSrcLocation (inner, "line")
	inner_last <- getSrcLocation (inner, "line", first=FALSE)
	if ((inner_first < outer_first) || (inner_last > outer_last)) return (NA)
	return (inner_first - outer_first + 1)
}
