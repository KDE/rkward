## create dialog to build a plugin skeleton
require(rkwarddev)
rkwarddev.required("0.06-5")

local({
# define where the plugin should write its files
output.dir <- tempdir()
# overwrite an existing plugin in output.dir?
overwrite <- TRUE

about.info <- rk.XML.about(
  name="RKWard Plugin Skeleton",
  author=c(
    person(given="Meik", family="Michalke",
      email="meik.michalke@hhu.de", role=c("aut","cre"))),
  about=list(desc="GUI interface to create RKWard plugin skeletons",
    # the version number should be in line with rkwarddev
    # to reflect when the script code was changed
    version="0.06-5", url="http://rkward.sf.net")
  )
dependencies.info <- rk.XML.dependencies(dependencies=list(rkward.min="0.6.0"))

rk.set.comp("Create RKWard plugin skeleton")

# tab1: information on the thing
about.plugin <- rk.XML.frame(
  rk.XML.row(
    pl.name <- rk.XML.input("Plugin name", required=TRUE, size="small",
      help="Give the name for your new plugin here."),
    pl.license <- rk.XML.input("License", initial="GPL (>= 3)", required=TRUE,
      help="Define the license for your plugin. A short form should be sufficient.")),
  rk.XML.row(pl.desc <- rk.XML.input("Short description", required=TRUE,
    help="Describe your plugin in a few sentences: What does it do?")),
  rk.XML.row(
    pl.version <- rk.XML.input("Version number", initial="0.01-0", required=TRUE,
      help="Version information for your plugin."),
    pl.date <- rk.XML.input("Release date (empty for today)",
      help="The release date of your plugin. If you leave this empty, the current date will be used automatically.")),
  rk.XML.row(
    pl.homepage <- rk.XML.input("Homepage",
      help="A URL where one can find more information on the plugin, download updates etc."),
    pl.category<- rk.XML.input("Category",
      help="A category for your plugin. This infromation is currently ignored by RKWard.")),
  label="About the plugin")
about.contact <- rk.XML.frame(
  rk.XML.row(
    rk.XML.col(
      aut.given <- rk.XML.input("Given name", required=TRUE,
        help="First name of the package author."),
      aut.family <- rk.XML.input("Family name", required=TRUE,
        help="Family name of the package author."),
      aut.email <- rk.XML.input("E-mail", required=TRUE,
        help="The authors e-mail address, important for bug reports and receiving a myriad of thank yous..."),
      rk.XML.stretch()),
    rk.XML.col(rk.XML.frame(
      aut.auth <- rk.XML.cbox("Author", chk=TRUE,
        help="Check this if you are the author of the plugin code."),
      aut.maint <- rk.XML.cbox("Maintainer", chk=TRUE,
        help="Check this if you maintain the plugin package."),
      rk.XML.stretch(), label="Author roles"))),
  label="Plugin author")

tab1.about <- rk.XML.col(about.plugin, about.contact)

# tab2: create options
crt.opts <- rk.XML.frame(
    rk.XML.row(pl.dir <- rk.XML.browser("Directory to save to (empty for $TEMPDIR)", type="dir", required=FALSE,
      help="Set the directory where all plugin files and its directory structure should be generated. The default is a temporary directory.")),
    rk.XML.row(
      rk.XML.col(
        pl.overw <- rk.XML.cbox("Overwrite existing files", chk=FALSE,
          help="If this is checked, existing files in the specified target directory will probably be replaced by new ones."),
        pl.wiz <- rk.XML.cbox("Add wizard section", chk=FALSE,
          help="If this is checked, a wizard section will be included in the skeleton."),
        pl.tests <- rk.XML.cbox("Include plugin tests", chk=TRUE,
          help="If this is checked, plugin tests will be included in the skeleton."),
        rk.XML.stretch()),
      rk.XML.col(
        pl.edit <- rk.XML.cbox("Open files for editing", chk=TRUE,
          help="If this is checked, all generated files will be opened for editing instantly."),
        pl.add <- rk.XML.cbox("Add plugin to RKWard configuration", chk=TRUE,
          help="If this is checkend, the generated plugin will automatically be registered in RKWard's configuration.
            If you store it in a temporary directory and remove it before the next start of RKWard, the entry will removed again as well."),
        pl.show <- rk.XML.cbox("Show the plugin", chk=FALSE,
          help="If this is checked, the generated plugin will be shown (opened) for you to see what it looks like."),
        pl.guessgetter <- rk.XML.cbox("Guess getter functions (RKWard >= 0.6.0)", chk=FALSE,
          help="If this is checked, rkwarddev tries to select the optimal getter functions to get data from the dialog into the R code. The plugin then requires RKWard >= 0.6.0."),
        rk.XML.stretch())
    ),
    rk.XML.frame(
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
        ),
        help="Specify where the plugin should appear in RKWard's top menus."
      ),
      pl.hier.name <- rk.XML.input("Name in menu (plugin name if empty)",
        help="You can set the exact entry name of your main component in the menu here. If left empty, the plugin name will be used as default.")
      )
    )
  )
