# Copyright 2010-2015 Meik Michalke <meik.michalke@hhu.de>
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
#' @import XiMpLe rkward

# set up an internal environment, e.g. for prompter settings or indentation
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
indent <- function(level, by=rk.get.indent()){
  paste(rep(by, max(0, level-1)), collapse="")
} ## end function indent()


## function rk.noquote()
# use noquote() in a slightly different manner:
# if "text" is a noquote object, returns "noquote(\"text\")"
rk.noquote <- function(text){
  if(inherits(text, "noquote")){
    return(paste0("noquote(", qp(paste0(text)), ")"))
  } else {
    return(text)
  }
} ## end function rk.noquote()


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
          this.tag.id.abbrev <- this.tag.id <- check.ID(this.tag)
        } else {
          this.tag.name <- XiMpLe:::XML.tagName(this.tag)
          this.tag.id.abbrev <- this.tag.id <- XiMpLe:::parseXMLAttr(this.tag)[["id"]]
        }
        # take care of one special case: optionsets
        # they need the set ID to access the value from the dialog,
        # but to be able to use only the optioncolumn in rkwaddev scripts
        # as reference, the JavaScript variable must be generated from the
        # column ID alone.
        if(identical(this.tag.name, "optioncolumn")){
          this.tag.id <- check.ID(this.tag.id, search.environment=TRUE)
          # for safety, prefix the column ID with a constant
          this.tag.id.abbrev <- paste0("ocol_", this.tag.id.abbrev)
        } else {}

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
  if(!is.null(optionset.nodes)){
    for (thisNode in optionset.nodes){
      optioncolumn.nodes <- child.list(XMLScan(thisNode, "optioncolumn"))
      # register column and set IDs internally
      rk.register.options(optioncolumn.nodes, parent.node=thisNode)
    }
  } else {}
  result <- get.single.tags(XML.obj=XML.obj, drop=drop)
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


