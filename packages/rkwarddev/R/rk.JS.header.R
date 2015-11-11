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
#'    character or a XiMpLe node from the dialog); if the second value is named \code{noquote} or \code{nq},
#'    the JS output will be nested inside \code{noquote()}. Entries named \code{addFromUI} must have exactly one value
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
#' 
#' # let's assume we create an R object called "results"
#' # in the plugin dialog, this is how you could fetch
#' # portions of it to be added as a parameter in the output
#' rk.JS.header(
#'   "Test results",
#'   add=c("Significance level", noquote="results[[\\\"alpha\\\"]]")
#' )

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
          if(length(content) != 1){
            stop(simpleError("rk.JS.header: \"addFromUI\" must have exactly one value. please use several elements of the same name if needed!"))
          } else {}
          content <- paste0("\"", id(content, js=FALSE), "\"")
        } else if(identical(functionName, "add")){
          stopifnot(length(content) == 2)
          if(length(content) != 2){
            stop(simpleError("rk.JS.header: \"add\" must have exactly two values. please use several elements of the same name if needed!"))
          } else {}
          if(is.character(content[[2]])){
            if(names(content)[[2]] %in% c("nq","noquote")){
              value <- paste0("noquote(\"", content[[2]], "\")")
            } else {
              value <- paste0("\"", content[[2]], "\"")
            }
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

  result <- paste0(
    "new Header(",
    force.i18n(title),
    if(!is.null(level)){
      paste0(", ", level)
    },
    ")",
    addToHeaderChar,
    ".print();"
  )

  return(result)
}
