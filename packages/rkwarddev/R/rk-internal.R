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

# collate voodoo
#' @include rk.comment.R

# set up an internal environment, e.g. for language settings
.rkdev.env <- new.env()

# internal functions for the rk.* functions

## wrapper for paste0() needed?
if(isTRUE(R_system_version(getRversion()) < 2.15)){
  # if this is an older R version, we need a wrapper function for paste0()
  # which was introduced with R 2.15 as a more efficient shortcut to paste(..., sep="")
  paste0 <- function(..., collapse=NULL){
    return(paste(..., sep="", collapse=collapse))
  }
} else {}

# info message
generator.info <- rk.comment(paste0("this code was generated using the rkwarddev package.\n",
      "perhaps don't make changes here, but in the rkwarddev script instead!"))

## function auto.ids()
auto.ids <- function(identifiers, prefix=NULL, suffix=NULL, chars=8){
  identifiers <- gsub("[[:space:]]*[^[0-9A-Za-z]]*", "", identifiers)
  id.names <- ifelse(nchar(identifiers) > 8, abbreviate(identifiers, minlength=chars), identifiers)
  # check for uniqueness
  if(any(duplicated(id.names))){
    warning("IDs are not unique, please check!")
  } else {}
  ids <- paste0(prefix, id.names, suffix)
  return(ids)
} ## end function auto.ids()


## function stripCont()
# get slots out of certain container objects
stripCont <- function(obj, get="printout"){
  if(inherits(obj, "rk.plot.opts")){
    # if this is a plot options object, extract the XML slot
    # and discard the rest
    obj <- slot(obj, get)
  } else {}
  return(obj)
}
## end function stripCont()


## function stripXML()
# get XML node out of container objects
stripXML <- function(obj){
  return(stripCont(obj, get="XML"))
}
## end function stripXML()


## function child.list()
# convenience function to let single children be provided without list()
# 'empty' can be used to make sure a tag is non-empty without actual value
# this function also reduces rk.plot.opts objects to their XiMpLe.node slot
child.list <- function(children, empty=TRUE){
  if(is.XiMpLe.node(children)){
    children <- list(children)
  } else {
    # if already a list, check if it's a list in a list and get it out
    if(inherits(children, "list") & length(children) == 1){
      if(inherits(children[[1]], "list")){
        children <- children[[1]]
      } else {}
    } else if(identical(children, list()) & !isTRUE(empty)){
      children <- list("")
    } else {}
    children <- lapply(children, function(this.child){
        stripXML(this.child)
      })
  }
  return(children)
} ## end function child.list()


## function trim()
# cuts off space at start and end of a character string
trim <- function(char){
  char <- gsub("^[[:space:]]*", "", char)
  char <- gsub("[[:space:]]*$", "", char)
  return(char)
} ## end function trim()


## function indent()
# will create tabs to format the output
indent <- function(level, by="\t"){
  paste(rep(by, level-1), collapse="")
} ## end function indent()


## function checkCreateFiles()
# used by rk.plugin.skeleton()
checkCreateFiles <- function(file.name, ow, action=NULL){
  if(all(file.exists(file.name), as.logical(ow)) | !file.exists(file.name)){
    return(TRUE)
  } else {
    if(!is.null(action)){
      action <- paste0(action, ": ")
    } else {}
    warning(paste0(action, "Skipping existing file ", file.name, "."), call.=FALSE)
    return(FALSE)
  }
} ## end function checkCreateFiles()


## function get.single.tags()
get.single.tags <- function(XML.obj, drop=NULL){
  # determine if we need to read a file or process an XiMpLe object
  if(is.XiMpLe.doc(XML.obj)){
    single.tags <- trim(unlist(strsplit(pasteXMLTree(XML.obj, shine=1, indent.by=""), split="\n")))
  } else if(is.XiMpLe.node(XML.obj)){
    single.tags <- trim(unlist(strsplit(pasteXML(XML.obj, shine=1, indent.by=""), split="\n")))
  } else if(!is.null(XML.obj)){
    xml.raw <- paste(readLines(XML.obj), collapse=" ")
    single.tags <- XiMpLe:::XML.single.tags(xml.raw, drop=drop)
  } else {
    return(NULL)
  }
  names(single.tags) <- NULL

  return(single.tags)
} ## end function get.single.tags()


