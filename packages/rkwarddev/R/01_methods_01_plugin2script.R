# Copyright 2015 Meik Michalke <meik.michalke@hhu.de>
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


#' Generate script code from XML objects
#' 
#' These methods try to translate plugin XML objects into \code{rkwarddev} function calls.
#' 
#' They are intended to make it easier to translate previously manually maintained plugin code
#' into \code{rkwarddev} scripts. The generated output should not be used as-is, but restructured
#' properly into a useful script.
#' 
#' You can either use a full XML document (read with \code{\link[XiMpLe:parseXMLTree]{parseXMLTree}})
#' or single (also nested) XiMpLe XML nodes.
#' 
#' @note The methods will probably fail, especially with highly complex plugins. Try to break these
#' into sensible chunks and try those speparately. Sometimes, slightly changing the input file
#' might also do the trick to get some usable results.
#'
#' @export
#' @docType methods
#' @return Either a character vector (if \code{obj} is a single XML node)
#'    or a list of character vectors named \code{"logic"}, \code{"dialog"} and \code{"wizard"}
#'    (if \code{obj} is a full XML document).
#' @rdname XiMpLe-methods
#' @examples
#' \dontrun{
#' # full XML document
#' myPlugin <- parseXMLTree("~/RKWardPlugins/myPlugin.xml")
#' rkwarddevScript <- plugin2script(myPlugin)
#' cat(rkwarddevScript)
#' }
#' 
#' # single XML node
#' (test.checkboxes <- rk.XML.row(
#'   rk.XML.col(
#'     list(
#'       rk.XML.cbox(label="foo", value="foo1", chk=TRUE),
#'       rk.XML.cbox(label="bar", value="bar2")
#'     )
#'   )
#' ))
#' rkwarddevScript <- plugin2script(test.checkboxes)
#' #see the generated script code
#' cat(rkwarddevScript)
#' 
#' # we can evaluate the generated code to check whether original
#' # XML and the newly generated one are identical
#' eval(parse(text=rkwarddevScript))
#' identical(rkdev.row.row_clmndc1212, test.checkboxes)
setGeneric("plugin2script", function(obj){standardGeneric("plugin2script")})

#' @export
#' @docType methods
#' @rdname XiMpLe-methods
#' @aliases plugin2script,XiMpLe.doc-method
#' @import XiMpLe
setMethod("plugin2script",
  signature(obj="XiMpLe.doc"),
  function(obj) {
    # search for logic, dialog and wizard sections
    secLogic <- XMLScan(obj, "logic")
    secDialog <- XMLScan(obj, "dialog")
    secWizard <- XMLScan(obj, "wizard")
    
    result <- list(
      logic=ifelse(is.null(secLogic), "", p2s(secLogic)),
      dialog=ifelse(is.null(secDialog), "", p2s(secDialog)),
      wizard=ifelse(is.null(secWizard), "", p2s(secWizard))
    )

    return(result)
  }
)

#' @export
#' @docType methods
#' @rdname XiMpLe-methods
#' @aliases plugin2script,XiMpLe.node-method
#' @import XiMpLe
setMethod("plugin2script",
  signature(obj="XiMpLe.node"),
  function(obj) {
    return(p2s(obj))
  }
)


## internal functions and objects

