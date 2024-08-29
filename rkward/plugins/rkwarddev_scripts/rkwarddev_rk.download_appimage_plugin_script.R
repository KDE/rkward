require(rkwarddev)
rkwarddev.required("0.08-1")

rk.local({
# define where the plugin should write its files
output.dir <- tempdir()
# overwrite an existing plugin in output.dir?
overwrite <- TRUE
# if you set guess.getters to TRUE, the resulting code will need RKWard >= 0.6.0
guess.getter <- TRUE
# define the indentation character for the generated code
rk.set.indent(by="  ")
# should empty "else" clauses be kept in the JavaScript code?
rk.set.empty.e(TRUE)
# make your plugin translatable by setting this to TRUE
update.translations <- FALSE

aboutPlugin <- rk.XML.about(
  name="rk.downloadAppImage",
  author=c(
    person(
      given="Meik",
      family="Michalke",
      email="meik.michalke@hhu.de",
      role=c("aut", "cre")
    )
  ),
  about=list(
    desc="Adds a dialog to install or update an AppImage of RKWard",
    version="0.01-0",
    url="https://rkward.kde.org",
    license="GPL (>= 3)"
  )
)

plugin.dependencies <- rk.XML.dependencies(
  dependencies=list(
    rkward.min="0.8.0"
  ),
  package=list(
    c(name="XiMpLe", min="0.11-3")
  )
)

# name of the main component, relevant for help page content
rk.set.comp("Download AppImage")

############
## your plugin dialog and JavaScript should be put here
############

aiu_file <- rk.XML.browser(
  label="Save to",
  type = "savefile",
  initial = NULL, # dirname(Sys.getenv("APPIMAGE")) basename(Sys.getenv("APPIMAGE"))
  filter = "*.AppImage",
  id.name = "aiu_file",
  help = "Select a target file to save the AppImage to."
)

noLoadMsg <- rk.XML.cbox(
  label="Suppress package loading messages",
  value="true",
  chk=TRUE,
  id.name="noLoadMsg"
)

tab_file <- rk.XML.row(
  rk.XML.col(
    rk.XML.frame(
      rk.XML.text(
        "Please note that downloading the AppImage file might take a while. Your R session can be occpied for some minutes once you submit. For a live feedback of the download progress, please run the <code>rk.download_appimage()</code> call in the R console manually.",
        type="warning",
        id.name="aiu_file_warning"
      )
    ),
    rk.XML.frame(
      aiu_file
    ),
    rk.XML.stretch(),
    noLoadMsg
  )
)

aiu_url <- rk.XML.input(
  label = "URL",
  initial = "https://cdn.kde.org/ci-builds/education/rkward/master/linux",
  size = "medium",
  required = TRUE,
  id.name = "aiu_url",
  help = "URL to look for the AppImage files."
)

aiu_pattern <- rk.XML.input(
  label = "Pattern",
  initial = "rkward-master.*linux-gcc-x86_64\\\\.AppImage",
  size = "medium",
  required = TRUE,
  id.name = "aiu_pattern",
  help = "Regular expression to find links to the AppImage in the HTML code of the URL."
)

tab_source <- rk.XML.row(
  rk.XML.col(
    aiu_url,
    aiu_pattern,
    rk.XML.stretch()
  )
)

aiu_method <- rk.XML.dropdown(
  label = "Download method",
  options = list(
    "auto"=c(
      val = "auto",
      chk = TRUE
    ),
    "internal"=c(
      val = "internal"
    ),
    "libcurl"=c(
      val = "libcurl"
    ),
    "wget"=c(
      val = "wget"
    ),
    "curl"=c(
      val = "curl"
    ),
    "wininet (Windows only)"=c(
      val = "wininet"
    )
  ),
  id.name = "aiu_method",
  help = "The method to use by download.file()."
)

# download.file {utils}
aiu_cacheok <- rk.XML.checkbox(
  "Allow cached files",
  chk = FALSE,
  id.name = "aiu_cacheok",
  help = "Is a server-side cached value acceptable? Disabling this is useful for ‘⁠http://⁠’ and ‘⁠https://⁠’ URLs. It will attempt to get a copy directly from the site rather than from an intermediate cache."
)

aiu_timeout <- rk.XML.spinbox(
  label = "Timeout",
  min = 400,
  initial = 400, # max(400, getOption("timeout"))
  real = FALSE,
  id.name = "aiu_timeout",
  help = "The number of seconds allowed to try downloading the AppImage."
)

tab_download <- rk.XML.row(
  rk.XML.col(
    aiu_method,
    aiu_cacheok,
    aiu_timeout,
    rk.XML.stretch()
  )
)

pluginDialog <- rk.XML.dialog(
  rk.XML.tabbook(
    label = "AppImage",
    tabs = list(
      "File" = tab_file,
      "Source" = tab_source,
      "Download" = tab_download
    )
  ),
  label = "Download AppImage"
)

################
## logic section

aiu_logic <- rk.XML.logic(rk.comment(id("
  doRCommand('Sys.getenv(\"APPIMAGE\")', \"commandFinished\");
        commandFinished = function (result, id) {
          if (result != \"\") {
            gui.setValue(\"", aiu_file, ".selection\", result);
            return;
          }
        }
", js=FALSE)))


#############
## JavaScript

overwriteChecked <- rk.JS.vars(aiu_file, modifiers="overwrite")
# dummy vars used for scripting
file_basename <- rk.JS.vars("fileBasename")
file_dirname <- rk.JS.vars("fileDirname")
aiu_js_calc <- rk.paste.JS(
  overwriteChecked,
  js(
    "var ", file_basename, " = ", aiu_file, ".replace(/.*\\//, '');",
    linebreaks = FALSE
  ),
  js(
    "var ", file_dirname, " = ", aiu_file, ".match(/.*\\//);",
    linebreaks = FALSE
  ),
  echo(
    "appimage <- rk.with.progress(\n  {rk.download_appimage(\n",
    "    dir = \"", file_dirname, "\"",          # dir = dirname(Sys.getenv("APPIMAGE"))
    ",\n    filename = \"", file_basename, "\""  # filename = "rkward-master-linux-gcc-x86_64.AppImage"
  ),
  js(
    if(overwriteChecked){
      echo(",\n    overwrite = TRUE")
    } else {
      echo(",\n    overwrite = FALSE")
    }
  ),
  echo(
    ",\n    url = \"", aiu_url, "\"",                # url = "https://cdn.kde.org/ci-builds/education/rkward/master/linux"
    ",\n    pattern = \"", aiu_pattern, "\"",        # pattern = "rkward-master.*linux-gcc-x86_64\\.AppImage"
    ",\n    method = \"", aiu_method, "\""           # method = "auto"
  ),
  tf(
    aiu_cacheok,                                   #   , cacheOK = FALSE
    opt = "cacheOK",
    level = 4
  ),
  echo(",\n    timeout = ", aiu_timeout,             # timeout =  max(400, getOption("timeout"))
    "\n  )},\n  text = \"Downloading RKWard AppImage...\"\n)\n\n"
  )
)

aiu_js_print <- rk.paste.JS(
  echo(
    "rk.print.literal(\"AppImage was saved as:\")\n",
    "rk.results(appimage)\n",
    "rk.print.literal(\"You must restart RKWard to use it.\")\n"
  )
)

############
## help page
plugin.summary <- rk.rkh.summary(
  "Adds a dialog to install or update an AppImage of RKWard"
)
plugin.usage <- rk.rkh.usage(
  ""
)

#############
## the main call
## if you run the following function call, files will be written to output.dir!
#############
# this is where things get serious, that is, here all of the above is put together into one plugin
plugin.dir <- rk.plugin.skeleton(
  about=aboutPlugin,
  #dependencies=plugin.dependencies,
  path=output.dir,
  guess.getter=guess.getter,
  scan=c("var", "saveobj", "settings"),
  xml=list(
      dialog=pluginDialog
#     , wizard=
    , logic=aiu_logic
#     , snippets=
  ),
  js=list(
#       results.header=FALSE
      load.silencer=noLoadMsg
    , require="XiMpLe"
#     , variables=
#     , globals=
#     , preprocess=
    , calculate=aiu_js_calc
    , printout=aiu_js_print
#     , doPrintout=
  ),
  rkh=list(
      summary=plugin.summary
    , usage=plugin.usage
#     , sections=
#     , settings=
#     , related=
#     , technical=
  ),
  create=c("pmap", "xml", "js", "desc", "rkh"),
  overwrite=overwrite,
  #components=list(),, 
  #provides=c("logic", "dialog"), 
  pluginmap=list(name="Download AppImage", hierarchy="file"), 
  tests=FALSE, 
  # edit=TRUE, 
  load=TRUE, 
  show=FALSE,
  gen.info="$SRC/inst/rkward/rkwarddev_rk.download_appimage_plugin_script.R"
)

# you can make your plugin translatable, see top of script
if(isTRUE(update.translations)){
  rk.updatePluginMessages(
    file.path(output.dir,"rk.downloadAppImage","inst","rkward","rk.downloadAppImage.pluginmap"),
    # where should translation bug reports go?
    bug_reports="https://mail.kde.org/mailman/listinfo/kde-i18n-doc"
  )
} else {}

})