## function get.JS.vars()
# see 60_JS.getters.default.R for definition of JS.getters.default and JS.getters.modif.default 
#   <tag id="my.id" ...>
# in XML will become
#   var my.id = getValue("my.id");
get.JS.vars <- function(JS.var, XML.var=NULL, tag.name=NULL, JS.prefix="", names.only=FALSE, modifiers=NULL, default=FALSE, join="",
  getter="getValue", guess.getter=FALSE, check.modifiers=TRUE, search.environment=FALSE, append.modifier=TRUE){
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
      # see 60_JS.getters.default.R for definition of JS.getters.default
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
          # see 60_JS.getters.default.R for definition of JS.getters.modif.default
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
    XML.var <- check.ID(XML.var, search.environment=search.environment)
  } else {
    XML.var <- check.ID(JS.var, search.environment=search.environment)
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
      append.modifier=append.modifier,
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
      if(!is.null(attrs[["email"]])){
        email <- paste0(", email=", make.vector(attrs[["email"]]))
      } else {
        email <- ""
      }
      role <- make.vector(attrs[["role"]])
      this.author <- paste0("person(given=", given, ", family=", family, email, ", role=", role, ")")
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


## function get.authors()
get.authors <- function(description, maintainer=TRUE, contributor=FALSE, copyright=FALSE){
  if("Authors@R" %in% names(description)){
    gotPersons <- TRUE
    authorsFromDescription <- description[["Authors@R"]]
  } else if("Author@R" %in% names(description)){
    gotPersons <- TRUE
    authorsFromDescription <- description[["Author@R"]]
  } else {
    gotPersons <- FALSE
  }
  
  if(isTRUE(gotPersons)){
    got.aut <- paste(format(get.by.role(eval(parse(text=authorsFromDescription))), include=c("given", "family")), collapse=", ")
    got.cre <- ifelse(isTRUE(maintainer),
      paste(format(get.by.role(eval(parse(text=authorsFromDescription)), role="cre"), include=c("given", "family", "email")), collapse=", "),
      "")
    got.ctb <- ifelse(isTRUE(contributor),
      paste(format(get.by.role(eval(parse(text=authorsFromDescription)), role="ctb"), include=c("given", "family")), collapse=", "),
      "")
    got.cph <- ifelse(isTRUE(copyright),
      paste(format(get.by.role(eval(parse(text=authorsFromDescription)), role="cph"), include=c("given", "family")), collapse=", "),
      "")
  } else {
    got.aut <- description[["Author"]]
    got.cre <- ifelse(isTRUE(maintainer),
      description[["Maintainer"]],
      "")
    # contributors should already be named in got.aut
    got.ctb <- ""
    got.cph <- ""
  }
  got.cre.clean <- gsub("<([^@]*)@([^>]*)>", "\\\\email{\\1@@\\2}", gsub("\n[[:space:]]*", "\n#' ", got.cre))
  # append contributors
  if(isTRUE(contributor) && got.ctb != ""){
    got.aut <- paste0(got.aut, ", with contributions from ", got.ctb)
  } else {}
  gotAuthors <- list(aut=got.aut, cre=got.cre, cre.clean=got.cre.clean, ctb=got.ctb, cph=got.cph)
  return(gotAuthors)
} ## end function get.authors()


## function check.ID()
# - node: a XiMpLe.node to search for an ID
# - search.environment: if TRUE, the internal environment is searched for the ID
#     as well; a use case for this is IDs of options, which need their parent IDs as well;
#     see get.optionIDs() below
# - env.get: the ID type to fetch from the environment, if search.environment=TRUE
check.ID <- function(node, search.environment=FALSE, env.get="XML"){
  if(is.list(node)){
    return(sapply(node, check.ID))
  } else {}

  if(is.XiMpLe.node(node)){
    node.ID <- XMLAttrs(node)[["id"]]
    if(isTRUE(search.environment)){
      optionIDs <- get.optionIDs()[[node.ID]]
      node.ID <- ifelse(is.null(optionIDs), node.ID, optionIDs[[env.get]])
    } else {}
  } else if(is.character(node)){
    node.ID <- node
    if(isTRUE(search.environment)){
      optionIDs <- get.optionIDs()[[node.ID]]
      node.ID <- ifelse(is.null(optionIDs), node.ID, optionIDs[[env.get]])
    } else {}
  } else {
    stop(simpleError("Can't find an ID!"))
  }

  if(is.null(node.ID)){
    warning("ID is NULL!")
  } else {}

  names(node.ID) <- NULL

  return(node.ID)
} ## end function check.ID()


## function modif.validity()
# checks if a modifier is valid for an XML node, if source is XiMpLe.node
# if bool=FALSE, returns the modifier or ""
# modifier can take multiple modifiers at once
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
      warning(paste0("Some modifier you provided is invalid for '", tag.name, "' and was ignored: \"",
        paste(modifier[invalid.modif], collapse="\", \""), "\"\n\n",
        "Known modifiers for '", tag.name, "' nodes are:\n  \"", paste0(unlist(modifiers(obj=tag.name)[[tag.name]]), collapse="\", \""), "\"\n\n",
        "For a list of all valid modifiers call modifiers(\"", tag.name, "\")"), call.=FALSE)
      if(isTRUE(bool)){
        return(!invalid.modif)
      } else {
        return("")
      }
    } else {
      stop(simpleError(paste0("Some modifier you provided is invalid for '", tag.name, "' and was ignored: \"",
        paste(modifier[invalid.modif], collapse="\", \""), "\"")))
    }
  } else {
    if(isTRUE(bool)){
      return(!invalid.modif)
    } else {
      return(modifier)
    }
  }
} ## end function modif.validity()


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
    node.names <- unlist(sapply(child.list(children), function(this.child){
        # if this is a plot options object, by default extract the XML slot
        # and discard the rest
        this.child <- stripXML(this.child)

        if(is.XiMpLe.node(this.child)){
          this.child.name <- XMLName(this.child)
          if(identical(this.child.name, "")){
            # special case: empty node name; this is used to combine
            # comments with the node they belong to, so rather check
            # the children of this special node
            return(unlist(sapply(XMLChildren(this.child), XMLName)))
          } else {
            return(this.child.name)
          }
        } else {
          stop(simpleError(paste0("Invalid object for ", section, " section, must be of class XiMpLe.node, but got class ", class(this.child), "!")))
        }
      }))
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
paste.JS.ite <- function(object, level=1, indent.by=rk.get.indent(), recurse=FALSE, empty.e=FALSE){
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
# opt.sep: the separator that comes *before* the option that is set, in the resulting code
paste.JS.array <- function(object, level=2, indent.by=rk.get.indent(), funct=NULL, opt.sep=NULL){
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
  if(is.null(opt.sep)){
    opt.sep <- slot(object, "opt.sep")
    if(is.null(opt.sep)){
      opt.sep <- ", "
    } else {}
  } else {}
  
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
    scnd.indent, "var ", opt.name, " = \"", opt.sep,
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
# opt.sep: the separator that comes *before* the option that is set, in the resulting code
paste.JS.options <- function(object, level=2, indent.by=rk.get.indent(), array=NULL, funct=NULL, opt.sep=NULL){
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
  if(is.null(opt.sep)){
    opt.sep <- slot(object, "opt.sep")
    if(is.null(opt.sep)){
      opt.sep <- ", "
    } else {}
  } else {}

  # a function to add the object stuff to ite objects
  add.opts <- function(this.ite, collapse, array){
    if(isTRUE(array)){
      slot(this.ite, "thenJS") <- paste0(arr.name, ".push(", slot(this.ite, "thenJS"),");")
      if(length(slot(this.ite, "elseJS")) == 1){
        slot(this.ite, "elseJS") <- paste0(arr.name, ".push(", slot(this.ite, "elseJS"),");")
      } else {}
    } else {
      slot(this.ite, "thenJS") <- paste0(variable, " += ", collapse, slot(this.ite, "thenJS"),";")
      if(length(slot(this.ite, "elseJS")) == 1){
        slot(this.ite, "elseJS") <- paste0(variable, " += ", collapse, slot(this.ite, "elseJS"),";")
      } else {}
    }
    if(length(slot(this.ite, "elifJS")) == 1){
      slot(this.ite, "elifJS") <- list(add.opts(slot(this.ite, "elifJS")[[1]], collapse=collapse, array=array))
    } else {}
    if(length(slot(this.ite, "thenifJS")) == 1){
      slot(this.ite, "thenifJS") <- list(add.opts(slot(this.ite, "thenifJS")[[1]], collapse=collapse, array=array))
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
        scnd.indent, "var ", variable, " = \"", opt.sep,
        ifelse(identical(option, ""), "", paste0(option, "=")),
        funct.start, "\" + ", arr.name, ".join(\", \") + \"",funct.end,"\";\n",
        main.indent, "} else {\n",
        scnd.indent, "var ", variable, " = \"\";\n",
        main.indent, "}\n")
    } else {})

  return(JS.options)
} ## end function paste.JS.options()


## function paste.JS.var()
# append.modifier: if a modifier is given, should that become part of the variable name? this is mostly
#   important for "checkbox", which has "state" as default modifier, but using the checkbox object will not
#   notice this. works only for the first modifier given.
# var: if FALSE, the variable is assumed to be already defined (globally?) and "var " will be omitted
paste.JS.var <- function(object, level=2, indent.by=rk.get.indent(), JS.prefix=NULL, modifiers=NULL, default=NULL, append.modifier=NULL,
  join=NULL, getter=NULL, names.only=FALSE, check.modifiers=FALSE, var=TRUE){
  # paste several objects
  results <- unlist(sapply(slot(object, "vars"), function(this.obj){
      paste.JS.var(this.obj,
          level=level,
          indent.by=indent.by,
          JS.prefix=JS.prefix,
          modifiers=modifiers,
          default=default,
          append.modifier=append.modifier,
          join=join,
          getter=getter,
          names.only=names.only,
          check.modifiers=check.modifiers,
          var=var)}))
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
  if(is.null(append.modifier)){
    append.modifier  <- slot(object, "append.modifier")
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
        results <- paste0(main.indent, ifelse(isTRUE(var), "var ", ""), camelCode(c(JS.prefix, JS.var)), " = ", getter, "(\"", XML.var, "\")", join.code, ";")
      }
    } else {}
    if(length(modifiers) > 0){
      if(isTRUE(check.modifiers)){
        # check modifiers
        modifiers <- modifiers[modif.validity(source="all", modifier=modifiers, ignore.empty=TRUE, warn.only=TRUE, bool=TRUE)]
      } else {}
      modif.results <- sapply(1:length(modifiers), function(this.modif.num){
          this.modif <- modifiers[[this.modif.num]]
          if(isTRUE(append.modifier) || this.modif.num > 1){
            this.name <- camelCode(c(JS.prefix, JS.var, this.modif))
          } else {
            this.name <- camelCode(c(JS.prefix, JS.var))
          }
          if(isTRUE(names.only)){
            return(this.name)
          } else {
            return(paste0(main.indent, ifelse(isTRUE(var), "var ", ""), this.name,
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


## function paste.JS.optionsset()
paste.JS.optionsset <- function(object, level=2, indent.by=rk.get.indent()){
  stopifnot(inherits(object, "rk.JS.oset"))
  # check indentation
  main.indent <- indent(level, by=indent.by)
  scnd.indent <- indent(level+1, by=indent.by)
  thrd.indent <- indent(level+2, by=indent.by)

  vars <- slot(object, "vars")
  loopvar <- slot(object, "loopvar")
  columns <- slot(object, "columns")
  body <- slot(object, "body")
  collapse <- slot(object, "collapse")

  if(length(slot(vars, "vars")) > 0 | length(slot(vars, "JS.var")) > 0 ){
    paste.vars <- paste.JS.var(vars, level=level, indent.by=indent.by)
  } else {
    paste.vars <- c()
  }

  # if there's no body, we don't need a loop
  if(length(body) > 0){
    ## the for loop body
    for.head <- paste0(main.indent, "for (var ", loopvar, " = 0; ", loopvar, " < ", id(columns[[1]]), ".length; ++", loopvar, "){")

    paste.body <- sapply(body, function(bodyPart){
        rk.paste.JS(bodyPart, level=level, indent.by=scnd.indent)
      })
    # replace the column IDs with indexed ones
    for (thisCol in sapply(columns, id)){
      paste.body <- gsub(
        paste0("([^[:alnum:]]+|^)", thisCol, "([^[:alnum:]]+|$)"),
        paste0("\\1", thisCol, "[", loopvar, "]\\2"),
        paste.body, perl=TRUE)
    }

    for.foot <- paste0(
      scnd.indent, "if(", loopvar, " + 1 < ", id(columns[[1]]), ".length) {\n",
      thrd.indent, "echo(\"", collapse, "\");\n",
      scnd.indent, "}\n",
      main.indent, "}"
    )
    
    results <- paste(c(paste.vars, for.head, paste.body, for.foot), collapse="\n")
  } else {
    results <- paste.vars
  }
  return(results)
} ## end function paste.JS.optionsset()


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
#       label=c(val=NULL, chk=FALSE, i18n=NULL)
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
        if("val" %in% names(options[[this.num]])){
          value <- options[[this.num]][["val"]]
        } else {
          value <- NULL
        }
        if("chk" %in% names(options[[this.num]])){
          checked <- isTRUE(as.logical(options[[this.num]][["chk"]]))
        } else {
          checked <- FALSE
        }
        if("i18n" %in% names(options[[this.num]])){
          i18n <- isTRUE(as.logical(options[[this.num]][["i18n"]]))
        } else {
          i18n <- NULL
        }
        return(
          rk.XML.option(
            label=names(options)[[this.num]],
            val=value,
            chk=checked,
            id.name=NULL,
            i18n=i18n
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
        thisID <- c(XML=opt.id, JS=id(options[[this.num]]))
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


## function check.i18n()
# checks for additional i18n info in XiMpLe nodes. returns either an appended or altered list of
# attributes, or a XiMpLe node with an i18n comment
# i18n: either a list with possible named elements "context" or "comment",
#   or a charcter string (for wich it is assumed to describe a context),
#   or FALSE; if the latter, "label" will be renamed to "noi18n_label", "title" to "noi18n_title"
# attrs: a list of previously defined attributes
# comment: if TRUE, returns a pseudo node (with name "") containing a comment node and the original node,
#   else a list of attributes
check.i18n <- function(i18n=NULL, attrs=list(), node=NULL, comment=FALSE){
  if(isTRUE(comment)){
    result <- node
  } else {
    result <- attrs
  }
  if(is.null(i18n)){
    return(result)
  } else {
    if(is.list(i18n)){
      if(!all(names(i18n) %in% c("comment", "context"))){
        stop(simpleError("i18n: only elements named \"comment\" or \"context\" are supported!"))
      } else {}
      if(isTRUE(comment)){
        if("comment" %in% names(i18n)){
          if(!is.XiMpLe.node(node)){
            stop(simpleError("i18n: to add a \"comment\" to a node, an XML node must be present!"))
          } else {}
          result <- XMLNode("", rk.i18n.comment(i18n[["comment"]]), node)
        } else {}
      } else {
        if("context" %in% names(i18n)){
          result[["i18n_context"]] <- i18n[["context"]]
        } else{}
      }
    } else if(is.character(i18n) & length(i18n) == 1 & !isTRUE(comment)){
      result[["i18n_context"]] <- i18n[[1]]
    } else if(is.logical(i18n) & !isTRUE(i18n) & !isTRUE(comment)){
      if("label" %in% names(result)){
        names(result)[names(result) == "label"] <- "noi18n_label"
      } else {}
      if("title" %in% names(result)){
        names(result)[names(result) == "title"] <- "noi18n_title"
      } else {}
    } else {}
  }
  return(result)
} ## end function check.i18n()


## function force.i18n
# ensures that obj is encapsulated in i18n() in the output
force.i18n <- function(obj){
  stopifnot(length(obj) == 1)
  if(inherits(obj, "rk.JS.i18n")){
    result <- slot(obj, "value")
  } else if(is.character(obj)){
    result <- paste0("i18n(\"", obj, "\")")
  } else {
    stop(simpleError(paste0("force.i18n: don't know how to deal with an object of class ", class(obj), "!")))
  }
  return(result)
} ## end function force.i18n


## function check.JS.lines()
# called by rk.JS.scan()
check.JS.lines <- function(relevant.tags, single.tags, add.abbrev, js, indent.by, guess.getter,
  tag.names=TRUE, modifiers=NULL, only.checkable=FALSE, append.modifier=TRUE, result=NULL){

  JS.id <- get.IDs(single.tags=single.tags, relevant.tags=relevant.tags, add.abbrev=add.abbrev,
    tag.names=tag.names, only.checkable=only.checkable)

  if("id" %in% colnames(JS.id)){
    if(isTRUE(js)){
      # now
      #   <tag id="my.id" ...>
      # will become
      #   var my.id = getValue("my.id");
      result <- paste(result, paste(unlist(sapply(1:nrow(JS.id), function(this.id){
            return(rk.paste.JS(get.JS.vars(
              JS.var=JS.id[this.id,"abbrev"],
              XML.var=JS.id[this.id,"id"],
              tag.name=JS.id[this.id,"tag"],
              modifiers=modifiers,
              append.modifier=append.modifier,
              guess.getter=guess.getter),
              level=2, indent.by=indent.by))
          }, USE.NAMES=FALSE)), collapse="\n"),
        sep="\n", collapse="\n")
    } else {
      result <- c(result, JS.id[,"id"])
      names(result) <- NULL
    }
  } else {}
  return(result)
} ## end function check.JS.lines()


## JS.operators
# a complilation of operators we would like to fetch from R calls and 
# substitute with character equivalents for JS code
JS.operators <- c(
  "+", "-", "*", "/", "%",
  "++", "--", "=", "+=", "-=", "*=", "/=", "%=",
  "==", "===", "!=", "!==", ">", "<", ">=", "<=",
  "!", "||", "&&"
) ## end JS.operators
# currently not working: "%", "++", "--", "=", "+=", "-=", "*=", "/=", "%=", "===", "!==", "!"


## function replaceJSOperators
# takes arbitrary R code and tries to replace R operators with character strings.
# makes it possible to use these operators in calls like id() without the need
# for quoting them
replaceJSOperators <- function(..., call="id"){
  dotlist <- eval(substitute(alist(...)))
  dotlist <- lapply(
    dotlist,
    function(thisItem){
      # operators like ">" or "|" are represented as call objects
      # with the operator as first argument (name).
      # there can also be calls nested in calls so we need to test this recursively
      if(inherits(thisItem, "call")){
        callList <- unlist(thisItem)
        if(as.character(callList[[1]]) %in% JS.operators){
          result <- list(
            # the "!" operator needs to come first
            if(as.character(callList[[1]]) %in% "!"){
              paste0(as.character(callList[[1]]))
            } else {},
            if(is.call(callList[[2]])){
              do.call("replaceJSOperators", list(callList[[2]]))
            } else if(is.character(callList[[2]])){
              paste0("\"", callList[[2]], "\"")
            } else if(is.name(callList[[2]])){
              # if this gets called inside a local() call, make sure we fetch the referenced object at all
              fetchedObject1 <- dynGet(as.character(callList[[2]]), ifnotfound=get(as.character(callList[[2]])))
              do.call(call, list(fetchedObject1))
            } else {
              do.call(call, list(callList[[2]]))
            },
            # all except the "!" operator come here
            if(!as.character(callList[[1]]) %in% "!"){
              paste0(" ", as.character(callList[[1]]), " ")
            } else {},
            # operators like "!" don't have a third element
            if(length(callList) > 2){
              if(is.call(callList[[3]])){
                do.call("replaceJSOperators", list(callList[[3]]))
              } else if(is.character(callList[[3]])){
                paste0("\"", callList[[3]], "\"")
              } else if(is.name(callList[[3]])){
                # same as fetchedObject1 above
                fetchedObject2 <- dynGet(as.character(callList[[3]]), ifnotfound=get(as.character(callList[[3]])))
                do.call(call, list(fetchedObject2))
              } else {
                do.call(call, list(callList[[3]]))
              }
            } else {}
          )
          return(paste0(unlist(result), collapse=""))
        } else {
          # replace object names with the actual objects for evaluation
          if(length(thisItem) > 1){
            for (itemParts in 2:length(thisItem)){
              if(is.name(thisItem[[itemParts]])){
                thisItem[[itemParts]] <- dynGet(as.character(thisItem[[itemParts]]), ifnotfound=get(as.character(thisItem[[itemParts]])))
              } else {}
            }
          } else {}
          thisItem <- eval(thisItem)
          # R vectors don't make much sense, collapse them for JS
          if(is.vector(thisItem)){
            thisItem <- paste0(thisItem, collapse=", ")
          } else {}
          return(thisItem)
        }
      } else {
        return(thisItem)
      }
    }
  )
  return(unlist(dotlist))
} ## end function replaceJSOperators


## function uncurl()
# used by js() to fetch calls from then/else segments of if conditions,
# omitting curly brackets that would get in the way with ite()
uncurl <- function(cond, level=1, indent.by=rk.get.indent()){
  if(!is.null(cond)){
    cond.list <- as.list(cond)
    # first check for the bracket
    if(identical(as.character(cond[[1]]), "{")){
      # now make sure the bracket isn't empty
      if(length(cond) > 1){
        cond <- paste0(
          sapply(
            2:length(cond.list),
            function(this.cond.num){
              do.call("js", args=list(cond[[this.cond.num]], level=level, by=indent.by))
            }
          ),
          collapse=paste0("\n", indent(level=level, by=indent.by))
        )
      } else {
        cond <- ""
      }
    } else {
      cond <- do.call("js", args=list(cond, level=level, by=indent.by))
    }
  } else {}
  return(cond)
} ## end function uncurl()


## function replaceJSIf
replaceJSIf <- function(cond, level=1, paste=TRUE, indent.by=rk.get.indent(), empty.e=FALSE){
  if(inherits(cond, "if")){
    # if condition -- should be save to give to js()
    cond.if   <- do.call(
      "js",
      args=list(
        cond[[2]],
        level=level,
        indent.by=indent.by,
        linebreaks=FALSE,
        empty.e=empty.e
      )
    )
    # then do -- could be nested with another if condition
    if(inherits(cond[[3]], "if")){
      cond.then <- replaceJSIf(cond[[3]], level=level+1, paste=FALSE, indent.by=indent.by, empty.e=empty.e)
    } else {
      cond.then <- do.call(
        "js",
        args=list(
          uncurl(cond[[3]], level=level+1, indent.by=indent.by),
          level=level,
          indent.by=indent.by,
          linebreaks=FALSE,
          empty.e=empty.e
        )
      )
    }
    # else do -- could be missing or yet another if condition
    cond.else <- NULL
    if(length(as.list(cond)) > 3){
      if(inherits(cond[[4]], "if")){
        cond.else <- replaceJSIf(cond[[4]], level=level+1, paste=FALSE, indent.by=indent.by, empty.e=empty.e)
      } else {
        cond.else <- do.call(
          "js",
          args=list(
            uncurl(cond[[4]], level=level+1, indent.by=indent.by),
            level=level,
            indent.by=indent.by,
            linebreaks=FALSE,
            empty.e=empty.e
          )
        )
      }
    } else {}

    iteObject <- ite(
      ifjs=cond.if,
      thenjs=cond.then,
      elsejs=cond.else 
    )
    if(isTRUE(paste)){
      # the pasted result needs to be trimmed, because js() adds indentation itself
      return(trim(rk.paste.JS(iteObject, level=level, indent.by=indent.by, empty.e=empty.e)))
    } else {
      return(iteObject)
    }
  } else {
    cond <- do.call(
      "js",
      args=list(cond, level=level, indent.by=indent.by, linebreaks=FALSE, empty.e=empty.e)
    )
    return(cond)
  }
} ## end function replaceJSIf


## function replaceJSFor
# this function is currently not publicly announced, but is available through the js() function
# 
#<documentation> 
# Using \code{for} loops is a bit more delicate, as they are very differently constructed in JavaScript. As
# a workaround, \code{js} will define an array and a counter variable with randomly generated names, fill
# the array with the values you provided and iterate through the array. In order to keep the iterator variable
# name you used in the original R loop, so you can use it inside the loop body, you will have to define it before
# the \code{js} call with a substitution of itself (see examples). Otherwise, you will get an "object not found" error.
#
# example:
# # let's try preserving a for loop
# # to use iterator variable i, we must initialize it first
# i <- substitute(i)  # DON'T FORGET THIS!
# cat(rk.paste.JS(js(
#   for (i in 1:10) {
#     echo(i)
#   }
# )))
#</documentation> 
replaceJSFor <- function(loop, level=1, indent.by=rk.get.indent()){
  if(inherits(loop, "for")){
    # for loops must be handled differently, we need to create an array
    # first and then interate through the array to imitate ho R does this
    # 
    # also, the array and iterator variables must not be named like any
    # of the given variables/objects. therefore, we will use some randomly
    # generated character strings for those
    arrayName <- paste0("a", paste0(sample(c(letters,LETTERS,0:9), 5, replace=TRUE), collapse=""))
    iterName <- paste0("i", paste0(sample(c(letters,LETTERS,0:9), 5, replace=TRUE), collapse=""))
    loop <- paste(
      paste0(indent(level=level, by=indent.by), "// the variable names \"", arrayName, "\" and \"", iterName, "\" were randomly generated"),
      paste0("var ", arrayName, " = new Array();"),
      paste0(arrayName, ".push(", do.call("js", args=list(loop[[3]], level=level, indent.by=indent.by)), ");"),
      paste0("for (var ", as.character(loop[[2]]), "=", arrayName, "[0], ", iterName, "=0; ",
        iterName, " < ", arrayName, ".length; ",
        iterName, "++, ", as.character(loop[[2]]), "=", arrayName, "[", iterName, "]) {"),
      paste0(
        indent(level=level, by=indent.by),
        do.call(
          "js",
          args=list(
            uncurl(loop[[4]], level=level+1, indent.by=indent.by),
            level=level,
            indent.by=indent.by
          )
        )
      ),
      "}\n",
      sep=paste0("\n", indent(level=level, by=indent.by))
    )
  } else {
    loop <- do.call("js", args=list(loop, level=level, indent.by=indent.by))
    return(loop)
  }
} ## end function replaceJSFor
