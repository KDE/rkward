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

#' Set plugin language for internationalisation
#' 
#' Stores the given language in an internal environment, so functions like
#' \code{\link[rkwarddev:i18n]{i18n}} can use it.
#' 
#' @param lang Character string, abbreviated language to use with \code{\link[rkwarddev:i18n]{i18n}}, e.g. \code{"en"}.
#' @param locales Character vector, all the locales this translation covers, e.g. \code{c("en_EN", "en_US")}.
#' @export
#' @examples
#' rk.set.language("en", c("en_EN", "en_US"))

rk.set.language <- function(lang=NULL, locales=NULL){
  if(is.null(lang)){
    if(exists("lang", envir=.rkdev.env, inherits=FALSE)){
      rm("lang", envir=.rkdev.env)
    } else {}
    if(exists("locales", envir=.rkdev.env, inherits=FALSE)){
      rm("locales", envir=.rkdev.env)
    } else {}
    message(paste("removed language setting"))
  } else {
    assign("lang", lang, envir=.rkdev.env)
    message(paste("set language to:", dQuote(lang)))
    if(is.null(locales)){
      warning("please provide at least one locale!")
    } else {
      assign("locales", locales, envir=.rkdev.env)
      message(paste("set locales to:", paste0(dQuote(locales), collapse=", ")))
    }
  }
  return(invisible(NULL))
}
