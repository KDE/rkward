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


#' Create i18n comment for RKWard plugin code
#'
#' This function is similar to rk.comment, but preceds the \code{text} with the
#' keyword \code{"i18n:"} to give context to translators.
#'
#' @param text Character string, the text to be displayed.
#' @param prefix Character string, the text to be prefixed to indicate this is
#'    an i18n comment.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @examples
#' test.comment <- rk.i18n.comment("Added this text.")
#' cat(pasteXML(test.comment))

rk.i18n.comment <- function(text, prefix="i18n:"){
  return(XMLNode(name="!--", paste(prefix, text)))
}
