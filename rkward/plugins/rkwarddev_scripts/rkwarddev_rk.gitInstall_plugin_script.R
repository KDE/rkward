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
update.translations <- TRUE

aboutPlugin <- rk.XML.about(
  name="rk.gitInstall",
  author=c(
    person(given="Meik", family="Michalke", email="meik.michalke@hhu.de", role=c("aut", "cre"))
  ),
  about=list(
    desc="RKWard GUI to install R packges directly from git or subversion repositories",
    version="0.01-1",
    url="https://rkward.kde.org",
    license="GPL (>= 3)"
  )
)

plugin.dependencies <- rk.XML.dependencies(
  dependencies=list(rkward.min="0.6.0"),
  package=list(
    c(name="devtools")
  )
)

# name of the main component, relevant for help page content
rk.set.comp("Install from git")

############
## your plugin dialog and JavaScript should be put here
############

packageSource <- rk.XML.dropdown("Package source",
  options=list(
    "GitHub"=c(val="github", chk=TRUE),
    "Bitbucket"=c(val="bitbucket"),
    "git (generic)"=c(val="git"),
    "subversion (generic)"=c(val="svn")
  ),
  id.name="packageSource",
  help="Set the location where the package resides that you want to install."
)

gitUserRepo <- rk.XML.row(
  gitUser <- rk.XML.input("Git user name",
    required=TRUE,
    id.name="gitUser",
    help="Specify the user name (GitHub and Bitbucket only)."
  ),
  gitRepo <- rk.XML.input("Repository",
    required=TRUE,
    id.name="gitRepo",
    help="Give the repository name (GitHub and Bitbucket only)."
  )
)

fullURL <- rk.XML.input("URL to repository",
  initial="https://example.com/...",
  required=TRUE,
  id.name="fullURL",
  help="Give the full URL to the repository (generic git and svn only)."
)

repoFrame <- rk.XML.frame(
  gitUserRepo,
  fullURL,
  label="Package reporitory",
  id.name="repoFrame"
)

gitReference <- rk.XML.input("Commit/tag/branch",
  help="If you want a certain commit, tag or branch for installation, define that here."
)

gitSubdir <- rk.XML.input("Subdirectory",
  help="If you want to install from a subdirectory of the repository, define that here."
)

authUser <- rk.XML.input("User",
  required=TRUE,
  help="To install from a private repository, set the user name (Bitbucket only)."
)

authPassword <- rk.XML.input("Password",
  required=TRUE,
  help="To install from a private repository, set the password (Bitbucket only)."
)

authToken <- rk.XML.input("Personal access token (PAT)",
  required=TRUE,
  help="To install from a private repository, set your personal access token (PAT, GitHub only)."
)

authFrame <- rk.XML.frame(
  authToken,
  rk.XML.row(
    authUser,
    authPassword
  ),
  label="Private repository",
  checkable=TRUE,
  chk=FALSE
)

pluginDialog <- rk.XML.dialog(
  rk.XML.col(
    packageSource,
    repoFrame,
    gitReference,
    gitSubdir,
    authFrame
  ),
  label="Install from git/svn"
)

logicSection <- rk.XML.logic(
  lgcIsGit <- rk.XML.convert(sources=list(string=packageSource), mode=c(equals="git"), id.name="lgcIsGit"),
  lgcIsSVN <- rk.XML.convert(sources=list(string=packageSource), mode=c(equals="svn"), id.name="lgcIsSVN"),
  lgcRequireURL <- rk.XML.convert(sources=list(lgcIsGit, lgcIsSVN), mode=c(or=""), id.name="lgcRequireURL"),
  rk.XML.connect(governor=lgcRequireURL, client=gitUserRepo, set="visible", not=TRUE),
  rk.XML.connect(governor=lgcRequireURL, client=fullURL, set="visible"),
  lgcIsGithub <- rk.XML.convert(sources=list(string=packageSource), mode=c(equals="github"), id.name="lgcIsGithub"),
  lgcIsBitbucket <- rk.XML.convert(sources=list(string=packageSource), mode=c(equals="bitbucket"), id.name="lgcIsBitbucket"),
  rk.XML.connect(governor=lgcIsGithub, client=authToken, set="enabled"),
  rk.XML.connect(governor=lgcIsBitbucket, client=authUser, set="enabled"),
  rk.XML.connect(governor=lgcIsBitbucket, client=authPassword, set="enabled"),
  rk.XML.connect(governor=lgcRequireURL, client=authFrame, set="enabled", not=TRUE)
)

## JavaScript
# see if frames are checked
authFrameChecked <- rk.JS.vars(authFrame, modifiers="checked")

JScalculate <- rk.paste.JS(
  authFrameChecked,
  echo("\tinstall_", packageSource, "(\n"),
  js(
    if(packageSource == "github" || packageSource == "bitbucket"){
      echo("\t\trepo=\"", gitUser, "/", gitRepo, "\"")
      if(gitReference){
        echo(",\n\t\tref=\"", gitReference, "\"")
      } else {}
      if(authFrameChecked){
        if(packageSource == "github" && authToken){
          echo(",\n\t\tauth_token=\"", authToken, "\"")
        } else if(packageSource == "bitbucket"){
          if(authUser){
            echo(",\n\t\tauth_user=\"", authUser, "\"")
          } else {}
          if(authPassword){
            echo(",\n\t\tpassword=\"", authPassword, "\"")
          } else {}
        } else {}
      } else {}
    } else if(packageSource == "git" || packageSource == "svn"){
      echo("\t\turl=\"", fullURL, "\"")
      if(gitReference){
        echo(",\n\t\tbranch=\"", gitReference, "\"")
      } else {}
    } else {},
    if(gitSubdir){
      echo(",\n\t\tsubdir=\"", gitSubdir, "\"")
    } else {}
  ),
  echo("\n\t)"),
  echo("\n")
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
    #results.header=FALSE,
    #load.silencer=,
    require="devtools",
    #variables=,
    #globals=,
    #preprocess=,
    calculate=JScalculate#,
    #printout=,
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
  pluginmap=list(name="Install from git", hierarchy="settings"), 
  tests=FALSE, 
  edit=FALSE, 
  load=TRUE, 
  show=TRUE,
  gen.info="$SRC/inst/rkward/rkwarddev_rk.gitInstall_plugin_script.R",
  hints=FALSE
)

  # you can make your plugin translatable, see top of script
  if(isTRUE(update.translations)){
    rk.updatePluginMessages(file.path(output.dir,"rk.gitInstall","inst","rkward","rk.gitInstall.pluginmap"))
  } else {}

})