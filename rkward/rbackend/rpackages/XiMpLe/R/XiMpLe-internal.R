## internal functions, not exported

## function XML.single.tags()
# Splits one character string or vector with an XML tree into a vector with its single tags.
# - tree: The XML tree, must be character.
# - drop: A character vector with the possible contens \code{c("comments","declarations","cdata","value")}
XML.single.tags <- function(tree, drop=NULL){
	if(!is.character(tree)){
		stop(simpleError("'tree' must be character!"))
	} else {}
	if(length(tree) > 1) {
		# force tree into one string
		tree <- paste(tree, collapse="")
	} else {}
	# remove space at beginning (and end)
	tree <- trim(tree)

	## the main splitting process
	# CDATA or comments can contain stuff which might ruin the outcome. we'll deal with those parts first.
	# this solution is perhaps a little too complex... it should rarely be needed, though
	special.treatment <- list(cdata=NULL, comments=NULL)
	if(grepl("<!\\[CDATA\\[(.*)>(.*)\\]\\]>", tree)){
		special.treatment[["cdata"]] <- c(split.start="<!\\[CDATA\\[", split.end="\\]\\]>", prefix="<![CDATA[", suffix="]]>")
	} else {}
	if(grepl("<!--(.*)>(.*)-->", tree)){
		special.treatment[["comments"]] <- c(split.start="<!--", split.end="-->", prefix="<!--", suffix="-->")
	} else {}
	if(any(!sapply(special.treatment, is.null))){
		for (treat.this in special.treatment){
			# skip NULL entries
			ifelse(is.null(treat.this), next, TRUE)
			# steps are as follows, to be sure:
			# - cut stream at beginning CDATA/comment entries
			cut.trees <- unlist(strsplit(tree, split=treat.this[["split.start"]]))
			# - re-add the cut-off CDATA/comment start
			got.cut <- grep(treat.this[["split.end"]], cut.trees)
			cut.trees[got.cut] <- paste(treat.this[["prefix"]], cut.trees[got.cut], sep="")
			# - cut stream at ending CDATA/comment entries
			cut.trees <- unlist(strsplit(cut.trees, split=treat.this[["split.end"]]))
			# - re-add the cut-off CDATA/comment ending
			got.cut <- grep(treat.this[["split.start"]], cut.trees)
			cut.trees[got.cut] <- paste(cut.trees[got.cut], treat.this[["suffix"]], sep="")
		}
		# now do the splitting
		single.tags <- unlist(sapply(cut.trees, function(this.tree){
				if(
					(!is.null(special.treatment[["cdata"]]) & grepl("<!\\[CDATA\\[", this.tree)) |
					(!is.null(special.treatment[["comments"]]) & grepl("<!--", this.tree))
				) {
					split.me <- FALSE
				} else {
					split.me <- TRUE
				}
				if(isTRUE(split.me)){
					return(paste(unlist(strsplit(trim(this.tree), split=">[[:space:]]*")), ">", sep=""))
				} else {
					return(this.tree)
				}
			}))
	} else {
		single.tags <- paste(unlist(strsplit(tree, split=">[[:space:]]*")), ">", sep="")
	}
	# if there's values between tags, they do now precede them
	has.value <- grepl("^[^<]", single.tags)
	if(any(has.value)){
		# each fix will add an entry, so we must correct for that
		already.fixed <- 0
		for (needs.split in which(has.value)){
			tags.length <- length(single.tags)
			split.me <- unlist(strsplit(single.tags[needs.split + already.fixed], split="[[:space:]]*<"))
			if(length(split.me) != 2){ # malformated XML?
				stop(simpleError(paste("Ouch, choking on input... malformatted XML? Don't know how to handle this:\n  ", single.tags[needs.split + already.fixed], sep="")))
 			} else {}
			# return the cut of "<"
			split.me[2] <- paste("<", split.me[2], sep="")
			if("value" %in% drop){
				single.tags[needs.split + already.fixed] <- split.me[2]
			} else {
				single.tags <- c(single.tags[1:(needs.split + already.fixed - 1)], split.me, single.tags[(needs.split + already.fixed + 1):tags.length])
			}
			already.fixed <- already.fixed + 1
		}
	} else {}
	if("comments" %in% drop){
		single.tags <- single.tags[!XML.comment(single.tags)]
	} else {}
	if("declarations" %in% drop){
		single.tags <- single.tags[!XML.declaration(single.tags)]
	} else {}
	if("doctype" %in% drop){
		single.tags <- single.tags[!XML.doctype(single.tags)]
	} else {}
	if("cdata" %in% drop){
		single.tags <- single.tags[!XML.cdata(single.tags)]
	} else {}
	return(single.tags)
} ## end function XML.single.tags()

