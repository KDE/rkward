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


#' Get plugin language for internationalisation
#' 
#' @param locales Logical, whether to query for language or locales set.
#' @export
#' @examples
#' rk.get.language()

rk.get.language <- function(locales=FALSE){
  if(isTRUE(locales)){
    if(exists("locales", envir=.rkdev.env, inherits=FALSE)){
      locales <- get("locales", envir=.rkdev.env)
      return(locales)
    } else {}
  } else {
    if(exists("lang", envir=.rkdev.env, inherits=FALSE)){
      lang <- get("lang", envir=.rkdev.env)
      return(lang)
    } else {
      return(invisible(NULL))
    }
  }
}