## function get.IDs()
# scans XML tags for defined IDs, returns a matrix with columns "id" and "abbrev",
# and optional "tag". "abbrev" is mostly used for the JavaScript variable name.
# 'single.tags' can also contain XiMpLe.node objects
get.IDs <- function(single.tags, relevant.tags, add.abbrev=FALSE, tag.names=FALSE, only.checkable=FALSE){

  # filter for relevant tags
  cleaned.tags <- list()
  for(this.tag in child.list(single.tags)){
    if(is.XiMpLe.node(this.tag)){
      this.tag.name <- XMLName(this.tag)
      if(this.tag.name %in% relevant.tags & "id" %in% names(XMLAttrs(this.tag))){
        if(isTRUE(only.checkable) & this.tag.name %in% "frame"){
          if("checkable" %in% names(XMLAttrs(this.tag))){
            if(identical(XMLAttrs(this.tag)[["checkable"]], "true")){
              cleaned.tags[length(cleaned.tags)+1] <- this.tag
            } else {}
          } else {}
        } else {
          cleaned.tags[length(cleaned.tags)+1] <- this.tag
        }
      } else {}
    } else {
      this.tag.name <- tolower(XiMpLe:::XML.tagName(this.tag))
      # we're only interested in entries with an ID
      if(this.tag.name %in% relevant.tags){
        if("id" %in% names(XiMpLe:::parseXMLAttr(this.tag))){
          if(isTRUE(only.checkable) & this.tag.name %in% "frame"){
            if("checkable" %in% names(XiMpLe:::parseXMLAttr(this.tag))){
              if(identical(XiMpLe:::parseXMLAttr(this.tag)[["checkable"]], "true")){
                cleaned.tags[length(cleaned.tags)+1] <- this.tag
              } else {}
            } else {}
          } else {
            cleaned.tags[length(cleaned.tags)+1] <- this.tag
          }
        } else {}
      } else {}
    }
  }

  ids <- t(sapply(cleaned.tags, function(this.tag){
        if(is.XiMpLe.node(this.tag)){
          this.tag.name <- XMLName(this.tag)
          this.tag.id.abbrev <- this.tag.id <- XMLAttrs(this.tag)["id"]
          # take care of one special case: optionsets
          # they need the set ID to access the value from the dialog,
          # but to be able to use only the optioncolumn in rkwaddev scripts
          # as reference, the JavaScript variable must be generated from the
          # column ID alone.
          if(identical(this.tag.name, "optioncolumn")){
            this.tag.setid <- XMLAttrs(this.tag)[["setid"]]
            if(!is.null(this.tag.setid)){
              this.tag.id <- paste(this.tag.setid, this.tag.id, sep=".")
            } else {}
            # for safety, prefix the column ID with a constant
            this.tag.id.abbrev <- paste0("ocol_", this.tag.id.abbrev)
          } else {}
        } else {
          this.tag.name <- XiMpLe:::XML.tagName(this.tag)
          this.tag.id.abbrev <- this.tag.id <- XiMpLe:::parseXMLAttr(this.tag)[["id"]]
          # see comment above for the next part
          if(identical(this.tag.name, "optioncolumn")){
            this.tag.setid <- XiMpLe:::parseXMLAttr(this.tag)[["setid"]]
            if(!is.null(this.tag.setid)){
              this.tag.id <- paste(this.tag.setid, this.tag.id, sep=".")
            } else {}
            # for safety, prefix the column ID with a constant
            this.tag.id.abbrev <- paste0("ocol_", this.tag.id.abbrev)
          } else {}
        }

        if(isTRUE(add.abbrev)){
          this.tag.id.abbrev <- paste0(ID.prefix(this.tag.name), this.tag.id.abbrev)
        } else {}
      if(isTRUE(tag.names)){
        return(c(id=this.tag.id, abbrev=this.tag.id.abbrev, tag=this.tag.name))
      } else {
        return(c(id=this.tag.id, abbrev=this.tag.id.abbrev))
      }
    }
  ))
  rownames(ids) <- NULL

  # do a check if all IDs are really unique
  if("id" %in% names(ids)){
    multiple.id <- duplicated(ids[,"id"])
    if(any(multiple.id)){
      warning(paste0("IDs are not unique:\n  ", paste(ids[multiple.id,"id"], collapse=", "), "\n  Expect errors!"))
    } else {}
  }

  return(ids)
} ## end function get.IDs()

## function check.optionset.tags()
# XML.obj may be a character string (file name) or XiMpLe object.
# this function will check if <optionset> nodes are present
# and return a possibly corrected result of get.single.tags(),
# where "corrected" means: optioncolumns internally will gain an
# attribute "setid" with the respective set ID, and the rest of the
# set is discarded.
# this extra attribute is evaluated by get.IDs().
check.optionset.tags <- function(XML.obj, drop=NULL){
  # if this is not a XiMpLe object, transform the file into one
  if(!is.XiMpLe.node(XML.obj) && !is.XiMpLe.doc(XML.obj)){
    XML.obj <- parseXMLTree(XML.obj, drop=drop)
  } else {}
  # first get a list of all optionsets
  optionset.nodes <- child.list(XMLScan(XML.obj, "optionset"))
  # are there any?
  if(is.null(optionset.nodes)){
    result <- get.single.tags(XML.obj=XML.obj, drop=drop)
  } else {
    # now go through all sets and combine setID with the IDs of optioncolumns
    optioncolumnNewIDs <- unlist(sapply(optionset.nodes, function(thisNode){
        thisCols <- child.list(XMLScan(thisNode, "optioncolumn"))
        thisSetID <- XMLAttrs(thisNode)[["id"]]
        thisNewCols <- unlist(sapply(thisCols, function(thisCol){
            XMLAttrs(thisCol)[["setid"]] <- thisSetID
            pastedTag <- get.single.tags(XML.obj=thisCol, drop=drop)
            return(pastedTag)
          }, USE.NAMES=FALSE))
        return(thisNewCols)
      }, USE.NAMES=FALSE))
    # we don't need the set nodes any longer
    XMLScan(XML.obj, "optionset") <- NULL
    result <- c(optioncolumnNewIDs, get.single.tags(XML.obj=XML.obj, drop=drop))
  }
  return(result)
} ## end function check.optionset.tags()

## function camelCode()
# changes the first letter of each string
# (except for the first one) to upper case
camelCode <- function(words){

  words <- as.vector(unlist(sapply(words, function(cur.word){
      unlist(strsplit(cur.word, split="[._]"))
    })))

  new.words <- sapply(words[-1], function(cur.word){
    word.vector <- unlist(strsplit(cur.word, split=""))
    word.vector[1] <- toupper(word.vector[1])
    word.new <- paste(word.vector, collapse="")
    return(word.new)
  })

  results <- paste0(words[1], paste(new.words, collapse=""))

  return(results)
} ## end function camelCode()


## default getters for JavaScript variables
# try to set useful default getter functions to query the values from XML nodes
# will only be used if "guess.getter" is true
JS.getters.default <- list(
  "browser"="getString",
  "checkbox"="getBoolean",
  "dropdown"="getString",
  "frame"="getBoolean",
  "input"="getString",
  "matrix"="getList",
  "optioncolumn"="getList",
  "radio"="getString",
  "saveobject"="getString",
  "spinbox"="getString",
  "varslot"="getString"
)
# we can also guess some fitting getter functions by the modifier set
JS.getters.modif.default <- list(
#  "active",
#  "available",
#  "calculate",
  "checked"="getBoolean",
  "checked.not"="getBoolean",
  "checked.numeric"="getBoolean",
  "dependent"="getString",
  "enabled"="getBoolean",
  "enabled.not"="getBoolean",
  "enabled.numeric"="getBoolean",
#  "false",
  "fixed_factors"="getString",
#  "int",
  "label"="getString",
  "labels"="getString",
  "model"="getString",
#  "not",
#  "number",
#  "numeric",
  "objectname"="getString",
  "parent"="getString",
  "preprocess"="getString",
  "preview"="getBoolean",
  "printout"="getString",
#  "real",
  "required"="getBoolean",
#  "root",
#  "selected",
#  "selection",
  "shortname"="getString",
  "source"="getString",
  "state"="getBoolean",
  "state.not"="getBoolean",
  "state.numeric"="getBoolean",
  "string"="getString",
#  "table",
  "text"="getString",
#  "true",
  "visible"="getBoolean",
  "visible.not"="getBoolean",
  "visible.numeric"="getBoolean"
)

