#' Call built-in RKWard plugin(s)
#' 
#' \code{rk.call.plugin} provides a high level wrapper to call any plugin
#' available in RKWard. The exact string to be used as \code{plugin}, and the
#' list of arguments available for a particular plugin, are generally not
#' transparent to the user. \code{rk.list.plugins} can be used to obtain a list
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
#' @author Thomas Friedrichsmeier \email{rkward-devel@@lists.sourceforge.net}
#' @seealso \code{\link{rk.results}}, \url{rkward://page/rkward_output}
#' @keywords utilities
#' @examples
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
#' @export
#' @rdname rk.call.plugin
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

#' (Re-)load the given pluginmap files into the RKWard GUI
#'
#' @param pluginmap.files a character vector of file names to add. This may be left empty,
#'                        if the only desired effect is to reload all active pluginmaps.
#' @param force.add logical. Whether the pluginmap files should also be added, if they had
#'                  been previously de-selected in the settings menu, and regardless of their
#'                  priority setting. In scripted usage, this should generally be set to FALSE.
#' @param force.reload logical. By default the active pluginmaps are reloaded, only if any new ones
#'                     were added. If set to TRUE, pluginmaps are reloaded in any case. In
#'                     scripted usage, this should generally be set to FALSE. NOTE: Since
#'                     a reload always means reloading _all_ active pluginmaps, This may be
#'                     slow, and should be used with care.
#'
#' \bold{Note}: It is not necessary to reload the pluginmap, in order to refresh an individual
#'              plugin (e.g. after modifying the dialog), as plugins are not kept in memory after closing.
#'              Any currently opened plugins are not affected by this function. 
#'
#' @author Thomas Friedrichsmeier \email{rkward-devel@@lists.sourceforge.net}
#' @seealso \code{\link{rk.call.plugin}}, @seealso \code{\link{rkwarddev::rk.plugin.skeleton}}
#' @keywords utilities
#'
#' @examples
#' 
#' ## NOT RUN
#'
#' ## reload all active pluginmaps
#' rk.load.pluginmaps()
#'
#' ## END NOT RUN
#' @export
#' @rdname rk.load.pluginmaps
"rk.load.pluginmaps" <- function (pluginmap.files=NULL, force.add = TRUE, force.reload = TRUE) {
	.rk.do.plain.call ("loadPluginMaps", c (ifelse (isTRUE (force.add), "force", "noforce"), ifelse (isTRUE (force.reload), "reload", "noreload"), as.character (pluginmap.files)), synchronous=FALSE)
}

#' List of modify loaded plugins
#'
#' \code{rk.list.plugins} returns the a list of all currently
#' registered plugins (in loaded pluginmaps).
#' \code{rk.set.plugin.status} allows to control the status of the given plugin(s). Currently,
#'    only visibility can be controlled.
#'
#' @param id vector of ids (character) of the plugins to modify
#' @param context in which the plugin should be shown / hidden. This can either be "",
#'    meaning the plugin will be affected in all contexts it occurs in, or a character vector
#'    of the same length as id.
#' @param visible logical, controlling whether the plugin should be shown (\code{TRUE}) or
#'    hidden (\code{FALSE}). Hidden plugins are essentially removed from the menu. They may still
#'    be accessible embedded into other plugins.
#'
#' @return \code{rk.list.plugins} returns a data.frame listing plugin ids, context, menu path
#'   (tab-separated), and label of the plugin. If a plugin is available in more
#'   than one context, it will be listed several times. The exact layout (number and order of columns)
#'   of this data.frame might be subject to change. However, the \bold{names} of the columns in the
#'   returned data.frame are expected to remain stable.
#'   \code{rk.set.plugin.status} returns \code{NULL}, invisibly
#'
#' \bold{Note}: Each call to \code{rk.set.plugin.status} will result in a complete rebuild of the
#'              menu (in the current implementation). While this should be hardly noticeable in interactive
#'              use, it could be an issue when changing the status of many plugins, programatically.
#'              In this case, make sure to do all changes in \bold{one} call to \code{rk.set.plugin.status},
#'              rather than many separate calls.
#'
#' @author Thomas Friedrichsmeier \email{rkward-devel@@lists.sourceforge.net}
#' @keywords utilities
#'
#' @seealso \code{\link{rk.call.plugin}} for invoking a plugin, programatically
#'
#' @examples
#' ## list all current plugins
#' rk.list.plugins ()
#'
#' ## NOT RUN
#' ## hide t.test plugin
#' rk.set.plugin.status ("rkward::t_test", visible=FALSE)
#' ## END NOT RUN
#'
#' @export
#' @rdname rk.list.plugins
#' @aliases rk.list.plugins rk.set.plugin.status
"rk.list.plugins" <- function () {
	plugs <- .rk.do.plain.call("listPlugins")
	columns = c ("ID", "Context", "Menupath", "Label")
	as.data.frame (matrix (plugs, ncol=length (columns), byrow=TRUE, dimnames=list (1:(length (plugs)/length (columns)), columns)), stringsAsFactors=FALSE)
}

#' @export
#' @rdname rk.list.plugins
"rk.set.plugin.status" <- function (id, context="", visible=TRUE) {
	id <- as.character (id)
	context <- rep (as.character (context), length.out=length (id))
	visible <- rep (as.character (as.numeric (visible)), length.out=length (id))
	.rk.do.plain.call ("setPluginStatus", c (id, context, visible))
	invisible (NULL)
}
