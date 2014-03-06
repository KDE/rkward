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


#' Create XML node "switch" for RKWard plugins
#'
#' This node can only be used in \code{<logic>} sections. If the provided property
#' is logical, in the \code{cases} list you must also provide lists called \code{true}
#' and \code{false}. If not, you must provide at least one list called \code{case}.
#' 
#' The values to be returned can be either \code{fixed_value} or \code{dynamic_value}.
#' A \code{fixed_value} must be a character string which will be returned if the condition
#' is met. Whereas a \code{dynamic_value} is the \code{id} of another property, an can
#' be provided as either a character string or an object of class \code{XiMpLe.node}.
#'
#' @note The \code{<switch>} node was introduced with RKWard 0.6.1, please set the dependencies
#'    of your component/plugin accordingly.
#'
#' @param condition Either a character string (the \code{id} of the property whose
#'    state should be queried), or an object of class \code{XiMpLe.node}
#'    (whose \code{id} will be extracted and used).
#' @param cases A named list of named lists. The lists contained must either be called
#'    \code{true} and \code{false}, setting the return values if \code{condition} is logical, or
#'    \code{case} and optionally \code{default}. You can provide as many \code{case} lists
#'    as you need, setting a return value for each \code{condition == case} respectively.
#'    Each list must contain either a \code{fixed_value} or a \code{dynamic_value} element.
#'    In addition, each \code{case} list must also have one \code{standard} element.
#' @param modifier Character string, an optional modifier to be appended to \code{condition}.
#' @param id.name Character string, a unique ID for this property.
#'    If \code{"auto"}, IDs will be generated automatically from the condition ID.
#' @return An object of class \code{XiMpLe.node}.
#' @export
#' @seealso
#'    \code{\link[rkwarddev:rk.XML.connect]{rk.XML.connect}},
#'    \code{\link[rkwarddev:rk.XML.convert]{rk.XML.convert}},
#'    \code{\link[rkwarddev:rk.XML.external]{rk.XML.external}},
#'    \code{\link[rkwarddev:rk.XML.logic]{rk.XML.logic}},
#'    \code{\link[rkwarddev:rk.XML.set]{rk.XML.set}},
#'    and the \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' # example for a boolean switch
#' myCheckbox <- rk.XML.cbox("foo")
#' rk.XML.switch(myCheckbox, cases=list(
#'   true=list(fixed_value="foo"),
#'   false=list(fixed_value="bar"))
#' )
#' 
#' # example for a case switch
#' MyRadio <- rk.XML.radio("Chose one",
#'   options=list(
#'     "First Option"=c(val="val1"),
#'     "Second Option"=c(val="val2", chk=TRUE))
#' )
#' rk.XML.switch(MyRadio, modifier="string", cases=list(
#'   case=list(standard="val1", fixed_value="foo"),
#'   case=list(standard="val2", fixed_value="bar"))
#' )

rk.XML.switch <- function(condition, cases, modifier=NULL, id.name="auto"){

  condition.id <- check.ID(condition)
  if(is.XiMpLe.node(condition) && !is.null(modifier)){
    # validate modifier
    if(modif.validity(condition, modifier=modifier)){
      condition.id <- paste(condition.id, modifier, sep=".")
    } else {}
  } else {}

  if(identical(id.name, "auto")){
    attr.list <- list(condition=condition.id, id=auto.ids(condition.id, prefix=ID.prefix("switch")))
  } else if(!is.null(id.name)){
    attr.list <- list(condition=condition.id, id=id.name)
  } else {
    stop(simpleError("'id.name' must have a value!"))
  }

  check.required.attrs <- function(obj, req=c("fixed_value", "dynamic_value")){
      # some sanity check here
      if(!any(req %in% names(obj))){
        stop(simpleError("Check your attributes!"))
      } else {}
    }

  # does also check for the required default attributes
  check.dyn.value <- function(condCase){
      check.required.attrs(obj=condCase)
      if("dynamic_value" %in% names(condCase)){
        condCase[["dynamic_value"]] <- check.ID(condCase[["dynamic_value"]])
      } else {}
      return(condCase)
    }

  child.nodes <- list()
  case.names <- names(cases)
  # check for a sane set of cases
  if(all(c("true","false") %in% case.names) && length(case.names) == 2){
    true <- check.dyn.value(cases[["true"]])
    false <- check.dyn.value(cases[["false"]])
    child.nodes <- append(child.nodes, XMLNode("true", attrs=true))
    child.nodes <- append(child.nodes, XMLNode("false", attrs=false))
  } else if(all(!c("true","false") %in% case.names) && "case" %in% case.names){
    for (thisNode in cases[case.names %in% "case"]){
      check.required.attrs(thisNode, req="standard")
      thisNode <- check.dyn.value(thisNode)
      child.nodes <- append(child.nodes, XMLNode("case", attrs=thisNode))
    }
    if(!is.null(cases[["default"]])){
      default <- check.dyn.value(cases[["default"]])
      child.nodes <- append(child.nodes, XMLNode("default", attrs=default))
    } else {}
  } else {
    stop(simpleError("Please either provide both 'true' and 'false', or 'case'!"))
  }

  node <- XMLNode("switch",
    attrs=attr.list,
    .children=child.nodes)

  return(node)

}