## function get.JS.vars()
#   <tag id="my.id" ...>
# in XML will become
#   var my.id = getValue("my.id");
get.JS.vars <- function(JS.var, XML.var=NULL, tag.name=NULL, JS.prefix="", names.only=FALSE, modifiers=NULL, default=FALSE, join="",
  getter="getValue", guess.getter=FALSE, check.modifiers=TRUE){
  # check for XiMpLe nodes
  JS.var <- check.ID(JS.var)
  have.XiMpLe.var <- FALSE
  if(!is.null(XML.var)){
    if(is.XiMpLe.node(XML.var)){
      have.XiMpLe.var <- TRUE
      tag.name <- XMLName(XML.var)
    } else if(is.null(tag.name)){
      # hm, not a XiMpLe object and no known tag name :-/
      # if this is simply a character string, the tag name will become ""
      tag.name <- XMLName(XMLChildren(parseXMLTree(XML.var, object=TRUE))[[1]])
    } else {}

    # check validity of modifiers value
    if(!is.null(modifiers)){
      if(identical(modifiers, "all")){
        if(tag.name %in% names(all.valid.modifiers)){
          modifiers <- all.valid.modifiers[[tag.name]]
        } else {
          modifiers <- NULL
        }
      } else {
        if(identical(tag.name, "")){
          modif.tag.name <- "all"
        } else {
          modif.tag.name <- tag.name
        }
        if(isTRUE(check.modifiers)){
          modifiers <- modifiers[modif.validity(modif.tag.name,
            modifier=child.list(modifiers), warn.only=TRUE, bool=TRUE)]
        } else {}
      }
    } else {}


    # check for getter guessing
    if(isTRUE(guess.getter)){
      if(tag.name %in% names(JS.getters.default)){
        # special case: is a <checkbox> has a value other than
        # "true" or "false", it's probably supposed to be fetched
        # as string, not boolean
        if(isTRUE(have.XiMpLe.var) && identical(tag.name, "checkbox") &&
          any(!c(XMLAttrs(XML.var)[["value"]], XMLAttrs(XML.var)[["value_unchecked"]]) %in% c("true","false"))){
          getter <- "getString"
        } else {
          # check if a modifier is given and we have a default for it
          # modifiers were probably checked already
          ## TODO: currently this only works for one modifier of if all
          ## modifiers are fine with the same getter; maybe "getter"
          ## should become a vector like "modifiers"
          if(!is.null(modifiers) && any(modifiers %in% names(JS.getters.modif.default))){
            # find all matching modifiers
            getter.modifs <- modifiers[modifiers %in% names(JS.getters.modif.default)]
            all.getters <- unique(unlist(JS.getters.modif.default[getter.modifs]))
            if(length(all.getters) > 1){
              warning("For the modifiers you specified, different getter functions were found. Only using the first one!", call.=FALSE)
              getter <- all.getters[1]
            } else {
              getter <- all.getters
            }
          } else {
            getter <- JS.getters.default[[tag.name]]
          }
        }
      } else {}
    } else {
      # if guess.getters is off but we're dealing with <matrix> or <optionset>,
      # throw in a warning:
      if(tag.name %in% c("matrix", "optioncolumn") && identical(getter, "getValue")){
        warning(paste0("Your plugin contains the <", tag.name, "> element, but 'guess.getter' is off. ",
          "Using the default getValue() on this node might cause problems!"), call.=FALSE)
      } else {}
    }
    XML.var <- check.ID(XML.var)
  } else {
    XML.var <- check.ID(JS.var)
  }

  if(is.null(JS.prefix)){
    JS.prefix <- ""
  } else {}

  if(isTRUE(names.only)){
    results <- c()
    if(is.null(modifiers) || isTRUE(default)){
      results <- camelCode(c(JS.prefix, JS.var))
    } else {}
    if(!is.null(modifiers)){
      results <- c(results,
        sapply(modifiers, function(this.modif){camelCode(c(JS.prefix, JS.var, this.modif))})
      )
    } else {}
  } else {
    if(is.null(modifiers)){
       modifiers <- list()
    } else {}
    results <- new("rk.JS.var",
      JS.var=JS.var,
      XML.var=XML.var,
      prefix=JS.prefix,
      modifiers=as.list(modifiers),
      default=default,
      join=join,
      getter=getter)
  }

  return(results)
} ## end function get.JS.vars()


## function ID.prefix()
ID.prefix <- function(initial, abbr=TRUE, length=3, dot=FALSE){
  if(isTRUE(abbr)){
    prfx <- abbreviate(initial, minlength=length, strict=TRUE)
  } else {
    # currently empty, but can later be used to define fixed abbreviations
    prfx <- NULL
  }
  if(isTRUE(dot)){
    prfx <- paste0(prfx, ".")
  } else {
    prfx <- paste0(prfx, "_")
  }
  return(prfx)
} ## end function ID.prefix()


## function node.soup()
# pastes the nodes as XML, only alphanumeric characters, e.g. to generate auto-IDs
node.soup <- function(nodes){
  the.soup <- paste0(unlist(sapply(child.list(nodes), function(this.node){
      if(is.XiMpLe.node(this.node)){
        return(gsub("[^[:alnum:]]", "", pasteXML(this.node, shine=0)))
      } else {
        stop(simpleError("Nodes must be of class XiMpLe.node!"))
      }
    })), collapse="")
  return(the.soup)
} ## end function node.soup()