dep.opts <- rk.XML.frame(
  rk.XML.row(
    dep.frame.RKWard <- rk.XML.frame(
        dep.rkmin <- rk.XML.input("RKWard min", size="small",
          help="The minimum version number of RKWard required to run this plugin."),
        dep.rkmax <- rk.XML.input("RKWard max", size="small",
          help="The maximum version number of RKWard required to run this plugin."),
        rk.XML.stretch(), label="Depends on RKWard version"),
    dep.frame.R <- rk.XML.frame(
        dep.rmin <- rk.XML.input("R min", size="small",
          help="The minimum version number of R required to run this plugin."),
        dep.rmax <- rk.XML.input("R max", size="small",
          help="The maximum version number of R required to run this plugin."),
        rk.XML.stretch(), label="Depends on R version")),
  rk.XML.row(
    dep.optionset.packages <- rk.XML.optionset(
        content=rk.XML.frame(rk.XML.stretch(before=list(
          rk.XML.row(
            dep.pckg.name <- rk.XML.input("Package",
              help="The names of R packages required to run this plugin."),
            dep.pckg.min <- rk.XML.input("min",
              help="The minimum version number of R packages required to run this plugin."),
            dep.pckg.max <- rk.XML.input("max",
              help="The maximum version number of R packages required to run this plugin."),
            dep.pckg.repo <- rk.XML.input("Repository",
              help="The repository to download R packages from required to run this plugin.")
         )
        )), label="Depends on R packages"),
        optioncolumn=list(
          dep.optioncol.pckg.name <- rk.XML.optioncolumn(connect=dep.pckg.name, modifier="text"),
          dep.optioncol.pckg.min <- rk.XML.optioncolumn(connect=dep.pckg.min, modifier="text"),
          dep.optioncol.pckg.max <- rk.XML.optioncolumn(connect=dep.pckg.max, modifier="text"),
          dep.optioncol.pckg.repo <- rk.XML.optioncolumn(connect=dep.pckg.repo, modifier="text")
        )
      )
  ),
  label="Define dependencies", checkable=TRUE, chk=FALSE
)

tab2.create <- rk.XML.col(crt.opts, dep.opts)

# # tab3: varslot to select the actual content
# children.text <- rk.XML.text("If you already created XML content for the plugin, select the main dialog object here (probably a tabbook?)")
# children.var <- rk.XML.row(
#   children.varselector <- rk.XML.varselector(label="Plugin content"),
#   rk.XML.col(
#     cont.dial <- rk.XML.varslot("Select an object of class XiMpLe.node", source=children.varselector, classes="XiMpLe.node",
#       help="If you already created XML content for the plugin, select the main dialog object here."),
#     rk.XML.frame(
#       js.prep <- rk.XML.varslot("preprocess()", source=children.varselector,
#         help="A JavaScript object to be used as the the preprocess() function."),
#       js.calc <- rk.XML.varslot("calculate()", source=children.varselector,
#         help="A JavaScript object to be used as the the calculate() function."),
#       js.prnt <- rk.XML.varslot("printout()", source=children.varselector,
#         help="A JavaScript object to be used as the the printout() function."),
#       rk.XML.stretch())
#   ))
# tab3.children <- rk.XML.col(rk.XML.row(children.text), rk.XML.row(children.var))
help.text.summary <- rk.XML.input("Summary", size="large",
  help="Give a short summary of the plugin for the help page. If empty, the short description is taken as the default.")
help.text.usage <- rk.XML.input("Usage", size="large",
  help="A general note on how to use the plugin.")
help.text <- rk.XML.frame(
  rk.XML.row(help.text.summary),
  rk.XML.row(help.text.usage),
  label="Write help files",
  checkable=TRUE,
  chk=FALSE)
  
