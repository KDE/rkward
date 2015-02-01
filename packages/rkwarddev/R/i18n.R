# Copyright 2014 Meik Michalke <meik.michalke@hhu.de>
#
# This file is part of the R package rkwarddev.
#
# rkwarddev is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# rkwarddev is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with rkwarddev.  If not, see <http://www.gnu.org/licenses/>.


#' Mark JavaScript strings as translatable
#' 
#' Similar to \code{\link[rkwarddev:echo]{echo}}, this function should help you to write
#' your JavaScript portions in R. Depending on the provided values for its arguments,
#' will return one of \code{i18n()}, \code{i18nc()}, \code{i18np()}, or \code{i18ncp()}.
#' 
#' @param msgid Character string, the message to be translated (if applicable, its singular form).
#' @param ... Either character string which will be pasted unquoted to be used in conjunctions with
#'    placeholders in msgid, or XiMpLe.node objects of which the JavaScript variable name will be
#'    used.
#' @param context Character string, optional context information for this string.
#' @param plural Character string for plural form of \code{msgid}, must at least include one
#'    placeholder, and the first one has to represent an integer value in the dialog.
#' @param newline Character string, can be set to e.g. \code{"\n"} to force a newline after the call.
#' @return An object of class \code{rk.JS.i18n}.
#' @export
#' @examples
#' i18n("Select data")
#' i18n("Comparing a single pair", "n_pairs", plural="Comparing %1 distinct pairs")

i18n <- function(msgid, ..., context=NULL, plural=NULL, newline=""){
  placeholders <- list(...)
  pluralQuoted <- placeholderString <- NULL
  JSfunction <- "i18n"

  if(!is.null(context)){
    JSfunction <- paste0(JSfunction, "c")
    context <- paste0(qp(context), ", ")
  } else {}
  if(!is.null(plural)){
    JSfunction <- paste0(JSfunction, "p")
    pluralQuoted <- paste0(", ", qp(plural))
  } else {}
 
  if(length(placeholders) > 0){
    # do some sanitiy checks here -- is there a placeholder in the strings for each dots value?
    # grep valid placeholders out of the messages
    msgCleaned <- gsub("([^%[:digit:]]{2,})", " ", paste(msgid, plural))
    msgSplit <- unique(unlist(strsplit(msgCleaned, "[[:space:]]+")))
    msgPlHd <- msgSplit[grep("%[[:digit:]]", msgSplit)]
    # which placeholders are needed?
    plHdNeeded <- paste0("%", 1:length(placeholders))
    missingPlHd <- plHdNeeded[!plHdNeeded %in% msgPlHd]
    missingVals <- msgPlHd[!msgPlHd %in% plHdNeeded]
    if(length(missingPlHd) > 0){
      stop(simpleError(paste0("i18n: some placeholders in this string are missing: ", paste0(missingPlHd, collapse=", "), "!")))
    } else {}
    if(length(missingVals) > 0){
      stop(simpleError(paste0("i18n: some placeholders in this string do not have a corresponding value: ", paste0(missingVals, collapse=", "), "!")))
    } else {}
    placeholderString <- paste0(", ", paste0(sapply(placeholders, function(ph){id(ph, js=TRUE)}), collapse=", "))
  } else {}
  
  result <- new("rk.JS.i18n",
    value=paste0(
      JSfunction, "(",
      context,
      qp(msgid),
      pluralQuoted,
      placeholderString,
      ")",
      newline
    )
  )

  return(result)
}

## internal class rk.JS.i18n
# this is a quick fix to be able to include i18n() calls inside echo()
setClass("rk.JS.i18n",
  representation=representation(
    value="character",
    end="character"
  ),
  prototype(
    value=character(),
    end=";"
  )
)
