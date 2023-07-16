/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Meik Michalke <meik.michalke@hhu.de>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// this code was generated using the rkwarddev package.
// perhaps don't make changes here, but in the rkwarddev script instead!
// 
// look for a file called: ../../rkwarddev_scripts/rkwarddev_rk.RMarkdown_plugin_script.R

// define variables globally
var markdownVersion;
var targetFormat;

function setGlobalVars(){
  markdownVersion = getString("markdownVersion");
  targetFormat = getString("targetFormat");
}



function preprocess(is_preview){
  setGlobalVars();
  // add requirements etc. here
  echo("require(knitr)\n");

  if(markdownVersion == 1) {
    echo("require(markdown)\n\n");  
  } else {
    echo("require(rmarkdown)\n");  
    if(targetFormat != "all" && targetFormat != "first" && targetFormat != "html_vignette") {
      echo("rk.check.for.pandoc(\n  stop_if_missing=TRUE,\n  output_format=\"" + targetFormat + "\"\n)\n\n");  
    } else {}  
  }
}

function calculate(is_preview){
  // read in variables from dialog
  var markdownFile = getString("markdownFile");
  var targetFile = getString("targetFile");
  var targetDir = getString("targetDir");

  // the R code to be evaluated
  var fileExtension = targetFormat;
  var renderFormat = "pandoc";
  if(markdownVersion == 1) {
    echo("md_tempfile <- tempfile()\n");  
    echo("knit2html(\n");  
    echo("  input=\"" + markdownFile + "\",\n  output=md_tempfile,\n  quiet=TRUE,\n");  
    echo(")\n");  
    fileExtension = 'html';  
    renderFormat = 'knit2html';  
  } else {
    if(targetFormat == "all") {
      renderFormat = 'all';  
    } else if(targetFormat == "first") {
      renderFormat = 'NULL';  
    } else if(targetFormat == "beamer") {
      fileExtension = 'pdf';  
          renderFormat = 'beamer_presentation';  
    } else if(targetFormat == "html") {
      renderFormat = 'html_document';  
    } else if(targetFormat == "html5") {
      fileExtension = 'html';  
    } else if(targetFormat == "markdown") {
      fileExtension = 'md';  
                renderFormat = 'md_document';  
    } else if(targetFormat == "gfm" || targetFormat == "markdown_github") {
      fileExtension = 'md';  
                  renderFormat = 'github_document';  
    } else if(targetFormat == "odt") {
      renderFormat = 'odt_document';  
    } else if(targetFormat == "latex") {
      fileExtension = 'pdf';  
                      renderFormat = 'pdf_document';  
    } else if(targetFormat == "pptx") {
      renderFormat = 'powerpoint_presentation';  
    } else if(targetFormat == "rtf") {
      renderFormat = 'rtf_document';  
    } else if(targetFormat == "html_vignette") {
      fileExtension = 'html';  
                            renderFormat = 'rmarkdown::html_vignette';  
    } else if(targetFormat == "docx") {
      renderFormat = 'word_document';  
    } else {}  
    if(renderFormat == "pandoc") {
      echo("md_tempfile <- tempfile()\n");  
      echo("suppressMessages(\n  pandoc(\n");  
      echo("    input=knit(\n      \"" + markdownFile + "\",\n      output=md_tempfile,\n      quiet=TRUE\n    ),\n");  
      echo("    format=\"" + targetFormat + "\"\n");  
      echo("  )\n)\n");  
    } else {
      echo("render(\n");  
      echo("  input=\"" + markdownFile + "\",\n");  
      if(renderFormat == "NULL") {
        echo("  output_format=" + renderFormat + ",\n");  
      } else {
        echo("  output_format=\"" + renderFormat + "\",\n");  
      }  
      if(renderFormat != "all" && renderFormat != "NULL") {
        echo("  output_file=\"" + targetFile + "\",\n");  
      } else {
        echo("  output_dir=\"" + targetDir + "\",\n");  
      }  
      echo("  quiet=TRUE\n");  
      echo(")\n");  
    }  
  }
  if(renderFormat == "pandoc" || renderFormat == "knit2html") {
    echo("\nfile.copy(\n");  
    echo("  from=paste0(md_tempfile, \"." + fileExtension + "\"),\n");  
    echo("  to=\"" + targetFile + "\",\n");  
    echo("  overwrite=TRUE\n)\n");  
    echo("unlink(paste0(md_tempfile, \"." + fileExtension + "\"))\n\n");  
  } else {}
}

function printout(is_preview){
  // printout the results
  if(targetFormat == "all" || targetFormat == "first") {
    new Header(i18n("Export RMarkdown file")).addFromUI("markdownFile").addFromUI("targetFormat").addFromUI("targetDir").print();  
  } else {
    new Header(i18n("Export RMarkdown file")).addFromUI("markdownFile").addFromUI("targetFormat").addFromUI("targetFile").print();  
  }

}

