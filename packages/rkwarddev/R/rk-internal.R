# internal functions for the rk.* functions

# info message
generator.info <- rk.comment(paste("this code was generated using the rkwarddev package.\n",
			"perhaps don't make changes here, but in the rkwarddev script instead!", sep=""))

## function auto.ids()
auto.ids <- function(identifiers, prefix=NULL, suffix=NULL, chars=8){
	identifiers <- gsub("[[:space:]]*[^[:alnum:]]*", "", identifiers)
	id.names <- ifelse(nchar(identifiers) > 8, abbreviate(identifiers, minlength=chars), identifiers)
	# check for uniqueness
	if(any(duplicated(id.names))){
		warning("IDs are not unique, please check!")
	} else {}
	ids <- paste(prefix, id.names, suffix, sep="")
	return(ids)
} ## end function auto.ids()


## function child.list()
# convenience function to let single children be provided without list()
# 'empty' van be used to make sure a tag is non-empty without actual value
child.list <- function(children, empty=TRUE){
	if(inherits(children, "XiMpLe.node")){
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
			action <- paste(action, ": ", sep="")
		} else {}
		warning(paste(action, "Skipping existing file ", file.name, ".", sep=""), call.=FALSE)
		return(FALSE)
	}
} ## end function checkCreateFiles()


## function get.single.tags()
get.single.tags <- function(XML.obj, drop=NULL){
	# determine if we need to read a file or process an XiMpLe object
	if(inherits(XML.obj, "XiMpLe.doc")){
		single.tags <- trim(unlist(strsplit(pasteXMLTree(XML.obj, shine=1, indent.by=""), split="\n")))
	} else if(inherits(XML.obj, "XiMpLe.node")){
		single.tags <- trim(unlist(strsplit(pasteXML(XML.obj, shine=1, indent.by=""), split="\n")))
	} else {
		xml.raw <- paste(readLines(XML.obj), collapse=" ")
		single.tags <- XiMpLe:::XML.single.tags(xml.raw, drop=drop)
	}
	names(single.tags) <- NULL

	return(single.tags)
} ## end function get.single.tags()