## function p2s.checkModifiers()
# takes an attribute value (character string) and checks whether it has a valid modifier suffix.
# returns a list with named elements:
#  - has.mod: logical value, TRUE if a modifier was found
#  - id: the actual ID value
#  - mod: the appended modifier (omitting ".not" if check.not=TRUE)
#  - not: logical value if ".not" was appended if check.not=TRUE
p2s.checkModifiers <- function(value, check.not=TRUE){
  result <- list(has.mod=FALSE, id="", mod="", not="FALSE")
  split.value <- unlist(strsplit(gsub("\"", "", value), "\\."))
  if(length(split.value) > 3){
    # hm, wouldn't know how this could happen, but let's just return it as-is
    result[["has.mod"]] <- TRUE
    result[["id"]] <- paste0("\"", split.value[1], "\"")
    result[["mod"]] <- paste0("\"", paste0(split.value[-1], collapse="."), "\"")
    warning(paste0("bogus modifier found: \"", value, "\"!"))
  } else {
    # check if the modifier is valid
    modif.validity("all", paste0(split.value[-1], collapse="."))
    if(length(split.value) == 3){
      result[["has.mod"]] <- TRUE
      result[["id"]] <- paste0("\"", split.value[1], "\"")
      if(identical(tolower(split.value[3]), "not")){
        result[["mod"]] <- paste0("\"", split.value[2], "\"")
        result[["not"]] <- "TRUE"
      } else {
        result[["mod"]] <- paste0(split.value[-1], collapse=".")
      }
    } else if(length(split.value) == 2){
      result[["has.mod"]] <- TRUE
      result[["id"]] <- paste0("\"", split.value[1], "\"")
      result[["mod"]] <- paste0("\"", split.value[2], "\"")
    } else {
      result[["id"]] <- paste0("\"", value, "\"")
    }
  }
  return(result)
} ## end function p2s.checkModifiers()


## function p2s.extractAttributes()
# translates node attributes into function options
# called by p2s()
p2s.extractAttributes <- function(nodeName, nodeAttrs, rkwdevAttributes, rkwdevLogical, rkwdevSplit){
  rkwdevOptions <- unlist(lapply(
    names(nodeAttrs),
    function(thisAttr){
      # consequently fail if the attribute is unknown
      if(!thisAttr %in% rkwdevAttributes){
        stop(simpleError(paste0("'", thisAttr, "' is an unknown attribute in node '", nodeName, "'!")))
      } else {}
      thisOption <- nodeAttrs[[thisAttr]]
      # check for logical/character transitions
      if(identical(nodeName, "spinbox") & identical(thisAttr, "type")){
        # special treatement of spinbox
        thisOption <- ifelse(identical(tolower(thisOption), "real"), TRUE, FALSE)
      } else if(thisAttr %in% rkwdevLogical){
        if(tolower(thisOption) %in% c("true", "t")){
          thisOption <- TRUE
        } else if(tolower(thisOption) %in% c("false", "f")){
          thisOption <- FALSE
        } else {
          warning("unknown logical value!")
        }
      } else if(thisAttr %in% rkwdevSplit){
        thisOption <- paste0("c(\"", paste0(unlist(strsplit(thisOption, "[[:space:]]")), collapse="\", \""), "\")")
      } else {
        thisOption <- paste0("\"", thisOption, "\"")
      }
      names(thisOption) <- names(rkwdevAttributes)[[which(rkwdevAttributes %in% thisAttr)]]
      return(thisOption)
    }
  ))

  # possible modifiers in the attributes?
  if(nodeName %in% "connect"){
    modGovernor <- p2s.checkModifiers(rkwdevOptions["governor"])
    modClient <- p2s.checkModifiers(rkwdevOptions["client"], check.not=FALSE)
    if(isTRUE(modGovernor[["has.mod"]])){
      rkwdevOptions["governor"] <- modGovernor[["id"]]
      rkwdevOptions["get"] <- modGovernor[["mod"]]
      rkwdevOptions["not"] <- modGovernor[["not"]]
    } else {}
    if(isTRUE(modClient[["has.mod"]])){
      rkwdevOptions["client"] <- modClient[["id"]]
      rkwdevOptions["set"] <- modClient[["mod"]]
    } else {}
  } else {}

  return(rkwdevOptions)
} ## end function p2s.extractAttributes()


