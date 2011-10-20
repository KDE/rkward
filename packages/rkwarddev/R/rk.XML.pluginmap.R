#' Write a pluginmap file for RKWard
#'
#' @param name Character string, name of the plugin.
#' @param about An object of class \code{XiMpLe.node} to be pasted as the \code{<about>} section,
#'		See \code{link[XiMpLe:rk.XML.about]{rk.XML.about}} for details. Skipped if \code{NULL}.
#' @param components Either an object of class \code{XiMpLe.node} to be pasted as the \code{<components>} section (see
#'		\code{\link[rkwarddev:rk.XML.components]{rk.XML.components}} for details). Or a character vector with at least
#'		one plugin component file name, relative path from the pluginmap file and ending with ".xml".
#' @param hierarchy Either an object of class \code{XiMpLe.node} to be pasted as the \code{<hierarchy>} section (see
#'		\code{\link[rkwarddev:rk.XML.hierarchy]{rk.XML.hierarchy}} for details). Or a character vector with instructions
#'		where to place the plugin in the menu hierarchy, one list or string for each included component. Valid single values are
#'		\code{"file"}, \code{"edit"}, \code{"view"}, \code{"workspace"}, \code{"run"}, \code{"data"},
#'		\code{"analysis"}, \code{"plots"}, \code{"distributions"}, \code{"windows"}, \code{"settings"} and \code{"help"},
#'		anything else will place it in a "test" menu. If \code{hierarchy} is a list, each entry represents the label of a menu level.
#' @param require Either a (list of) objects of class \code{XiMpLe.node} to be pasted as a \code{<require>} section (see
#'		\code{\link[rkwarddev:rk.XML.require]{rk.XML.require}} for details). Or a character vector with at least
#'		one .pluginmap filename to be included in this one.
#' @param x11.context An object of class \code{XiMpLe.node} to be pasted as a \code{<context id="x11">} section, see
#'		\code{\link[rkwarddev:rk.XML.context]{rk.XML.context}} for details.
#' @param import.context An object of class \code{XiMpLe.node} to be pasted as the \code{<context id="import">} section, see
#'		\code{\link[rkwarddev:rk.XML.context]{rk.XML.context}} for details.
#' @param clean.name Logical, if \code{TRUE}, all non-alphanumeric characters except the underscore (\code{"_"}) will be removed from \code{name}.
#' @param hints Logical, if \code{TRUE} and you leave out optional entries (like \code{about=NULL}), dummy sections will be added as comments.
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @export

