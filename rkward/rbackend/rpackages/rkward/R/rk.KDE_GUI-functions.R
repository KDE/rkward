#' Message boxes and selection list using native KDE GUI
#' 
#' Multi-purpose pop-up message boxes and selection list using native KDE GUI
#' elements. The message boxes can be used either to show some information or
#' ask some question. The selection list can be used to get a vector of
#' selected items.
#' 
#' For \code{rk.show.question}, the R interpreter always waits for the user's
#' choice.
#' 
#' \code{rk.select.list} replaces \code{utils::select.list} for the running
#' session acting as a drop-in replacement for \code{tk_select.list}. Use
#' \code{.rk.backups$select.list} for the original \code{utils::select.list}
#' function (see Examples).
#' 
#' @aliases rk.show.message rk.show.question rk.select.list
#' @param message a string for the content of the message box.
#' @param caption a string for title of the message box.
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
#' @param default.button The button will be the default (selected if the user
#'   simply pressed enter.
#' @param wait a logical (not NA) indicating whether the R interpreter should
#'   wait for the user's action, or run it asynchronously.
#' @param list a vector, coerced into a character vector.
#' @param preselct a vector, coerced into a character vector, items to be
#'   preselected.
#' @param multiple a logical (not NA), when \code{TRUE} multiple selection
#'   selection is allowed.
#' @param title a string, for the window title of the displayed list
#' @return \code{rk.show.message} always returns \code{TRUE}, invisibly.
#' 
#' \code{rk.show.question} returns \code{TRUE} for \bold{Yes}, \code{FALSE} for
#'   \bold{No}, and \code{NULL} for \bold{Cancel} actions. If the dialog is closed
#'   without clicking on any button, \code{NULL} is returned, as well.
#' 
#' \code{rk.select.list} returns the value of \code{\link{select.list}}.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@kde.org}
#' @seealso \code{\link{system}}, \code{\link{select.list}}
#' @keywords utilities
#' @rdname rk.show.messages
#' @examples
#' 
#' require (rkward)
#' 
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

#' @export
"rk.show.message" <- function (message, caption = gettext("Information"), wait=TRUE) {
	.Call ("rk.dialog", caption, message, "ok", "", "", "ok", isTRUE (wait), PACKAGE="(embedding)")
	invisible (TRUE)
}

# to disable a button, set it to ""
#' @export
#' @rdname rk.show.messages
"rk.show.question" <- function (message, caption = gettext("Question"), button.yes = "yes", button.no = "no", button.cancel = "cancel", default.button=button.yes) {
	res <- .Call ("rk.dialog", caption, message, button.yes, button.no, button.cancel, default.button, TRUE, PACKAGE="(embedding)")
	if (res > 0) return (TRUE)
	else if (res < 0) return (FALSE)
	else return (NULL)	# cancelled
}

# drop-in-replacement for tk_select.list()
#' @export
#' @rdname rk.show.messages
"rk.select.list" <- function (list, preselect = NULL, multiple = FALSE, title = NULL) {
	preselect <- as.character (preselect)
	preselect.len = length (preselect)
	list <- as.character (list)
	list.len <- length (list)
	params <- list ()

	# serialize all parameters
	params[1] <- as.character (title)
	if (multiple) params[2] <- "multi"
	else params[2] <- "single"
	params[3] <- as.character (preselect.len)
	if (preselect.len) {
		for (i in 1:preselect.len) {
			params[3+i] <- preselect[i]
		}
	}
	if (list.len) {	# we should hope, the list is not empty...
		for (i in 1:list.len) {
			params[3+preselect.len+i] <- list[i]
		}
	}

	.rk.do.plain.call ("select.list", params)
}
