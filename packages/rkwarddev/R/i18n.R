# Copyright 2010-2014 Meik Michalke <meik.michalke@hhu.de>
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


#' Translate parts of a plugin
#' 
#' Takes a list of entries named after abbreviated languages, and returns
#' either the one matching the language set with \code{\link[rkwarddev:rk.set.language]{rk.set.language}},
#' or the first entry if no language was set at all or the set language cannot be found in
#' \code{...}.
#' 
#' If used in an \code{rkwarddev} script, this can be used to toggle the generation of plugins
#' in a certain language.
#' 
#' @param ... Comma separated, named elements, see description.
#' @param lang Character string, the language to return.
#' @export
#' @examples
#' rk.set.language("en", c("en_EN", "en_US"))
#' (var.select <- rk.XML.varselector(label=i18n(en="Select data", de="Wähle Daten")))
#' 
#' # now try the same with the alternate language
#' rk.set.language("de", "de_DE")
#' (var.select <- rk.XML.varselector(label=i18n(en="Select data", de="Wähle Daten")))

i18n <- function(..., lang=rk.get.language()){
  obj <- list(...)
  # check if any language is set at all
  if(is.null(lang)){
    # if not, simply return the first entry as the default
    warning("i18n() was called, but no language is set, using default values!")
    return(obj[[1]])
  } else {
    # there is a language set, but is there also a translation
    # given for that language?
    if(lang %in% names(obj)){
      return(obj[[lang]])
    } else {
      # if not, again fall back to the first entry as default
      warning(paste("i18n() was called, but no translation into", dQuote(lang), "was found, using default values!"))
      return(obj[[1]])
    }
  }
}