## function XML2person()
# extracts the person/author info from XML "about" nodes
XML2person <- function(node, eval=FALSE){
    if(is.XiMpLe.node(node)){
      # check if this is *really* a about section, otherwise die of boredom
      if(!identical(XMLName(node), "about")){
        stop(simpleError("I don't know what this is, but 'about' is not an about section!"))
      } else {}
    } else {
      stop(simpleError("'about' must be a XiMpLe.node, see ?rk.XML.about()!"))
    }
  make.vector <- function(value){
    if(grepl(",", value)){
      value <- paste0("c(\"", paste(trim(unlist(strsplit(value, ","))), collapse="\", \""), "\")")
    } else {
      value <- paste0("\"", value, "\"")
    }
    return(value)
  }
  all.authors <- c()
  for (this.child in XMLChildren(node)){
    if(identical(XMLName(this.child), "author")){
      attrs <- XMLAttrs(this.child)
      given <- make.vector(attrs[["given"]])
      family <- make.vector(attrs[["family"]])
      email <- make.vector(attrs[["email"]])
      role <- make.vector(attrs[["role"]])
      this.author <- paste0("person(given=", given, ", family=", family, ", email=", email, ", role=", role, ")")
      all.authors[length(all.authors) + 1] <- this.author
    } else {}
  }
  if(length(all.authors) > 1){
    all.authors <- paste0("c(", paste(all.authors, collapse=", "), ")")
  } else {}
  if(isTRUE(eval)){
    all.authors <- eval(parse(text=all.authors))
  } else {}
  return(all.authors)
} ## end function XML2person()


## function XML2dependencies()
# extracts the package dependencies info from XML "about"/"dependencies" nodes
# in "suggest" mode only suggestions will be returned, in "depends" mode only dependencies.
# suggest=TRUE: Depends: R & RKWard; Suggests: packages
# suggest=FALSE: Depends: R & RKWard & packages; suggests: none
XML2dependencies <- function(node, suggest=TRUE, mode="suggest"){
  if(!isTRUE(suggest) && identical(mode, "suggest")){
    return("")
  } else {}
  if(is.XiMpLe.node(node)){
    # check if this is *really* a about section, otherwise die of boredom
    if(!XMLName(node) %in% c("about", "dependencies")){
      # are these perhaps commented out? then just quit silently
      if(XMLName(node) %in% "!--"){
        return("")
      } else {
        stop(simpleError("Please provide a valid about or dependencies section!"))
      }
    } else {}
  } else {
    stop(simpleError("'about' and/or 'dependencies' must be XiMpLe.nodes, see ?rk.XML.about() and ?rk.XML.dependencies()!"))
  }
  got.deps <- XMLScan(node, "dependencies")
  if(!is.null(got.deps)){
    deps.packages <- list()
    # first see if RKWard and R versions are given
    deps.RkR <- XMLAttrs(got.deps)
    deps.RkR.options  <- names(deps.RkR)
    R.min <- ifelse("R_min_version" %in% deps.RkR.options, paste0(">= ", deps.RkR[["R_min_version"]]), "")
    R.max <- ifelse("R_max_version" %in% deps.RkR.options, paste0("< ", deps.RkR[["R_max_version"]]), "")
    R.version.indices <- sum(!identical(R.min, ""), !identical(R.max, ""))
    if(R.version.indices > 0 & identical(mode, "depends")){
      deps.packages[[length(deps.packages) + 1]] <- paste0("R (", R.min, ifelse(R.version.indices > 1, ", ", ""), R.max, ")")
    } else {}
    Rk.min <- ifelse("rkward_min_version" %in% deps.RkR.options, paste0(">= ", deps.RkR[["rkward_min_version"]]), "")
    Rk.max <- ifelse("rkward_max_version" %in% deps.RkR.options, paste0("< ", deps.RkR[["rkward_max_version"]]), "")
    Rk.version.indices <- sum(!identical(Rk.min, ""), !identical(Rk.max, ""))
    if(Rk.version.indices > 0 && identical(mode, "depends")){
      deps.packages[[length(deps.packages) + 1]] <- paste0("rkward (", Rk.min, ifelse(Rk.version.indices > 1, ", ", ""), Rk.max, ")")
    } else {}
    check.deps.pckg <- sapply(XMLChildren(got.deps), function(this.child){identical(XMLName(this.child), "package")})
    if(any(check.deps.pckg) && ((isTRUE(suggest) && identical(mode, "suggest")) | !isTRUE(suggest))){
      deps.packages[[length(deps.packages) + 1]] <- paste(sapply(which(check.deps.pckg), function(this.pckg){
          this.pckg.dep <- XMLAttrs(XMLChildren(got.deps)[[this.pckg]])
          pckg.options <- names(this.pckg.dep)
          pckg.name <- this.pckg.dep[["name"]]
          pckg.min <- ifelse("min" %in% pckg.options, paste0(">= ", this.pckg.dep[["min"]]), "")
          pckg.max <- ifelse("max" %in% pckg.options, paste0("< ", this.pckg.dep[["max"]]), "")
          version.indices <- sum(!identical(pckg.min, ""), !identical(pckg.max, ""))
          if(version.indices > 0){
            pckg.version <- paste0(" (", pckg.min, ifelse(version.indices > 1, ", ", ""), pckg.max, ")")
          } else {
            pckg.version <- ""
          }
          return(paste0(pckg.name, pckg.version))
        }), collapse=", ")
    } else {}
    results <- paste(unlist(deps.packages), collapse=", ")
  } else {
    results <- ""
  }
  return(results)
} ## end function XML2dependencies()


## function get.by.role()
# filters a vector with person objects by roles
get.by.role <- function(persons, role="aut"){
  role.filter <- function(x){is.null(r <- x$role) | role %in% r}
  filtered.persons <- Filter(role.filter, persons)
  return(filtered.persons)
} ## end function get.by.role()


## function check.ID()
# - node: a XiMpLe.node to search for an ID
# - search.environment: if TRUE, the internal environment is searched for the ID
#     as well; a use case for this is IDs of oprions, which need their parent IDs as well;
#     see get.optionIDs() below
check.ID <- function(node, search.environment=FALSE){
  if(is.list(node)){
    return(sapply(node, check.ID))
  } else {}

  if(is.XiMpLe.node(node)){
    node.ID <- XMLAttrs(node)[["id"]]
    if(isTRUE(search.environment)){
      optionIDs <- get.optionIDs()[[node.ID]]
      node.ID <- ifelse(is.null(optionIDs), node.ID, optionIDs[["XML"]])
    } else {}
  } else if(is.character(node)){
    node.ID <- node
  } else {
    stop(simpleError("Can't find an ID!"))
  }

  if(is.null(node.ID)){
    warning("ID is NULL!")
  } else {}

  names(node.ID) <- NULL

  return(node.ID)
} ## end function check.ID()