## function p2s.checkOption
# takes a XiMpLe node and returns an <option> node as
# a character string, either showing a list() call.
# option nodes with an ID are automatically returned as
# rk.XML.option() calls istead of the usual list entries.
# called by p2s()
p2s.checkOption <- function(node, level=1, indent=TRUE){
  nodeName <- XMLName(node)
  nodeAttrs <- XMLAttrs(node)

  if(identical(nodeName, "option")){
    if("id" %in% names(nodeAttrs)){
      # generate rk.XML.option()
      thisEntry <- p2s(node=node, indent=indent, level=level)
    } else {
      # name the list entry after option label
      thisEntry <- paste0("\"", nodeAttrs[["label"]], "\"=c(")
      # set the value
      thisEntry <- paste0(thisEntry, "val=\"", nodeAttrs[["value"]], "\"")
      # check if option is checked
      if("checked" %in% names(nodeAttrs)){
        if(tolower(nodeAttrs[["checked"]]) %in% c("true", "t")){
          thisEntry <- paste0(thisEntry, ", chk=TRUE")
        } else if(tolower(nodeAttrs[["checked"]]) %in% c("false", "f")){
          thisEntry <- paste0(thisEntry, ", chk=FALSE")
        } else {
          warning("unknown logical value!")
        }
      } else {}
      thisEntry <- paste0(thisEntry, ")")
    }
  } else {
    warning(paste0("'", nodeName, "' is not an option! returning NULL!"))
    return(NULL)
  }
  
  return(thisEntry)
} ## end function p2s.checkOption()


## end function p2s.checkTabs
# analyses <tab> nodes and returns them for inclusion by p2s()
p2s.checkTabs <- function(node, level=0, indent=TRUE){
  nodeName <- XMLName(node)
  nodeAttrs <- XMLAttrs(node)

  if(identical(nodeName, "tab")){
    # name the list entry after tab label
    thisEntry <- paste0(
      "\"", nodeAttrs[["label"]], "\"=list(\n", paste0(rep("  ", level+1), collapse=""),
      paste0(
        sapply(
          XMLChildren(node),
          function(thisChild){
            p2s(node=thisChild, level=level+2, indent=indent)
          }
        ),
        collapse=paste0(",\n", rep("  ", level+1), collapse=""))
    )
    thisEntry <- paste0(thisEntry, "\n", paste0(rep("  ", level), collapse=""), ")")
  } else {
    warning(paste0("'", nodeName, "' is not a tab! returning NULL!"))
    thisEntry <- NULL
  }
  
  return(thisEntry)
} ## end function p2s.checkTabs


## function p2s.checkTabIDs()
# rk.XML.tabbook() expects IDs of tabbook and all tabs in one vector, check for existing IDs
p2s.checkTabIDs <- function(node){
  nodeName <- XMLName(node)
  nodeAttrs <- XMLAttrs(node)

  if(identical(nodeName, "tab")){
    return(id(node, js=FALSE))
  } else {
    warning(paste0("'", nodeName, "' is not a tab! returning NULL!"))
    thisID <- NULL
  }
  
  return(thisEntry)
} ## end function p2s.checkTabIDs()


