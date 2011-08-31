#' Call or list built-in RKWard plugin(s)
#' 
#' \code{rk.call.plugin} provides a high level wrapper to call any plugin
#' available in RKWard. The exact string to be used as \code{plugin}, and the
#' list of arguments available for a particular plugin, are generally not
#' transparent to the user.\code{rk.list.plugins} can be used to obtain a list
#' of current plugins. For plugin arguments, it is recommended to run the
#' plugin, and inspect the "Run again" link that is generated on the output.
#' 
#' \bold{Warning}: Using \code{rk.call.plugin}, especially with submit.modes
#' \code{"auto"} or \code{"submit"} to program a sequence of analyses has
#' important drawbacks. First, the semantics of plugins are not guaranteed to
#' remain unchanged across different versions of RKWard, thus your code may
#' stop working after an upgrade. Second, your code will not be usable outside
#' of an RKWard session. Consider copying the generated code for each plugin,
#' instead. The primary use-cases for \code{rk.call.plugin} are automated
#' tests, cross-references, and scripted tutorials.
#' 
#' \bold{Note}: Even when using \code{"submit.mode=submit"}, the plugin code is
#' run in the global context. Any local variables of the calling context are
#' not available to the plugin.
#' 
#' \code{rk.list.plugins} returns the list of the names of all currently
#' registered plugins.
#' 
#' @aliases rk.call.plugin rk.list.plugins
#' @param plugin character string, giving the name of the plugin to call. See
#'   Details.
#' @param \dots arguments passed to the \code{plugin}
#' @param submit.mode character string, specifying the submission mode:
#'   \code{"manual"} will open the plugin GUI and leave it to the user to
#'   submit it manually, \code{"auto"} will try to submit the plugin, if it can
#'   be submitted with the current settings (i.e. if the "Submit"-button is
#'   enabled after applying all specified parameters). If the plugin cannot be
#'   submitted, with the current settings, it will behave like \code{"manual"}.
#'   \code{"submit"} is like \code{"auot"}, but will close the plugin, and
#'   generate an error, if it cannot be submitted. \code{"manual"} will always
#'   return immediately, \code{"auto"} may or may not return immediately, and
#'   \code{"submit"} will always wait until the plugin has been run, or produce
#'   an error.
#' @return \code{rk.call.plugin} returns \code{TRUE} invisibly.
#' 
#' \code{rk.list.plugins} returns a character vector of plugin names. If none
#'   found, \code{NULL} is returned.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@lists.sourceforge.net}
#' @seealso \code{\link{rk.results}}, \url{rkward://page/rkward_output}
#' @keywords utilities
#' @rdname rk.call.plugin
#' @examples
#' 
#' ## list all current plugins
#' rk.list.plugins ()
#' 
#' ## "t_test_two_vars" plugin:
#' ## see the output: Windows->Show Output
#' local({
#' x1 <- rnorm (100)
#' x2 <- rnorm (100, 2)
#' 
#' rk.call.plugin ("rkward::t_test_two_vars", 
#'   confint.state="1", conflevel.real="0.95", hypothesis.string="greater", paired.state="0", varequal.state="0", 
#'   x.available="x1", y.available="x2", 
#'   submit.mode="submit")
#' })
#'
# list all available plugins in RKWard; this is a companion function for rk.call.plugin:
# the output provides possible strings for "plugin" argument in rk.call.plugin
rk.list.plugins <- function () {
	.rk.do.plain.call ("listPlugins")
}

"rk.call.plugin" <- function (plugin, ..., submit.mode = c ("manual", "auto", "submit")) {
	# prepare arguments
	settings <- list (...)
	callstrings <- list ()
	callstrings[1] <- plugin
	callstrings[2] <- match.arg (submit.mode)
	if (length (settings) > 0) {
		for (i in 1:length(settings)) {
			# properly passing on escaped characters is a pain. This seems to work.
			deparsed <- deparse (settings[[i]])
			deparsed_unquoted <- substr (deparsed, 2, nchar (deparsed) - 1)
			callstrings[i + 2] <- paste(names(settings)[i], deparsed_unquoted, 
			sep = "=")
		}
	}

	# do call
	res <- .rk.do.call ("doPlugin", callstrings)

	# handle result
	if (!is.null (res)) {
		if (res$type == "warning") {
			warning (res$message)
		} else {
			stop (res$message)
		}
	}

	invisible (TRUE)
}
