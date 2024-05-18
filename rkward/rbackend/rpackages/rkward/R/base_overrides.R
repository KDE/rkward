# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later

#' Overrides for base functions
#'
#' RKWard replaces these functions seamlessly with wrappers that add specific
#' features, i.e. to integrate well with the GUI:
#'
#' \itemize{
#'   \item{\code{\link[base:require]{require}}}
#'   \item{\code{\link[base:q]{q/quit}}}
#'   \item{\code{\link[base:Sys.setlocale]{Sys.setlocale}}}
#'   \item{\code{\link[base:setwd]{setwd}}}
#' }
#'
#' @param ... additional arguments, see respective base function.
#' @return See \code{\link[base:require]{require}}, \code{\link[base:q]{q}},
#'   \code{\link[base:Sys.setlocale]{Sys.setlocale}}, or \code{\link[base:setwd]{setwd}}.

### this is currently unused and therefore commented out
## override makeActiveBinding: If active bindings are created in globalenv (), watch them properly
## Ideally this would not be needed, but there seems to be no user-accessible way to copy unevaluated active bindings.
#.rk.makeActiveBinding.default <- base::makeActiveBinding
## @export
#"makeActiveBinding" <- function (sym, fun, env, ...) {
#	if (identical (env, globalenv ())) {
#		f <- .rk.make.watch.f (sym)
#		.rk.makeActiveBinding.default ("x", fun, environment(f), ...)
#		ret <- .rk.makeActiveBinding.default (sym, f, globalenv ())
#		.rk.watched.symbols[[sym]] <- TRUE
#		invisible(ret)
#	} else {
#		.rk.makeActiveBinding.default (sym, fun, env, ...)
#	}
#}


#' @param package see \code{\link[base:require]{require}}
#' @param quietly see \code{\link[base:require]{require}}
#' @param character.only see \code{\link[base:require]{require}}
#' @export
#' @rdname base_overrides
"require" <- function (package, quietly = FALSE, character.only = FALSE, ...)
{
	if (!character.only) {
		package <- as.character(substitute(package))
	}
	if (!suppressWarnings(base::require(as.character(package), quietly = quietly, character.only = TRUE, ...))) {
		if (missing (package)) stop ("No package name given")
		rk.capture.output(allow.nesting=FALSE)
		try(.rk.call.nested("require", as.character(package)))
		rk.end.capture.output()
		invisible(base::require(as.character(package), quietly = TRUE, character.only = TRUE, ...))
	} else {
		invisible(TRUE)
	}
}


# overriding q, to ask via GUI instead. Arguments are not interpreted.
#' @param save see \code{\link[base:q]{q}}
#' @param status see \code{\link[base:q]{q}}
#' @param runLast see \code{\link[base:q]{q}}
#' @export
#' @rdname base_overrides
"q" <- function (save = "default", status = 0, runLast = TRUE, ...) {
	# test if this is running in RKWard, otherwise pass through to the actual q()
	if (isTRUE(.rk.inside.rkward.session())){
		.rk.call("quit")
	} else {
		base::q(save = save, status = status, runLast = runLast)
	}
}


#' @export
#' @rdname base_overrides
"quit" <- function (save = "default", status = 0, runLast = TRUE, ...) {
	q (save, status, runLast, ...)
}


#' @param category see \code{\link[base:Sys.setlocale]{Sys.setlocale}}
#' @param locale see \code{\link[base:Sys.setlocale]{Sys.setlocale}}
#' @export
#' @rdname base_overrides
"Sys.setlocale" <- function (category = "LC_ALL", locale = "", ...) {
	if (category == "LC_ALL" || category == "LC_CTYPE" || category == "LANG") {
		 .rk.call("preLocaleChange")

		ret <- base::Sys.setlocale (category, locale, ...)

		.Call ("rk.update.locale", PACKAGE="(embedding)")
		ret
	} else {
		base::Sys.setlocale (category, locale, ...)
	}
}


#' @param dir see \code{\link[base:setwd]{setwd}}
#' @export
#' @rdname base_overrides
"setwd" <- function () {
	ret <- eval (body (base::setwd))
	.rk.call.async("wdChange", base::getwd())
	invisible (ret)
}
formals (setwd) <- formals (base::setwd)