## list with valid modifiers
all.valid.modifiers <- list(
  all=c("", "visible", "visible.not", "visible.numeric", "enabled", "enabled.not", "enabled.numeric",
  "required", "true", "false", "not", "numeric", "preprocess", "calculate", "printout", "preview"),
  browser=c("selection"),
  checkbox=c("state", "state.not", "state.numeric"),
  dropdown=c("string", "number"),
# removed embed, can be all sorts of stuff, see e.g. generic plot options
#  embed=c("code"),
# for the same reason external is not listed here
  frame=c("checked", "checked.not", "checked.numeric"),
  input=c("text"),
  formula=c("model", "table", "labels", "fixed_factors", "dependent"),
  matrix=c("rows", "columns", "tsv", "cbind"), # TODO: missing a solution for 1,2,3,... here
  # option=c(),
  optionset=c("row_count", "current_row", "optioncolumn_ids"),
  preview=c("state", "state.not", "state.numeric"),
  radio=c("string", "number"),
  saveobject=c("selection", "parent", "objectname", "active"),
  spinbox=c("int", "real"),
  text=c("text"),
  varselector=c("selected", "root"),
  varslot=c("available", "selected", "source", "shortname", "label")
) ## end list with valid modifiers


## function modif.validity()
# checks if a modifier is valid for an XML node, if source is XiMpLe.node
# if bool=FALSE, returns the modifier or ""
modif.validity <- function(source, modifier, ignore.empty=TRUE, warn.only=TRUE, bool=TRUE){
  if(identical(modifier, "") & isTRUE(ignore.empty)){
    if(isTRUE(bool)){
      return(TRUE)
    } else {
      return(modifier)
    }
  } else {}

  if(is.XiMpLe.node(source)){
    tag.name <- XMLName(source)
    # certain elemens/embedded plugins can have all sorts of modifiers
    if(tag.name %in% c("embed", "external", "switch")){
      if(isTRUE(bool)){
        return(TRUE)
      } else {
        return(modifier)
      }
    } else {}
  } else if(identical(source, "all")){
    tag.name <- "<any tag>"
  } else {
    tag.name <- source
  }

  if(tag.name %in% names(all.valid.modifiers)){
    valid.modifs <- c(all.valid.modifiers[["all"]], all.valid.modifiers[[tag.name]])
  } else if(identical(tag.name, "<any tag>")){
    valid.modifs <- unique(unlist(all.valid.modifiers))
  } else {
    valid.modifs <- c(all.valid.modifiers[["all"]])
  }

  invalid.modif <- !unlist(modifier) %in% valid.modifs
  if(any(invalid.modif)){
    if(isTRUE(warn.only)){
      warning(paste0("Some modifier you provided is invalid for '",tag.name,"' and was ignored: ",
        paste(modifier[invalid.modif], collapse=", ")), call.=FALSE)
      if(isTRUE(bool)){
        return(!invalid.modif)
      } else {
        return("")
      }
    } else {
      stop(simpleError(paste0("Some modifier you provided is invalid for '",tag.name,"' and was ignored: ",
        paste(modifier[invalid.modif], collapse=", "))))
    }
  } else {
    if(isTRUE(bool)){
      return(!invalid.modif)
    } else {
      return(modifier)
    }
  }
} ## end function modif.validity()


## list with valid child nodes
# important for certain parent nodes, as long as
# XiMpLe doesn't interpret doctypes
all.valid.children <- list(
  # 'as' is not a node, but an attribute of <copy>
  as=c("browser", "checkbox", "column", "copy",
    "dropdown", "formula", "frame", "input", "page", "radio", "row", "saveobject",
    "spinbox", "stretch", "tabbook", "text", "varselector", "varslot"),
  component=c("dependencies"),
  components=c("component"),
  context=c("menu", "!--"),
  dialog=c("browser", "checkbox", "column", "copy",
    "dropdown", "embed", "formula", "frame", "include", "input", "insert", "matrix",
    "optionset", "preview", "radio", "row", "saveobject", "spinbox", "stretch", "tabbook",
    "text", "varselector", "varslot", "!--"),
  dropdown=c("option"),
  hierarchy=c("menu", "!--"),
  logic=c("connect", "convert", "dependency_check", "external", "include", "insert",
    "script", "set", "switch"),
  menu=c("entry", "menu", "!--"),
  optionset=c("content", "logic", "optioncolumn"),
  page=c("browser", "checkbox", "column", "copy",
    "dropdown", "formula", "frame", "input", "matrix", "optionset", "page", "radio",
    "row", "saveobject", "spinbox", "stretch", "tabbook", "text", "varselector",
    "varslot", "!--"),
  radio=c("option"),
  settings=c("setting", "caption", "!--"),
  wizard=c("browser", "checkbox", "column", "copy",
    "dropdown", "embed", "formula", "frame", "include", "input", "insert", "matrix",
    "optionset", "page", "preview", "radio", "row", "saveobject", "spinbox", "stretch",
    "tabbook", "text", "varselector", "varslot", "!--")
) ## end list with valid child nodes


## function valid.child()
# - parent: character string, name of the parent node
# - children: (list of) XiMpLe.node objects, child nodes to check
# - warn: warning or stop?
# - section: an optional name for the section for the warning/error
#   (if it shouldn't be the parent name)
# - node names: can alternatively be given instead of 'children', as character vector
valid.child <- function(parent, children, warn=FALSE, section=parent, node.names=NULL){
  if(is.null(node.names)){
    # check the node names and allow only valid ones
    node.names <- sapply(child.list(children), function(this.child){
        # if this is a plot options object, by default extract the XML slot
        # and discard the rest
        this.child <- stripXML(this.child)

        if(is.XiMpLe.node(this.child)){
          return(XMLName(this.child))
        } else {
          stop(simpleError(paste0("Invalid object for ", section, " section, must be of class XiMpLe.node, but got class ", class(this.child), "!")))
        }
      })
  } else {}

  invalid.sets <- !node.names %in% all.valid.children[[parent]]
  if(any(invalid.sets)){
    return.message <- paste0("Invalid XML nodes for ", section, " section: ", paste(node.names[invalid.sets], collapse=", "))
    if(isTRUE(warn)){
      warning(return.message)
      return(FALSE)
    } else {
      stop(simpleError(return.message))
    }
  } else {
    return(TRUE)
  }
} ## end function valid.child()


