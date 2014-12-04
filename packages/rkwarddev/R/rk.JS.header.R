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

#' Generate JavaScript header object
#'
#' @param title Either a character string or object of class \code{rk.JS.i18n}. Will become the header title,
#'    nested in an i18n() call.
#' @param ... An optional number of additional info to add to the header. Each entry must be named \code{add}
#'    or \code{addFromUI} -- note that you can use multiple entries with the same name here. Entries named
#'    \code{add} must be vectors of legth 2, the first being the caption (character), the second its value (either
#'    character or a XiMpLe node from the dialog). Entries named \code{addFromUI} must have exactly one value
#'    specifying the GUI element to query (either character or a XiMpLe node from the dialog).
#' @param level Integer, if not \code{NULL} will be added as the header level.
#' @param guess.getter Locigal, if \code{TRUE} try to get a good default getter function for JavaScript
#'    variable values.
#' @param .add Same as \code{...}, but provided as a single list. If used, values will be appended to \code{...}.
#' @return A character string.
#' @export
#' @examples
#' my.cbox <- rk.XML.cbox("This is a test")
#' rk.JS.header("Test results", addFromUI=my.cbox)

rk.JS.header <- function(title, ..., level=NULL, guess.getter=FALSE, .add=list()){
  addToHeaderChar <- addLevel <- NULL
  addToHeader <- list(...)
  if(is.list(.add) & length(.add) > 0){
    addToHeader <- append(addToHeader, .add)
  } else {}
  headerNames <- names(addToHeader)
  
  if(!all(headerNames %in% c("add", "addFromUI"))){
    stop(simpleError("rk.JS.header: currently only \"add\" and \"addFromUI\" are supported!"))
  } else {}
  
  if(length(addToHeader) > 0){
    addToHeaderChar <- paste(sapply(1:length(addToHeader), function(this.add){
        functionName <- headerNames[[this.add]]
        content <- addToHeader[[this.add]]
        if(identical(functionName, "addFromUI")){
          stopifnot(length(content) == 1)
          content <- paste0("\"", id(content, js=FALSE), "\"")
        } else if(identical(functionName, "add")){
          stopifnot(length(content) == 2)
          if(is.character(content[[2]])){
            value <- paste0("\"", content[[2]], "\"")
          } else if(is.XiMpLe.node(content[[2]])){
            JS.var.value <- rk.JS.vars(content[[2]], guess.getter=guess.getter)
            value <- paste0(slot(JS.var.value, "getter"), "(\"", id(content[[2]], js=FALSE), "\")")
          } else {
            stop(simpleError("rk.JS.header: you can only provide character values or XiMpLe nodes!"))
          }
          content <- paste0(force.i18n(content[[1]]), ", ", value)
        } else {}
        paste0(functionName, "(", content, ")")
      }),
      sep="", collapse="."
    )
    addToHeaderChar <- paste0(".", addToHeaderChar)
  } else {}

  if(!is.null(level)){
    addToHeaderChar <- paste0(addToHeaderChar, ".level(", level, ")")
  } else {} 

  result <- paste0(
    "new Header(",
    force.i18n(title),
    ")",
    addToHeaderChar,
    ".print();\n"
  )

  return(result)
}
