# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#' Class and functions for organizing RKWard output.
#' @name RK.Output
#'
#' @description Since version 0.7.3, RKWard supports more than one output document. While dealing with only a single output page, there will be no need for the user to call any of
#'              the following functions, directly, as exactly one output is always opened for writing in RKWard (unless rk.set.output.html.file() has been called, explicitly).
#'              However, these functions allow to manage several distinct output pages, programmatically.
#'
#'              The primary entry point is the function \code{rk.output}, which allows to retrieve the current output directly, or to load or create a new one. This will return
#'              an instance of the \code{RK.Output} reference class, which has methods for subsequent manipulation. Two instances of this class may be pointing to the same
#'              logical output file/directory (e.g. when loading the same output file, twice), in which case any operation will affect both instances the same.
#'
#'              Internally, outputs are managed by the RKWard frontend. The frontend will ask to save any unsaved modified output pages on exit, even if those are not currently shown.
#'
#'              Output references can be assigned to a symbol, which may be useful when writing to several output files in turn. However, such references should be considered
#'              short-lived. Importantly, they will not currently remain valid across sessions. Where this may be a concern, code should obtain a new reference using
#'              rk.output(filename="something") at relevant entry points to subsequent code.
#'
#'              At the time of this writing, output is stored in directories containing an HTML index file, and, usually, several image files, and possibly more.
#'              However other types of output may be supported in the future, and therefore assumptions about the details of the output storage should be avoided.
#'
#'              \code{rk.output} can be used to create or load output files, as well as to obtain a reference to an already loaded output file. After that, use the class methods
#'              to operate on the reference obtained.
#'
#'              \code{rk.import.legacy.output} can be used to import an legacy (pre 0.7.3) output file to .rko-format. This function is going to be removed after some time.
#'
#' @param filename The location where an existing output directory is loaded from or saved/exported to. Note that this is usually not the same location where functions such as
#'                 \link{rk.print} will write to (these operate on a temporary "working copy", but rather the target directory, where changes should eventually be saved
#'                 back, to.
#'
#' @param create If \code{TRUE}, create a new output directory. The parameter \code{filename}, if specified, is the target save file/directory, in this case. Should this already exist,
#'               an error will be raised. If \code{create=FALSE}, load or re-use an existing output directory. If the parameter \code{filename} is left \code{NULL}, \code{rk.output} will
#'               return the currently active output in this case (creating and/or activating an output file, in case all outputs have been closed or deactivated).
#'
#' @param all If \code{TRUE}, return a list of all currently loaded output directories.
#'
#' @param overwrite If \code{TRUE}, RKWard will overwrite any existing output when saving, or discard any existing modifications when closing or reverting an output. If \code{FALSE}, trying
#'                  to overwrite/discard existing files/modifications will result in an error. If \code{NULL} (the default), the frontend will ask what to do in this case.
#'
#' @param discard See \code{overwrite} for the meaning.
#'
#' @param raise Raise the output window, if it is already visble.
#'
#' @returns NULL
#' @field id An internal identifier. NULL for a closed output. This should be treated as read-only, but you can use this to test whether two output handles are the same.
#' @import methods
#' @export RK.Output
#'
#' @examples
#' \dontrun{
#' x <- rk.output(create=TRUE)
#' x$activate()
#' rk.print("Hello World!")
#' x$view()
#' x$save() # Will prompt for filename
#' x$close()
#' }
RK.Output <- setRefClass(Class="RK.Output", fields=list(id="character"),
	methods=list(
	# The implementation of most of these is not terribly complex, but we need an implementation in the frontend, anyway, so we use that.
		activate=function() {
"Set this output as the one that rk.print and other RKWard Output functions will write to."
			.rk.call.nested("output", c ("activate", .checkId()))
		},
		isEmpty=function() {
"Returns TRUE, if the output is currently empty."
			.rk.call.nested("output", c ("isEmpty", .checkId()))
		},
		isModified=function() {
"Returns TRUE, if this output has any changes that may need saving."
			.rk.call.nested("output", c ("isModified", .checkId()))
		},
		revert=function(discard=NULL) {
"Revert this output to the last saved state. If no previous state is available (never saved, before), clears the output."
			.rk.call.nested("output", c ("revert", .checkId(), if(is.null(discard)) "ask" else if(isTRUE(discard)) "force" else "fail"))
		},
		save=function(filename, overwrite=NULL) {
"Save this output, either to the last known save location (if no filename is specified) or to a new location (\"save as\")."
			if (missing(filename)) filename <- ""
			.rk.call.nested("output", c ("save", .checkId(), if(is.null(overwrite)) "ask" else if(isTRUE(overwrite)) "force" else "fail", filename))
		},
		export=function(filename, overwrite=NULL) {
"Save this output, to the specified location, but keep it associated with the previous location (\"save a copy\")."
			if (missing(filename)) stop("No file name specified")
			.rk.call.nested("output", c ("export", .checkId(), if(is.null(overwrite)) "ask" else if(isTRUE(overwrite)) "force" else "fail", filename))
		},
		clear=function(discard=NULL) {
"Clear all content from this output. As with any function in this class, this affects the working copy, only, until you call save. Therefore, by default, the user will be prompted for confirmation
if and only if there are unsaved changes pending."
			.rk.call.nested("output", c ("clear", .checkId(), if(is.null(discard)) "ask" else if(isTRUE(discard)) "force" else "fail"))
		},
		close=function(discard=NULL) {
"Forget about this output file, also closing any open views. Note: Trying to call any further methods on this object will fail."
			.rk.call.nested("output", c ("close", .checkId(), if(is.null(discard)) "ask" else if(isTRUE(discard)) "force" else "fail"))
			id<<-character(0)
		},
		view=function(raise=TRUE) {
"Open this output for viewing in the frontend."
			.rk.call.nested("output", c ("view", .checkId(), if(isTRUE(raise)) "raise" else ""))
		},
		.workingDir=function() {
"The path of the working copy of this object. Please don't use this except for automated tests. The internals may be subject to change."
			.rk.call.nested("output", c ("workingDir", .checkId()))
		},
		filename=function() {
"Return the target filename for this output, i.e. the location where it will be saved, to. This will be an empty string for newly created outputs that have not been saved, yet.
Do not write anything to the target filename, directly! This is purely for information."
			.rk.call.nested("output", c ("filename", .checkId()))
		},
		.checkId=function() {
"For internal use: Throws an error, if the id parameter is NULL or too long, returns a length one character vector otherwise."
			i <- as.character(id)
			if (length(i) != 1) stop ("Invalid output id. Use rk.output() to obtain a valid output handle.")
			i
		}
	))