## function valid.parent()
# checks if a node is what it's supposed to be
# - parent: character string, name of the parent node
# - node: a XiMpLe.node object to check
# - warn: warning or stop?
# - see: name of the function to check docs for
# - arg.name: optional argument name of a function where valid.parent() is called from,
#     e.g. if an object is given via "cbox" but checked for "checkbox"
valid.parent <- function(parent, node, warn=FALSE, see=NULL, arg.name=NULL, comment.ok=FALSE){
  if(is.XiMpLe.node(node)){
    node.name <- XMLName(node)
    if(identical(node.name, parent)){
      return(TRUE)
    } else {
      if(isTRUE(comment.ok) & identical(node.name, "!--")){
        return(TRUE)
      } else {}
      if(is.null(arg.name)){
        arg.name <- parent
      } else {}
      return.message <- paste0("I don't know what this is, but '", arg.name, "' is not a <", parent, "> section!")
      if(isTRUE(warn)){
        warning(return.message)
        return(FALSE)
      } else {
        stop(simpleError(return.message))
      }
    }
  } else {
    stop(simpleError(
        paste0("'", parent, "' must be a XiMpLe.node",
          if(!is.null(see)){paste0(", see ?", see)},
          "!"))
      )
  }
} ## end function valid.parent()


## function check.type()
check.type <- function(value, type, var.name, warn.only=TRUE){
  if(inherits(value, type)){
    return(invisible(NULL))
  } else {
    msg.text <- paste0(sQuote(var.name), " should be of type ", type, "!")
    if(isTRUE(warn.only)){
      warning(msg.text)
    } else {
      stop(simpleError(msg.text))
    }
  }
} ## end function check.type()


## function clean.name()
clean.name <- function(name, message=TRUE){
  name.orig <- name
  name <- gsub("[[:space:]]*[^[:alnum:]_.]*", "", name)
  if(!identical(name.orig, name)){
    if(isTRUE(message)){
      message(paste0("For file names ", sQuote(name.orig), " was renamed to ", sQuote(name), "."))
    } else {}
  } else {}
  return(name)
} ## end function clean.name()



## function paste.JS.ite()
paste.JS.ite <- function(object, level=1, indent.by="\t", recurse=FALSE, empty.e=FALSE){
  stopifnot(inherits(object, "rk.JS.ite"))
  # check indentation
  main.indent <- indent(level, by=indent.by)
  scnd.indent <- indent(level+1, by=indent.by)

  # if this is not a single "if" but an "else if", do not indent
  if(isTRUE(recurse)){
    ifJS <- paste0("if(", slot(object, "ifJS"), ") {\n")
  } else {
    ifJS <- paste0(main.indent, "if(", slot(object, "ifJS"), ") {\n")
  }

  if(nchar(slot(object, "thenJS")) > 0) {
    # chop off beginning indent strings, otherwiese they ruin the code layout
    thenJS.clean <- gsub(paste0("^", indent.by, "*"), "", slot(object, "thenJS"))
    thenJS <- paste0(scnd.indent, thenJS.clean, "\n", main.indent, "}")
  } else {
    # if there is another rk.JS.ite object, call with recursion
    if(length(slot(object, "thenifJS")) == 1){
      thenJS <- paste0(paste.JS.ite(slot(object, "thenifJS")[[1]], level=level+1, indent.by=indent.by), "\n", main.indent, "}")
    } else {}
  }

  if(nchar(slot(object, "elseJS")) > 0) {
    # chop off beginning indent strings, otherwiese they ruin the code layout
    elseJS.clean <- gsub(paste0("^", indent.by, "*"), "", slot(object, "elseJS"))
    elseJS <- paste0(" else {\n", scnd.indent, elseJS.clean, "\n", main.indent, "}")
  } else {
    # if there is another rk.JS.ite object, call with recursion
    if(length(slot(object, "elifJS")) == 1){
      elseJS <- paste0(" else ", paste.JS.ite(slot(object, "elifJS")[[1]], level=level, indent.by=indent.by, recurse=TRUE))
    } else {
      if(isTRUE(empty.e)){
        # close for sure with an empty "else"
        elseJS <- " else {}"
      } else {
        elseJS <- NULL
      }
    }
  }

  result <- paste0(ifJS, thenJS, elseJS, collapse="")

  return(result)
} ## end function paste.JS.ite()


## function paste.JS.array()
paste.JS.array <- function(object, level=2, indent.by="\t", funct=NULL){
  stopifnot(inherits(object, "rk.JS.arr"))
  # check indentation
  main.indent <- indent(level, by=indent.by)
  scnd.indent <- indent(level+1, by=indent.by)

  arr.name  <- slot(object, "arr.name")
  opt.name  <- slot(object, "opt.name")
  variables <- slot(object, "variables")
  quote     <- slot(object, "quote")
  option    <- slot(object, "option")
  if(is.null(funct)){
    funct <- slot(object, "funct")
  } else {}
  if(is.null(funct) | identical(funct, "")){
    funct.start <- ""
    funct.end <- ""
  } else {
    funct.start <- paste0(funct, "(")
    funct.end <- ")"
  }
  
  JS.array <- paste0(
    main.indent, "// define the array ", arr.name, " for values of R option \"", option, "\"\n",
    main.indent, "var ", arr.name, " = new Array();\n",
    main.indent, arr.name, ".push(",
    paste(variables, collapse=", "), ");\n",
    main.indent, "// clean array ", arr.name, " from empty strings\n",
    main.indent, arr.name, " = ", arr.name, ".filter(String);\n",
    main.indent, "// set the actual variable ", opt.name,
    ifelse(identical(option, ""), "", paste0(" for R option \"", option)),
    ifelse(identical(funct, ""), "\"", paste0("=", funct, "()\"")), "\n",
    main.indent, "if(", arr.name, ".length > 0) {\n",
    scnd.indent, "var ", opt.name, " = \", ",
    ifelse(identical(option, ""), "", paste0(option, "=")),
    ifelse(isTRUE(quote),
      paste0(funct.start, "\\\"\" + ", arr.name, ".join(\"\\\", \\\"\") + \"\\\"",funct.end,"\";\n"),
      paste0(funct.start, "\" + ", arr.name, ".join(\", \") + \"",funct.end,"\";\n")
    ),
    main.indent, "} else {\n",
    scnd.indent, "var ", opt.name, " = \"\";\n",
    main.indent, "}\n")

  return(JS.array)
} ## end function paste.JS.array()