## function p2s()
# this is the main work horse, going through nested XML nodes recursively
# called by the actual methods
p2s <- function(node, indent=TRUE, level=1, prefix="rkdev", drop.defaults=TRUE){
  nodeName <- XMLName(node)
  nodeAttrs <- XMLAttrs(node)
  # fail if we don't know this node type
  if(!nodeName %in% names(FONA)){
    stop(simpleError(paste0("'", nodeName, "' is an unknown node!")))
  } else {}
  rkwdevFunction <- FONA[[nodeName]][["funct"]]
  if("opt" %in% names(FONA[[nodeName]])){
    rkwdevAttributes <- FONA[[nodeName]][["opt"]]
  } else {
    rkwdevAttributes <- c()
  }
  if("logical" %in% names(FONA[[nodeName]])){
    rkwdevLogical <- FONA[[nodeName]][["logical"]]
  } else {
    rkwdevLogical <- c()
  }
  if("split" %in% names(FONA[[nodeName]])){
    rkwdevSplit <- FONA[[nodeName]][["split"]]
  } else {
    rkwdevSplit <- c()
  }
  if("children" %in% names(FONA[[nodeName]])){
    rkwdevChildren <- FONA[[nodeName]][["children"]]
    recursive <- TRUE
  } else {
    recursive <- FALSE
  }
  if("modifiers" %in% names(FONA[[nodeName]])){
    checkModifiers <- TRUE
  } else {
    checkModifiers <- FALSE
  }
  if("text" %in% names(FONA[[nodeName]])){
    rkwdevText <- FONA[[nodeName]][["text"]]
    checkText <- TRUE
  } else {
    checkText <- FALSE
  }

  rkwdevOptions <- p2s.extractAttributes(
    nodeName=nodeName,
    nodeAttrs=nodeAttrs,
    rkwdevAttributes=rkwdevAttributes,
    rkwdevLogical=rkwdevLogical,
    rkwdevSplit=rkwdevSplit
  )

  # need to include text?
  if(isTRUE(checkText)){
    nodeChildren <- XMLValue(XMLChildren(node)[[1]])
    if(inherits(nodeChildren, "character")){
      rkwdevOptions[[rkwdevText]] <- paste0("\"", nodeChildren, "\"", collapse=" ")
    } else {}
  } else {}

  # get the child nodes
  if(isTRUE(recursive)){
    nodeChildren <- XMLChildren(node)
    if(length(nodeChildren) > 0){
      # some nodes need special treatment, because they take option children as named lists
      if(nodeName %in% c("dropdown", "radio", "select", "valueselector")){
        rkwdevChildnodes <- sapply(nodeChildren,
            function(thisChild){
              return(p2s.checkOption(node=thisChild, level=level+2, indent=indent))
            }
          )
        rkwdevOptions[[rkwdevChildren]] <- paste0("list(\n", paste0(rep("  ", level+1), collapse=""),
            paste0(rkwdevChildnodes, paste0(rep("  ", level-1), collapse=""),
              collapse=paste0(",\n", paste0(rep("  ", level+1), collapse=""))), 
          "\n", paste0(rep("  ", level), collapse=""), ")")
      } else if(nodeName %in% c("tabbook")){
        rkwdevChildnodes <- sapply(nodeChildren,
            function(thisChild){
              return(p2s.checkTabs(node=thisChild, level=level+1, indent=indent))
            }
          )
        rkwdevOptions[[rkwdevChildren]] <- paste0("list(\n", paste0(rep("  ", level+1), collapse=""),
            paste0(rkwdevChildnodes, paste0(rep("  ", level-1), collapse=""),
              collapse=paste0(",\n", paste0(rep("  ", level+1), collapse=""))), 
          "\n", paste0(rep("  ", level), collapse=""), ")")
        if("id.name" %in% names(rkwdevOptions)){
          rkwdevTabIDs <- sapply(nodeChildren,
              function(thisChild){
                return(p2s.checkTabIDs(thisChild))
              }
            )
          rkwdevOptions[["id.name"]] <- paste0("c(", rkwdevOptions[["id.name"]], ", \"", paste0(rkwdevTabIDs, collapse="\", \""), "\")")
        } else {}
      } else {
        rkwdevChildnodes <- sapply(nodeChildren,
            function(thisChild){
              return(p2s(node=thisChild, indent=indent, level=level+1))
            }
          )
        rkwdevOptions[[rkwdevChildren]] <- paste0(rkwdevChildnodes,
          collapse=paste0(",\n", paste0(rep("  ", level), collapse="")))
      }
    } else {}
  } else {}

  # check for default values and drop them
  if(isTRUE(drop.defaults)){
    defaults <- formals(rkwdevFunction)
    # bring formals into same format as rkwdevOptions
    defaults[sapply(defaults, is.character)] <- paste0("\"", defaults[sapply(defaults, is.character)], "\"")
    defaults <- sapply(defaults, as.character)
    for (thisOption in names(rkwdevOptions)){
      if(identical(rkwdevOptions[[thisOption]], defaults[[thisOption]])){
        rkwdevOptions <- rkwdevOptions[!names(rkwdevOptions) %in% thisOption]
      }
    }
  } else {}

  # bring options in optimized order
  rkwdevOptions <- rkwdevOptions[order(sapply(names(rkwdevOptions), function(thisOpt){which(names(rkwdevAttributes) %in% thisOpt)}))]

  if(isTRUE(indent)){
    ind.start <- ind.char <- paste0("\n", paste0(rep("  ", level), collapse=""))
    ind.end <- paste0("\n", paste0(rep("  ", level-1), collapse=""))
  } else {
    ind.char <- " "
    ind.start <- ind.end <- ""
  }
  # write into an object, but only if the node has an ID value,
  # because otherwise we can assume that it is not referenced anywhere else
  nodeID <- id(node, js=FALSE)
  if(!identical(nodeID, "NULL")){
    rkObject <- paste0(prefix, ".", nodeName, ".", id(node, js=FALSE), " <- ")
  } else {
    rkObject <- ""
  }
  
  if(length(rkwdevOptions) > 0){
    result <- paste0(
      rkObject, rkwdevFunction, "(",
        ind.start, paste0(names(rkwdevOptions), "=", rkwdevOptions, collapse=paste0(",", ind.char)),
        ind.end,
      ")"
    )
    # remove "...=", if present
    result <- gsub("\\.\\.\\.=", "", result)
  } else {
    result <- paste0(rkwdevFunction, "()")
  }

  return(result)
} ## end function p2s()