## function indent()
# will create tabs to format the output
indent <- function(level, by="\t"){
	paste(rep(by, level-1), collapse="")
} ## end function indent()

## function lookupAttrName()
# takes the original input element names and returns
# the according XML attribute name
lookupAttrName <- function(tag, attr, rename){
	if(is.null(tag)){
		attr.name <- attr
	} else {
		attr.name <- rename[[tag]][[attr]]
	}
	return(attr.name)
} ## end function lookupAttrName()

## function pasteXMLAttr()
# pastes all attributes in a nicely readable way
pasteXMLAttr <- function(attr=NULL, tag=NULL, level=1, rename=NULL, shine=2, indent.by="\t"){
	if(is.null(attr)){
		return("")
	} else {}

	new.indent <- ifelse(shine > 1, indent(level+1, by=indent.by), "")
	new.attr   <- ifelse(shine > 1, "\n", " ")

	# only use formatting if more than one attribute
	if(length(attr) > 1){
		full.attr <- c()
		for (this.attr in names(attr)){
			# skip empty elements
			if(is.null(attr[[this.attr]])){next}
			if(nchar(attr[[this.attr]]) > 0){
				if(!is.null(rename)){
					# look up attribute name to paste
					attr.name <- lookupAttrName(tag, this.attr, rename=rename)
				} else {
					attr.name <- this.attr
				}
				full.attr <- trim(paste(full.attr, new.attr, new.indent, attr.name, "=\"", attr[[this.attr]], "\"", sep=""))
			} else {}
		}
	} else {
		if(!is.null(rename)){
			# look up attribute name to paste
			attr.name <- lookupAttrName(tag, names(attr), rename=rename)
		} else {
			attr.name <- names(attr)
		}
		# look up attribute name to paste
		full.attr <- paste(attr.name, "=\"", attr[[1]], "\"", sep="")
	}
	return(full.attr)
} ## end function pasteXMLAttr()

## function parseXMLAttr()
# takes a whole XML tag and returns a named list with its attributes
parseXMLAttr <- function(tag){
	if(XML.doctype(tag)){
		stripped.tag <- gsub("<!((?i)DOCTYPE)[[:space:]]+([^[:space:]]+)[[:space:]]*([^\"[:space:]]*)[[:space:]]*.*>",
			"doctype=\"\\2\", id=\"\\3\"", tag)
		stripped.tag2 <- eval(parse(text=paste("c(",gsub("[^\"]*[\"]?([^\"]*)[\"]?[^\"]*", "\"\\1\",", tag),"NULL)")))
		is.dtd <- grepl("\\.dtd", stripped.tag2)
		doct.decl <- ifelse(sum(!is.dtd) > 0, paste(stripped.tag2[!is.dtd][1], sep=""), paste("", sep=""))
		doct.ref <- ifelse(sum(is.dtd) > 0, paste(stripped.tag2[is.dtd][1], sep=""), paste("", sep=""))
		parsed.list <- eval(parse(text=paste("list(", stripped.tag, ", decl=\"", doct.decl,"\"", ", refer=\"", doct.ref,"\")", sep="")))
	} else if(XML.endTag(tag) | XML.comment(tag) |XML.cdata(tag)){
		# end tags, comments and CDATA don't have attributes
		parsed.list <- ""
	} else {
		# first strip of start and end characters
		stripped.tag <- gsub("<([?[:space:]]*)[^[:space:]]+[[:space:]]*(.*)", "\\2", tag, perl=TRUE)
		stripped.tag <- gsub("[/?]*>$", "", stripped.tag, perl=TRUE)
		# fill in commas, so we can evaluate this as elements of a named list
		separated.tag <- gsub("=[[:space:]]*\"([^\"]*)\"[[:space:]]+([^[:space:]=]+)", "=\"\\1\", \\2", stripped.tag, perl=TRUE)
		parsed.list <- eval(parse(text=paste("list(", separated.tag, ")")))
	}
	if(XML.declaration(tag)){
		valid.attr <- c("version", "encoding", "standalone")
		parsed.list <- parsed.list[tolower(names(parsed.list)) %in% valid.attr]
		for (miss.attr in valid.attr[!valid.attr %in% tolower(names(parsed.list))]){
			parsed.list[[miss.attr]] <- ""
		}
	} else {}

	return(parsed.list)
} ## end function parseXMLAttr()

