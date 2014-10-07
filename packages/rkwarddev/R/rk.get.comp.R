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

#' Get the name of the component you're currently working on
#' 
#' This is the "get"-equivalent to \code{\link[rkwarddev:rk.set.comp]{rk.set.comp}}.
#' It is used by functions like, e.g., \code{\link[rkwarddev:rk.XML.cbox]{rk.XML.cbox}},
#' to add text for .rkh pages automatically to the current plugin component.
#' @export

rk.get.comp <- function(){
  rkh.prompter <- get.rkh.prompter()
  return(rkh.prompter[[".active.component"]])
}
