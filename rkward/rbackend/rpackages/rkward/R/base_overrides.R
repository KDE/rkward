# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later

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


#' @export
"require" <- function (package, quietly = FALSE, character.only = FALSE, ...)
{
	if (!character.only) {
		package <- as.character(substitute(package))
	}
	if (!suppressWarnings(base::require(as.character(package), quietly = quietly, character.only = TRUE, ...))) {
		if (missing (package)) stop ("No package name given")
		rk.capture.output(allow.nesting=FALSE)
		try(.rk.do.call("require", as.character(package)))
		rk.end.capture.output()
		invisible(base::require(as.character(package), quietly = TRUE, character.only = TRUE, ...))
	} else {
		invisible(TRUE)
	}
}


# overriding q, to ask via GUI instead. Arguments are not interpreted.
#' @export
"q" <- function (save = "default", status = 0, runLast = TRUE, ...) {
	# test if this is running in RKWard, otherwise pass through to the actual q()
	if (isTRUE(.rk.inside.rkward.session())){
		.rk.do.plain.call ("quit")
	} else {
		base::q(save = save, status = status, runLast = runLast)
	}
}


#' @export
"quit" <- function (save = "default", status = 0, runLast = TRUE, ...) {
	q (save, status, runLast, ...)
}


#' @export
"Sys.setlocale" <- function (category = "LC_ALL", locale = "", ...) {
	if (category == "LC_ALL" || category == "LC_CTYPE" || category == "LANG") {
		 .rk.do.plain.call ("preLocaleChange", NULL)

		ret <- base::Sys.setlocale (category, locale, ...)

		.Call ("rk.update.locale", PACKAGE="(embedding)")
		ret
	} else {
		base::Sys.setlocale (category, locale, ...)
	}
}


#' @export
"setwd" <- function () {
	ret <- eval (body (base::setwd))
	.rk.do.plain.call ("wdChange", base::getwd (), synchronous=FALSE)
	invisible (ret)
}
formals (setwd) <- formals (base::setwd)