## function trim()
# cuts off space at start and end of a character string
trim <- function(char){
	char <- gsub("^[[:space:]]*", "", char)
	char <- gsub("[[:space:]]*$", "", char)
	return(char)
} ## end function trim()

## function XML.emptyTag()
# checks if a tag is a pair of start/end tags or an empty tag;
# returns either TRUE/FALSE, or the tag name if it is an empty tag and get=TRUE
XML.emptyTag <- function(tag, get=FALSE){
	empty.tags <- sapply(tag, function(this.tag){
			empty <- grepl("/>$", this.tag)
			if(isTRUE(get)){
				result <- ifelse(isTRUE(empty), XML.tagName(this.tag), "")
			} else {
				result <- empty
			}
			return(result)
		})
	names(empty.tags) <- NULL
	return(empty.tags)
} ## end function XML.emptyTag()

## function XML.endTag()
# checks if a tag an end tag;
# returns either TRUE/FALSE, or the tag name if it is an end tag and get=TRUE
XML.endTag <- function(tag, get=FALSE){
	end.tags <- sapply(tag, function(this.tag){
			end <- grepl("^</", this.tag)
			if(isTRUE(get)){
				result <- ifelse(isTRUE(end), XML.tagName(this.tag), "")
			} else {
				result <- end
			}
			return(result)
		})
	names(end.tags) <- NULL
	return(end.tags)
} ## end function XML.endTag()

## function XML.comment()
# checks if a tag is a comment, returns TRUE or FALSE, or the comment (TRUE & get=TRUE)
XML.comment <- function(tag, get=FALSE, trim=TRUE){
	comment.tags <- sapply(tag, function(this.tag){
			comment <- grepl("<!--(.*)-->", this.tag)
			if(isTRUE(get)){
				result <- ifelse(isTRUE(comment), gsub("<!--(.*)-->", "\\1", this.tag, perl=TRUE), "")
				if(isTRUE(trim)){result <- trim(result)} else {}
			} else {
				result <- comment
			}
			return(result)
		})
	names(comment.tags) <- NULL
	return(comment.tags)
} ## end function XML.comment()

## function XML.cdata()
# checks if a tag is a CDATA declaration, returns TRUE or FALSE, or the data (TRUE & get=TRUE)
XML.cdata <- function(tag, get=FALSE, trim=TRUE){
	cdata.tags <- sapply(tag, function(this.tag){
			cdata <- grepl("<!\\[CDATA\\[(.*)\\]\\]>", this.tag)
			if(isTRUE(get)){
				result <- ifelse(isTRUE(cdata), gsub("<!\\[CDATA\\[(.*)\\]\\]>", "\\1", this.tag, perl=TRUE), "")
				if(isTRUE(trim)){result <- trim(result)} else {}
			} else {
				result <- cdata
			}
			return(result)
		})
	names(cdata.tags) <- NULL
	return(cdata.tags)
} ## end function XML.cdata()

## function XML.value()
# checks if 'tag' is actually not a tag but value/content/data. returns TRUE or FALSE, or the value (TRUE & get=TRUE)
XML.value <- function(tag, get=FALSE, trim=TRUE){
	all.values <- sapply(tag, function(this.tag){
			value <- grepl("^[[:space:]]*[^<]", this.tag)
			if(isTRUE(get)){
				result <- ifelse(isTRUE(value), this.tag, "")
				if(isTRUE(trim)){result <- trim(result)} else {}
			} else {
				result <- value
			}
			return(result)
		})
	names(all.values) <- NULL
	return(all.values)
} ## end function XML.value()

## function XML.declaration()
# checks for a declaration, like <?xml bar?>
XML.declaration <- function(tag, get=FALSE){
	decl.tags <- sapply(tag, function(this.tag){
			declaration <- grepl("<\\?((?i)xml).*\\?>", this.tag, perl=TRUE)
			if(isTRUE(get)){
				result <- ifelse(isTRUE(declaration), XML.tagName(this.tag), "")
			} else {
				result <- declaration
			}
			return(result)
		})
	names(decl.tags) <- NULL
	return(decl.tags)
} ## end function XML.declaration()

## function XML.doctype()
# checks for a doctype declaration, like <!DOCTYPE foo>
XML.doctype <- function(tag, get=FALSE){
	decl.tags <- sapply(tag, function(this.tag){
			declaration <- grepl("<!((?i)DOCTYPE).*>", this.tag, perl=TRUE)
			if(isTRUE(get)){
				result <- ifelse(isTRUE(declaration), XML.tagName(this.tag), "")
			} else {
				result <- declaration
			}
			return(result)
		})
	names(decl.tags) <- NULL
	return(decl.tags)
} ## end function XML.doctype()

