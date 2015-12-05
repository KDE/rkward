// this code was generated using the rkwarddev package.
// perhaps don't make changes here, but in the rkwarddev script instead!
// 
// look for a file called: $SRC/demo/skeleton_dialog.R



function preprocess(){
  // add requirements etc. here
  echo("require(rkwarddev)\n");

  var outDir = getValue("outDir");
  var overwrite = getValue("overwrite");
  var guessGetters = getValue("guessGetters");
  var codeIndent = getValue("codeIndent");
  var emptyElse = getValue("emptyElse");
  echo("rkwarddev.required(\"0.08-1\")");
  echo("\n\n# define where the plugin should write its files\noutput.dir <- ");
  if(outDir) {
    echo("\"" + outDir + "\"");  
  } else {
    echo("tempdir()");  
  }
  echo("\n# overwrite an existing plugin in output.dir?\noverwrite <- ");
  if(overwrite) {
    echo("TRUE");  
  } else {
    echo("FALSE");  
  }
  echo("\n# if you set guess.getters to TRUE, the resulting code will need RKWard >= 0.6.0\nguess.getter <- ");
  if(guessGetters) {
    echo("TRUE");  
  } else {
    echo("FALSE");  
  }
  echo("\n# define the indentation character for the generated code\nrk.set.indent(by=\"" + codeIndent + "\")" + "\n# should empty \"else\" clauses be kept in the JavaScript code?\nrk.set.empty.e(");
  if(emptyElse) {
    echo("TRUE)");  
  } else {
    echo("FALSE)");  
  }
  echo("\n# make your plugin translatable by setting this to TRUE" + "\nupdate.translations <- FALSE");
  echo("\n\n");
}