tab3.help <- rk.XML.col(help.text)

## glue all of the above together in one tabbook
# sklt.tabbook <- rk.XML.dialog(rk.XML.tabbook("Plugin Skeleton",
#   tab.labels=c("About the plugin", "Create options", "XML content"),
#   children=list(tab1.about, tab2.create, tab3.children)), label="RKWard Plugin Skeleton")
sklt.tabbook <- rk.XML.dialog(rk.XML.tabbook("Plugin Skeleton",
  tabs=list(
    "About the plugin"=tab1.about,
    "Create options"=tab2.create,
    "Help page"=tab3.help)),
  label="RKWard Plugin Skeleton")

## some logic
logic.section <- rk.XML.logic(
    rk.XML.connect(governor=dep.opts, get="checked", client=dep.frame.RKWard, set="enabled"),
    rk.XML.connect(governor=dep.opts, get="checked", client=dep.frame.R, set="enabled")#,
#     rk.XML.connect(governor=dep.opts, client=dep.frame.packages, set="enabled")
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
  ite(pl.hier.name,
    qp("name=\"", pl.hier.name, "\""),
    qp("name=\"", pl.name, "\"")
  ),
  ite(pl.hier,  qp("hierarchy=\"", pl.hier, "\"")),
  funct="list", option="pluginmap", collapse="")
js.opt.skeleton <- rk.JS.options("optSkeleton",
  ite(pl.wiz, qp("\n\tprovides=c(\"logic\", \"dialog\", \"wizard\")"), qp("\n\t#provides=c(\"logic\", \"dialog\")")),
  ite(js.opt.skel.pluginmap,
    qp("\n\t", js.opt.skel.pluginmap),
    qp("\n\t#pluginmap=list(name=\"\", hierarchy=\"\", require=\"\")")
  ),
  ite(id(js.frm.dep.opts, " && (", js.opt.about.dep, " || ", dep.optioncol.pckg.name, ")"),
    qp("\n\tdependencies=plugin.dependencies"),
    qp("\n\t#dependencies=plugin.dependencies")
  ),
  ite(pl.tests, qp("\n\ttests=TRUE"), qp("\n\ttests=FALSE")),
  ite(pl.edit, qp("\n\tedit=TRUE"), qp("\n\tedit=FALSE")),
  ite(pl.add, qp("\n\tload=TRUE"), qp("\n\tload=FALSE")),
  ite(pl.show, qp("\n\tshow=TRUE"), qp("\n\tshow=FALSE")),
  collapse="")

JS.prepare <- rk.paste.JS(
  rk.JS.vars(pl.dir, pl.overw, pl.guessgetter),
  echo("rkwarddev.required(\"0.06-5\")"),
  echo("\n\n# define where the plugin should write its files\noutput.dir <- "),
  ite(pl.dir, echo("\"", pl.dir, "\""), echo("tempdir()")),
  echo("\n# overwrite an existing plugin in output.dir?\noverwrite <- "),
  ite(pl.overw, echo("TRUE"), echo("FALSE")),
  echo("\n# if you set guess.getters to TRUE, the resulting code will need RKWard >= 0.6.0\nguess.getter <- "),
  ite(pl.guessgetter, echo("TRUE"), echo("FALSE")),
  echo("\n\n"),
  level=2)
  