## function XML.def()
XML.def <- function(tag, get=FALSE){
	decl.tags <- sapply(tag, function(this.tag){
			declaration <- grepl("<[!?]+[^-]*>", this.tag)
			if(isTRUE(get)){
				result <- ifelse(isTRUE(declaration), XML.tagName(this.tag), "")
			} else {
				result <- declaration
			}
			return(result)
		})
	names(decl.tags) <- NULL
	return(decl.tags)
} ## end function XML.def()

## function XML.tagName()
XML.tagName <- function(tag){
	tag.names <- sapply(tag, function(this.tag){
			tagName <- gsub("<([[:space:]!?/]*)([^[:space:]>]+).*", "\\2", this.tag, perl=TRUE)
			return(tagName)
		})
	names(tag.names) <- NULL
	return(tag.names)
} ## end function XML.tagName()

## function parseXMLTag()
parseXMLTag <- function(tag){
	tag.name <- XML.tagName(tag)
	tag.attr <- parseXMLAttr(tag)
	if(!is.null(tag.attr)){
		parsed.tag <- list()
		parsed.tag[[tag.name]] <- list(attr=tag.attr)
	} else {
		parsed.tag <- list()
		parsed.tag[[tag.name]] <- list()
	}
	return(parsed.tag)
} ## end function parseXMLTag()

## function XML.nodes()
XML.nodes <- function(single.tags, end.here=NA, start=1){
	# try to iterate through the single tags
	children <- list()
	tag.no <- start
	## uncomment to debug:
	# cat(start,"\n")
	while (tag.no < length(single.tags)){
		## uncomment to debug:
		# time.spent <- system.time({
		this.tag <- single.tags[tag.no]
		nxt.child <- length(children) + 1
		child.name <- XML.tagName(this.tag)
		child.end.tag <- paste("</[[:space:]]*", end.here,"[[:space:]>]+.*", sep="")
		if(isTRUE(grepl(child.end.tag, this.tag))){
		## uncomment to debug:
		# cat(this.tag, ": break (",tag.no,")\n")
			break
		} else {}
		if(XML.value(this.tag)){
			children[nxt.child] <- new("XiMpLe.node",
				name="",
				value=XML.value(this.tag, get=TRUE))
			names(children)[nxt.child] <- "!value!"
			tag.no <- tag.no + 1
			next
		} else {
			child.attr <- parseXMLAttr(this.tag)
		}
		if(XML.declaration(this.tag)){
			children[nxt.child] <- new("XiMpLe.node",
				name=child.name,
				attributes=child.attr)
			names(children)[nxt.child] <- child.name
			tag.no <- tag.no + 1
			next
		} else {}
		if(XML.comment(this.tag)){
			children[nxt.child] <- new("XiMpLe.node",
				name="!--",
				value=XML.comment(this.tag, get=TRUE))
			names(children)[nxt.child] <- "!--"
			tag.no <- tag.no + 1
			next
		} else {}
		if(XML.cdata(this.tag)){
			children[nxt.child] <- new("XiMpLe.node",
				name="![CDATA[",
				value=XML.cdata(this.tag, get=TRUE))
			names(children)[nxt.child] <- "![CDATA["
			tag.no <- tag.no + 1
			next
		} else {}
		if(XML.endTag(this.tag)){
			break
		} else {}
		if(!XML.emptyTag(this.tag)){
		## uncomment to debug:
		# cat(child.name, ":", tag.no, "-", child.end.tag,"\n")
			rec.nodes <- XML.nodes(single.tags, end.here=child.name, start=tag.no + 1)
			children[nxt.child] <- new("XiMpLe.node",
				name=child.name,
				attributes=child.attr,
				children=rec.nodes$children,
				# this value will force the node to remain non-empty if it had no children,
				# it would be turned into an empty tag otherwise
				value="")
			names(children)[nxt.child] <- child.name
			tag.no <- rec.nodes$tag.no + 1
			next
		} else {
			children[nxt.child] <- new("XiMpLe.node",
				name=child.name,
				attributes=child.attr)
			names(children)[nxt.child] <- child.name
			tag.no <- tag.no + 1
			next
		}
		## uncomment to debug:
		# })
		# cat("system.time:", time.spent, "\n")
	}
	return(list(children=children, tag.no=tag.no))
} ## end function XML.nodes()
