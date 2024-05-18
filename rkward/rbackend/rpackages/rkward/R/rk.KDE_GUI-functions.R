# - This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#' Message boxes and selection list using native KDE GUI
#'
#' Multi-purpose pop-up message boxes, selection list and file selectors using native
#' KDE GUI elements. The message boxes can be used either to show some information or
#' ask some question. The selection list can be used to get a vector of
#' selected items. The file selector opens a file system browser to chose one or
#' multiple files, a directory, or a path for a new file.
#'
#' For \code{rk.show.question}, the R interpreter always waits for the user's
#' choice.
#'
#' \code{rk.select.list} replaces \code{\link[utils:select.list]{utils::select.list}}
#' for the running session acting as a drop-in replacement for \code{tk_select.list}.
#' Use \code{.rk.backups$select.list} for the original \code{utils::select.list}
#' function (see Examples).
#'
#' \code{rk.select.file} combines the functionality of \code{\link[base:file.choose]{file.choose}}
#' and \code{utils::choose.dir}. The latter is only available in R for Windows, but
#' \code{rk.select.file} can be used on all systems where RKWard is available. \code{rk.choose.dir}
#' simply is a wrapper for \code{rk.select.file(mode="dir")}. Please note that if users have configured
#' KDE to change directories with just one click, they may find the \code{OK} button not to work as
#' they expect. In those cases, the name of the target directory must be entered into the name
#' input field of the dialog manually.
#'
#' @aliases rk.show.message rk.show.question rk.select.list
#' @param message a string for the content of the message box.
#' @param msg like \code{message}, only the argument was renamed to mimic the formals of
#'   \code{askYesNo}.
#' @param caption a string for title of the message box. Optional for \code{rk.select.file} and \code{rk.choose.dir}.
#' @param button.yes a string for the text label of the \bold{Yes} button. Can
#'   be an empty string (\code{""}), in which case the button is not displayed
#'   at all. Note that the default value of "yes" (lower case) means to use a default
#'   rendering of a "Yes" button, which may or may not be the literal string "Yes"
#'   (importantly, it will be translated, automatically).
#' @param button.no a string used for the text label of the \bold{No} button.
#'   This behaves similar to \code{button.yes}, including the meaning of the
#'   default value "no".
#' @param button.cancel a string used for the text label of the \bold{Cancel} button.
#'   This behaves similar to \code{button.yes}, including the meaning of the
#'   default value "cancel".
#' @param prompts either a character vector containing 3 prompts corresponding to return
#'   values of TRUE, FALSE, or NA, or a single character value containing the prompts
#'   separated by \code{/} characters.
#' @param default The expected or "safe" default response (e.g. \code{TRUE} for "Yes button").
#'   The corresponding button will focused, so that it will become selected option,
#'   if the user simply presses enter.
#' @param wait a logical (not NA) indicating whether the R interpreter should
#'   wait for the user's action, or run it asynchronously.
#' @param choices a vector, coerced into a character vector.
#' @param preselect a vector, coerced into a character vector, items to be
#'   preselected.
#' @param multiple a logical (not NA), when \code{TRUE} multiple selection
#'   selection is allowed.
#' @param title a string, for the window title of the displayed list
#' @param is.rk.askYesNo a logical value, you can safely ignore this argument if you call
#'    \code{rk.askYesNo} manually. This argument is needed if \code{rk.askYesNo} is set
#'    via \code{options("askYesNo"=rk.askYesNo)} because otherwise we'd either need more
#'    complicated function code there, fail with an error or end up in an infinite loop.
#' @return \code{rk.show.message} always returns \code{TRUE}, invisibly.
#' 
#' \code{rk.show.question} returns \code{TRUE} for \bold{Yes}, \code{FALSE} for
#'   \bold{No}, and \code{NULL} for \bold{Cancel} actions. If the dialog is closed
#'   without clicking on any button, \code{NULL} is returned, as well.
#' 
#' \code{rk.askYesNo} has the same return values as \code{rk.show.question}, except
#'   it returns \code{NA} for \bold{Cancel} actions.
#' 
#' \code{rk.select.list} returns the value of \code{\link{select.list}}, or an empty string
#'    \code{""} if cancelled.
#'
#' \code{rk.select.file} and \code{rk.choose.dir} return the path to the selected target,
#'    or \code{character(0)} if cancelled.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@kde.org}
#' @seealso \code{\link{system}}, \code{\link{select.list}},
#'    \code{\link[base:file.choose]{file.choose}}
#' @keywords utilities
#' @rdname rk.show.messages
#' @export
#' @examples
#' \dontrun{
#' ## Message boxes
#' if (rk.show.question ("Question:\nDo you want to know about RKWard?", 
#'     button.yes = "Yes, I do!", button.no = "No, I don't care!", button.cancel = "")) {
#'   rk.show.message ("Message:\nRKWard is a KDE GUI for R.", "RKWard Info")
#' } else {
#'   rk.show.message ("You must be joking!", "RKWard Info", wait = FALSE) ## Run asynchronously
#' }
#' 
#' ## Selection lists:
#' rk.select.list (LETTERS, preselect = c("A", "E", "I", "O", "U"), 
#'   multiple = TRUE, title = "vowels")
#' .rk.backups$select.list (LETTERS, preselect = c("A", "E", "I", "O", "U"), 
#'   multiple = TRUE, title = "vowels")
#' }
"rk.show.message" <- function (message, caption = gettext("Information"), wait=TRUE) {
	.Call ("rk.dialog", caption, message, "ok", "", "", "ok", isTRUE (wait), PACKAGE="(embedding)")
	invisible (TRUE)
}