## list object FONA
# this list describes which rkwarddev function belongs to which XML node,
# and which option handles which attribute
# the structure is:
#  "<node name>"=list(
#     funct="<function name>",
#     opt=c(
#      <option1 name>="<attribute1 name>",
#      <option2 name>="<attribute2 name>",
#       ...)
#     ),
#     text="<does this node nest text?>",
#     children="<option name for nested child nodes that need to be checked recursively?>",
#     split=c(
#       "<attribute names that need to be split into a character vector>"
#     ),
#     logical=c(
#       "<attribute name that needs translation from character to logical>"
#     ),
#     modifiers=c(
#       "<attribute name that could contain a modifier>"
#     )

# the name stands for function/option/node/attribute
# 
# tests needed:
# - rk.rkh.related()
# 
# unsolved functions (or parts of them):
# - rk.rkh.doc()
# - rk.XML.about()
# - children (dependencies) in rk.XML.component()
# - standards/min/max/mode etc. in rk.XML.convert()
# - children in rk.XML.dependencies()
# - children in rk.XML.dependency_check()
# - modifier in rk.XML.optioncolumn()
# - a lot of rk.XML.optionset()...
# - children in rk.XML.plugin()
# - children in rk.XML.pluginmap()
# - set in rk.XML.set()
# - cases & modifier in rk.XML.switch()
# - rk.XML.values()
# - rk.XML.vars()
FONA <- list(
  "caption"=list(
    funct="rk.rkh.caption",
    opt=c(
      id="id",
      title="title"
    )
  ),
  "label"=list(
    funct="rk.rkh.label",
    opt=c(
      id="id"
    )
  ),
  "link"=list(
    funct="rk.rkh.link",
    opt=c(
      href="href",
      text="text"
    ),
    text="text"
  ),
  "related"=list(
    funct="rk.rkh.related",
    opt=c(
      "..."="...",
      text="text"
    ),
    text="text",
    children="..."
  ),
  "section"=list(
    funct="rk.rkh.section",
    opt=c(
      title="title",
      text="text",
      short="short_title",
      id.name="id"
    ),
    text="text"
  ),
  "setting"=list(
    funct="rk.rkh.setting",
    opt=c(
      id="id",
      text="text",
      title="title"
    ),
    text="text"
  ),
  "settings"=list(
    funct="rk.rkh.settings",
    opt=c(
      "..."="..."
    ),
    children="..."
  ),
  "summary"=list(
    funct="rk.rkh.summary",
    text="text"
  ),
  "technical"=list(
    funct="rk.rkh.technical",
    opt=c(
      text="text"
    ),
    text="text"
  ),
  "title"=list(
    funct="rk.rkh.title",
    opt=c(
      text="text"
    ),
    text="text"
  ),
  "usage"=list(
    funct="rk.rkh.usage",
    opt=c(
      text="text"
    ),
    text="text"
  ),
  "attribute"=list(
    funct="rk.XML.attribute",
    opt=c(
      id="id",
      value="value",
      label="label"
    )
  ),
  "browser"=list(
    funct="rk.XML.browser",
    opt=c(
      label="label",
      type="type",
      initial="initial",
      urls="allow_urls",
      filter="filter",
      required="required",
      id.name="id"
    ),
    split="filter",
    logical=c("required", "allow_urls")
  ),
  "checkbox"=list(
    funct="rk.XML.cbox",
    opt=c(
      label="label",
      value="value",
      un.value="value_unchecked",
      chk="checked",
      id.name="id"
    ),
    logical=c("checked")
  ),
  "code"=list(
    funct="rk.XML.code",
    opt=c(
      file="file"
    )
  ),
  "column"=list(
    funct="rk.XML.col",
    opt=c(
      "..."="...",
      id.name="id"
    ),
    children="..."
  ),
  "component"=list(
    funct="rk.XML.component",
    opt=c(
      label="label",
      file="file",
      id.name="id",
      type="type"
    ),
    children="..."
#      dependencies="dependencies"
  ),
  "components"=list(
    funct="rk.XML.components",
    opt=c(
      "..."="..."
    ),
    children="..."
  ),
  "connect"=list(
    funct="rk.XML.connect",
    opt=c(
      governor="governor",
      client="client",
      get="get",     # these three
      set="set",     # are needed
      not="not",     # for ordering purposes
      reconcile="reconcile"
    ),
    logical=c("reconcile"),
    modifiers=c("governor", "client")
  ),
  "context"=list(
    funct="rk.XML.context",
    opt=c(
      "..."="...",
      id="id"
    ),
    children="..."
  ),
  "convert"=list(
    funct="rk.XML.convert",
    opt=c(
      sources="sources",
      mode="mode",
      required="required",
      id.name="id",
      standard="standard"
    ),
    logical=c("required")
  ),
  "copy"=list(
    funct="rk.XML.copy",
    opt=c(
      id="id",
      as="as"
    )
  ),
  "dependencies"=list(
    funct="rk.XML.dependencies",
    children="..."
  ),
  "dependency_check"=list(
    funct="rk.XML.dependency_check",
    opt=c(
      id.name="id"
    ),
    children="..."
  ),
  "dialog"=list(
    funct="rk.XML.dialog",
    opt=c(
      "..."="...",
      label="label",
      recommended="recommended"
    ),
    children="...",
    logical=c("recommended")
  ),
  "dropdown"=list(
    funct="rk.XML.dropdown",
    opt=c(
      label="label",
      options="options",
      id.name="id"
    ),
   children="options"
  ),
  "embed"=list(
    funct="rk.XML.embed",
    opt=c(
      component="component",
      button="as_button",
      label="label",
      id.name="id"
    ),
    logical=c("as_button")
  ),
  "entry"=list(
    funct="rk.XML.",
    opt=c(
      component="component",
      index="index"
    )
  ),
  "external"=list(
    funct="rk.XML.external",
    opt=c(
      id="id",
      default="default"
    )
  ),
  "formula"=list(
    funct="rk.XML.formula",
    opt=c(
      fixed="fixed_factors",
      dependent="dependent",
      id.name="id"
    )
  ),
  "frame"=list(
    funct="rk.XML.frame",
    opt=c(
      "..."="...",
      label="label",
      checkable="checkable",
      chk="checked",
      id.name="id"
    ),
    children="...",
    logical=c("checkable", "checked")
  ),
  "help"=list(
    funct="rk.XML.help",
    opt=c(
      file="file"
    )
  ),
  "hierarchy"=list(
    funct="rk.XML.hierarchy",
    opt=c(
      "..."="..."
    ),
    children="..."
  ),
  "i18n"=list(
    funct="rk.XML.i18n",
    opt=c(
      label="label",
      id.name="id"
    )
  ),
  "include"=list(
    funct="rk.XML.include",
    opt=c(
      file="file"
    )
  ),
  "input"=list(
    funct="rk.XML.input",
    opt=c(
      label="label",
      initial="initial",
      size="size",
      required="required",
      id.name="id"
    ),
    logical=c("required")
  ),
  "insert"=list(
    funct="rk.XML.insert",
    opt=c(
      snippet="snippet"
    )
  ),
  "logic"=list(
    funct="rk.XML.logic",
    opt=c(
      "..."="..."
    ),
    children="..."
  ),
  "matrix"=list(
    funct="rk.XML.matrix",
    opt=c(
      label="label",
      mode="mode",
      rows="rows",
      columns="columns",
      min="min",
      max="max",
      min_rows="",
      min_columns="",
      allow_missings="allow_missings",
      allow_user_resize_columns="allow_user_resize_columns",
      allow_user_resize_rows="allow_user_resize_rows",
      fixed_width="fixed_width",
      fixed_height="fixed_height",
      horiz_headers="horiz_headers",
      vert_headers="vert_headers",
      id.name="id"
    ),
    logical=c("allow_missings", "allow_user_resize_columns", "allow_user_resize_rows", "fixed_width", "fixed_height")
  ),
  "menu"=list(
    funct="rk.XML.menu",
    opt=c(
      label="label",
      "..."="...",
      index="index",
      id.name="id"
    ),
    children="..."
  ),
  "option"=list(
    funct="rk.XML.option",
    opt=c(
      label="label",
      val="value",
      chk="checked",
      id.name="id"
    ),
    logical=c("checked")
  ),
  "optioncolumn"=list(
    funct="rk.XML.optioncolumn",
    opt=c(
      connect="connect",
#       modifier=NULL,
      label="label",
      external="external",
      default="default",
      id.name="id"
    ),
    logical=c("external")
  ),
  "optiondisplay"=list(
    funct="rk.XML.optiondisplay",
    opt=c(
      index="index",
      id.name="id"
    ),
    logical=c("index")
  ),
  "optionset"=list(
    funct="rk.XML.optionset",
    opt=c(
#      content,
#      optioncolumn,
      min_rows="min_rows",
      min_rows_if_any="min_rows_if_any",
      max_rows="max_rows",
      keycolumn="keycolumn",
#      logic=NULL,
#      optiondisplay=TRUE,
      id.name="id"
    ),
    children="..."
  ),
  "page"=list(
    funct="rk.XML.page",
    opt=c(
      "..."="...",
      id.name="id"
    ),
    children="..."
  ),
  "plugin"=list(
    funct="rk.XML.plugin",
    children="..."
  ),
  "pluginmap"=list(
    funct="rk.XML.pluginmap",
    children="..."
  ),
  "preview"=list(
    funct="rk.XML.preview",
    opt=c(
      label="label"
    )
  ),
  "radio"=list(
    funct="rk.XML.radio",
    opt=c(
      label="label",
      options="options",
      id.name="id"
    ),
    children="options"
  ),
  "require"=list(
    funct="rk.XML.require",
    opt=c(
      file="file",
      map="map"
    )
  ),
  "row"=list(
    funct="rk.XML.row",
    opt=c(
      "..."="...",
      id.name="id"
    ),
    children="..."
  ),
  "saveobject"=list(
    funct="rk.XML.saveobj",
    opt=c(
      label="label",
      chk="checked",
      checkable="checkable",
      initial="initial",
      required="required",
      id.name="id"
    ),
    logical=c("checked", "checkable", "required")
  ),
  "select"=list(
    funct="rk.XML.select",
    opt=c(
      label="label",
      options="options",
      id.name="id"
    ),
    children="options"
  ),
  "set"=list(
    funct="rk.XML.set",
    opt=c(
      id="id",
#       set="set",
      to="to"
    )
  ),
  "snippet"=list(
    funct="rk.XML.snippet",
    opt=c(
      "..."="...",
      id.name="id"
    ),
    children="..."
  ),
  "snippets"=list(
    funct="rk.XML.snippets",
    opt=c(
      "..."="..."
    ),
    children="..."
  ),
  "spinbox"=list(
    funct="rk.XML.spinbox",
    opt=c(
      label="label",
      min="min",
      max="max",
      initial="initial",
      real="type",              ## need special treatment
      precision="precision",
      max.precision="max_precision", 
      id.name="id"
    )
  ),
  "stretch"=list(
    funct="rk.XML.stretch"
  ),
  "switch"=list(
    funct="rk.XML.switch",
    opt=c(
      condition="condition",
#      cases, 
#      modifier=NULL,
      id.name="id"
    ),
    children="..."
  ),
  "tabbook"=list(
    funct="rk.XML.tabbook",
    opt=c(
      label="label",
      tabs="tabs",
      id.name="id"
    ),
    children="tabs"
  ),
  "text"=list(
    funct="rk.XML.text",
    opt=c(
      text="text",
      type="type",
      id.name="id"
    ),
    text="text"
  ),
  "values"=list(
    funct="rk.XML.values",
    opt=c(
      label="label",
## values <- function(label, slot.text, options=list(label=c(val=NULL, chk=FALSE, i18n=NULL)),
##     required=FALSE, multi=FALSE, duplicates=FALSE, min=1, any=1, max=0,
##     horiz=TRUE, add.nodes=NULL, frame.label=NULL
      id.name="id"
    ),
#     children="...",
    logical=c()
  ),
  "valueselector"=list(
    funct="rk.XML.valueselector",
    opt=c(
      label="label",
      options="options",
      id.name="id"
  ),
    children="options",
    logical=c()
  ),
  "valueslot"=list(
    funct="rk.XML.valueslot",
    opt=c(
      label="label",
      source="source",
      property="source_property",
      required="required",
      multi="multi",
      duplicates="allow_duplicates",
      min="min_vars",
      any="min_vars_if_any",
      max="max_vars",
      id.name="id"
    ),
    logical=c("required", "multi", "allow_duplicates")
  ),
  "vars"=list(
    funct="rk.XML.vars",
    opt=c(
      label="label",
# rk.XML.vars <- function(label, slot.text, required=FALSE, multi=FALSE, duplicates=FALSE, min=1, any=1, max=0,
#     dim=0, min.len=0, max.len=NULL, classes=NULL, types=NULL, horiz=TRUE, add.nodes=NULL,
#     frame.label=NULL, formula.dependent=NULL, dep.options=list(),
      id.name="id"
    ),
#     children="...",
    logical=c()
  ),
  "varselector"=list(
    funct="rk.XML.varselector",
    opt=c(
      label="label",
      id.name="id"
    )
  ),
  "varslot"=list(
    funct="rk.XML.varslot",
    opt=c(
      label="label",
      source="source",
      property="source_property",
      required="required",
      multi="multi",
      duplicates="allow_duplicates",
      min="min_vars",
      any="min_vars_if_any",
      max="max_vars",
      dim="num_dimensions",
      min.len="min_length",
      max.len="max_length",
      classes="classes",
      types="types",
      id.name="id"
    ),
    logical=c("required", "multi", "allow_duplicates"),
    split=c("classes", "types")
  ),
  "wizard"=list(
    funct="rk.XML.wizard",
    opt=c(
      "..."="...",
      label="label",
      recommended="recommended"
    ),
    children="...",
    logical=c("recommended")
  )
) ## end list object FONA
