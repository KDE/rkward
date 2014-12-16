## this function can be used to generate a list of XiMpLe nodes as testing standards
require(rkwarddev)

# returns the list if file=NULL
XMLTestNodes <- function(file=NULL, compress="xz", compression_level=-9){
    about <- rk.XML.about(
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
    )
    attribute <- rk.XML.attribute(
        id="testID",
        label="an attribute label",
        value="checked"
    )
    browser <- rk.XML.browser(
        label="a browser label",
        type="dir",
        initial="/tmp/initial",
        filter=c("*initial", "*csv"),
        required=TRUE,
        i18n=list(context="context info here")
    )
    checkbox <- rk.XML.cbox(
        label="a checbox label",
        value="checked",
        chk=TRUE,
        un.value="unchecked",
        i18n=list(context="context info here")
    )
    code <- rk.XML.code(
        file="some_file.js"
    )
    col <- rk.XML.col(
        rk.XML.cbox(label="a column label")
    )
    component <- rk.XML.component(
        label="a component label",
        file="plugins/MyGUIdialog.xml",
        i18n=list(context="context info here")
    )
    components <- rk.XML.components(
        rk.XML.component(
            label="a components label",
            file="plugins/MyOtherGUIdialog.xml"
        )
    )
    connect <- rk.XML.connect(
        governor="myGovernor",
        client="myCLient",
        get="visible",
        set="string",
        not=TRUE,
        reconcile=TRUE
    )
    context <- rk.XML.context(
        rk.XML.menu("Analysis", 
            rk.XML.entry(
                rk.XML.component(
                    label="a context label",
                    file="plugins/MyOtherGUIdialog.xml"
                )
            )
        )
    )
    convert <- rk.XML.convert(
        sources=list(text=rk.XML.input("a convert label")),
        mode=c(equals="myValue"),
        required=TRUE
    )
    copy <- rk.XML.copy(
        id="aCopyID",
        as="page"
    )
    dependencies <- rk.XML.dependencies(
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
    )
    dependency_check <- rk.XML.dependency_check(
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
    )
    dialog <- rk.XML.dialog(
        rk.XML.col(
            rk.XML.cbox(label="a dialog column label")
        ),
        label="a dialog label",
        recommended=TRUE,
        i18n=list(context="context info here")
    )
    dropdown <- rk.XML.dropdown(
        label="a dropdown label",
        options=list(
            "First Option"=c(val="val1"),
            rk.XML.option(
                "Second Option", val="val2", id.name="auto",
                i18n=list(context="context info1 here")
            ),
            "Third Option"=c(val="val3", chk=TRUE)
        ),
        i18n=list(context="context info2 here")
    )
    embed <- rk.XML.embed(
        component="componentID",
        button=TRUE,
        label="an embed label"
    )
    entry <- rk.XML.entry(
        component=rk.XML.component(
            label="an entry component label",
            file="plugins/MyOtherGUIdialog.xml"
        ),
        index=3
    )
    external <- rk.XML.external(
        id="externalID",
        default="none"
    )
    formula <- rk.XML.formula(
        fixed=rk.XML.varslot("Fixed factors", source=test.varselector <- rk.XML.varselector("Select some vars")),
        dependent=rk.XML.varslot("Dependent variables", source=test.varselector)
    )
    frame <- rk.XML.frame(
        rk.XML.row(
            rk.XML.cbox(label="a frame row label")
        ),
        label="a frame label",
        checkable=TRUE,
        chk=FALSE
    )
    help <- rk.XML.help(
        "help_file.rkh"
    )
    hierarchy <- rk.XML.hierarchy(
        rk.XML.menu("Analysis", 
            rk.XML.entry(
                rk.XML.component(
                    label="a hierarchy label",
                    file="plugins/MyOtherGUIdialog.xml"
                )
            )
        )
    )
    include <- rk.XML.include(
        "../include_file.xml"
    )
    input <- rk.XML.input(
        label="an input label",
        initial="init",
        size="small",
        required=TRUE,
    )
    insert <- rk.XML.insert(
        snippet=rk.XML.snippet(
            rk.XML.vars(
                "Variables",
                "Fixed",
                formula.dependent="Dependent"
            )
        )
    )
    logic=rk.XML.logic(
        rk.XML.connect(
            governor="myGovernor",
            client="myCLient"
        )
    )
    matrix <- rk.XML.matrix(
        label="a matrix label",
        mode="integer",
        rows=3,
        columns=3,
        min=0,
        max=100,
        allow_missings=TRUE,
        allow_user_resize_columns=FALSE,
        allow_user_resize_rows=FALSE,
        fixed_width=TRUE,
        fixed_height=TRUE,
        horiz_headers=c("hone", "htwo", "hthree"),
        vert_headers=c("vone", "vtwo", "vthree")
    )
    menu <- rk.XML.menu(
        "Analysis",
        rk.XML.entry(
            rk.XML.component(
                label="a hierarchy label",
                file="plugins/MyOtherGUIdialog.xml"
            )
        ),
        index=3
    )
    optioncolumn <- rk.XML.optioncolumn(
        connect=rk.XML.input(label="an optioncolumn label"),
        modifier="text",
        label=TRUE,
        external=TRUE,
        default="rarely useful"
    )
    optiondisplay <- rk.XML.optiondisplay(
        index=FALSE
    )
    option <- rk.XML.option(  
        label="an option label",
        val="value",
        chk=TRUE
    )
    optionset <- rk.XML.optionset(
        content=list(
            rk.XML.row(
                rk.XML.cbox(label="a content row label")
            )
        ),
        optioncolumn=list(
                rk.XML.optioncolumn("an optioncolumn")
            ),
            min_rows=1,
            min_rows_if_any=1,
            max_rows=5,
            keycolumn="myKey",
            logic=rk.XML.logic(
                rk.XML.connect(
                    governor="myGovernor",
                    client="myCLient"
                )
            ),
            optiondisplay=TRUE
        )
    page <- rk.XML.page(
        rk.XML.text("a page text")
    )
#     pluginmap <- rk.XML.pluginmap(
#         name="a plugin name",
#         about=NULL,
#         components=components,
#         hierarchy="test",
#         require=NULL,
#         x11.context=NULL,
#         import.context=NULL,
#         clean.name=TRUE,
#         hints=FALSE,
#         gen.info=TRUE,
#         dependencies=NULL,
#         namespace=name,
#         priority="medium"
#     )
#     plugin <- rk.XML.plugin(
#     )
#     preview <- rk.XML.preview(
#     )
#     radio <- rk.XML.radio(
#     )
#     require <- rk.XML.require(
#     )
#     row <- rk.XML.row(
#     )
#     saveobj <- rk.XML.saveobj(
#     )
#     select <- rk.XML.select(
#     )
#     set <- rk.XML.set(
#     )
#     snippet <- rk.XML.snippet(
#     )
#     snippets <- rk.XML.snippets(
#     )
#     spinbox <- rk.XML.spinbox(
#     )
#     stretch <- rk.XML.stretch(
#     )
#     XMLswitch <- rk.XML.switch(
#     )
#     tabbook <- rk.XML.tabbook(
#     )
#     text <- rk.XML.text(
#     )
#     valueselector <- rk.XML.valueselector(
#     )
#     valueslot <- rk.XML.valueslot(
#     )
#     values <- rk.XML.values(
#     )
#     varselector <- rk.XML.varselector(
#     )
#     varslot <- rk.XML.varslot(
#     )
#     vars <- rk.XML.vars(
#     )
#     wizard <- rk.XML.wizard(
#     )

    XML_test_standards <- list(
        about=about,
        attribute=attribute,
        browser=browser,
        checkbox=checkbox,
        code=code,
        col=col,
        component=component,
        components=components,
        connect=connect,
        context=context,
        convert=convert,
        copy=copy,
        dependencies=dependencies,
        dependency_check=dependency_check,
        dialog=dialog,
        dropdown=dropdown,
        embed=embed,
        entry=entry,
        external=external,
        formula=formula,
        frame=frame,
        help=help,
        hierarchy=hierarchy,
        include=include,
        input=input,
        insert=insert,
        logic=logic,
        matrix=matrix,
        menu=menu,
        optioncolumn=optioncolumn,
        optiondisplay=optiondisplay,
        option=option,
        optionset=optionset,
        page=page#,
#         pluginmap=pluginmap,
#         plugin=plugin,
#         preview=preview,
#         radio=radio,
#         require=require,
#         row=row,
#         saveobj=saveobj,
#         select=select,
#         set=set,
#         snippet=snippet,
#         snippets=snippets,
#         spinbox=spinbox,
#         stretch=stretch,
#         switch=XMLswitch,
#         tabbook=tabbook,
#         text=text,
#         valueselector=valueselector,
#         valueslot=valueslot,
#         values=values,
#         varselector=varselector,
#         varslot=varslot,
#         vars=vars,
#         wizard=wizard
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