js.frm.help.text <- rk.JS.vars(help.text, modifiers="checked")
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
  ite(id(js.frm.dep.opts, " && (", js.opt.about.dep, " || ", dep.optioncol.pckg.name, ")"),
    rk.paste.JS(
      echo("plugin.dependencies <- rk.XML.dependencies("),
      ite(id(js.opt.about.dep), echo(js.opt.about.dep)),
      ite(id(js.opt.about.dep, " && ", dep.optioncol.pckg.name), echo(",")),
      ite(id(dep.optioncol.pckg.name , "!= \"\""),
        rk.paste.JS(
          echo("\n\tpackage=list(\n\t\t"),
          rk.JS.optionset(dep.optionset.packages,
            echo("c("),
            echo("name=\"", dep.optioncol.pckg.name, "\""),
            ite(dep.optioncol.pckg.min, echo(", min=\"", dep.optioncol.pckg.min, "\"")),
            ite(dep.optioncol.pckg.max, echo(", max=\"", dep.optioncol.pckg.max, "\"")),
            ite(dep.optioncol.pckg.repo, echo(", repository=\"", dep.optioncol.pckg.repo, "\"")),
            echo(")"),
            collapse=",\\n\\t\\t"
          ),
          echo("\n\t)")
        )
      ),
      echo("\n)\n\n"),
    level=3)),
  echo("# name of the main component, relevant for help page content\nrk.set.comp(\""),
  ite(pl.hier.name,
    echo(pl.hier.name, "\")\n\n"),
    echo(pl.name, "\")\n\n")
  ),
  echo("############\n## your plugin dialog and JavaScript should be put here\n############\n\n"),
  ite(js.frm.help.text,
    rk.paste.JS(
      echo("############\n## help page\nplugin.summary <- rk.rkh.summary(\n\t"),
      ite(help.text.summary, echo("\"", help.text.summary, "\"\n)"), echo("\"", pl.desc, "\"\n)")),
      echo("\nplugin.usage <- rk.rkh.usage(\n\t\"", help.text.usage, "\"\n)\n\n"),
    level=3)),
  echo("#############\n",
    "## the main call\n",
    "## if you run the following function call, files will be written to output.dir!\n",
    "#############\n",
    "# this is where things get serious, that is, here all of the above is put together into one plugin\n",
    "plugin.dir <- rk.plugin.skeleton(\n\tabout=about.plugin,"),
  ite(id(js.frm.dep.opts, " && ", js.opt.about.dep), echo("\n\tdependencies=plugin.dependencies,")),
  echo("\n\tpath=output.dir,",
    "\n\tguess.getter=guess.getter,",
    "\n\tscan=c(\"var\", \"saveobj\", \"settings\"),",
    "\n\txml=list(\n\t\t#dialog=,\n\t\t#wizard=,\n\t\t#logic=,\n\t\t#snippets=\n\t),",
    "\n\tjs=list(\n\t\t#results.header=FALSE,\n\t\t#load.silencer=,\n\t\t#require=,\n\t\t#variables=,",
      "\n\t\t#globals=,\n\t\t#preprocess=,\n\t\t#calculate=,\n\t\t#printout=,\n\t\t#doPrintout=\n\t),"
  ),
  ite(js.frm.help.text,
    echo(
      "\n\trkh=list(\n\t\tsummary=plugin.summary,\n\t\tusage=plugin.usage#,",
      "\n\t\t#sections=,\n\t\t#settings=,\n\t\t#related=,\n\t\t#technical=\n\t),",
      "\n\tcreate=c(\"pmap\", \"xml\", \"js\", \"desc\", \"rkh\"),"
    ),
    echo("\n\trkh=list(","\n\t\t#summary=,\n\t\t#usage=,",
      "\n\t\t#sections=,\n\t\t#settings=,\n\t\t#related=,\n\t\t#technical=\n\t),",
      "\n\tcreate=c(\"pmap\", \"xml\", \"js\", \"desc\"),"
    )
  ),
  echo("\n\toverwrite=overwrite,"),
  echo("\n\t#components=list(),"),
  echo(js.opt.skeleton),
  echo("\n)\n\n"),
  level=2)

############
## help file

rkh.summary <- rk.rkh.summary("Generate a plugin skeleton for RKWard.")

rkh.usage <- rk.rkh.usage("This plugin is both, an example for a plugin written with the rkwarddev package,
 and a quick way to get a skeleton for new plugins.")

#############
## the main call
## if you run the following function call, files will be written to output.dir!
#############
# this is where it get's serious, that is, here all of the above is put together into one plugin
#plugin.dir <<- rk.plugin.skeleton(
rk.plugin.skeleton(
  about=about.info,
  path=output.dir,
  guess.getter=TRUE,
  xml=list(
    dialog=sklt.tabbook,
    logic=logic.section),
  js=list(
    results.header=FALSE,
    require="rkwarddev",
    preprocess=JS.prepare,
    calculate=JS.calculate,
    printout=""),
  rkh=list(
    summary=rkh.summary,
    usage=rkh.usage
  ),
  pluginmap=list(name="Create RKWard plugin skeleton", hierarchy=list("file", "export")),
  overwrite=TRUE,
  create=c("pmap","xml","js","desc", "rkh"),
  dependencies=dependencies.info,
  tests=FALSE,
  show=TRUE,
#  load=TRUE,
#  edit=TRUE,
  hints=FALSE)
})