function calculate(){
  // read in variables from dialog
  var pluginName = getString("pluginName");
  var pluginLicense = getString("pluginLicense");
  var pluginDescription = getString("pluginDescription");
  var pluginVersion = getString("pluginVersion");
  var pluginDate = getString("pluginDate");
  var pluginHomepage = getString("pluginHomepage");
  var pluginCategory = getString("pluginCategory");
  var authorGivenName = getString("authorGivenName");
  var authorFamiliyName = getString("authorFamiliyName");
  var authorMail = getString("authorMail");
  var ocolOptcolAuthorGivenName = getList("optionsetAuthors.optcolAuthorGivenName");
  var ocolOptcolAuthorFamiliyName = getList("optionsetAuthors.optcolAuthorFamiliyName");
  var ocolOptcolAuthorMail = getList("optionsetAuthors.optcolAuthorMail");
  var ocolOptcolAuthorAut = getList("optionsetAuthors.optcolAuthorAut");
  var ocolOptcolAuthorCre = getList("optionsetAuthors.optcolAuthorCre");
  var ocolOptcolAuthorCtb = getList("optionsetAuthors.optcolAuthorCtb");
  var outDir = getString("outDir");
  var codeIndent = getString("codeIndent");
  var menuHier = getString("menuHier");
  var menuName = getString("menuName");
  var RKMin = getString("RKMin");
  var RKMax = getString("RKMax");
  var RMin = getString("RMin");
  var RMax = getString("RMax");
  var pckgName = getString("pckgName");
  var pckgMin = getString("pckgMin");
  var pckgMax = getString("pckgMax");
  var pckgRepo = getString("pckgRepo");
  var ocolOptcolPckgName = getList("dependencyOptionset.optcolPckgName");
  var ocolOptcolPckgMin = getList("dependencyOptionset.optcolPckgMin");
  var ocolOptcolPckgMax = getList("dependencyOptionset.optcolPckgMax");
  var ocolOptcolPckgRepo = getList("dependencyOptionset.optcolPckgRepo");
  var helpSummary = getString("helpSummary");
  var helpUsage = getString("helpUsage");
  var authorAut = getBoolean("authorAut.state");
  var authorCre = getBoolean("authorCre.state");
  var authorCtb = getBoolean("authorCtb.state");
  var overwrite = getBoolean("overwrite.state");
  var addWizard = getBoolean("addWizard.state");
  var addTests = getBoolean("addTests.state");
  var showPlugin = getBoolean("showPlugin.state");
  var editPlugin = getBoolean("editPlugin.state");
  var addToConfig = getBoolean("addToConfig.state");
  var guessGetters = getBoolean("guessGetters.state");
  var emptyElse = getBoolean("emptyElse.state");
  var dependencyFrameChecked = getBoolean("dependencyFrame.checked");
  var helpTextChecked = getBoolean("helpText.checked");

  // the R code to be evaluated
  // define the array arrOptAbout for values of R option "about"
  var arrOptAbout = new Array();
  if(pluginDescription) {
    arrOptAbout.push("desc=\"" + pluginDescription + "\""  );
  } else {
    arrOptAbout.push();
  }
  if(pluginVersion) {
    arrOptAbout.push("version=\"" + pluginVersion + "\""  );
  } else {
    arrOptAbout.push();
  }
  if(pluginDate) {
    arrOptAbout.push("date=\"" + pluginDate + "\""  );
  } else {
    arrOptAbout.push();
  }
  if(pluginHomepage) {
    arrOptAbout.push("url=\"" + pluginHomepage + "\""  );
  } else {
    arrOptAbout.push();
  }
  if(pluginLicense) {
    arrOptAbout.push("license=\"" + pluginLicense + "\""  );
  } else {
    arrOptAbout.push();
  }
  if(pluginCategory) {
    arrOptAbout.push("category=\"" + pluginCategory + "\""  );
  } else {
    arrOptAbout.push();
  }
  // clean array arrOptAbout from empty strings
  arrOptAbout = arrOptAbout.filter(String);
  // set the actual variable optAbout with all values for R option "about"
  if(arrOptAbout.length > 0) {
    var optAbout = ",\n\tabout=list(" + arrOptAbout.join(", ") + ")";
  } else {
    var optAbout = "";
  }

  // define the array arrOptDependencies for values of R option "dependencies"
  var arrOptDependencies = new Array();
  if(dependencyFrameChecked && RKMin) {
    arrOptDependencies.push("rkward.min=\"" + RKMin + "\""  );
  } else {
    arrOptDependencies.push();
  }
  if(dependencyFrameChecked && RKMax) {
    arrOptDependencies.push("rkward.max=\"" + RKMax + "\""  );
  } else {
    arrOptDependencies.push();
  }
  if(dependencyFrameChecked && RMin) {
    arrOptDependencies.push("R.min=\"" + RMin + "\""  );
  } else {
    arrOptDependencies.push();
  }
  if(dependencyFrameChecked && RMax) {
    arrOptDependencies.push("R.max=\"" + RMax + "\""  );
  } else {
    arrOptDependencies.push();
  }
  // clean array arrOptDependencies from empty strings
  arrOptDependencies = arrOptDependencies.filter(String);
  // set the actual variable optDependencies with all values for R option "dependencies"
  if(arrOptDependencies.length > 0) {
    var optDependencies = ", dependencies=list(" + arrOptDependencies.join(", ") + ")";
  } else {
    var optDependencies = "";
  }

  // define the array arrOptPluginmap for values of R option "pluginmap"
  var arrOptPluginmap = new Array();
  if(menuName) {
    arrOptPluginmap.push("name=\"" + menuName + "\""  );
  } else {
    arrOptPluginmap.push("name=\"" + pluginName + "\""  );
  }
  if(menuHier) {
    arrOptPluginmap.push("hierarchy=\"" + menuHier + "\""  );
  } else {
    arrOptPluginmap.push();
  }
  // clean array arrOptPluginmap from empty strings
  arrOptPluginmap = arrOptPluginmap.filter(String);
  // set the actual variable optPluginmap with all values for R option "pluginmap"
  if(arrOptPluginmap.length > 0) {
    var optPluginmap = "pluginmap=list(" + arrOptPluginmap.join(", ") + ")";
  } else {
    var optPluginmap = "";
  }

  // define the array arrOptSkeleton for values of R option ""
  var arrOptSkeleton = new Array();
  if(addWizard) {
    arrOptSkeleton.push("\n\tprovides=c(\"logic\", \"dialog\", \"wizard\")"  );
  } else {
    arrOptSkeleton.push("\n\t#provides=c(\"logic\", \"dialog\")"  );
  }
  if(optPluginmap) {
    arrOptSkeleton.push("\n\t" + optPluginmap  );
  } else {
    arrOptSkeleton.push("\n\t#pluginmap=list(name=\"\", hierarchy=\"\", require=\"\")"  );
  }
  if(addTests) {
    arrOptSkeleton.push("\n\ttests=TRUE"  );
  } else {
    arrOptSkeleton.push("\n\ttests=FALSE"  );
  }
  if(editPlugin) {
    arrOptSkeleton.push("\n\tedit=TRUE"  );
  } else {
    arrOptSkeleton.push("\n\tedit=FALSE"  );
  }
  if(addToConfig) {
    arrOptSkeleton.push("\n\tload=TRUE"  );
  } else {
    arrOptSkeleton.push("\n\tload=FALSE"  );
  }
  if(showPlugin) {
    arrOptSkeleton.push("\n\tshow=TRUE"  );
  } else {
    arrOptSkeleton.push("\n\tshow=FALSE"  );
  }
  // clean array arrOptSkeleton from empty strings
  arrOptSkeleton = arrOptSkeleton.filter(String);
  // set the actual variable optSkeleton with all values for R option ""
  if(arrOptSkeleton.length > 0) {
    var optSkeleton = ", " + arrOptSkeleton.join(", ") + "";
  } else {
    var optSkeleton = "";
  }

  echo("aboutPlugin <- rk.XML.about(");
  if(pluginName) {
    echo("\n\tname=\"" + pluginName + "\",\n");  
  } else {}
  var ocolOptcolAuthorGivenName = getList("optionsetAuthors.optcolAuthorGivenName");
  var ocolOptcolAuthorFamiliyName = getList("optionsetAuthors.optcolAuthorFamiliyName");
  var ocolOptcolAuthorMail = getList("optionsetAuthors.optcolAuthorMail");
  var ocolOptcolAuthorAut = getList("optionsetAuthors.optcolAuthorAut");
  var ocolOptcolAuthorCre = getList("optionsetAuthors.optcolAuthorCre");
  var ocolOptcolAuthorCtb = getList("optionsetAuthors.optcolAuthorCtb");
  if(ocolOptcolAuthorGivenName != "") {
    echo("\tauthor=c(\n\t\t");
  for (var i = 0; i < ocolOptcolAuthorGivenName.length; ++i){
    // define the array arrOptAuthorRole for values of R option "role"
    var arrOptAuthorRole = new Array();
    if(ocolOptcolAuthorAut[i] == 1) {
      arrOptAuthorRole.push("\"aut\""  );
    } else {
      arrOptAuthorRole.push();
    }
    if(ocolOptcolAuthorCre[i] == 1) {
      arrOptAuthorRole.push("\"cre\""  );
    } else {
      arrOptAuthorRole.push();
    }
    if(ocolOptcolAuthorCtb[i] == 1) {
      arrOptAuthorRole.push("\"ctb\""  );
    } else {
      arrOptAuthorRole.push();
    }
    // clean array arrOptAuthorRole from empty strings
    arrOptAuthorRole = arrOptAuthorRole.filter(String);
    // set the actual variable optAuthorRole with all values for R option "role"
    if(arrOptAuthorRole.length > 0) {
      var optAuthorRole = ", role=c(" + arrOptAuthorRole.join(", ") + ")";
    } else {
      var optAuthorRole = "";
    }

    echo("person(");
    echo("given=\"" + ocolOptcolAuthorGivenName[i] + "\"");
    if(ocolOptcolAuthorFamiliyName[i]) {
      echo(", family=\"" + ocolOptcolAuthorFamiliyName[i] + "\"");  
    } else {}
    if(ocolOptcolAuthorMail[i]) {
      echo(", email=\"" + ocolOptcolAuthorMail[i] + "\"");  
    } else {}
    if(optAuthorRole) {
      echo(optAuthorRole);  
    } else {}
    echo(")");
    if(i + 1 < ocolOptcolAuthorGivenName.length) {
      echo(",\n\t\t");
    }
  }
  echo("\n\t)");
  } else {}
  echo(optAbout);
  echo("\n)\n\n");
  if(dependencyFrameChecked && (optDependencies || ocolOptcolPckgName)) {
    echo("plugin.dependencies <- rk.XML.dependencies(");
    if(optDependencies) {
      echo(optDependencies);  
    } else {}
    if(optDependencies && ocolOptcolPckgName) {
      echo(",");  
    } else {}
    if(ocolOptcolPckgName!= "") {
      echo("\n\tpackage=list(\n\t\t");
      for (var i = 0; i < ocolOptcolPckgName.length; ++i){
        echo("c(");
        echo("name=\"" + ocolOptcolPckgName[i] + "\"");
        if(ocolOptcolPckgMin[i]) {
          echo(", min=\"" + ocolOptcolPckgMin[i] + "\"");  
        } else {}
        if(ocolOptcolPckgMax[i]) {
          echo(", max=\"" + ocolOptcolPckgMax[i] + "\"");  
        } else {}
        if(ocolOptcolPckgRepo[i]) {
          echo(", repository=\"" + ocolOptcolPckgRepo[i] + "\"");  
        } else {}
        echo(")");
        if(i + 1 < ocolOptcolPckgName.length) {
          echo(",\n\t\t");
        }
      }
      echo("\n\t)");
    } else {}
    echo("\n)\n\n");
  } else {}
  echo("# name of the main component, relevant for help page content\nrk.set.comp(\"");
  if(menuName) {
    echo(menuName + "\")\n\n");  
  } else {
    echo(pluginName + "\")\n\n");  
  }
  echo("############\n## your plugin dialog and JavaScript should be put here\n############\n\n");
  if(helpTextChecked) {
    echo("############\n## help page\nplugin.summary <- rk.rkh.summary(\n\t");  
    if(helpSummary) {
      echo("\"" + helpSummary + "\"\n)");  
    } else {
      echo("\"" + pluginDescription + "\"\n)");  
    }  
    echo("\nplugin.usage <- rk.rkh.usage(\n\t\"" + helpUsage + "\"\n)\n\n");  
  } else {}
  echo("#############\n" + "## the main call\n" + "## if you run the following function call, files will be written to output.dir!\n" + "#############\n" + "# this is where things get serious, that is, here all of the above is put together into one plugin\n" + "plugin.dir <- rk.plugin.skeleton(\n\tabout=aboutPlugin,");
  if(dependencyFrameChecked && optDependencies) {
    echo("\n\tdependencies=plugin.dependencies,");  
  } else {
    echo("\n\t#dependencies=plugin.dependencies,");  
  }
  echo("\n\tpath=output.dir," + "\n\tguess.getter=guess.getter," + "\n\tscan=c(\"var\", \"saveobj\", \"settings\")," + "\n\txml=list(\n\t\t#dialog=,\n\t\t#wizard=,\n\t\t#logic=,\n\t\t#snippets=\n\t)," + "\n\tjs=list(\n\t\t#results.header=FALSE,\n\t\t#load.silencer=,\n\t\t#require=,\n\t\t#variables=," + "\n\t\t#globals=,\n\t\t#preprocess=,\n\t\t#calculate=,\n\t\t#printout=,\n\t\t#doPrintout=\n\t),");
  if(helpTextChecked) {
    echo("\n\trkh=list(\n\t\tsummary=plugin.summary,\n\t\tusage=plugin.usage#," + "\n\t\t#sections=,\n\t\t#settings=,\n\t\t#related=,\n\t\t#technical=\n\t)," + "\n\tcreate=c(\"pmap\", \"xml\", \"js\", \"desc\", \"rkh\"),");  
  } else {
    echo("\n\trkh=list(" + "\n\t\t#summary=,\n\t\t#usage=," + "\n\t\t#sections=,\n\t\t#settings=,\n\t\t#related=,\n\t\t#technical=\n\t)," + "\n\tcreate=c(\"pmap\", \"xml\", \"js\", \"desc\"),");  
  }
  echo("\n\toverwrite=overwrite,");
  echo("\n\t#components=list(),");
  echo(optSkeleton);
  echo("\n)\n\n");
  echo("\t# you can make your plugin translatable, see top of script" + "\n\tif(isTRUE(update.translations)){" + "\n\t\trk.updatePluginMessages(file.path(output.dir,\"" + pluginName + "\",\"inst\",\"rkward\",\"" + pluginName + ".pluginmap\"))" + "\n\t} else {}\n\n");
}

function printout(){
  // printout the results




}