## function get.IDs()
# scans XML tags for defined IDs, returns a matrix with columns "id" and "abbrev",
# and optional "tag"
# 'single.tags' can also contain XiMpLe.node objects
get.IDs <- function(single.tags, relevant.tags, add.abbrev=FALSE, tag.names=FALSE, only.checkable=FALSE){

	# filter for relevant tags
	cleaned.tags <- list()
	for(this.tag in child.list(single.tags)){
		if(inherits(this.tag, "XiMpLe.node")){
			this.tag.name <- slot(this.tag, "name")
			if(this.tag.name %in% relevant.tags & "id" %in% names(slot(this.tag, "attributes"))){
				if(isTRUE(only.checkable) & this.tag.name %in% "frame"){
					if("checkable" %in% names(slot(this.tag, "attributes"))){
						if(identical(slot(this.tag, "attributes")[["checkable"]], "true")){
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
				if(inherits(this.tag, "XiMpLe.node")){
					this.tag.name <- slot(this.tag, "name")
					this.tag.id <- slot(this.tag, "attributes")["id"]
				} else {
					this.tag.name <- XiMpLe:::XML.tagName(this.tag)
					this.tag.id <- XiMpLe:::parseXMLAttr(this.tag)[["id"]]
				}

				if(isTRUE(add.abbrev)){
					this.tag.id.abbrev <- paste(ID.prefix(this.tag.name), this.tag.id, sep="")
				} else {
					this.tag.id.abbrev <- this.tag.id
				}
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
			warning(paste("IDs are not unique:\n  ", paste(ids[multiple.id,"id"], collapse=", "), "\n  Expect errors!", sep=""))
		} else {}
	}

	return(ids)
} ## end function get.IDs()


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

	results <- paste(words[1], paste(new.words, collapse=""), sep="")

	return(results)
} ## end function camelCode()


## function get.JS.vars()
#   <tag id="my.id" ...>
# in XML will become
#   var my.id = getValue("my.id");
get.JS.vars <- function(JS.var, XML.var=NULL, JS.prefix="", names.only=FALSE, modifiers=NULL, default=FALSE, join="", check.modifiers=TRUE){
	# check for XiMpLe nodes
	JS.var <- check.ID(JS.var)
	if(!is.null(XML.var)){
		# check validity of modifiers value
		if(!is.null(modifiers)){
			if(identical(modifiers, "all")){
				if(inherits(XML.var, "XiMpLe.node")){
					tag.name <- slot(XML.var, "name")
				} else {
					tag.name <- XML.var
				}
				if(tag.name %in% names(all.valid.modifiers)){
					modifiers <- all.valid.modifiers[[tag.name]]
				} else {
					modifiers <- NULL
				}
			} else {
				if(inherits(XML.var, "XiMpLe.node")){
					modif.tag.name <- slot(XML.var, "name")
				} else {
					modif.tag.name <- "all"
				}
				if(isTRUE(check.modifiers)){
					modifiers <- modifiers[modif.validity(modif.tag.name, modifier=child.list(modifiers), warn.only=TRUE, bool=TRUE)]
				} else {}
			}
		} else {}
		XML.var <- check.ID(XML.var)
	} else {
		XML.var <- check.ID(JS.var)
	}

	if(is.null(JS.prefix)){
		JS.prefix <- ""
	} else {}

	if(isTRUE(names.only)){
		results <- c()
		if(is.null(modifiers) | isTRUE(default)){
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
			join=join)
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
		prfx <- paste(prfx, ".", sep="")
	} else {
		prfx <- paste(prfx, "_", sep="")
	}
	return(prfx)
} ## end function ID.prefix()


## function node.soup()
# pastes the nodes as XML, only alphanumeric characters, e.g. to generate auto-IDs
node.soup <- function(nodes){
	the.soup <- paste(unlist(sapply(child.list(nodes), function(this.node){
			if(inherits(this.node, "XiMpLe.node")){
				return(gsub("[^[:alnum:]]", "", pasteXML(this.node, shine=0)))
			} else {
				stop(simpleError("Nodes must be of class XiMpLe.node!"))
			}
		})), sep="", collapse="")
	return(the.soup)
} ## end function node.soup()


## function XML2person()
# extracts the person/author info from XML "about" nodes
XML2person <- function(node, eval=FALSE){
		if(inherits(node, "XiMpLe.node")){
			# check if this is *really* a about section, otherwise die of boredom
			if(!identical(slot(node, "name"), "about")){
				stop(simpleError("I don't know what this is, but 'about' is not an about section!"))
			} else {}
		} else {
			stop(simpleError("'about' must be a XiMpLe.node, see ?rk.XML.about()!"))
		}
	make.vector <- function(value){
		if(grepl(",", value)){
			value <- paste("c(\"", paste(trim(unlist(strsplit(value, ","))), collapse="\", \""), "\")", sep="")
		} else {
			value <- paste("\"", value, "\"", sep="")
		}
		return(value)
	}
	all.authors <- c()
	for (this.child in slot(node, "children")){
		if(identical(slot(this.child, "name"), "author")){
			attrs <- slot(this.child, "attributes")
			given <- make.vector(attrs[["given"]])
			family <- make.vector(attrs[["family"]])
			email <- make.vector(attrs[["email"]])
			role <- make.vector(attrs[["role"]])
			this.author <- paste("person(given=", given, ", family=", family, ", email=", email, ", role=", role, ")", sep="")
			all.authors[length(all.authors) + 1] <- this.author
		} else {}
	}
	if(length(all.authors) > 1){
		all.authors <- paste("c(", paste(all.authors, collapse=", "), ")", sep="")
	} else {}
	if(isTRUE(eval)){
		all.authors <- eval(parse(text=all.authors))
	} else {}
	return(all.authors)
} ## end function XML2person()


## function XML2dependencies()
# extracts the package dependencies info from XML "about" nodes
# in "suggest" mode only suggestions will be returned, in "depends" mode only dependencies.
# suggest=TRUE: Depends: R & RKWard; Suggests: packages
# suggest=FALSE: Depends: R & RKWard & packages; suggests: none
XML2dependencies <- function(node, suggest=TRUE, mode="suggest"){
	if(!isTRUE(suggest) & identical(mode, "suggest")){
		return("")
	} else {}
	if(inherits(node, "XiMpLe.node")){
		# check if this is *really* a about section, otherwise die of boredom
		if(!identical(slot(node, "name"), "about")){
			stop(simpleError("I don't know what this is, but 'about' is not an about section!"))
		} else {}
	} else {
		stop(simpleError("'about' must be a XiMpLe.node, see ?rk.XML.about()!"))
	}
	check.deps <- sapply(slot(node, "children"), function(this.child){identical(slot(this.child, "name"), "dependencies")})
	if(any(check.deps)){
		got.deps <- slot(node, "children")[[which(check.deps)]]
		deps.packages <- list()
		# first see if RKWard and R versions are given
		deps.RkR <- slot(got.deps, "attributes")
		deps.RkR.options  <- names(deps.RkR)
		R.min <- ifelse("R_min_version" %in% deps.RkR.options, paste(">= ", deps.RkR[["R_min_version"]], sep=""), "")
		R.max <- ifelse("R_max_version" %in% deps.RkR.options, paste("< ", deps.RkR[["R_max_version"]], sep=""), "")
		R.version.indices <- sum(!identical(R.min, ""), !identical(R.max, ""))
		if(R.version.indices > 0 & identical(mode, "depends")){
			deps.packages[[length(deps.packages) + 1]] <- paste("R (", R.min, ifelse(R.version.indices > 1, ", ", ""), R.max, ")", sep="")
		} else {}
		Rk.min <- ifelse("rkward_min_version" %in% deps.RkR.options, paste(">= ", deps.RkR[["rkward_min_version"]], sep=""), "")
		Rk.max <- ifelse("rkward_max_version" %in% deps.RkR.options, paste("< ", deps.RkR[["rkward_max_version"]], sep=""), "")
		Rk.version.indices <- sum(!identical(Rk.min, ""), !identical(Rk.max, ""))
		if(Rk.version.indices > 0 & identical(mode, "depends")){
			deps.packages[[length(deps.packages) + 1]] <- paste("rkward (", Rk.min, ifelse(Rk.version.indices > 1, ", ", ""), Rk.max, ")", sep="")
		} else {}
		check.deps.pckg <- sapply(slot(got.deps, "children"), function(this.child){identical(slot(this.child, "name"), "package")})
		if(any(check.deps.pckg & ((isTRUE(suggest) & identical(mode, "suggest")) | !isTRUE(suggest)))){
			deps.packages[[length(deps.packages) + 1]] <- paste(sapply(which(check.deps.pckg), function(this.pckg){
					this.pckg.dep <- slot(slot(got.deps, "children")[[this.pckg]], "attributes")
					pckg.options <- names(this.pckg.dep)
					pckg.name <- this.pckg.dep[["name"]]
					pckg.min <- ifelse("min" %in% pckg.options, paste(">= ", this.pckg.dep[["min"]], sep=""), "")
					pckg.max <- ifelse("max" %in% pckg.options, paste("< ", this.pckg.dep[["max"]], sep=""), "")
					version.indices <- sum(!identical(pckg.min, ""), !identical(pckg.max, ""))
					if(version.indices > 0){
						pckg.version <- paste(" (", pckg.min, ifelse(version.indices > 1, ", ", ""), pckg.max, ")", sep="")
					} else {
						pckg.version <- ""
					}
					return(paste(pckg.name, pckg.version, sep=""))
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
check.ID <- function(node){
	if(is.list(node)){
		return(sapply(node, check.ID))
	} else {}

	if(inherits(node, "XiMpLe.node")){
		node.ID <- slot(node, "attributes")[["id"]]
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
	all=c("", "visible", "enabled", "required", "true", "false", "not", "numeric",
	"preprocess", "calculate", "printout", "preview"),
	text=c("text"),
	varselector=c("selected", "root"),
	varslot=c("available", "selected", "source", "shortname", "label"),
	radio=c("string", "number"),
	dropdown=c("string", "number"),
	# option=c(),
	checkbox=c("state"),
	frame=c("checked"),
	input=c("text"),
	browser=c("selection"),
	saveobject=c("selection", "parent", "objectname", "active"),
	spinbox=c("int", "real"),
	formula=c("model", "table", "labels", "fixed_factors", "dependent"),
# removed embed, can be all sorts of stuff, see e.g. generic plot options
#	embed=c("code"),
	preview=c("state")
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

	if(inherits(source, "XiMpLe.node")){
		tag.name <- slot(source, "name")
		# embedded plugins can have all sorts of modifiers
		if(identical(tag.name, "embed")){
			if(isTRUE(bool)){
				return(TRUE)
			} else {
				return(modifier)
			}
		} else {}
	} else if(identical(source, "all")){
		tag.name <- "<any tag>"
	} else {
		if(isTRUE(bool)){
			return(TRUE)
		} else {
			return(modifier)
		}
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
			warning(paste("Some modifier you provided is invalid for '",tag.name,"' and was ignored: ",
				paste(modifier[invalid.modif], collapse=", "), sep=""), call.=FALSE)
			if(isTRUE(bool)){
				return(!invalid.modif)
			} else {
				return("")
			}
		} else {
			stop(simpleError(paste("Some modifier you provided is invalid for '",tag.name,"' and was ignored: ",
				paste(modifier[invalid.modif], collapse=", "), sep="")))
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
	components=c("component"),
	context=c("menu", "!--"),
	dialog=c("browser", "checkbox", "column", "copy",
		"dropdown", "embed", "formula", "frame", "include", "input", "insert",
		"preview", "radio", "row", "saveobject", "spinbox", "stretch", "tabbook",
		"text", "varselector", "varslot", "!--"),
	hierarchy=c("menu", "!--"),
	logic=c("connect", "convert","include","insert","external","set","script"),
	menu=c("entry", "menu", "!--"),
	page=c("browser", "checkbox", "column", "copy",
		"dropdown", "formula", "frame", "input", "page", "radio", "row", "saveobject",
		"spinbox", "stretch", "tabbook", "text", "varselector", "varslot", "!--"),
	settings=c("setting", "caption", "!--"),
	wizard=c("browser", "checkbox", "column", "copy",
		"dropdown", "embed", "formula", "frame", "include", "input", "insert",
		"page", "preview", "radio", "row", "saveobject", "spinbox", "stretch",
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
				if(inherits(this.child, "XiMpLe.node")){
					return(slot(this.child, "name"))
				} else {
					stop(simpleError(paste("Invalid object for ", section, " section, must be of class XiMpLe.node, but got class ", class(this.child), "!", sep="")))
				}
			})
	} else {}

	invalid.sets <- !node.names %in% all.valid.children[[parent]]
	if(any(invalid.sets)){
		return.message <- paste("Invalid XML nodes for ", section, " section: ", paste(node.names[invalid.sets], collapse=", "), sep="")
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
valid.parent <- function(parent, node, warn=FALSE, see=NULL){
	if(inherits(node, "XiMpLe.node")){
		node.name <- slot(node, "name")
		if(identical(node.name, parent)){
			return(TRUE)
		} else {
			return.message <- paste("I don't know what this is, but '", parent, "' is not a <", parent, "> section!", sep="")
			if(isTRUE(warn)){
				warning(return.message)
				return(FALSE)
			} else {
				stop(simpleError(return.message))
			}
		}
	} else {
		stop(simpleError(
				paste("'", parent, "' must be a XiMpLe.node",
					if(!is.null(see)){paste(", see ?", see, sep="")},
					"!", sep=""))
			)
	}
} ## end function valid.parent()


## function check.type()
check.type <- function(value, type, var.name, warn.only=TRUE){
	if(inherits(value, type)){
		return(invisible(NULL))
	} else {
		msg.text <- paste(sQuote(var.name), " should be of type ", type, "!", sep="")
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
			message(paste("For filenames ", sQuote(name.orig), " was renamed to ", sQuote(name), ".", sep=""))
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
		ifJS <- paste("if(", slot(object, "ifJS"), ") {\n", sep="")
	} else {
		ifJS <- paste(main.indent, "if(", slot(object, "ifJS"), ") {\n", sep="")
	}

	if(nchar(slot(object, "thenJS")) > 0) {
		# chop off beginning indent strings, otherwiese they ruin the code layout
		thenJS.clean <- gsub(paste("^", indent.by, "*", sep=""), "", slot(object, "thenJS"))
		thenJS <- paste(scnd.indent, thenJS.clean, "\n", main.indent, "}", sep="")
	} else {
		# if there is another rk.JS.ite object, call with recursion
		if(length(slot(object, "thenifJS")) == 1){
			thenJS <- paste(paste.JS.ite(slot(object, "thenifJS")[[1]], level=level+1, indent.by=indent.by), "\n", main.indent, "}", sep="")
		} else {}
	}

	if(nchar(slot(object, "elseJS")) > 0) {
		# chop off beginning indent strings, otherwiese they ruin the code layout
		elseJS.clean <- gsub(paste("^", indent.by, "*", sep=""), "", slot(object, "elseJS"))
		elseJS <- paste(" else {\n", scnd.indent, elseJS.clean, "\n", main.indent, "}", sep="")
	} else {
		# if there is another rk.JS.ite object, call with recursion
		if(length(slot(object, "elifJS")) == 1){
			elseJS <- paste(" else ", paste.JS.ite(slot(object, "elifJS")[[1]], level=level, indent.by=indent.by, recurse=TRUE), sep="")
		} else {
			if(isTRUE(empty.e)){
				# close for sure with an empty "else"
				elseJS <- " else {}"
			} else {
				elseJS <- NULL
			}
		}
	}

	result <- paste(ifJS, thenJS, elseJS, collapse="", sep="")

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
	option    <- slot(object, "option")
	if(is.null(funct)){
		funct <- slot(object, "funct")
	} else {}
	if(is.null(funct) | identical(funct, "")){
		funct.start <- ""
		funct.end <- ""
	} else {
		funct.start <- paste(funct, "(", sep="")
		funct.end <- ")"
	}

	JS.array <- paste(
		main.indent, "// define the array ", arr.name, " for values of R option \"", option, "\"\n",
		main.indent, "var ", arr.name, " = new Array();\n",
		main.indent, arr.name, ".push(",
		paste(variables, collapse=", "), ");\n",
		main.indent, "// clean array ", arr.name, " from empty strings\n",
		main.indent, arr.name, " = ", arr.name, ".filter(String);\n",
		main.indent, "// set the actual variable ", opt.name,
		ifelse(identical(option, ""), "", paste(" for R option \"", option, sep="")),
		ifelse(identical(funct, ""), "\"", paste("=", funct, "()\"", sep="")), "\n",
		main.indent, "if(", arr.name, ".length > 0) {\n",
		scnd.indent, "var ", opt.name, " = \", ",
		ifelse(identical(option, ""), "", paste(option, "=", sep="")),
		funct.start, "\" + ", arr.name, ".join(\", \") + \"",funct.end,"\";\n",
		main.indent, "} else {\n",
		scnd.indent, "var ", opt.name, " = \"\";\n",
		main.indent, "}\n",
		sep="")

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
		funct.start <- paste(funct, "(", sep="")
		funct.end <- ")"
	}

	# a function to add the object stuff to ite objects
	add.opts <- function(this.ite, collapse, array){
		if(isTRUE(array)){
			slot(this.ite, "thenJS") <- paste(arr.name, ".push(", slot(this.ite, "thenJS"),");", sep="")
		} else {
			slot(this.ite, "thenJS") <- paste(variable, " += ", collapse, slot(this.ite, "thenJS"),";", sep="")
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

	JS.options <- paste(
		if(isTRUE(array)){
			paste(
				main.indent, "// define the array ", arr.name, " for values of R option \"", option, "\"\n",
				main.indent, "var ", arr.name, " = new Array();\n", sep="")
		} else {
			paste(main.indent, "var ", variable, " = \"\";\n", sep="")
		},
		paste(ifs.pasted, sep="", collapse="\n"), "\n",
		if(isTRUE(array)){
			paste(
				main.indent, "// clean array ", arr.name, " from empty strings\n",
				main.indent, arr.name, " = ", arr.name, ".filter(String);\n",
				main.indent, "// set the actual variable ", variable, " with all values for R option \"", option, "\"\n",
				main.indent, "if(", arr.name, ".length > 0) {\n",
				scnd.indent, "var ", variable, " = \"", collapse,
				ifelse(identical(option, ""), "", paste(option, "=", sep="")),
				funct.start, "\" + ", arr.name, ".join(\", \") + \"",funct.end,"\";\n",
				main.indent, "} else {\n",
				scnd.indent, "var ", variable, " = \"\";\n",
				main.indent, "}\n",
				sep="")
		} else {},
		sep="")

	return(JS.options)
} ## end function paste.JS.options()


## function paste.JS.var()
paste.JS.var <- function(object, level=2, indent.by="\t", JS.prefix=NULL, modifiers=NULL, default=NULL, join=NULL, names.only=FALSE, check.modifiers=FALSE){
	# paste several objects
	results <- unlist(sapply(slot(object, "vars"), function(this.obj){
			paste.JS.var(this.obj,
					level=level,
					indent.by=indent.by,
					JS.prefix=JS.prefix,
					modifiers=modifiers,
					default=default,
					join=join,
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

	if(!identical(join, "")){
		join.code <- paste(".split(\"\\n\").join(\"", join, "\")", sep="")
	} else {
		join.code <- ""
	}

	# only paste something if there's variables outside the 'vars' slot
	if(length(nchar(JS.var)) > 0 & length(nchar(XML.var)) > 0){
		if(length(modifiers) == 0 | isTRUE(default)){
			if(isTRUE(names.only)){
				results <- c(results, camelCode(c(JS.prefix, JS.var)))
			} else {
				results <- paste(main.indent, "var ", camelCode(c(JS.prefix, JS.var)), " = getValue(\"", XML.var, "\")", join.code, ";", sep="")
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
						return(paste(main.indent, "var ", camelCode(c(JS.prefix, JS.var, this.modif)),
							" = getValue(\"", XML.var, ".", this.modif, "\")", join.code, ";", sep=""))
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