rk.XML.pluginmap <- function(name, about=NULL, components, hierarchy="test",
	require=NULL, x11.context=NULL, import.context=NULL, clean.name=TRUE, hints=FALSE){
	name.orig <- name
	if(isTRUE(clean.name)){
		# to besure, remove all non-character symbols from name
		name <- clean.name(name)
	} else {}

	# .pluginmap has these children in <document>:
	# - about (optional)
	# - require (optional, multiple)
	# - components (once)
	#   - component
	#     - attribute
	# - hierarchy (once)
	#   - menu
	#     - entry
	# - context (optional, "x11")
	#   - menu
	#     - entry
	# - context (optional, "import")
	#   - menu
	#     - entry

	## about section
	if(!is.null(about)){
		if(inherits(about, "XiMpLe.node")){
			about.node.name <- about@name
			# check if this is *really* a about section, otherwise quit and go dancing
			if(!identical(about.node.name, "about")){
				stop(simpleError("I don't know what this is, but 'about' is not an about section!"))
			} else {
				# initialize all.children list
				all.children <- list(about)
			}
		} else {
			stop(simpleError("'about' must be a XiMpLe.node, see ?rk.XML.about()!"))
		}
	} else {
		if(isTRUE(hints)){
			about.XML <- new("XiMpLe.node",
				name="!--",
				children=list(new("XiMpLe.node", name="about", value="")))
			# initialize all.children list
			all.children <- list(about.XML)
		} else {
			# initialize all.children list
			all.children <- list()
		}
	}

	## require section
	if(!is.null(require)){
		# check if this is *really* require nodes
		for(this.child in child.list(require)){
				if(inherits(this.child, "XiMpLe.node")){
					node.name <- this.child@name
					if(!identical(node.name, "require")){
						stop(simpleError("I don't know what this is, but 'require' is not made of require nodes!"))
					} else {
						all.children[[length(all.children)+1]] <- this.child
					}
				} else {
					if(grepl(".pluginmap", this.child)){
						all.children[[length(all.children)+1]] <- rk.XML.require(file=this.child)
					} else {
						stop(simpleError("Only .pluginmap files are valid for require nodes!"))
					}
				}
			}
	} else {
		if(isTRUE(hints)){
			require.XML <- new("XiMpLe.node",
				name="!--",
				children=list(rk.XML.require("path/file.pluginmap")))
			all.children[[length(all.children)+1]] <- require.XML
		} else {}
	}

	## components section
	if(inherits(components, "XiMpLe.node")){
		components.node.name <- components@name
		# check if this is *really* a components section, otherwise quit and go dancing
		if(!identical(components.node.name, "components")){
			stop(simpleError("I don't know what this is, but 'components' is not a components section!"))
		} else {}
		all.children[[length(all.children)+1]] <- components
		# get the IDs for hierarchy section
		component.IDs <- sapply(components@children, function(this.comp){this.comp@attributes$id})
	} else {
			components.XML.list <- list()
			num.compos <- length(components)
			compo.names <- names(components)
			for (this.comp.num in 1:num.compos){
				this.comp <- components[this.comp.num]
				if(num.compos > 1) {
					# let's see if we have entry names
					if(length(compo.names) == length(components)){
						xml.basename <- compo.names[this.comp.num]
					} else {
						# remove any directory names and .EXT endings
						xml.basename <- gsub("(.*/)?([[:alnum:]_]*).+(.*)?", "\\2", this.comp, perl=TRUE)
					}
				} else {
					xml.basename <- name.orig
				}
				components.XML.list[[length(components.XML.list) + 1]] <- rk.XML.component(
					label=xml.basename,
					file=this.comp,
					# if this ID get's a change, also change it in rk.plugin.skeleton(show=TRUE)!
					id.name=auto.ids(paste(name, xml.basename, sep=""), prefix=ID.prefix("component"), chars=25))
				}
		components.XML <- rk.XML.components(components.XML.list)

		all.children[[length(all.children)+1]] <- components.XML
		# get the IDs for hierarchy section
		component.IDs <- sapply(components.XML@children, function(this.comp){this.comp@attributes$id})
	}

	## hierachy section
	if(inherits(hierarchy, "XiMpLe.node")){
		hierarchy.node.name <- hierarchy@name
		# check if this is *really* a hierarchy section, otherwise quit and go dancing
		if(!identical(hierarchy.node.name, "hierarchy")){
			stop(simpleError("I don't know what this is, but 'hierarchy' is not a hierarchy section!"))
		} else {}
		all.children[[length(all.children)+1]] <- hierarchy
	} else {
		# correct for cases with one component and a list
		if(length(component.IDs) == 1 & is.list(hierarchy)){
			if(!is.list(hierarchy[[1]]))
			hierarchy <- list(hierarchy)
		} else {}
		# check if the numbers fit
		if(length(hierarchy) != length(component.IDs)){
			stop(simpleError("Length of 'hierarchy' and number of components must be the same!"))
		} else {}
		# predefined menu points
		main.menu <- c(file="File", edit="Edit", view="View", workspace="Workspace", run="Run",
			data="Data", analysis="Analysis", plots="Plots", distributions="Distributions",
			windows="Windows", settings="Settings", help="Help")

		hier.comp.XML <- sapply(1:length(hierarchy), function(this.dial){
			this.comp <- component.IDs[this.dial]
			if(is.list(hierarchy)){
				this.hier <- hierarchy[[this.dial]]
			} else {
				this.hier <- hierarchy[this.dial]
			}

			# hierachy can either be a list with menu paths, or predefined
			if(is.list(this.hier)){
				new.hierarchy <- this.hier[2:length(this.hier)]
				new.hierarchy[[length(new.hierarchy) + 1]] <- this.comp
				if(this.hier[[1]] %in% names(main.menu)){
					id.names <- sapply(this.hier, function(hier.id){
							return(clean.name(hier.id))
						})
					hier.XML <- rk.XML.menu(
						label=unlist(main.menu[this.hier[[1]]]),
						new.hierarchy,
						id.name=id.names)
				} else {
					hier.XML <- rk.XML.menu(
						label=this.hier[[1]],
						new.hierarchy)
				}
			} else {
				entry.XML <- rk.XML.menu(
					label=name.orig,
					rk.XML.entry(component=this.comp),
					id.name=auto.ids(paste(name, this.comp, sep=""), prefix=ID.prefix("menu"), chars=12))

				if(this.hier %in% names(main.menu)){
					hier.XML <- rk.XML.menu(
						label=main.menu[this.hier],
						entry.XML,
						id.name=this.hier)
				} else {
					hier.XML <- rk.XML.menu(
						label="Test",
						entry.XML,
						id.name="test")
				}
			}
			return(hier.XML)
		})
		all.children[[length(all.children)+1]] <- rk.XML.hierarchy(hier.comp.XML)
	}

	## context sections
	if(!is.null(x11.context)){
		# check if this is *really* a context node for x11
		if(inherits(x11.context, "XiMpLe.node")){
			node.name <- x11.context@name
			ctxt.name <- x11.context@attributes$id
		} else {
			node.name <- ctxt.name <- "buhahahahaa"
		}
		if(!identical(node.name, "context") | !identical(ctxt.name, "x11")){
			stop(simpleError("I don't know what this is, but 'x11.context' is not a context node for x11!"))
		} else {
			all.children[[length(all.children)+1]] <- x11.context
		}
	} else {
		if(isTRUE(hints)){
			context.x11.XML <- new("XiMpLe.node",
				name="!--",
				children=list(rk.XML.context(id="x11")))
			all.children[[length(all.children)+1]] <- context.x11.XML
		} else {}
	}
	# import
	if(!is.null(import.context)){
		# check if this is *really* a context node for import
		if(inherits(import.context, "XiMpLe.node")){
			node.name <- import.context@name
			ctxt.name <- import.context@attributes$id
		} else {
			node.name <- ctxt.name <- "buhahahahaa"
		}
		if(!identical(node.name, "context") | !identical(ctxt.name, "import")){
			stop(simpleError("I don't know what this is, but 'import.context' is not a context node for import!"))
		} else {
			all.children[[length(all.children)+1]] <- import.context
		}
	} else {
		if(isTRUE(hints)){
			context.import.XML <- new("XiMpLe.node",
				name="!--",
				children=list(rk.XML.context(id="import")))
			all.children[[length(all.children)+1]] <- context.import.XML
		} else {}
	}

	top.doc <- new("XiMpLe.node",
		name="document",
		attributes=list(base_prefix="", namespace="rkward", id=paste(name, "_rkward", sep="")),
		children=all.children
	)

	pluginmap <- new("XiMpLe.doc",
			dtd=list(doctype="rkpluginmap"),
			children=child.list(top.doc)
	)

	return(pluginmap)
}
