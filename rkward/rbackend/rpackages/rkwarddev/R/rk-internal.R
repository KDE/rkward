# internal functions for the rk.* functions

auto.ids <- function(identifiers, prefix=NULL, chars=8){
	identifiers <- gsub("[[:space:]]*[^[:alnum:]]*", "", identifiers)
	id.names <- ifelse(nchar(identifiers) > 8, abbreviate(identifiers, minlength=chars), identifiers)
	# check for uniqueness
	if(any(duplicated(id.names))){
		warning("IDs are not unique, please check!")
	} else {}
	ids <- paste(prefix, id.names, sep="")
	return(ids)
}

# convenience function to let single children be provided without list()
child.list <- function(children){
	if(inherits(children, "XiMpLe.node")){
		children <- list(children)
	} else {}
	return(children)
}

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

## function get.single.tags()
get.single.tags <- function(XML.obj, drop=NULL){
	# determine if we need to read a file or process an XiMpLe object
	if(inherits(XML.obj, "XiMpLe.doc")){
		single.tags <- trim(unlist(strsplit(pasteXMLTree(XML.obj, shine=1, indent.by=""), split="\n")))
	} else if(inherits(XML.obj, "XiMpLe.node")){
		single.tags <- trim(unlist(strsplit(pasteXMLNode(XML.obj, shine=1, indent.by=""), split="\n")))
	} else {
		xml.raw <- paste(readLines(XML.obj), collapse=" ")
		single.tags <- XiMpLe:::XML.single.tags(xml.raw, drop=drop)
	}
	names(single.tags) <- NULL

	return(single.tags)
} ## end function get.single.tags()


## function get.IDs()
# scans XML tags for defined IDs, returns a matrix with columns "id" and "abbrev"
get.IDs <- function(single.tags, relevant.tags, add.abbrev){

	single.tags <- single.tags[tolower(XiMpLe:::XML.tagName(single.tags)) %in% relevant.tags]
	# we're only interested in entries with an ID
	single.tags <- single.tags[grepl("[[:space:]]+id=", single.tags)]

	ids <- t(sapply(single.tags, function(this.tag){
			this.tag.name <- XiMpLe:::XML.tagName(this.tag)
			this.tag.id <- XiMpLe:::parseXMLAttr(this.tag)[["id"]]
				if(isTRUE(add.abbrev)){
					this.tag.abbrev <- abbreviate(this.tag.name, minlength=3, strict=TRUE)
					this.tag.id.abbrev <- paste(this.tag.abbrev, ".", this.tag.id, sep="")
				} else {
					this.tag.id.abbrev <- this.tag.id
				}
			return(c(id=this.tag.id, abbrev=this.tag.id.abbrev))
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
			unlist(strsplit(cur.word, split="\\."))
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
get.JS.vars <- function(JS.var, XML.var=NULL, JS.prefix="", indent.by="", names.only=FALSE){
	if(isTRUE(names.only)){
		results <- camelCode(c(JS.prefix, JS.var))
	} else {
		results <- paste(indent.by, "var ", camelCode(c(JS.prefix, JS.var)), " = getValue(\"", XML.var, "\");\n", sep="")
	}
	return(results)
} ## function get.JS.vars()
