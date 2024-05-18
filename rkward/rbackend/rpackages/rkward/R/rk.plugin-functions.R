# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
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
#' @author Thomas Friedrichsmeier \email{rkward-devel@@kde.org}
#' @seealso \code{\link{rk.results}}, \url{rkward://page/rkward_output}
#' @keywords utilities
#' @export
#' @rdname rk.call.plugin
#'
#' @examples
#'
#' ## "t_test_two_vars" plugin:
#' ## see the output: Windows->Show Output
#' \dontrun{
#' local({
#' x1 <- rnorm (100)
#' x2 <- rnorm (100, 2)
#' rk.call.plugin ("rkward::t_test_two_vars",
#'   confint.state="1", conflevel.real="0.95", hypothesis.string="greater", paired.state="0", varequal.state="0", 
#'   x.available="x1", y.available="x2", 
#'   submit.mode="submit")
#' })
#' }
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
	.rk.call.nested("doPlugin", callstrings)
	invisible (TRUE)
}

#' (Re-)load the given pluginmap files into the RKWard GUI
#'
#' @param pluginmap.files a character vector of file names to add. This may be left empty,
#'                        if the only desired effect is to reload all active pluginmaps.
#' @param force.add logical. Whether the pluginmap files should also be added, if# they had
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
#' @author Thomas Friedrichsmeier \email{rkward-devel@@kde.org}
#' @seealso \code{\link{rk.call.plugin}}, \code{\link[rkwarddev:rk.plugin.skeleton]{rk.plugin.skeleton}}
#' @keywords utilities
#' @rdname rk.load.pluginmaps
#' @export
#'
#' @examples
#' \dontrun{
#' ## reload all active pluginmaps
#' rk.load.pluginmaps()
#' }
"rk.load.pluginmaps" <- function (pluginmap.files, force.add = TRUE, force.reload = TRUE) {
	# for the time being, translate NULL into missingness and throw a warning
	if(!missing(pluginmap.files)){
		if (is.null (pluginmap.files)) {
			warning("Deprecated: pluginmap.files = NULL, leave missing if unused!")
			pluginmap.files <- substitute()
		}
	}
	if (missing (pluginmap.files)) pluginmap.files <- character(0)
	.rk.call.async("loadPluginMaps", c(ifelse(isTRUE(force.add), "force", "noforce"), ifelse(isTRUE(force.reload), "reload", "noreload"), as.character(pluginmap.files)))
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
#' @author Thomas Friedrichsmeier \email{rkward-devel@@kde.org}
#' @keywords utilities
#' @seealso \code{\link{rk.call.plugin}} for invoking a plugin, programatically
#' @rdname rk.list.plugins
#' @aliases rk.list.plugins rk.set.plugin.status
#' @export
#'
#' @examples
#' \dontrun{
#' ## list all current plugins
#' rk.list.plugins ()
#'
#' ## hide t.test plugin
#' rk.set.plugin.status ("rkward::t_test", visible=FALSE)
#' }
"rk.list.plugins" <- function () {
	plugs <- .rk.call("listPlugins")
	columns = c ("ID", "Context", "Menupath", "Label")
	as.data.frame (matrix (plugs, ncol=length (columns), byrow=TRUE, dimnames=list (1:(length (plugs)/length (columns)), columns)), stringsAsFactors=FALSE)
}

#' @export
#' @rdname rk.list.plugins
"rk.set.plugin.status" <- function (id, context="", visible=TRUE) {
	id <- as.character (id)
	context <- rep (as.character (context), length.out=length (id))
	visible <- rep (as.character (as.numeric (visible)), length.out=length (id))
	# NOTE: Passing params as flat character vector for purely historical reasons. This could be changed.
	.rk.call("setPluginStatus", c(id, context, visible))
	invisible (NULL)
}

assign(".rk.preview.data", list (), envir=.rk.variables)

#' Manage (shortly) persistent data for previews (for use in RKWard plugins wishing to provide custom previews)
#'
#' \code{rk.assign.preview.data} stores data associated with a specific "id". Usually this id is
#'    provided by the <preview>-feature of a plugin.
#' \code{rk.get.preview.data} retrieves data previously stored with \code{rk.assign.preview.data}
#' \code{rk.discard.preview.data} discards data previously stored with \code{rk.assign.preview.data}.
#'    This gets called by the <preview>-box of the plugin, automtically, when the plugin dialog is closed.
#'    You do not generally have to call it manually. See the notes for running custom clearnup code, below.
#'
#' @param id character, ID associated with the data. Usually this will be the 'id' value of the <preview>-box.
#' @param value the value to assign. If this is a list, and contains a function named "on.delete", this function
#'    will be run by rk.discard.preview.data (with the \code{id} as argument. This is useful for running custom clearnup
#'    code, such as removing temporary files, etc.
#'
#' @return \code{rk.assign.preview.data} amd \code{rk.get.preview.data} returns the preview data (newly) associated
#'    with the given id. \code{rk.discard.preview.data} returns \code{NULL}, invisibly.
#'
#' \bold{Note}: Plugins that want to produce a single plot, or open a single object via \code{\link{rk.edit}()} do \bold{not}
#'              have to call these functions, manually. See the chapter on providing previews in the Introduction to
#'              writing plugins for RKWard.
#'
#' @author Thomas Friedrichsmeier \email{rkward-devel@@kde.org}
#' @keywords utilities
#' @rdname rk.assign.preview.data
#' @aliases rk.get.preview.data .rk.discard.preview.data
#' @export
#'
#' @examples
#' ## To be generated in the preview() code section of a plugin
#' \dontrun{
#' pdata &lt;- rk.get.preview.data("SOMEID")
#' if (is.null (pdata)) {
#'   outfile &lt;- rk.get.tempfile.name(prefix="preview", extension=".txt", directory=rk.tempdir ())
#'   pdata &lt;- list(filename=outfile, on.delete=function (id) {
#'     unlink(rk.get.preview.data(id)$filename)
#'   })
#'   rk.assign.preview.data("SOMEID", pdata)
#' }
#' try ({
#'   cat ("This is a test", pdata$filename)
#'   rk.edit.files(file=pdata$filename)
#' })
#' }
"rk.assign.preview.data" <- function (id, value=list ()) {
	pdata <- .rk.variables$.rk.preview.data
	pdata[[id]] <- value
	assign (".rk.preview.data", pdata, envir=.rk.variables)
	rk.sync (.rk.variables$.rk.preview.data)
	invisible (pdata[[id]])
}

#' @export
#' @rdname rk.assign.preview.data
"rk.get.preview.data" <- function (id) {
	.rk.variables$.rk.preview.data[[id]]
}

#' @export
#' @rdname rk.assign.preview.data
"rk.discard.preview.data" <- function (id) {
	pdata <- .rk.variables$.rk.preview.data
	if (is.list (pdata[[id]]) && !is.null (pdata[[id]]$on.delete)) pdata[[id]]$on.delete (id)
	pdata[[id]] <- NULL
	assign (".rk.preview.data", pdata, envir=.rk.variables)
	rk.sync (.rk.variables$.rk.preview.data)
	invisible (NULL)
}
