# override makeActiveBinding: If active bindings are created in globalenv (), watch them properly
# Ideally this would not be needed, but there seems to be no user-accessible way to copy unevaluated active bindings.
.rk.makeActiveBinding.default <- base::makeActiveBinding
#' @export
"makeActiveBinding" <- function (sym, fun, env, ...) {
	if (identical (env, globalenv ())) {
		f <- .rk.make.watch.f (sym)
		.rk.makeActiveBinding.default ("x", fun, environment(f), ...)
		ret <- .rk.makeActiveBinding.default (sym, f, globalenv ())
		.rk.watched.symbols[[sym]] <- TRUE
		invisible(ret)
	} else {
		.rk.makeActiveBinding.default (sym, fun, env, ...)
	}
}


#' @export
"require" <- function (package, quietly = FALSE, character.only = FALSE, ...)
{
	if (!character.only) {
		package <- as.character(substitute(package))
	}
	if (!suppressWarnings(base::require(as.character(package), quietly = quietly, character.only = TRUE, ...))) {
		.rk.do.call("require", as.character(package))
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
		res <- .rk.do.plain.call ("quit")
		if (length (res) && (res == "FALSE")) stop ("Quitting was cancelled")
	} else {
		base:::q(save = save, status = status, runLast = runLast)
	}
}


#' @export
"quit" <- function (save = "default", status = 0, runLast = TRUE, ...) {
	q (save, status, runLast, ...)
}


#' @export
"Sys.setlocale" <- function (category = "LC_ALL", locale = "", ...) {
	if (category == "LC_ALL" || category == "LC_CTYPE" || category == "LANG") {
		allow <- .rk.do.plain.call ("preLocaleChange", NULL)
		if (length (allow) && (allow == "FALSE")) stop ("Changing the locale was cancelled by user")

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
