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


#' Keep operators when replace XiMpLe.nodes with ID value
#' 
#' This function is a wrapper for \code{\link[rkwarddev:id]{id}} similar to \code{\link[rkwarddev:qp]{qp}}
#' that uses \code{eval(substitute(alist(...)))} to preserve the value of \code{...} as-is to be able to
#' keep operators like \code{">="} or \code{"!="} unevaluated in the resulting output. This can be used to
#' in "if" clauses, e.g. with \code{\link[rkwarddev:ite]{ite}}, so you don't have to quote half of it.
#' 
#' Normally, \code{id} would simply evaluate the condition and then return the result of that evaluation, which
#' most of the time is not what you want. With this function, you can test conditions in usual R syntax, yet
#' the operators will end up pasted in the result.
#' 
#' The abbreviation stands for "JavaScript operators".
#' 
#' The following operators are supported: +, -, *, /, ==, !=, >, <, >=, <=, || and &&
#' 
#' These are currently unsupported and still need to be quoted: \%, ++, --, =, +=, -=, *=, /=, \%=, ===, !== and !
#'
#' @param ... One or several character strings and/or \code{XiMpLe.node} objects with plugin nodes,
#'   and/or objects of classes \code{rk.JS.arr} or \code{rk.JS.opt}, simply separated by comma.
#'   JavaScript operators will be kept as-is.
#' @return A character string.
#' @export
#' @seealso \code{\link[rkwarddev:rk.JS.vars]{rk.JS.vars}},
#'    \code{\link[rkwarddev:rk.JS.array]{rk.JS.array}},
#'    \code{\link[rkwarddev:rk.JS.options]{rk.JS.options}},
#'    \code{\link[rkwarddev:echo]{echo}},
#'    \code{\link[rkwarddev:id]{id}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # an example checkbox XML node
#' cbox1 <- rk.XML.cbox(label="foo", value="foo1", id.name="CheckboxFoo.ID")
#' ite(jo(cbox1 == "foo1"), echo("gotcha!"))

jo <- function(...){
  full.content <- eval(substitute(alist(...)))
  ID.content <- sapply(
    full.content,
    function(this.part){
      # get the object, not just a name from eval(substitute(alist(...)))
      if (is.name(this.part)){
        this.part <- eval(this.part)
      } else {}
      if(is.call(this.part)){
        # replace JS operators
        return(do.call("replaceJSOperators", args=list(this.part)))
      } else {
        return(this.part)
      }
    }
  )
  return(id(js=TRUE, .objects=ID.content))
}
