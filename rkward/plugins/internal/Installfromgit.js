/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Meik Michalke <meik.michalke@hhu.de>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// this code was generated using the rkwarddev package.
// perhaps don't make changes here, but in the rkwarddev script instead!



function preprocess(is_preview){
  // add requirements etc. here
  echo("require(devtools)\n");
}

function calculate(is_preview){
  // read in variables from dialog
  var packageSource = getString("packageSource");
  var gitUser = getString("gitUser");
  var gitRepo = getString("gitRepo");
  var fullURL = getString("fullURL");
  var inpCmmttgbr = getString("inp_Cmmttgbr");
  var inpSbdrctry = getString("inp_Sbdrctry");
  var inpPrsnlPAT = getString("inp_PrsnlPAT");
  var inpUser = getString("inp_User");
  var inpPassword = getString("inp_Password");
  var frmPrvtrpstChecked = getBoolean("frm_Prvtrpst.checked");

  // the R code to be evaluated
  var frmPrvtrpstChecked = getValue("frm_Prvtrpst.checked");
  echo("  install_" + packageSource + "(\n");
  if(packageSource == "github" || packageSource == "gitlab" || packageSource == "bitbucket") {
    echo("    repo=\"" + gitUser + "/" + gitRepo);  
    if(inpSbdrctry) {
      echo("/" + inpSbdrctry);  
    } else {}  
    if(inpCmmttgbr) {
      echo("@" + inpCmmttgbr);  
    } else {}  
    echo("\"");  
    if(frmPrvtrpstChecked) {
      if(packageSource == "github" || packageSource == "gitlab") {
        if(inpPrsnlPAT) {
          echo(",\n    auth_token=\"" + inpPrsnlPAT + "\"");  
        } else {}  
      } else if(packageSource == "bitbucket") {
        if(inpUser) {
            echo(",\n    auth_user=\"" + inpUser + "\"");  
          } else {}  
          if(inpPassword) {
            echo(",\n    password=\"" + inpPassword + "\"");  
          } else {}  
      } else {}  
    } else {}  
  } else if(packageSource == "git" || packageSource == "svn") {
    echo("    url=\"" + fullURL + "\"");  
      if(inpCmmttgbr) {
        echo(",\n    branch=\"" + inpCmmttgbr + "\"");  
      } else {}  
      if(inpSbdrctry) {
        echo(",\n    subdir=\"" + inpSbdrctry + "\"");  
      } else {}  
  } else {}
  echo("\n  )");
  echo("\n");
}

function printout(is_preview){
  // printout the results
  new Header(i18n("Install from git results")).print();

}

