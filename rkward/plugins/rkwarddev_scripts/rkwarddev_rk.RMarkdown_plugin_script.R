# - This file is part of the RKWard project.
# SPDX-FileCopyrightText: by Meik Michalke <meik.michalke@hhu.de>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
# require(rkwarddev)
rkwarddev.required("0.08-3")

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
update.translations <- TRUE

aboutPlugin <- rk.XML.about(
  name="rk.RMarkdown",
  author=c(
    person(given="Meik", family="Michalke", email="meik.michalke@hhu.de", role=c("aut", "cre"))
  ),
  about=list(
    desc="RKWard GUI to export RMarkdown documents in various formats",
    version="0.01-1",
    url="https://rkward.kde.org",
    license="GPL (>= 3)"
  )
)

plugin.dependencies <- rk.XML.dependencies(
  dependencies=list(rkward.min="0.7.0"),
  package=list(
    c(name="knitr"),
    c(name="markdown"),
    c(name="rmarkdown")
  )
)

# name of the main component, relevant for help page content
rk.set.comp("Export RMarkdown")

############
## your plugin dialog and JavaScript should be put here
############

markdownFile <- rk.XML.browser(
  "RMarkdown file",
  filter=c("*.rmd", "*.md"),
  id.name="markdownFile",
  help="Choose the file to be exported."
)

targetFile <- rk.XML.browser(
  "Save as",
  type="savefile",
  required=FALSE,
  id.name="targetFile",
  help="Choose the target file name to export to."
)

targetDir <- rk.XML.browser(
  "Save to",
  type="dir",
  required=FALSE,
  id.name="targetDir",
  help="Choose the target directory to export to, file names are automatically generated according to the formats defined in the document."
)

targetFormat <- rk.XML.dropdown("Target format",
  options=list(
    "All formats defined in document"   =c(val="all"),              # all
    "First format defined in document"  =c(val="first", chk=TRUE),  # NULL
    "Beamer presentation"               =c(val="beamer"),           # beamer_presentation
    "DokuWiki"                          =c(val="dokuwiki"),         #   (no render object)
    "HTML"                              =c(val="html"),             # html_document
    "HTML 5"                            =c(val="html5"),            #   (no render object)
    "Markdown"                          =c(val="markdown"),         # md_document
    "Markdown (GitHub, old)"            =c(val="markdown_github"),  # github_document
    "Markdown (GitHub)"                 =c(val="gfm"),              # github_document
    "MediaWiki"                         =c(val="mediawiki"),        #   (no render object)
    "ODT"                               =c(val="odt"),              # odt_document
    "PDF"                               =c(val="latex"),            # pdf_document
    "PowerPoint"                        =c(val="pptx"),             # powerpoint_presentation
    "RTF"                               =c(val="rtf"),              # rtf_document
    "Vignette (HTML)"                   =c(val="html_vignette"),    # html_vignette
    "Word"                              =c(val="docx")              # word_document
  ),
  id.name="targetFormat",
  help="Set the target format for exported files. If you choose either \"All formats defined in document\" or \"First format defined in document\", the formats defined via output_format will be used, see the documentation on rmarkdown::render()."
)

markdownVersion <- rk.XML.radio(
  label="Markdown version",
  options=list(
    "v1"=c(val="1"),
    "v2"=c(val="2", chk=TRUE)
  ),
  id.name="markdownVersion",
  help="Set the markdown version to use. Older v1 documents are rendered into HTML format only, using the markdown package. If in doubt, leave it to the newer v2; those documents are rendered using pandoc, which allows you to choose between various output formats. But be aware that some formats may require special configuration or templates to produce proper results."
)

exportTabbook <- rk.XML.tabbook(label="Export RMarkdown",
  tabs=list(
    "Source and target"=rk.XML.row(
      rk.XML.col(
        markdownFile,
        targetFormat,
        targetFile,
        targetDir,
        rk.XML.stretch()
      )
    ),
    "Advanced options"=rk.XML.row(
      rk.XML.col(
        markdownVersion,
        rk.XML.stretch()
      )
    )
  ),
  id.name=c("exportTabbook", "tabSourceAndTarget", "tabAdvancedOptions")
)


pluginDialog <- rk.XML.dialog(
  exportTabbook,
  label="Export RMarkdown"
)