## function paste.JS.options()
paste.JS.options <- function(object, level=2, indent.by="\t", array=NULL, funct=NULL){
  stopifnot(inherits(object, "rk.JS.opt"))
  # check indentation
  main.indent <- indent(level, by=indent.by)
  scnd.indent <- indent(level+1, by=indent.by)

  variable  <- slot(object, "var.name")
  option    <- slot(object, "opt.name")
  arr.name  <- camelCode(c("arr", variable))
  collapse  <- slot(object, "collapse")
  ifs       <- slot(object, "ifs")
  if(is.null(array)){
    array  <- slot(object, "array")
  } else {}
  if(is.null(funct)){
    funct <- slot(object, "funct")
  } else {}
  if(is.null(funct) | identical(funct, "")){
    funct.start <- ""
    funct.end <- ""
  } else {
    funct.start <- paste0(funct, "(")
    funct.end <- ")"
  }

  # a function to add the object stuff to ite objects
  add.opts <- function(this.ite, collapse, array){
    if(isTRUE(array)){
      slot(this.ite, "thenJS") <- paste0(arr.name, ".push(", slot(this.ite, "thenJS"),");")
    } else {
      slot(this.ite, "thenJS") <- paste0(variable, " += ", collapse, slot(this.ite, "thenJS"),";")
    }
    if(length(slot(this.ite, "elifJS")) == 1){
      slot(this.ite, "elifJS") <- list(add.opts(slot(this.ite, "elifJS")[[1]]))
    } else {}
    return(this.ite)
  }

  # the object class makes sure this is a list of rk.JS.ite objects
  ifs.pasted <- sapply(1:length(ifs), function(thisIf.num){
    thisIf <- ifs[[thisIf.num]]
    # skip the first collapse
    if(thisIf.num > 1){
      this.collapse <- collapse
    } else {
      this.collapse <- ""
    }
    paste.JS.ite(add.opts(thisIf, collapse=this.collapse, array=array), level=level+1, indent.by=indent.by)
  })

#return(ifs.pasted)

  JS.options <- paste0(
    if(isTRUE(array)){
      paste0(
        main.indent, "// define the array ", arr.name, " for values of R option \"", option, "\"\n",
        main.indent, "var ", arr.name, " = new Array();\n")
    } else {
      paste0(main.indent, "var ", variable, " = \"\";\n")
    },
    paste0(ifs.pasted, collapse="\n"), "\n",
    if(isTRUE(array)){
      paste0(
        main.indent, "// clean array ", arr.name, " from empty strings\n",
        main.indent, arr.name, " = ", arr.name, ".filter(String);\n",
        main.indent, "// set the actual variable ", variable, " with all values for R option \"", option, "\"\n",
        main.indent, "if(", arr.name, ".length > 0) {\n",
        scnd.indent, "var ", variable, " = \"", collapse,
        ifelse(identical(option, ""), "", paste0(option, "=")),
        funct.start, "\" + ", arr.name, ".join(\", \") + \"",funct.end,"\";\n",
        main.indent, "} else {\n",
        scnd.indent, "var ", variable, " = \"\";\n",
        main.indent, "}\n")
    } else {})

  return(JS.options)
} ## end function paste.JS.options()


## function paste.JS.var()
paste.JS.var <- function(object, level=2, indent.by="\t", JS.prefix=NULL, modifiers=NULL, default=NULL, join=NULL,
  getter=NULL, names.only=FALSE, check.modifiers=FALSE){
  # paste several objects
  results <- unlist(sapply(slot(object, "vars"), function(this.obj){
      paste.JS.var(this.obj,
          level=level,
          indent.by=indent.by,
          JS.prefix=JS.prefix,
          modifiers=modifiers,
          default=default,
          join=join,
          getter=getter,
          names.only=names.only)}))
  if(!isTRUE(names.only) & !is.null(results)){
    results <- paste(results, collapse="\n")
  }
  if(!isTRUE(names.only)){
    results <- paste(results, collapse="")
  } else {}

  stopifnot(inherits(object, "rk.JS.var"))
  # check indentation
  main.indent <- indent(level, by=indent.by)

  JS.var         <- slot(object, "JS.var")
  XML.var        <- slot(object, "XML.var")
  if(is.null(JS.prefix)){
    JS.prefix  <- slot(object, "prefix")
  } else {}
  if(is.null(modifiers)){
    modifiers  <- slot(object, "modifiers")
  } else {}
  if(is.null(default)){
    default     <- slot(object, "default")
  } else {}
  if(is.null(join)){
    join        <- slot(object, "join")
  } else {}
  if(is.null(getter)){
    getter      <- slot(object, "getter")
  } else {}

  if(!identical(join, "")){
    join.code <- paste0(".split(\"\\n\").join(\"", join, "\")")
  } else {
    join.code <- ""
  }

  # only paste something if there's variables outside the 'vars' slot
  if(length(nchar(JS.var)) > 0 & length(nchar(XML.var)) > 0){
    if(length(modifiers) == 0 | isTRUE(default)){
      if(isTRUE(names.only)){
        results <- c(results, camelCode(c(JS.prefix, JS.var)))
      } else {
        results <- paste0(main.indent, "var ", camelCode(c(JS.prefix, JS.var)), " = ", getter, "(\"", XML.var, "\")", join.code, ";")
      }
    } else {}
    if(length(modifiers) > 0){
      if(isTRUE(check.modifiers)){
        # check modifiers
        modifiers <- modifiers[modif.validity(source="all", modifier=modifiers, ignore.empty=TRUE, warn.only=TRUE, bool=TRUE)]
      } else {}
      modif.results <- sapply(modifiers, function(this.modif){
          if(isTRUE(names.only)){
            return(camelCode(c(JS.prefix, JS.var, this.modif)))
          } else {
            return(paste0(main.indent, "var ", camelCode(c(JS.prefix, JS.var, this.modif)),
              " = ", getter, "(\"", XML.var, ".", this.modif, "\")", join.code, ";"))
          }
        })
      if(identical(results, "")){
        results <- modif.results
      } else {
        results <- c(results, modif.results)
      }
    }
  } else {}

  if(isTRUE(names.only)){
    results <- c(results)
  } else {
    results <- paste(results, collapse="\n")
  }
  
  return(results)
} ## end function paste.JS.var()


