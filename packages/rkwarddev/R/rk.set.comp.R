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

#' Define the component you're currently working on
#' 
#' This small tool let's you set a component name as kind of "active", which simply
#' means it will be returned by \code{\link[rkwarddev:rk.get.comp]{rk.get.comp}}. This can be
#' used by functions like, e.g., \code{\link[rkwarddev:rk.XML.cbox]{rk.XML.cbox}}, to add text
#' for .rkh pages automatically to the current plugin component.
#' 
#' @param component Character string, name of the component to set. If \code{NULL},
#'    no component will be set as default, and \code{rk.get.comp} will return \code{NULL}
#'    subsequently.
#' @export

rk.set.comp <- function(component=NULL){
  rkh.prompter <- get.rkh.prompter()
  rkh.prompter[[".active.component"]] <- component
  set.rk.env("rkh.prompter", value=rkh.prompter)
  return(invisible(NULL))
}