# for logic section
lgcSetTargetScript <- rk.comment(id("
  gui.addChangeCommand(\"", markdownFile, ".selection\", \"dataChanged()\");
  gui.addChangeCommand(\"", targetFormat, ".string\", \"dataChanged()\");
  gui.addChangeCommand(\"", markdownVersion, ".string\", \"dataChanged()\");
  // this function is called whenever a new source file
  // is selected or the target format changes
  dataChanged = function(){
    var fromFile = gui.getString(\"", markdownFile, "\");
    var toFile = \"\";
    var fileExt = gui.getString(\"", targetFormat, "\");
    if(fileExt == \"all\" || fileExt == \"first\"){
      toFile = fromFile.substr(0, fromFile.lastIndexOf(\"/\"));
      gui.setValue(\"", targetDir, ".selection\", toFile);
    } else {
      var mdVers = gui.getString(\"", markdownVersion, "\");
      if(mdVers == \"1\" || fileExt == \"html5\" || fileExt == \"html_vignette\"){
        fileExt = \"html\";
      } else if(fileExt == \"beamer\" || fileExt == \"latex\"){
        fileExt = \"pdf\";
      } else if(fileExt == \"markdown\" || fileExt == \"markdown_github\" || fileExt == \"gfm\"){
        fileExt = \"md\";
      } else {};
      var fex = fromFile.lastIndexOf(\".\");
      toFile = fromFile.substr(0, fex < 0 ? fromFile.length : fex) + \".\" + fileExt;
      // prevent overwriting source with target
      if(fromFile == toFile){
        toFile = fromFile.substr(0, fex < 0 ? fromFile.length : fex) + \"_new.\" + fileExt;
      } else {}
      gui.setValue(\"", targetFile, ".selection\", toFile);
    }
  }
  dataChanged (); // initialize", js=FALSE)
)

logicSection <- rk.XML.logic(
  logicMDVersion <- rk.XML.convert(sources=list(string=markdownVersion), mode=c(equals="2"), id.name="logicMDVersion"),
  rk.XML.connect(governor=logicMDVersion, client=targetFormat, set="enabled"),
  rk.XML.connect(governor="current_filename", client=markdownFile, get="", set="selection"),
  # replace the target file browser with a dir, as the format is defined in the document only
  logicMDAllFormats <- rk.XML.convert(sources=list(string=targetFormat), mode=c(equals="all"), id.name="logicMDAllFormats"),
  logicMDFirstFormat <- rk.XML.convert(sources=list(string=targetFormat), mode=c(equals="first"), id.name="logicMDFirstFormat"),
  logicMDAllOrFirst <- rk.XML.convert(sources=list(logicMDAllFormats, logicMDFirstFormat), mode=c(or=""), id.name="logicMDAllOrFirst"),
  rk.XML.connect(governor=logicMDAllOrFirst, client=targetFile, set="visible", not=TRUE),
  rk.XML.connect(governor=logicMDAllOrFirst, client=targetFile, set="required", not=TRUE),
  rk.XML.connect(governor=logicMDAllOrFirst, client=targetDir, set="visible"),
  rk.XML.connect(governor=logicMDAllOrFirst, client=targetDir, set="required"),
  lgcSetTargetScript
)

## JavaScript
JSpreprocess <- rk.paste.JS(
  js(
    if(markdownVersion == 1){
      echo("require(markdown)\n\n")
    } else {
      echo("require(rmarkdown)\n")
      if(targetFormat != "all" && targetFormat != "first" && targetFormat != "html_vignette"){
        echo("rk.check.for.pandoc(\n  stop_if_missing=TRUE,\n  output_format=\"", targetFormat, "\"\n)\n\n")
      } else {}
    }
  )
)

fileExtension <- rk.JS.vars("fileExtension")
# render expects different output format definitions than pandoc,
# e.g. "pdf_document" instead of "latex"
renderFormat <- rk.JS.vars("renderFormat")

JScalculate <- rk.paste.JS(
  id("var ", fileExtension, " = ", targetFormat, ";"),
  id("var ", renderFormat, " = \"pandoc\";"),
  js(
    if(markdownVersion == 1){
      echo("md_tempfile <- tempfile()\n")
      echo("knit2html(\n")
      echo("  input=\"", markdownFile, "\",\n  output=md_tempfile,\n  quiet=TRUE,\n")
      echo(")\n")
      id(fileExtension, " = 'html';")
      id(renderFormat, " = 'knit2html';")
    } else {
      if(targetFormat == "all"){
        id(renderFormat, " = 'all';")
      } else if(targetFormat == "first"){
        id(renderFormat, " = 'NULL';")
      } else if(targetFormat == "beamer"){
        id(fileExtension, " = 'pdf';")
        id(renderFormat, " = 'beamer_presentation';")
      # } else if(targetFormat == "dokuwiki"){
      } else if(targetFormat == "html"){
        id(renderFormat, " = 'html_document';")
      } else if(targetFormat == "html5"){
        id(fileExtension, " = 'html';")
      } else if(targetFormat == "markdown"){
        id(fileExtension, " = 'md';")
        id(renderFormat, " = 'md_document';")
      } else if(targetFormat == "gfm" || targetFormat == "markdown_github"){
        id(fileExtension, " = 'md';")
        id(renderFormat, " = 'github_document';")
      # } else if(targetFormat == "mediawiki"){
      } else if(targetFormat == "odt"){
        id(renderFormat, " = 'odt_document';")
      } else if(targetFormat == "latex"){
        id(fileExtension, " = 'pdf';")
        id(renderFormat, " = 'pdf_document';")
      } else if(targetFormat == "pptx"){
        id(renderFormat, " = 'powerpoint_presentation';")
      } else if(targetFormat == "rtf"){
        id(renderFormat, " = 'rtf_document';")
      } else if(targetFormat == "html_vignette"){
        id(fileExtension, " = 'html';")
        id(renderFormat, " = 'rmarkdown::html_vignette';")
      } else if(targetFormat == "docx"){
        id(renderFormat, " = 'word_document';")
      } else {}
      if(renderFormat == "pandoc"){
        echo("md_tempfile <- tempfile()\n")
        echo("suppressMessages(\n  pandoc(\n")
        echo("    input=knit(\n      \"", markdownFile, "\",\n      output=md_tempfile,\n      quiet=TRUE\n    ),\n")
        echo("    format=\"", targetFormat, "\"\n")
        echo("  )\n)\n")
      } else {
        echo("render(\n")
        echo("  input=\"", markdownFile, "\",\n")
        if(renderFormat == "NULL"){
          echo("  output_format=", renderFormat, ",\n")
        } else {
          echo("  output_format=\"", renderFormat, "\",\n")
        }
        if(renderFormat != "all" && renderFormat != "NULL"){
          echo("  output_file=\"", targetFile, "\",\n")
        } else {
          echo("  output_dir=\"", targetDir, "\",\n")
        }
        echo("  quiet=TRUE\n")
        echo(")\n")
      }
    },
    if(renderFormat == "pandoc" || renderFormat == "knit2html"){
      echo("\nfile.copy(\n")
      echo("  from=paste0(md_tempfile, \".", fileExtension, "\"),\n")
      echo("  to=\"", targetFile, "\",\n")
      echo("  overwrite=TRUE\n)\n")
      echo("unlink(paste0(md_tempfile, \".", fileExtension, "\"))\n\n")
    } else {}
  )
)

JSprintout <- rk.paste.JS(
  js(
    if(targetFormat == "all" || targetFormat == "first"){
      rk.JS.header(
        title="Export RMarkdown file",
        addFromUI=markdownFile,
        addFromUI=targetFormat,
        addFromUI=targetDir,
        guess.getter=TRUE
      )
    } else {
      rk.JS.header(
        title="Export RMarkdown file",
        addFromUI=markdownFile,
        addFromUI=targetFormat,
        addFromUI=targetFile,
        guess.getter=TRUE
      )
    }
  )
)

#############
## the main call
## if you run the following function call, files will be written to output.dir!
#############
# this is where things get serious, that is, here all of the above is put together into one plugin
plugin.dir <- rk.plugin.skeleton(
  about=aboutPlugin,
  dependencies=plugin.dependencies,
  path=output.dir,
  guess.getter=guess.getter,
  scan=c("var", "saveobj", "settings"),
  xml=list(
    dialog=pluginDialog,
    #wizard=,
    logic=logicSection#,
    #snippets=
  ),
  js=list(
    results.header=FALSE,
    #load.silencer=,
    require="knitr",
    #variables=,
    globals=list(
      rk.JS.vars(
        markdownVersion,
        targetFormat,
        guess.getter=TRUE
      )
    ),
    preprocess=JSpreprocess,
    calculate=JScalculate,
    printout=JSprintout#,
    #doPrintout=
  ),
  rkh=list(
    #summary=,
    #usage=,
    #sections=,
    #settings=,
    #related=,
    #technical=
  ),
  create=c("pmap", "xml", "js", "desc", "rkh"),
  overwrite=overwrite,
  #components=list(),, 
  #provides=c("logic", "dialog"), 
  pluginmap=list(name="Export RMarkdown", hierarchy=c("file","export")), 
  tests=FALSE, 
  edit=FALSE, 
  load=TRUE, 
  show=FALSE,
  gen.info="../../rkwarddev_scripts/rkwarddev_rk.RMarkdown_plugin_script.R",
  hints=FALSE,
  internal=TRUE
)

  # you can make your plugin translatable, see top of script
  if(isTRUE(update.translations)){
    rk.updatePluginMessages(file.path(output.dir,"rk.RMarkdown","inst","rkward","rk.RMarkdown.pluginmap"))
  } else {}

})
