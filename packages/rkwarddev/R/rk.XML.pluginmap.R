#' Write a pluginmap file for RKWard
#'
#' @param name Character string, name of the plugin.
#' @param about Either an object of class \code{XiMpLe.node} to be pasted as the \code{<about>} section,
#'		or a list with descriptive information on the plugin,its authors and dependencies.
#'		See \code{link[XiMpLe:rk.XML.about]{rk.XML.about}} for details! Skipped if \code{NULL}.
#' @param components Either an object of class \code{XiMpLe.node} to be pasted as the \code{<components>} section (see
#'		\code{\link[rkwarddev:rk.XML.components]{rk.XML.components}} for details). Or a character vector with at least
#'		one plugin component file name, relative path from the pluginmap file and ending with ".xml".
#' @param hierarchy Either an object of class \code{XiMpLe.node} to be pasted as the \code{<hierarchy>} section (see
#'		\code{\link[rkwarddev:rk.XML.hierarchy]{rk.XML.hierarchy}} for details). Or a character vector with instructions
#'		where to place the plugin in the menu hierarchy, one string for each included component. Valid values are
#'		\code{"file"}, \code{"edit"}, \code{"view"}, \code{"workspace"}, \code{"run"}, \code{"data"},
#'		\code{"analysis"}, \code{"plots"}, \code{"distributions"}, \code{"windows"}, \code{"settings"} and \code{"help"}.
#'		Anything else will place it in a "test" menu.
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
			about.XML <- rk.XML.about(
				name=name.orig,
				author=about[["author"]],
				about=about[["about"]],
				dependencies=about[["dependencies"]],
				package=about[["package"]],
				pluginmap=about[["pluginmap"]])
			# initialize all.children list
			all.children <- list(about.XML)
		}
	} else {
		if(isTRUE(hints)){
			about.XML <- new("XiMpLe.node",
				name="!--",
				value="<about></about>")
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
				value="<require file=\"path/file.pluginmap\" />")
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
		components.XML <- rk.XML.components(
			as.list(sapply(components, function(this.comp){
				# remove any directory names and .EXT endings
				xml.basename <- gsub("(.*/)?([[:alnum:]]*).+(.*)?", "\\2", this.comp, perl=TRUE)
				rk.XML.component(
					label=xml.basename,
					file=this.comp,
					# if this ID get's a change, also change it in rk.plugin.skeleton(show=TRUE)!
					id.name=auto.ids(paste(name, xml.basename, sep=""), prefix=ID.prefix("component"), chars=25))
				}))
		)
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
			this.hier <- hierarchy[this.dial]

			entry.XML <- rk.XML.menu(
				label=name.orig,
				nodes=rk.XML.entry(component=this.comp),
				id.name=auto.ids(paste(name, this.comp, sep=""), prefix=ID.prefix("menu"), chars=12))

			if(this.hier %in% names(main.menu)){
				hier.XML <- rk.XML.menu(
					label=main.menu[this.hier],
					nodes=entry.XML,
					id.name=this.hier)
			} else {
				hier.XML <- rk.XML.menu(
					label="Test",
					nodes=entry.XML,
					id.name="test")
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
				value="<context id=\"x11\"></context>")
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
				value="<context id=\"import\"></context>")
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
