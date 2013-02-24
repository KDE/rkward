## create dialog to build a plugin skeleton
require(rkwarddev)

local({
# define where the plugin should write its files
output.dir <- tempdir()

about.info <- rk.XML.about(
	name="RKWard Plugin Skeleton",
	author=c(
		person(given="Meik", family="Michalke",
			email="meik.michalke@hhu.de", role=c("aut","cre"))),
	about=list(desc="GUI interface to create RKWard plugin skeletons",
		# the version number should be in line with rkwarddev
		# to reflect when the script code was changed
		version="0.06-2", url="http://rkward.sf.net")
	)
dependencies.info <- rk.XML.dependencies(dependencies=list(rkward.min="0.6.0"))

# tab1: information on the thing
about.plugin <- rk.XML.frame(
	rk.XML.row(
		pl.name <- rk.XML.input("Plugin name", required=TRUE, size="small"),
		pl.license <- rk.XML.input("License", initial="GPL (>= 3)", required=TRUE)),
	rk.XML.row(pl.desc <- rk.XML.input("Short description", required=TRUE)),
	rk.XML.row(
		pl.version <- rk.XML.input("Version number", initial="0.01-0", required=TRUE),
		pl.date <- rk.XML.input("Release date (empty for today)")),
	rk.XML.row(
		pl.homepage <- rk.XML.input("Homepage"),
		pl.category<- rk.XML.input("Category")), label="About the plugin")
about.contact <- rk.XML.frame(
	rk.XML.row(
		rk.XML.col(
			aut.given <- rk.XML.input("Given name", required=TRUE),
			aut.family <- rk.XML.input("Family name", required=TRUE),
			aut.email <- rk.XML.input("E-mail", required=TRUE),
			rk.XML.stretch()),
		rk.XML.col(rk.XML.frame(
			aut.auth <- rk.XML.cbox("Author", chk=TRUE),
			aut.maint <- rk.XML.cbox("Maintainer", chk=TRUE),
			rk.XML.stretch(), label="Author roles"))),
	label="Plugin author")

tab1.about <- rk.XML.col(about.plugin, about.contact)

# tab2: create options
crt.opts <- rk.XML.frame(
		rk.XML.row(pl.dir <- rk.XML.browser("Directory to save to (empty for $TEMPDIR)", type="dir", required=FALSE)),
		rk.XML.row(
			rk.XML.col(
				pl.overw <- rk.XML.cbox("Overwrite existing files", chk=FALSE),
				pl.wiz <- rk.XML.cbox("Add wizard section", chk=FALSE),
				pl.tests <- rk.XML.cbox("Include plugin tests", chk=TRUE),
				rk.XML.stretch()),
			rk.XML.col(
				pl.edit <- rk.XML.cbox("Open files for editing", chk=TRUE),
				pl.add <- rk.XML.cbox("Add plugin to RKWard configuration", chk=TRUE),
				pl.show <- rk.XML.cbox("Show the plugin", chk=FALSE),
				rk.XML.stretch())
		),
		rk.XML.row(pl.hier <- rk.XML.dropdown("Place in top menu",
			options=list(
					"Test (created if needed)"=c(val="test", chk=TRUE),
					"File"=c(val="file"),
					"Edit"=c(val="edit"),
					"View"=c(val="view"),
					"Workspace"=c(val="workspace"),
					"Run"=c(val="run"),
					"Data"=c(val="data"),
					"Analysis"=c(val="analysis"),
					"Plots"=c(val="plots"),
					"Distributions"=c(val="distributions"),
					"Windows"=c(val="windows"),
					"Settings"=c(val="settings"),
					"Help"=c(val="help")
			)
		))
	)
dep.opts <- rk.XML.frame(
	rk.XML.row(
		dep.frame.RKWard <- rk.XML.frame(
				dep.rkmin <- rk.XML.input("RKWard min", size="small"),
				dep.rkmax <- rk.XML.input("RKWard max", size="small"),
				rk.XML.stretch(), label="Depends on RKWard version"),
		dep.frame.R <- rk.XML.frame(
				dep.rmin <- rk.XML.input("R min", size="small"),
				dep.rmax <- rk.XML.input("R max", size="small"),
				rk.XML.stretch(), label="Depends on R version")#,
# 	rk.XML.row(dep.frame.packages <- rk.XML.frame(rk.XML.stretch(before=list(
# 			rk.XML.text("Separate package names by space:"),
# 			dep.pckg <- rk.XML.input("Packages"))), label="Depends on R packages"))
	), label="Define dependencies", checkable=TRUE, chk=FALSE)

tab2.create <- rk.XML.col(crt.opts, dep.opts)

# tab3: varslot to select the actual content
children.text <- rk.XML.text("If you already created XML content for the plugin, select the main dialog object here (probably a tabbook?)")
children.var <- rk.XML.row(
	children.varselector <- rk.XML.varselector(label="Plugin content"),
	rk.XML.col(
		cont.dial <- rk.XML.varslot("Select an object of class XiMpLe.node", source=children.varselector, classes="XiMpLe.node"),
		rk.XML.frame(
			js.prep <- rk.XML.varslot("preprocess()", source=children.varselector),
			js.calc <- rk.XML.varslot("calculate()", source=children.varselector),
			js.prnt <- rk.XML.varslot("printout()", source=children.varselector),
			rk.XML.stretch())
	))
tab3.children <- rk.XML.col(rk.XML.row(children.text), rk.XML.row(children.var))

## glue all of the above together in one tabbook
# sklt.tabbook <- rk.XML.dialog(rk.XML.tabbook("Plugin Skeleton",
# 	tab.labels=c("About the plugin", "Create options", "XML content"),
# 	children=list(tab1.about, tab2.create, tab3.children)), label="RKWard Plugin Skeleton")
sklt.tabbook <- rk.XML.dialog(rk.XML.tabbook("Plugin Skeleton",
	tabs=list("About the plugin"=tab1.about, "Create options"=tab2.create)),
	label="RKWard Plugin Skeleton")

## some logic
logic.section <- rk.XML.logic(
		rk.XML.connect(governor=dep.opts, get="checked", client=dep.frame.RKWard, set="enabled"),
		rk.XML.connect(governor=dep.opts, get="checked", client=dep.frame.R, set="enabled")#,
# 		rk.XML.connect(governor=dep.opts, client=dep.frame.packages, set="enabled")
	)

## JS code generation
# author section
js.opt.about.author.role <- rk.JS.options("optAuthorRole",
	ite(aut.auth, qp("\"aut\"")),
	ite(aut.maint, qp("\"cre\"")),
	funct="c", option="role", collapse="")
js.opt.about.author <- rk.JS.options("optAuthor",
	ite(aut.given, qp("given=\"",aut.given,"\"")),
	ite(aut.family, qp("family=\"",aut.family,"\"")),
	ite(aut.email, qp("email=\"",aut.email,"\"")),
	ite(js.opt.about.author.role, js.opt.about.author.role),
	funct="person", option="author", collapse=",\\n\\t")
# about section
js.opt.about.about <- rk.JS.options("optAbout",
	ite(pl.desc, qp("desc=\"",pl.desc,"\"")),
	ite(pl.version, qp("version=\"",pl.version,"\"")),
	ite(pl.date, qp("date=\"",pl.date,"\"")),
	ite(pl.homepage, qp("url=\"",pl.homepage,"\"")),
	ite(pl.license, qp("license=\"",pl.license,"\"")),
	ite(pl.category, qp("category=\"",pl.category,"\"")),
	funct="list", option="about", collapse=",\\n\\t")
# dependencies section
js.frm.dep.opts <- rk.JS.vars(dep.opts, modifiers="checked") # see to it frame is checked
js.opt.about.dep <- rk.JS.options("optDependencies",
	ite(id(js.frm.dep.opts, " && ", dep.rkmin), qp("rkward.min=\"",dep.rkmin,"\"")),
	ite(id(js.frm.dep.opts, " && ", dep.rkmax), qp("rkward.max=\"",dep.rkmax,"\"")),
	ite(id(js.frm.dep.opts, " && ", dep.rmin), qp("R.min=\"",dep.rmin,"\"")),
	ite(id(js.frm.dep.opts, " && ", dep.rmax), qp("R.max=\"",dep.rmax,"\"")),
	funct="list", option="dependencies", collapse=",\\n\\t")

js.opt.skel.pluginmap <- rk.JS.options("optPluginmap",
	ite(id(pl.hier, "!= \"test\""), qp("hierarchy=\"", pl.hier, "\"")),
	funct="list", option="pluginmap", collapse="")
js.opt.skeleton <- rk.JS.options("optSkeleton",
	ite(pl.dir, qp("\n\tpath=\"", pl.dir, "\"")),
	ite(pl.wiz, qp("\n\tprovides=c(\"logic\", \"dialog\", \"wizard\")")),
	ite(js.opt.skel.pluginmap, qp("\n\t", js.opt.skel.pluginmap)),
	ite(pl.overw, qp("\n\toverwrite=TRUE")),
	ite(pl.tests, qp("\n\ttests=TRUE")),
	ite(pl.edit, qp("\n\tedit=TRUE")),
	ite(pl.add, qp("\n\tload=TRUE")),
	ite(pl.show, qp("\n\tshow=TRUE")),
	collapse="")

JS.calculate <- rk.paste.JS(
	js.opt.about.author.role,
	js.opt.about.author,
	js.opt.about.about,
	js.opt.about.dep,
	js.opt.skel.pluginmap,
	js.opt.skeleton,
	echo("about.plugin <- rk.XML.about("),
		ite(pl.name, echo("\n\tname=\"", pl.name, "\"")),
		echo(js.opt.about.author),
		echo(js.opt.about.about),
	echo("\n)\n\n"),
	ite(id(js.frm.dep.opts, " && ", js.opt.about.dep),
		echo("plugin.dependencies <- rk.XML.dependencies(", js.opt.about.dep, "\n)\n\n")),
	echo("plugin.dir <- rk.plugin.skeleton(\n\tabout=about.plugin,"),
		ite(id(js.frm.dep.opts, " && ", js.opt.about.dep), echo("\n\tdependencies=plugin.dependencies,")),
		echo(js.opt.skeleton),
	echo("\n)\n\n"),
	level=2)

## the main call
#plugin.dir <<- rk.plugin.skeleton(
rk.plugin.skeleton(
	about=about.info,
	path=output.dir,
	guess.getter=FALSE,
	xml=list(
		dialog=sklt.tabbook,
		logic=logic.section),
	js=list(
		require="rkwarddev",
		calculate=JS.calculate),
	pluginmap=list(name="Create RKWard plugin skeleton", hierarchy=list("file", "export")),
	overwrite=TRUE,
	create=c("pmap","xml","js","desc"),
	dependencies=dependencies.info,
	tests=FALSE,
	show=TRUE,
	edit=TRUE,
	hints=FALSE)
})
