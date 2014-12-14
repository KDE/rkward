## this function can be used to generate a list of XiMpLe nodes as testing standards
require(rkwarddev)

# returns the list if file=NULL
XMLTestNodes <- function(file=NULL, compress="xz", compression_level=-9){
  XML_test_standards <- list(
    about=rk.XML.about(
        name="Square the circle",
        author=c(
          person(given="E.A.", family="Dölle",
            email="doelle@eternalwondermaths.example.org", role="aut"),
          person(given="A.", family="Assistant",
            email="alterego@eternalwondermaths.example.org", role=c("cre","ctb"))
          ),
        about=list(
          desc="Squares the circle using Heisenberg compensation.",
          version="0.1-3",
          date=as.Date("2014-12-12"),
          url="http://eternalwondermaths.example.org/23/stc.html",
          license="GPL",
          category="Geometry")
      ),
      attribute=rk.XML.attribute(
        id="testID",
        label="an attribute label",
        value="checked"
      ),
      browser=rk.XML.browser(
        label="a browser label",
        type="dir",
        initial="/tmp/initial",
        filter=c("*initial", "*csv"),
        required=TRUE,
        i18n=list(context="context info here")
      ),
      checkbox=rk.XML.cbox(
        label="a checbox label",
        value="checked",
        chk=TRUE,
        un.value="unchecked",
        i18n=list(context="context info here")
      ),
      code=rk.XML.code(
        file="some_file.js"
      ),
      col=rk.XML.col(
        rk.XML.cbox(label="a column label")
      ),
      component=rk.XML.component(
        label="a component label",
        file="plugins/MyGUIdialog.xml",
        i18n=list(context="context info here")
      ),
      components=rk.XML.components(
        rk.XML.component(
          label="a components label",
          file="plugins/MyOtherGUIdialog.xml"
        )
      ),
      connect=rk.XML.connect(
        governor="myGovernor",
        client="myCLient",
        get="visible",
        set="string",
        not=TRUE,
        reconcile=TRUE
      ),
      context=rk.XML.context(
        rk.XML.menu("Analysis", 
          rk.XML.entry(
            rk.XML.component(
              label="a context label",
              file="plugins/MyOtherGUIdialog.xml"
            )
          )
        )
      ),
      convert=rk.XML.convert(
        sources=list(text=rk.XML.input("a convert label")),
        mode=c(equals="myValue"),
        required=TRUE
      ),
      copy=rk.XML.copy(
        id="aCopyID",
        as="page"
      ),
      dependencies=rk.XML.dependencies(
        dependencies=list(
          rkward.min="0.6.3",
          rkward.max="0.6.5",
          R.min="3.1",
          R.max="3.2"),
        package=list(
          c(name="heisenberg", min="0.11-2", max="0.14-1",
            repository="http://hsb.example.org"),
          c(name="DreamsOfPi", min="0.2", max="3.1", repository="http://dop.example.org")),
        pluginmap=list(
          c(name="heisenberg.pluginmap", url="http://eternalwondermaths.example.org/hsb"))
      ),
      dependency_check=rk.XML.dependency_check(
        id.name="dependency",
        dependencies=list(
          rkward.min="0.6.3",
          rkward.max="0.6.5",
          R.min="3.1",
          R.max="3.2"),
        package=list(
          c(name="heisenberg", min="0.11-2", max="0.14-1",
            repository="http://hsb.example.org"),
          c(name="DreamsOfPi", min="0.2", max="3.1", repository="http://dop.example.org")),
        pluginmap=list(
          c(name="heisenberg.pluginmap", url="http://eternalwondermaths.example.org/hsb"))
      ),
      dialog=rk.XML.dialog(
        rk.XML.col(
          rk.XML.cbox(label="a dialog column label")
        ),
        label="a dialog label",
        recommended=TRUE,
        i18n=list(context="context info here")
      ),
      dropdown=rk.XML.dropdown(
        label="myDropdownMenu",
        options=list(
          "First Option"=c(val="val1"),
          rk.XML.option(
            "Second Option", val="val2", id.name="auto",
            i18n=list(context="context info1 here")
          ),
          "Third Option"=c(val="val3", chk=TRUE)
        ),
        i18n=list(context="context info2 here")
      )#,
#     embed=rk.XML.embed(),
#     entry=rk.XML.entry(),
#     external=rk.XML.external(),
#     formula=rk.XML.formula(),
#     frame=rk.XML.frame(),
#     help=rk.XML.help(),
#     hierarchy=rk.XML.hierarchy(),
#     include=rk.XML.include(),
#     input=rk.XML.input(),
#     insert=rk.XML.insert(),
#     logic=rk.XML.logic(),
#     matrix=rk.XML.matrix(),
#     menu=rk.XML.menu(),
#     optioncolumn=rk.XML.optioncolumn(),
#     optiondisplay=rk.XML.optiondisplay(),
#     option=rk.XML.option(),
#     optionset=rk.XML.optionset(),
#     page=rk.XML.page(),
#     pluginmap=rk.XML.pluginmap(),
#     plugin=rk.XML.plugin(),
#     preview=rk.XML.preview(),
#     radio=rk.XML.radio(),
#     require=rk.XML.require(),
#     row=rk.XML.row(),
#     saveobj=rk.XML.saveobj(),
#     select=rk.XML.select(),
#     set=rk.XML.set(),
#     snippet=rk.XML.snippet(),
#     snippets=rk.XML.snippets(),
#     spinbox=rk.XML.spinbox(),
#     stretch=rk.XML.stretch(),
#     switch=rk.XML.switch(),
#     tabbook=rk.XML.tabbook(),
#     text=rk.XML.text(),
#     valueselector=rk.XML.valueselector(),
#     valueslot=rk.XML.valueslot(),
#     values=rk.XML.values(),
#     varselector=rk.XML.varselector(),
#     varslot=rk.XML.varslot(),
#     vars=rk.XML.vars(),
#     wizard=rk.XML.wizard()
  )

  if(is.null(file)){
    return(XML_test_standards)
  } else {
    save(XML_test_standards,
      file=file,
      compress=compress,
      compression_level=compression_level)
    message(paste0("written to file ", file))
    return(invisible(NULL))
  }
}