#' @export
#' @rdname RK.Output
"rk.output" <- function(filename=NULL, create=FALSE, all=FALSE) {
	if(all && (!is.null(filename) || create)) stop("'all' cannot be combined with 'create' or 'filename'")
	id <- .rk.call.nested("output", c ("get", if(isTRUE(all)) "all" else "one", if(isTRUE(create)) "create" else "", if(is.null(filename)) "" else as.character(filename)))
	ret <- lapply(id, function(id) RK.Output(id=id))
	if (all) ret
	else ret[[1]]
}

#' @param import logical, whether to import file \code{filename}.
#' @param delete logical, whether to delete file \code{filename} and all its images on exit.
#' @export
#' @rdname RK.Output
"rk.import.legacy.output" <- function(filename=file.path(rk.home(), "rk_out.html"), import=TRUE, delete=FALSE) {
	f <- filename
	stopifnot(file.exists(f))
	files <- .rk.get.images.in.html.file(f)
	css <- file.path(rk.home(), "rk_out.css")
	if (file.exists(css)) files <- c(files, css)

	if(import) {
		out <- rk.output(create=TRUE)
		out$activate()
		wd = out$.workingDir()   # Don't do this at home, please. For internal use, only, and might change

		cat(gsub(paste("file://", rk.home(), sep=""), "./", readLines(f)), file=file.path(wd, "index.html"), sep="\n")
		stopifnot(all(file.copy(files, file.path(wd, basename(files)), overwrite=TRUE)))
		out$view()

		rk.show.message("The legacy output file has been imported. If satisfied with the result, you can now save it in the new format. To remove the old output file, run rk.import.legacy.output(import=FALSE,delete=TRUE)")
	}

	if (delete) {
		unlink(f)
		unlink(files)
	}
}