## function dependenciesCompatWrapper()
# with RKWard 0.6.1, the dependencies will no longer be a part of <about>
# this wrapper takes both, "about" and "dependencies" arguments,
# splits dependencies off and returns both in a list
dependenciesCompatWrapper <- function(dependencies, about, hints=FALSE){
  if(!is.null(about)){
    # check if this is *really* a about section
    valid.parent("about", node=about, see="rk.XML.about")
    # check for <dependencies> in <about>; is NULL if not found
    # this will only be used if dependencies is NULL
    deps.in.about <- XMLScan(about, "dependencies")
    if(!is.null(deps.in.about)){
      warning("<dependencies> inside <about> is deprecated, use the 'dependencies' argument instead!")
      # remove the misplaced node
      XMLScan(about, "dependencies") <- NULL
    }
  } else {
    if(isTRUE(hints)){
      about <- XMLNode("!--", XMLNode("about", ""))
    } else {}
    deps.in.about <- NULL
  }

  # initialize results list
  results <- list(about=about)

  if(!is.null(dependencies)){
    # check if this is *really* a dependencies section
    valid.parent("dependencies", node=dependencies, see="rk.XML.dependencies", comment.ok=TRUE)
    results[["dependencies"]] <- dependencies
  } else if(is.XiMpLe.node(deps.in.about)){
    results[["dependencies"]] <- deps.in.about
  } else if(isTRUE(hints)){
    dependencies.XML <- XMLNode("!--", XMLNode("dependencies", ""))
    results[["dependencies"]] <- dependencies.XML
  } else {
    results[["dependencies"]] <- NULL
  }
  return(results)
} ## end function dependenciesCompatWrapper()

## function get.rk.env()
# generic function to query the internal environment and declare a desired object, if not present yet
get.rk.env <- function(name, value=list()){
  if(exists(name, envir=.rkdev.env, inherits=FALSE)){
    this.env <- as.list(.rkdev.env)[[name]]
  } else {
    assign(name, value, envir=.rkdev.env)
    this.env <- value
  }
  return(this.env)
} ## end function get.rk.env()


## function set.rk.env()
# generic function to write to the internal environment
set.rk.env <- function(name, value){
  assign(name, value, envir=.rkdev.env)
  return(invisible(NULL))
} ## end function set.rk.env()


## function get.rkh.prompter()
# returns either an empty list or the contents of rkh.prompter from the internal enviroment 
get.rkh.prompter <- function(){
  rkh.prompter <- get.rk.env("rkh.prompter", value=list())
  return(rkh.prompter)
} ## end function get.rkh.prompter()


## function get.optionIDs()
# returns either an empty list or the contents of rkh.prompter from the internal enviroment 
get.optionIDs <- function(){
  optionIDs <- get.rk.env("optionIDs", value=list())
  return(optionIDs)
} ## end function get.optionIDs()


## function rk.check.options()
# - options: a list, containig either named vectors in the form of
#       label=c(val=NULL, chk=FALSE)
#     or an "option" node of class XiMpLe.node
# - parent: the parent node type, e.g. "radio"
rk.check.options <- function(options, parent){
  num.opt <- length(options)
  all.options <- sapply(1:num.opt, function(this.num){
      if(is.XiMpLe.node(options[[this.num]])){
        # check the node names and allow only valid ones
        valid.child(parent, children=options[[this.num]])
        return(options[[this.num]])
      } else {
        if("chk" %in% names(options[[this.num]])){
          checked <- isTRUE(as.logical(options[[this.num]][["chk"]]))
        } else {
          checked <- FALSE
        }
        return(
          rk.XML.option(
            label=names(options)[[this.num]],
            val=options[[this.num]][["val"]],
            chk=checked,
            id.name=NULL
          )
        )
      }
    })
  # see to it that only one options is "checked"
  is.checked <- sapply(all.options, function(this.opt){
      return(!is.null(XMLScanDeep(this.opt, find="checked")))
    })
  if(sum(is.checked) > 1){
    stop(simpleError("you defined options where more than one is 'checked' -- this is wrong!"))
  } else {}
  return(all.options)
}
## end function rk.check.options()


## function rk.register.options()
# - options: a list, containig either named vectors in the form of
#       label=c(val=NULL, chk=FALSE)
#     or an "option" node of class XiMpLe.node; only the latter will be
#     searched for IDs
# - parent.node: full parent XiMpLe.node option IDs will be registered in
#     an internal environment, which makes it easier to fetch a directly
#     usable ID (because it has to be prefixed with the parent ID)
rk.register.options <- function(options, parent.node){
  num.opt <- length(options)
  all.options <- sapply(1:num.opt, function(this.num){
    if(is.XiMpLe.node(options[[this.num]])){
      opt.id <- XMLAttrs(options[[this.num]])[["id"]]
      if(!is.null(opt.id)){
        # save ID with parents
        optionIDs <- get.optionIDs()
        thisID <- c(XML=id(options[[this.num]], js=FALSE), JS=id(options[[this.num]]))
        parentID <- c(XML=id(parent.node, js=FALSE), JS=id(parent.node))
        optionIDs[[opt.id]] <- list(
          XML=paste(parentID[["XML"]], thisID[["XML"]], sep="."),
          JS=paste(parentID[["JS"]], thisID[["JS"]], sep="."),
          parent=parentID
        )
        set.rk.env("optionIDs", value=optionIDs)
        } else {}
      } else {}
    })
}
## end function rk.register.options()