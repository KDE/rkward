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


#' Create JavaScript variables from plugin XML
#'
#' @param pXML Either an object of class \code{XiMpLe.doc} or \code{XiMpLe.node}, or path to a plugin XML file.
#' @param js Logical, if \code{TRUE} usable JavaScript code will be returned, otherwise a character
#'    vector with only the relevant ID names.
#' @param add.abbrev Logical, if \code{TRUE} the JavaScript variables will all have a prefix with an
#'    three letter abbreviation of the XML tag type to improve the readability of the code. But it's
#'    probably better to add this in the XML code in the first place.
#' @param guess.getter Logical, if \code{TRUE} try to get a good default getter function for JavaScript
#'    variable values. This will use some functions which were added with RKWard 0.6.1, and therefore
#'    raise the dependencies for your plugin/component accordingly. Nonetheless, it's recommended.
#' @param indent.by Character string used to indent each entry if \code{js=TRUE}.
#' @return A character vector.
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export

rk.JS.scan <- function(pXML, js=TRUE, add.abbrev=FALSE, guess.getter=FALSE, indent.by=rk.get.indent()){

  # these are tags to scan normally, no special treatment
  JS.relevant.tags.default <- c("browser", "dropdown", "input", "matrix", "optioncolumn",
    "radio", "saveobject", "select", "spinbox", "valueslot", "varslot")
  # these tags should get a default modifier if guess.getter=TRUE
  if(isTRUE(guess.getter)){
    JS.relevant.tags.state <- c("checkbox")
  } else {
    JS.relevant.tags.state <- c()
    JS.relevant.tags.default <- c(JS.relevant.tags.default, "checkbox")
  }
  # special tags: must be checkable and get "checked" property
  JS.relevant.tags.checked <- c("frame")


  # getting the relevant IDs out of optionsets is a little tricky
  # this function will probe for sets and return single tags
  single.tags <- check.optionset.tags(XML.obj=pXML, drop=c("comments","cdata", "declarations", "doctype"))

  # now go through the various cases of XML nodes, appending the results
  result <- check.JS.lines(relevant.tags=JS.relevant.tags.default, single.tags=single.tags,
    add.abbrev=add.abbrev, js=js, indent.by=indent.by, guess.getter=guess.getter)
  result <- check.JS.lines(relevant.tags=JS.relevant.tags.state, single.tags=single.tags,
    add.abbrev=add.abbrev, js=js, indent.by=indent.by, guess.getter=guess.getter,
    modifiers="state", append.modifier=FALSE, result=result)
  result <- check.JS.lines(relevant.tags=JS.relevant.tags.checked, single.tags=single.tags,
    add.abbrev=add.abbrev, js=js, indent.by=indent.by, guess.getter=guess.getter,
    modifiers="checked", only.checkable=TRUE, result=result)
  
  return(result)
}