# to disable a button, set it to ""
#' @export
#' @rdname rk.show.messages
"rk.show.question" <- function (message, caption = gettext("Question"), button.yes = "yes", button.no = "no", button.cancel = "cancel", default=TRUE) {
  .Deprecated("rk.askYesNo")
	if (isTRUE (default)) default_button <- button.yes
	else if (identical (default, FALSE)) default_button <- button.no
	else default_button <- button.cancel
	res <- .Call ("rk.dialog", caption, message, button.yes, button.no, button.cancel, default_button, TRUE, PACKAGE="(embedding)")
	if (res > 0) return (TRUE)
	else if (res < 0) return (FALSE)
	else return (NULL)	# cancelled
}

#' @param ... currently unused.
#' @export
#' @rdname rk.show.messages
"rk.askYesNo" <- function (msg, default = TRUE, prompts = c("yes", "no", "cancel"), caption = gettext("Question"), is.rk.askYesNo=TRUE, ...) {
 if(is.function(prompts)){
    # using options() to set the prompts value for askYesNo() to this function also replaces our prompts and we'd
    # end up in an infinite loop. we can check for the presence of the "rk.askYesNo" argument to see if that's the case
    if(isTRUE(is.rk.askYesNo)){
      prompts <- eval(formals("rk.askYesNo")[["prompts"]])
    } else {
      stop(simpleError("'rk.askYesNo' was designed to be used as the function code of 'askYesNo' and cannot be given a function "))
    }
  } else {}
  if(is.character(prompts)){
    if(length(prompts) == 1){
      prompts <- unlist(strsplit(prompts, "/"))
    } else {}
    if(length(prompts) != 3){
      stop(simpleError("'prompts' must be either a single character string or three character values!"))
    } else {}
    button.yes <- prompts[1]
    button.no <- prompts[2]
    button.cancel <- prompts[3]
  } else {
    stop(simpleError("'prompts' must be character!"))
  }

  default_button <- switch(
    as.character(as.logical(default)),
    "TRUE"=button.yes,
    "FALSE"=button.no,
    button.cancel
  )

  res <- .Call(
    "rk.dialog",
    caption,
    msg,
    button.yes,
    button.no,
    button.cancel,
    default_button,
    TRUE,
    PACKAGE="(embedding)"
  )

  if (res > 0){
    return (TRUE)
  } else if (res < 0){
    return (FALSE)
  } else {
    return (NA) # cancelled
  }
}

# drop-in-replacement for tk_select.list()
#' @export
#' @rdname rk.show.messages
"rk.select.list" <- function (choices, preselect, multiple = FALSE, title) {
  # for the time being, translate NULL into missingness and throw a warning
  if(!missing(preselect)){
    if (is.null (preselect)) {
      warning("Deprecated: preselect = NULL, leave missing if unused!")
      preselect <- substitute()
    }
  }
  if (missing (preselect)) {
      preselect <- character(0)
  } else {
      preselect <- as.character (preselect)
  }
  preselect.len = length (preselect)
  choices <- as.character (choices)
  choices.len <- length (choices)
  params <- list ()

  # serialize all parameters
  if(!missing(title)){
    if (is.null (title)) {
      warning("Deprecated: title = NULL, leave missing if unused!")
      title <- substitute()
    }
  }
  params[1] <- if(missing (title)) "" else as.character (title)
  if (multiple) params[2] <- "multi"
  else params[2] <- "single"
  params[3] <- as.character (preselect.len)
  if (preselect.len) {
    for (i in 1:preselect.len) {
      params[3+i] <- preselect[i]
    }
  }
  if (choices.len) {	# we should hope, the list is not empty...
    for (i in 1:choices.len) {
      params[3+preselect.len+i] <- choices[i]
    }
  }

  # NOTE: Passing all params as flat plain character vector for purely historical reasons. This could be changed.
  .rk.call("select.list", as.character(params))
}

# file dialog
#' @param initial character string, specifies initial directory to start at (or file to select).
#'    If this starts with a \code{"#"}, the last file selected in a dialog with the same initial parameter
#'    will be used, instead. E.g. \code{initial="#images"}.
#' @param filter regular expression to filter for valid file or directory names.
#' @param mode one of \code{"file"} (select one file), \code{"files"} (select one or more files),
#'    \code{"dir"} (select a directory), or \code{"newfile"} (set path for a new file).
#' @export
#' @rdname rk.show.messages
"rk.select.file" <- function(caption, initial, filter = '*', mode=c("file", "files", "dir", "newfile")) {
    mode <- match.arg(mode)
    # NOTE: Passing all params as flat plain character vector for purely historical reasons. This could be changed.
    .rk.call(
        "choosefile",
        as.character(c(
            if (missing(caption)) "" else caption,
            if (missing(initial)) "" else initial,
            filter,
            mode[1]
        ))
    )
}

# wrapper for dirs only
#' @export
#' @rdname rk.show.messages
"rk.choose.dir" <- function(caption, initial, filter = '*'){
    rk.select.file(caption = caption, initial = initial, filter = filter, mode="dir")
}
