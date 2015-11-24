## create dialog to build a plugin skeleton
require(rkwarddev)
rkwarddev.required("0.07-4")

local({
# define where the plugin should write its files
output.dir <- tempdir()
# overwrite an existing plugin in output.dir?
overwrite <- TRUE
rk.set.indent(by="  ")
rk.set.empty.e(TRUE)
update.translations <- TRUE
# the script generates a folder called "RKWardPluginSkeleton", set the
# following to FALSE if you want to translate plugin files in the
# rkwarddev package sources instead
standalonePlugin <- TRUE

about.info <- rk.XML.about(
  name="RKWard Plugin Skeleton",
  author=c(
    person(given="Meik", family="Michalke",
      email="meik.michalke@hhu.de", role=c("aut","cre"))),
  about=list(desc="GUI interface to create RKWard plugin skeletons",
    # the version number should be in line with rkwarddev
    # to reflect when the script code was changed
    version="0.07-4", url="http://rkward.kde.org")
  )
dependencies.info <- rk.XML.dependencies(dependencies=list(rkward.min="0.6.0"))

rk.set.comp("Create RKWard plugin skeleton")

# tab1: information on the thing
aboutPlugin <- rk.XML.frame(
  rk.XML.row(
    pluginName <- rk.XML.input("Plugin name", required=TRUE, size="small", id.name="pluginName",
      help="Give the name for your new plugin here."),
    pluginLicense <- rk.XML.input("License", initial="GPL (>= 3)", required=TRUE, id.name="pluginLicense",
      help="Define the license for your plugin. A short form should be sufficient.")),
  rk.XML.row(pluginDescription <- rk.XML.input("Short description", required=TRUE, id.name="pluginDescription",
    help="Describe your plugin in a few sentences: What does it do?")),
  rk.XML.row(
    pluginVersion <- rk.XML.input("Version number", initial="0.01-0", required=TRUE, id.name="pluginVersion",
      help="Version information for your plugin."),
    pluginDate <- rk.XML.input("Release date (empty for today)", id.name="pluginDate",
      help="The release date of your plugin. If you leave this empty, the current date will be used automatically.")),
  rk.XML.row(
    pluginHomepage <- rk.XML.input("Homepage", id.name="pluginHomepage",
      help="A URL where one can find more information on the plugin, download updates etc."),
    pluginCategory <- rk.XML.input("Category", id.name="pluginCategory",
      help="A category for your plugin. This infromation is currently ignored by RKWard.")),
  label="About the plugin",
  id.name="aboutPlugin"
)
# aboutContact <- rk.XML.frame(
#   rk.XML.row(
#     rk.XML.col(
#       authorGivenName <- rk.XML.input("Given name", required=TRUE,
#         help="First name of the package author."),
#       authorFamiliyName <- rk.XML.input("Family name", required=TRUE,
#         help="Family name of the package author."),
#       authorMail <- rk.XML.input("E-mail", required=TRUE,
#         help="The authors e-mail address, important for bug reports and receiving a myriad of thank yous..."),
#       rk.XML.stretch()),
#     rk.XML.col(rk.XML.frame(
#       authorAut <- rk.XML.cbox("Author", chk=TRUE,
#         help="Check this if you are the author of the plugin code."),
#       authorCre <- rk.XML.cbox("Maintainer", chk=TRUE,
#         help="Check this if you maintain the plugin package."),
#       rk.XML.stretch(), label="Author roles"))),
#   label="Plugin author")

aboutContact <- rk.XML.frame(
  rk.XML.row(
    optionsetAuthors <- rk.XML.optionset(
      content=rk.XML.frame(rk.XML.stretch(before=list(
        rk.XML.row(
        aboutContactFrame <- rk.XML.frame(
          rk.XML.row(
            rk.XML.col(
              authorGivenName <- rk.XML.input("Given name", required=TRUE, id.name="authorGivenName",
                help="First name of the package author."),
              authorFamiliyName <- rk.XML.input("Family name", required=TRUE, id.name="authorFamiliyName",
                help="Family name of the package author."),
              authorMail <- rk.XML.input("E-mail", required=FALSE, id.name="authorMail",
                help="The authors e-mail address, important for bug reports and receiving a myriad of thank yous..."),
              rk.XML.stretch()),
            rk.XML.col(rk.XML.frame(
              authorAut <- rk.XML.cbox("Author", chk=TRUE, id.name="authorAut",
                help="Check this if this person is the author of the plugin code."),
              authorCre <- rk.XML.cbox("Maintainer", chk=TRUE, id.name="authorCre",
                help="Check this if this person maintains the plugin package."),
              authorCtb <- rk.XML.cbox("Contributor", chk=FALSE, id.name="authorCtb",
                help="Check this if this person is a contributor to the plugin code (e.g., translations)."),
              rk.XML.stretch(), label="Roles"))),
            label="Package author",
            id.name="aboutContactFrame"
          )
        )
      )), label="Package authors"),
      optioncolumn=list(
        optcolAuthorGivenName <- rk.XML.optioncolumn(connect=authorGivenName, modifier="text", id.name="optcolAuthorGivenName"),
        optcolAuthorFamiliyName <- rk.XML.optioncolumn(connect=authorFamiliyName, modifier="text", id.name="optcolAuthorFamiliyName"),
        optcolAuthorMail <- rk.XML.optioncolumn(connect=authorMail, modifier="text", id.name="optcolAuthorMail"),
        optcolAuthorAut <- rk.XML.optioncolumn(connect=authorAut, modifier="state", id.name="optcolAuthorAut"),
        optcolAuthorCre <- rk.XML.optioncolumn(connect=authorCre, modifier="state", id.name="optcolAuthorCre"),
        optcolAuthorCtb <- rk.XML.optioncolumn(connect=authorCtb, modifier="state", id.name="optcolAuthorCtb")
      ),
      logic=rk.XML.logic(
        rk.XML.connect(governor=authorCre, client=authorMail, set="required")
      ),
      id.name="optionsetAuthors"
    )
  ),
  label="Plugin author",
  id.name="aboutContact"
)


tab1.about <- rk.XML.col(aboutPlugin, aboutContact)

# tab2: create options
createOptionsFrame <- rk.XML.frame(
    rk.XML.row(outDir <- rk.XML.browser("Directory to save to (empty for $TEMPDIR)", type="dir", required=FALSE, id.name="outDir",
      help="Set the directory where all plugin files and its directory structure should be generated. The default is a temporary directory.")),
    rk.XML.row(
      rk.XML.col(
        overwrite <- rk.XML.cbox("Overwrite existing files", chk=FALSE, id.name="overwrite",
          help="If this is checked, existing files in the specified target directory will probably be replaced by new ones."),
        addWizard <- rk.XML.cbox("Add wizard section", chk=FALSE, id.name="addWizard",
          help="If this is checked, a wizard section will be included in the skeleton."),
        addTests <- rk.XML.cbox("Include plugin tests", chk=TRUE, id.name="addTests",
          help="If this is checked, plugin tests will be included in the skeleton."),
        rk.XML.stretch()),
      rk.XML.col(
        editPlugin <- rk.XML.cbox("Open files for editing", chk=TRUE, id.name="editPlugin",
          help="If this is checked, all generated files will be opened for editing instantly."),
        addToConfig <- rk.XML.cbox("Add plugin to RKWard configuration", chk=TRUE, id.name="addToConfig",
          help="If this is checkend, the generated plugin will automatically be registered in RKWard's configuration.
            If you store it in a temporary directory and remove it before the next start of RKWard, the entry will removed again as well."),
        showPlugin <- rk.XML.cbox("Show the plugin", chk=FALSE, id.name="showPlugin",
          help="If this is checked, the generated plugin will be shown (opened) for you to see what it looks like."),
        guessGetters <- rk.XML.cbox("Guess getter functions (RKWard >= 0.6.0)", chk=FALSE, id.name="guessGetters",
          help="If this is checked, rkwarddev tries to select the optimal getter functions to get data from the dialog into the R code. The plugin then requires RKWard >= 0.6.0."),
        rk.XML.stretch())
    ),
    rk.XML.frame(
      rk.XML.row(menuHier <- rk.XML.dropdown("Place in top menu",
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
        id.name="menuHier",
        help="Specify where the plugin should appear in RKWard's top menus."
      ),
      menuName <- rk.XML.input("Name in menu (plugin name if empty)", id.name="menuName",
        help="You can set the exact entry name of your main component in the menu here. If left empty, the plugin name will be used as default.")
      )
    ),
    id.name="createOptionsFrame"
  )
dependencyFrame <- rk.XML.frame(
  rk.XML.row(
    RKFrame <- rk.XML.frame(
        RKMin <- rk.XML.input("RKWard min", size="small", id.name="RKMin",
          help="The minimum version number of RKWard required to run this plugin."),
        RKMax <- rk.XML.input("RKWard max", size="small", id.name="RKMax",
          help="The maximum version number of RKWard required to run this plugin."),
        rk.XML.stretch(), label="Depends on RKWard version", id.name="RKFrame"),
    RFrame <- rk.XML.frame(
        RMin <- rk.XML.input("R min", size="small", id.name="RMin",
          help="The minimum version number of R required to run this plugin."),
        RMax <- rk.XML.input("R max", size="small", id.name="RMax",
          help="The maximum version number of R required to run this plugin."),
        rk.XML.stretch(), label="Depends on R version", id.name="RFrame")),
  rk.XML.row(
    dependencyOptionset <- rk.XML.optionset(
        content=rk.XML.frame(rk.XML.stretch(before=list(
          rk.XML.row(
            pckgName <- rk.XML.input("Package", id.name="pckgName",
              help="The names of R packages required to run this plugin."),
            pckgMin <- rk.XML.input("min", id.name="pckgMin",
              help="The minimum version number of R packages required to run this plugin."),
            pckgMax <- rk.XML.input("max", id.name="pckgMax",
              help="The maximum version number of R packages required to run this plugin."),
            pckgRepo <- rk.XML.input("Repository", id.name="pckgRepo",
              help="The repository to download R packages from required to run this plugin.")
         )
        )), label="Depends on R packages"),
        optioncolumn=list(
          optcolPckgName <- rk.XML.optioncolumn(connect=pckgName, modifier="text", id.name="optcolPckgName"),
          optcolPckgMin <- rk.XML.optioncolumn(connect=pckgMin, modifier="text", id.name="optcolPckgMin"),
          optcolPckgMax <- rk.XML.optioncolumn(connect=pckgMax, modifier="text", id.name="optcolPckgMax"),
          optcolPckgRepo <- rk.XML.optioncolumn(connect=pckgRepo, modifier="text", id.name="optcolPckgRepo")
        ),
        id.name="dependencyOptionset"
      )
  ),
  label="Define dependencies",
  checkable=TRUE,
  chk=FALSE,
  id.name="dependencyFrame"
)

tab2.create <- rk.XML.col(createOptionsFrame, dependencyFrame)

# # tab3: varslot to select the actual content
# children.text <- rk.XML.text("If you already created XML content for the plugin, select the main dialog object here (probably a tabbook?)")
# children.var <- rk.XML.row(
#   children.varselector <- rk.XML.varselector(label="Plugin content"),
#   rk.XML.col(
#     cont.dial <- rk.XML.varslot("Select an object of class XiMpLe.node", source=children.varselector, classes="XiMpLe.node",
#       help="If you already created XML content for the plugin, select the main dialog object here."),
#     rk.XML.frame(
#       js.prep <- rk.XML.varslot("preprocess()", source=children.varselector,
#         help="A JavaScript object to be used as the preprocess() function."),
#       js.calc <- rk.XML.varslot("calculate()", source=children.varselector,
#         help="A JavaScript object to be used as the calculate() function."),
#       js.prnt <- rk.XML.varslot("printout()", source=children.varselector,
#         help="A JavaScript object to be used as the printout() function."),
#       rk.XML.stretch())
#   ))
# tab3.children <- rk.XML.col(rk.XML.row(children.text), rk.XML.row(children.var))
helpSummary <- rk.XML.input("Summary", size="large", id.name="helpSummary",
  help="Give a short summary of the plugin for the help page. If empty, the short description is taken as the default.")
helpUsage <- rk.XML.input("Usage", size="large", id.name="helpUsage",
  help="A general note on how to use the plugin.")
helpText <- rk.XML.frame(
  rk.XML.row(helpSummary),
  rk.XML.row(helpUsage),
  label="Write help files",
  checkable=TRUE,
  chk=FALSE,
  id.name="helpText"
)
  
tab3.help <- rk.XML.col(helpText)

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
    rk.XML.connect(governor=dependencyFrame, get="checked", client=RKFrame, set="enabled"),
    rk.XML.connect(governor=dependencyFrame, get="checked", client=RFrame, set="enabled")#,
#     rk.XML.connect(governor=dependencyFrame, client=dep.frame.packages, set="enabled")
  )

## JS code generation
# about section
js.opt.about.about <- rk.JS.options("optAbout",
  .ite=js(
    if(pluginDescription){
      qp("desc=\"",pluginDescription,"\"")
    } else {},
    if(pluginVersion){
      qp("version=\"",pluginVersion,"\"")
    } else {},
    if(pluginDate){
      qp("date=\"",pluginDate,"\"")
    } else {},
    if(pluginHomepage){
      qp("url=\"",pluginHomepage,"\"")
    } else {},
    if(pluginLicense){
      qp("license=\"",pluginLicense,"\"")
    } else {},
    if(pluginCategory){
      qp("category=\"",pluginCategory,"\"")
    } else {},
    keep.ite=TRUE
  ),
  funct="list", option="about", collapse=",\\n\\t")
# dependencies section
js.frm.dependencyFrame <- rk.JS.vars(dependencyFrame, modifiers="checked") # see to it frame is checked
js.opt.about.dep <- rk.JS.options("optDependencies",
  .ite=js(
    if(js.frm.dependencyFrame && RKMin){
      qp("rkward.min=\"",RKMin,"\"")
    } else {},
    if(js.frm.dependencyFrame && RKMax){
      qp("rkward.max=\"",RKMax,"\"")
    } else {},
    if(js.frm.dependencyFrame && RMin){
      qp("R.min=\"",RMin,"\"")
    } else {},
    if(js.frm.dependencyFrame && RMax){
      qp("R.max=\"",RMax,"\"")
    } else {},
    keep.ite=TRUE
  ),
  funct="list", option="dependencies", collapse=",\\n\\t")

js.opt.skel.pluginmap <- rk.JS.options("optPluginmap",
  .ite=js(
    if(menuName){
      qp("name=\"", menuName, "\"")
    } else {
      qp("name=\"", pluginName, "\"")
    },
    if(menuHier){
      qp("hierarchy=\"", menuHier, "\"")
    } else {},
    keep.ite=TRUE
  ),
  funct="list", option="pluginmap", collapse="", opt.sep="")
js.opt.skeleton <- rk.JS.options("optSkeleton",
  .ite=js(
    if(addWizard){
      qp("\n\tprovides=c(\"logic\", \"dialog\", \"wizard\")")
    } else {
      qp("\n\t#provides=c(\"logic\", \"dialog\")")
    },
    if(js.opt.skel.pluginmap){
      qp("\n\t", js.opt.skel.pluginmap)
    } else {
      qp("\n\t#pluginmap=list(name=\"\", hierarchy=\"\", require=\"\")")
    },
  #   ite(id(js.frm.dependencyFrame, " && (", js.opt.about.dep, " || ", optcolPckgName, ")"),
  #     qp("\n\tdependencies=plugin.dependencies"),
  #     qp("\n\t#dependencies=plugin.dependencies")
  #   ),
    if(addTests){
      qp("\n\ttests=TRUE")
    } else {
      qp("\n\ttests=FALSE")
    },
    if(editPlugin){
      qp("\n\tedit=TRUE")
    } else {
      qp("\n\tedit=FALSE")
    },
    if(addToConfig){
      qp("\n\tload=TRUE")
    } else {
      qp("\n\tload=FALSE")
    },
    if(showPlugin){
      qp("\n\tshow=TRUE")
    } else {
      qp("\n\tshow=FALSE")
    },
    keep.ite=TRUE
  ),
  collapse="")

JS.prepare <- rk.paste.JS(
  rk.JS.vars(outDir, overwrite, guessGetters),
  echo("rkwarddev.required(\"0.07-4\")"),
  echo("\n\n# define where the plugin should write its files\noutput.dir <- "),
  js(
    if(outDir){
      echo("\"", outDir, "\"")
    } else {
      echo("tempdir()")
    },
    echo("\n# overwrite an existing plugin in output.dir?\noverwrite <- "),
    if(overwrite){
      echo("TRUE")
    } else {
      echo("FALSE")
    },
    echo("\n# if you set guess.getters to TRUE, the resulting code will need RKWard >= 0.6.0\nguess.getter <- "),
    if(guessGetters){
      echo("TRUE")
    } else {
      echo("FALSE")
    }
  ),
  echo("\n\n"),
  level=2)
  
js.frm.helpText <- rk.JS.vars(helpText, modifiers="checked")
JS.calculate <- rk.paste.JS(
  js.opt.about.about,
  js.opt.about.dep,
  js.opt.skel.pluginmap,
  js.opt.skeleton,
  echo("aboutPlugin <- rk.XML.about("),
    js(
      if(pluginName){
        echo("\n\tname=\"", pluginName, "\"")
      } else {}
    ),
    # author section
    rk.JS.optionset(optionsetAuthors, vars=TRUE, guess.getter=TRUE),
    ite(id(optcolAuthorGivenName, " != \"\""),
      rk.paste.JS(
        echo("\tauthor=c(\n\t\t\t"),
        rk.JS.optionset(optionsetAuthors,
          js.optionsetAuthors.role <- rk.JS.options("optAuthorRole",
            .ite=js(
              if(optcolAuthorAut == 1){
                qp("\"aut\"")
              } else {},
              if(optcolAuthorCre == 1){
                qp("\"cre\"")
              } else {},
              if(optcolAuthorCtb == 1){
                qp("\"ctb\"")
              } else {},
              keep.ite=TRUE
            ),
            funct="c", option="role", collapse=""),
          echo("person("),
          echo("given=\"", optcolAuthorGivenName, "\""),
          js(
            if(optcolAuthorFamiliyName){
              echo(", family=\"", optcolAuthorFamiliyName, "\"")
            } else {},
            if(optcolAuthorMail){
              echo(", email=\"", optcolAuthorMail, "\"")
            } else {},
            if(js.optionsetAuthors.role){
              echo(js.optionsetAuthors.role)
            } else {},
            level=3
          ),
          echo(")"),
          collapse=",\\n\\t\\t\\t"
        ),
        echo("\n\t\t),\n")
      )
    ),
    echo(js.opt.about.about),
  echo("\n)\n\n"),
  ite(id(js.frm.dependencyFrame, " && (", js.opt.about.dep, " || ", optcolPckgName, ")"),
    rk.paste.JS(
      echo("plugin.dependencies <- rk.XML.dependencies("),
      js(
        if(js.opt.about.dep){
          echo(js.opt.about.dep)
        } else {},
        if(js.opt.about.dep && optcolPckgName){
          echo(",")
        } else {},
        level=3
      ),
      ite(id(optcolPckgName , "!= \"\""),
        rk.paste.JS(
          echo("\n\tpackage=list(\n\t\t"),
          rk.JS.optionset(dependencyOptionset,
            echo("c("),
            echo("name=\"", optcolPckgName, "\""),
            js(
              if(optcolPckgMin){
                echo(", min=\"", optcolPckgMin, "\"")
              } else {},
              if(optcolPckgMax){
                echo(", max=\"", optcolPckgMax, "\"")
              } else {},
              if(optcolPckgRepo){
                echo(", repository=\"", optcolPckgRepo, "\"")
              } else {},
              level=5
            ),
            echo(")"),
            collapse=",\\n\\t\\t"
          ),
          echo("\n\t)"),
          level=4
        )
      ),
      echo("\n)\n\n"),
    level=3)),
  echo("# name of the main component, relevant for help page content\nrk.set.comp(\""),
  js(
    if(menuName){
      echo(menuName, "\")\n\n")
    } else {
      echo(pluginName, "\")\n\n")
    },
    echo("############\n## your plugin dialog and JavaScript should be put here\n############\n\n"),
    if(js.frm.helpText){
      echo("############\n## help page\nplugin.summary <- rk.rkh.summary(\n\t")
      if(helpSummary){
        echo("\"", helpSummary, "\"\n)")
      } else {
        echo("\"", pluginDescription, "\"\n)")
      }
      echo("\nplugin.usage <- rk.rkh.usage(\n\t\"", helpUsage, "\"\n)\n\n")
    } else {},
    echo("#############\n",
      "## the main call\n",
      "## if you run the following function call, files will be written to output.dir!\n",
      "#############\n",
      "# this is where things get serious, that is, here all of the above is put together into one plugin\n",
      "plugin.dir <- rk.plugin.skeleton(\n\tabout=aboutPlugin,"),
    if(js.frm.dependencyFrame && js.opt.about.dep){
      echo("\n\tdependencies=plugin.dependencies,")
    } else {
      echo("\n\t#dependencies=plugin.dependencies,")
    },
    echo("\n\tpath=output.dir,",
      "\n\tguess.getter=guess.getter,",
      "\n\tscan=c(\"var\", \"saveobj\", \"settings\"),",
      "\n\txml=list(\n\t\t#dialog=,\n\t\t#wizard=,\n\t\t#logic=,\n\t\t#snippets=\n\t),",
      "\n\tjs=list(\n\t\t#results.header=FALSE,\n\t\t#load.silencer=,\n\t\t#require=,\n\t\t#variables=,",
        "\n\t\t#globals=,\n\t\t#preprocess=,\n\t\t#calculate=,\n\t\t#printout=,\n\t\t#doPrintout=\n\t),"
    ),
    if(js.frm.helpText){
      echo(
        "\n\trkh=list(\n\t\tsummary=plugin.summary,\n\t\tusage=plugin.usage#,",
        "\n\t\t#sections=,\n\t\t#settings=,\n\t\t#related=,\n\t\t#technical=\n\t),",
        "\n\tcreate=c(\"pmap\", \"xml\", \"js\", \"desc\", \"rkh\"),"
      )
    } else {
      echo("\n\trkh=list(","\n\t\t#summary=,\n\t\t#usage=,",
        "\n\t\t#sections=,\n\t\t#settings=,\n\t\t#related=,\n\t\t#technical=\n\t),",
        "\n\tcreate=c(\"pmap\", \"xml\", \"js\", \"desc\"),"
      )
    }
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
  hints=FALSE,
  gen.info="$SRC/demo/skeleton_dialog.R")
  
  if(isTRUE(update.translations)){
    if(isTRUE(standalonePlugin)){
      rk.updatePluginMessages(file.path(output.dir,"RKWardPluginSkeleton","inst","rkward","RKWardPluginSkeleton.pluginmap"))
    } else {
      rk.updatePluginMessages(file.path(output.dir,"rkwarddev","inst","rkward","rkwarddev.pluginmap"))
    }
  } else {}
})
