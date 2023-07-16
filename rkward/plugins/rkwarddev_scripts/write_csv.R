# - This file is part of the RKWard project.
# SPDX-FileCopyrightText: by Meik Michalke <meik.michalke@hhu.de>
# SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
# local({
## Vorbereiten
require(rkwarddev)
rkwarddev.required("0.06-5")

# define where the plugin should write its files
output.dir <- tempdir()
# overwrite an existing plugin in output.dir?
overwrite <- TRUE
# if you set guess.getters to TRUE, the resulting code will need RKWard >= 0.6.0
guess.getter <- TRUE

## Berechne
about.plugin <- rk.XML.about(
  name="Export Table / CSV files",
  author=person(given="m.eik", family="michalke", email="meik.michalke@uni-duesseldorf.de", role=c("aut", "cre")),
  about=list(desc="Export data in table/CSV format", version="0.01-0", license="GPL (>= 3)", category="export")
)

# name of the main component, relevant for help page content
rk.set.comp("Export Table / CSV files")

############
## your plugin dialog and JavaScript should be put here
############

etc.var.select <- rk.XML.varselector("Select a variable or table")
etc.var.data <- rk.XML.varslot("Data", source=etc.var.select, required=TRUE,
  classes=c("array", "data.frame", "matrix", "character", "numeric", "integer"),
  id.name="x",
  help="Select a data object to export. Valid data types are array, data.frame, matrix, character, numeric, and integer."
)

etc.browser <- rk.XML.browser("", type="savefile", id.name="file",
  help="The file to save to. Existing files will be overwritten, unless the append option is checked."
)

etc.dropdown.encoding <- rk.XML.dropdown("File encoding",
  options=list(
    "(default)"=c(val="", chk=TRUE),
    "UTF-8"=c(val="UTF-8"),
    "Latin 1"=c(val="latin1"),
    "ISO8859-1 Latin-1 Western European"=c(val="ISO8859-1"),
    "ISO8859-2 Latin-2 Central European"=c(val="ISO8859-2"),
    "ISO8859-3 Latin-3 South European"=c(val="ISO8859-3"),
    "ISO8859-4 Latin-4 North European"=c(val="ISO8859-4"),
    "ISO8859-5 Latin/Cyrillic"=c(val="ISO8859-5"),
    "ISO8859-6 Latin/Arabic"=c(val="ISO8859-6"),
    "ISO8859-7 Latin/Greek"=c(val="ISO8859-7"),
    "MS-GREEK"=c(val="MS-GREEK"),
    "ISO8859-8 Latin/Hebrew"=c(val="ISO8859-8"),
    "ISO8859-9 Latin-5 Turkish"=c(val="ISO8859-9"),
    "ISO8859-10 Latin-6 Nordic"=c(val="ISO8859-10"),
    "ISO8859-11 Latin/Thai"=c(val="ISO8859-11"),
    "ISO8859-13 Latin-7 Baltic Rim"=c(val="ISO8859-13"),
    "ISO8859-14 Latin-8 Celtic"=c(val="ISO8859-14"),
    "ISO8859-15 Latin-9 Western European (EUR)"=c(val="ISO8859-15"),
    "ISO8859-16 Latin-10 South-Eastern European"=c(val="ISO8859-16"),
    "Other (specify below)"=c(val="other")
  ),
  id.name="encoding",
  help="If the default character encoding doesn't work for you, you can set a specific one here."
)
etc.input.encoding <- rk.XML.input("Custom encoding", required=TRUE, id.name="user_encoding",
  help="Use this field to set a custom encoding, in case the dropdown menu doesn't list the one you need."
)


etc.drop.format <- rk.XML.dropdown("File format",
  options=list(
    "Comma separated values (CSV)"=c(val="csv", chk="true"),
    "Semicolon separated values, comma as decimal separator (CSV2)"=c(val="csv2"),
    "Tab separated values (TAB)"=c(val="delim"),
    "Tab separated values, comma as decimal separator (TAB2)"=c(val="delim2"),
    "Custom"=c(val="table")
  ),
  id.name="quick",
  help="Select one of the default file formats here. They will set some of the options (like field separator and decimal point) with reasonable defaults. Use the custom format if you need more control."
)

etc.frame.characters <- rk.XML.frame(
  rk.XML.row(
    rk.XML.col(
      etc.radio.decimal <- rk.XML.radio("Decimal point character",
        options=list(
          etc.radio.decimal.opt.period <- rk.XML.option("'.' (Period)", val=".", id.name="decPeriod", chk=TRUE),
          etc.radio.decimal.opt.comma <- rk.XML.option("',' (Comma)", val=",", id.name="decComma"),
          etc.radio.decimal.opt.other <- rk.XML.option("Custom (specify below)", val="other", id.name="decOther")
        ),
        id.name="dec",
        help="Specify the desired decimal point character. If this is greyed out, the selected preset sets this for you."
      ),
      rk.XML.stretch(),
      etc.input.decimal <- rk.XML.input("", required=TRUE, id="custom_dec",
        help="Use this value as the custom decimal point character."),
      id.name="col_decimal"
    ),
    rk.XML.col(
      etc.radio.field <- rk.XML.radio("Field separator character",
        options=list(
          etc.radio.field.opt.tab <- rk.XML.option("Tab", val="\\t", id.name="sepTab"),
          etc.radio.field.opt.semcl <- rk.XML.option("';' (Semicolon)", val=";", id.name="sepSemiC"),
          etc.radio.field.opt.comma <- rk.XML.option("',' (Comma)", val=",", id.name="sepComma"),
          etc.radio.field.opt.space <- rk.XML.option("Space", val=" ", id.name="sepSpace", chk=TRUE),
          etc.radio.field.opt.other <- rk.XML.option("Custom (specify below)", val="other", id.name="sepOther")
        ),
        id.name="sep",
        help="Specify the desired field separator character. If this is greyed out, the selected preset sets this for you."
      ),
      rk.XML.stretch(),
      etc.input.field <- rk.XML.input("", required=TRUE, id="custom_sep",
        help="Use this value as the custom field separator character."),
      id.name="col_field"
    )
  )
)

etc.frame.specialChars <- rk.XML.frame(
  rk.XML.row(
    rk.XML.col(
      etc.cbox.quote <- rk.XML.cbox("Quote all strings", id.name="quote", chk=TRUE,
        help="Check this to have all string values quoted."),
      etc.radio.quote <- rk.XML.radio("Handling double quote characters",
        options=list(
          etc.radio.quote.opt.escape <- rk.XML.option("Escape", val="escape", id.name="qmethodEsc", chk=TRUE),
          etc.radio.quote.opt.double <- rk.XML.option("Double", val="double", id.name="qmethodDbl")
        ),
        id.name="qmethod",
        help="Controls how existing quotes in values are handled -- they can either be escaped or double quoted. If this is greyed out, the selected preset sets this for you."
      )
    ),
    rk.XML.col(
      etc.input.na <- rk.XML.input("Character for missing values", initial="NA", size="small", id.name="na",
        help="Set the character to indicate missing data."
      ),
      etc.dropdown.eol <- rk.XML.dropdown("Newline character",
        options=list(
          "\\n (LF: GNU/Linux, BSD, OS X)"=c(val="\\n", chk=TRUE),
          "\\r\\n (CR+LF: Windows, DOS)"=c(val="\\r\\n"),
          "\\r (CR: Mac OS <= 9)"=c(val="\\r"),
          "\\n\\r (LF+CR: RISC OS)"=c(val="\\n\\r")
        ),
        id.name="eol",
        help="Set the character used to indicate a line break (EOL). This varies between operating systems, changing this option might help in case you run into problems when exchanging files with other people."
      )
    )
  )
)


etc.var.rc.select <- rk.XML.varselector("Select row or column names")
etc.var.rc.rows <- rk.XML.varslot("Row names (character vector)", source=etc.var.rc.select, required=TRUE,
  classes=c("character"), id.name="custRowNames",
  help="Select a character vector to use for custom row names."
)
etc.var.rc.cols <- rk.XML.varslot("Column names (character vector)", source=etc.var.rc.select, required=TRUE,
  classes=c("character"), id.name="custColNames",
  help="Select a character vector to use for custom column names.")

etc.frame.specs.rows <- rk.XML.frame(
  etc.radio.rownames <- rk.XML.radio("Row names",
    options=list(
      etc.radio.rownames.names.rows <- rk.XML.option("Use current row names", val="TRUE", id.name="temp1"),
      etc.radio.rownames.names.none <- rk.XML.option("No names", val="FALSE", id.name="temp2"),
      etc.radio.rownames.names.custom <- rk.XML.option("Custom (specify below)", val="custoRow")
    ),
    id.name="rowname",
    help="Decide how to deal with row names."
  ),
  etc.var.rc.rows,
  rk.XML.stretch(),
  label="Row  specifications"
)
etc.frame.specs.cols <- rk.XML.frame(
  etc.radio.colnames <- rk.XML.radio("Column names",
    options=list(
      etc.radio.colnames.names.cols <- rk.XML.option("Use current column names", val="TRUE"),
      etc.radio.colnames.names.none <- rk.XML.option("No names", val="FALSE"),
      etc.radio.colnames.names.custom <- rk.XML.option("Custom (specify below)", val="custoCol")
    ),
    id.name="colname",
    help="Decide how to deal with column names. If this is greyed out, the selected preset sets this for you."
  ),
  etc.var.rc.cols,
  rk.XML.stretch(),
  label="Column specifications"
)

etc.tabs <- rk.XML.tabbook(tabs=list(
    "Data and File Format"=list(
      rk.XML.row(
        rk.XML.col(
          etc.var.select
        ),
        rk.XML.col(
          rk.XML.frame(
            etc.var.data
          ),
          rk.XML.frame(
            etc.browser,
            etc.cbox.append <- rk.XML.cbox("Append to file (if it exists)", id.name="append",
              help="Should existing files be overwritten or the data appended? If this is greyed out, the selected preset sets this for you."
            ),
            label="Save to"
          )
        )
      ),
      rk.XML.row(
        rk.XML.col(
          etc.drop.format,
          etc.frame.characters
        )
      )
    ),
    "Rows and Columns"=list(
      rk.XML.row(
        rk.XML.col(
          etc.var.rc.select,
          id.name="select_rowcol_names"
        ),
        rk.XML.col(
          etc.frame.specs.rows,
          etc.frame.specs.cols
        )
      )
    ),
    "Encoding and Character Options"=list(
      rk.XML.row(
        rk.XML.col(
          rk.XML.frame(
            etc.dropdown.encoding,
            etc.input.encoding,
            label="Encoding"
          ),
          rk.XML.stretch(),
          etc.frame.specialChars
        )
      )
    )
  )
)

etc.dialog <- rk.XML.dialog(etc.tabs, label="Export Table / CSV files")

## logic

etc.logic <- rk.XML.logic(
  etc.external <- rk.XML.external("filename"),
  rk.XML.connect(governor=etc.external, client=etc.browser, set="selection"),

  customEnc <- rk.XML.convert(sources=list(string=etc.dropdown.encoding), mode=c(equals="other"), id.name="customEnc"),
  rk.XML.connect(governor=customEnc, client=etc.input.encoding),

  quickNone <- rk.XML.convert(sources=list(string=etc.drop.format), mode=c(equals="table"), id.name="quickNone"),
  quickCSV <- rk.XML.convert(sources=list(string=etc.drop.format), mode=c(equals="csv"), id.name="quickCSV"),
  quickCSV2 <- rk.XML.convert(sources=list(string=etc.drop.format), mode=c(equals="csv2"), id.name="quickCSV2"),
  quickTAB <- rk.XML.convert(sources=list(string=etc.drop.format), mode=c(equals="delim"), id.name="quickTAB"),
  quickTAB2 <- rk.XML.convert(sources=list(string=etc.drop.format), mode=c(equals="delim2"), id.name="quickTAB2"),
  quickCC2TT2 <- rk.XML.convert(sources=list(string=etc.drop.format), mode=c(notequals="table"), id.name="quickCC2TT2"),

  quickNCT <- rk.XML.convert(sources=list(quickNone, quickCSV, quickTAB), mode=c(or=""), id.name="quickNCT"),
  rk.XML.connect(governor=quickNCT, client=etc.radio.decimal.opt.period),
  quickNC2T2 <- rk.XML.convert(sources=list(quickNone, quickCSV2, quickTAB2), mode=c(or=""), id.name="quickNC2T2"),
  rk.XML.connect(governor=quickNC2T2, client=etc.radio.decimal.opt.comma),
  rk.XML.connect(governor=quickNone, client=etc.radio.decimal.opt.other),

  customdec <- rk.XML.convert(sources=list(string=etc.radio.decimal), mode=c(equals="other"), id.name="customdec"),
  rk.XML.connect(governor=customdec, client=etc.input.decimal),

  customsep <- rk.XML.convert(sources=list(string=etc.radio.field), mode=c(equals="other"), id.name="customsep"),
  rk.XML.connect(governor=customsep, client=etc.input.field),

  quickNTT2 <- rk.XML.convert(sources=list(quickNone, quickTAB, quickTAB2), mode=c(or=""), id.name="quickNTT2"),
  rk.XML.connect(governor=quickNTT2, client=etc.radio.field.opt.tab),
  quickNC2 <- rk.XML.convert(sources=list(quickNone, quickCSV2), mode=c(or=""), id.name="quickNC2"),
  rk.XML.connect(governor=quickNC2, client=etc.radio.field.opt.semcl),
  quickNC <- rk.XML.convert(sources=list(quickNone, quickCSV), mode=c(or=""), id.name="quickNC"),
  rk.XML.connect(governor=quickNC, client=etc.radio.field.opt.comma),
  rk.XML.connect(governor=quickNone, client=etc.radio.field.opt.space),
  rk.XML.connect(governor=quickNone, client=etc.radio.field.opt.other),

  quickCC2 <- rk.XML.convert(sources=list(quickCSV, quickCSV2), mode=c(or=""), id.name="quickCC2"),
  rk.XML.connect(governor=quickCC2, client=etc.cbox.append, not=TRUE),
  rk.XML.connect(governor=quickCC2, client=etc.frame.specs.cols, not=TRUE),
  rk.XML.connect(governor=quickCC2, client=etc.radio.quote.opt.escape, not=TRUE),

  customizerow <- rk.XML.convert(sources=list(string=etc.radio.rownames), mode=c(equals="custoRow"), id.name="customizerow"),
  rk.XML.connect(governor=customizerow, client=etc.var.rc.rows),

  customizecol <- rk.XML.convert(sources=list(string=etc.radio.colnames), mode=c(equals="custoCol"), id.name="customizecol"),
  rk.XML.connect(governor=customizecol, client=etc.var.rc.cols)#,
)

## JavaScript

etc.JS <- rk.paste.JS(
  ite(id(etc.drop.format, " == \"csv\" || ", etc.drop.format, " == \"csv2\""),
    rk.paste.JS(
      echo(
        "\n\t# some options can't be changed with write.", etc.drop.format, "() and are set to these values:\n",
        "\t# append=FALSE, sep=\"", etc.radio.field, "\", dec=\"", etc.radio.decimal, "\""
      ),
      ite(id(etc.radio.rownames, " == \"TRUE\""),
        echo(", col.names=NA"),
        echo(", col.names=TRUE")
      ),
      echo(", qmethod=\"double\"")
    )
  ),
  ite(id(etc.drop.format, " != \"delim\" && ", etc.drop.format, " != \"delim2\""),
    echo("\n\twrite.", etc.drop.format, "("),
    echo("\n\twrite.table(")
  ),
  echo("\n\t\tx=", etc.var.data),
  # file=etc.browser,             # default=""
  echo(",\n\t\tfile=\"", etc.browser, "\""),

  # append=etc.cbox.append,       # default=FALSE
    # csv, csv2: hide
  ite(id(etc.drop.format, " != \"csv\" && ", etc.drop.format, " != \"csv2\""),
    tf(etc.cbox.append, opt="append")
  ),
  # quote=etc.radio.quote,        # default=TRUE
  tf(etc.cbox.quote, true=FALSE, not=TRUE, opt="quote"),

  # sep=etc.radio.field,          # default=" ",
    # csv, csv2: hide
    # delim, delim2: sep="\t",
  ite(id(etc.drop.format, " != \"csv\" && ", etc.drop.format, " != \"csv2\" && ", etc.radio.field, " != \" \""),
    ite(id(etc.radio.field, " != \"other\""),
      echo(",\n\t\tsep=\"", etc.radio.field, "\""),
      ite(id(etc.input.field, " != \" \""),
        echo(",\n\t\tsep=\"", etc.input.field, "\"")
      )
    )
  ),

  # eol=etc.dropdown.eol,         # default="\n",
  ite(id(etc.dropdown.eol, " != \"\\\\n\""),
    echo(",\n\t\teol=\"", etc.dropdown.eol, "\"")
  ),
  # na=etc.input.na,              # default="NA",
  ite(id(etc.input.na, " != \"NA\""),
    echo(",\n\t\tna=\"", etc.input.na, "\"")
  ),

  # dec=etc.radio.decimal,        # default=".",
    # csv, csv2: hide
    # delim: dec="."
    # delim2: dec=","
  ite(id(etc.drop.format, " != \"csv\" && ", etc.drop.format, " != \"csv2\" && ", etc.radio.decimal, " != \".\""),
    ite(id(etc.radio.decimal, " != \"other\""),
      echo(",\n\t\tdec=\"", etc.radio.decimal, "\""),
      ite(id(etc.input.decimal, " != \".\""),
        echo(",\n\t\tdec=\"", etc.input.decimal, "\"")
      )
    )
  ),

  # row.names=etc.radio.rownames, # default=TRUE, etc.var.rc.rows
  ite(id(etc.radio.rownames, " == \"custoRow\""),
    echo(",\n\t\trow.names=", etc.var.rc.rows),
    ite(id(etc.radio.rownames, " != \"TRUE\""),
      echo(",\n\t\trow.names=", etc.radio.rownames)
    )
  ),

  # col.names=etc.radio.colnames, # default=TRUE, etc.var.rc.cols
    # csv, csv2: hide
  ite(id(etc.drop.format, " != \"csv\" && ", etc.drop.format, " != \"csv2\""),
    ite(id(etc.radio.colnames, " == \"custoCol\""),
      echo(",\n\t\tcol.names=", etc.var.rc.cols),
      ite(id(etc.radio.colnames, " != \"TRUE\""),
        echo(",\n\t\tcol.names=", etc.radio.colnames)
      )
    )
  ),

  # qmethod=etc.radio.quote,      # default="escape"
    # csv, csv2: hide
  ite(id(etc.drop.format, " != \"csv\" && ", etc.drop.format, " != \"csv2\" && ", etc.radio.quote, " != \"escape\""),
    echo(",\n\t\tqmethod=\"", etc.radio.quote, "\"")
  ),

  # fileEncoding=,                # default=""
  ite(id(etc.dropdown.encoding, " != \"other\""),
    echo(",\n\t\tfileEncoding=\"", etc.dropdown.encoding, "\""),
    ite(id(etc.input.encoding, " != \"\""),
      echo(",\n\t\tfileEncoding=\"", etc.input.encoding, "\"")
    )
  ),

  # closing bracket of write function
  echo("\n\t)\n\n")
)

# etc.JS.printout <- rk.paste.JS(
# )


############
## help file

etc.rkh.summary <- rk.rkh.summary("Export data to CSV or similar file formats.")

etc.rkh.usage <- rk.rkh.usage("Select a tabular data object (like a data frame or matrix) and chose a file to save to. All other options allow you to define the file format, handling of column and row names as well as special character handling.")

etc.rkh.related <- rk.rkh.related(rk.rkh.link("write.table", "R manual page for write.table(), which is called by this plugin."))


#############
## the main call
## if you run the following function call, files will be written to output.dir!
#############
# this is where things get serious, that is, here all of the above is put together into one plugin
plugin.dir <- rk.plugin.skeleton(
  about=about.plugin,
  path=output.dir,
  guess.getter=guess.getter,
  scan=c("var", "saveobj", "settings"),
  xml=list(
    dialog=etc.dialog,
    #wizard=,
    logic=etc.logic#,
    #snippets=
  ),
  js=list(
    results.header="Export Table / CSV files",
    header.add=list(add=c("File", etc.browser), addFromUI=etc.var.data),
    #load.silencer=,
    #require=,
    #variables=,
    #globals=,
    #preprocess=,
    calculate=etc.JS#,
#    printout=etc.JS.printout#,
    #doPrintout=
  ),
  rkh=list(
    summary=etc.rkh.summary,
    usage=etc.rkh.usage,
    #sections=,
    #settings=,
    related=etc.rkh.related#,
    #technical=
  ),
  create=c("pmap", "xml", "js", "desc", "rkh"),
  overwrite=overwrite,
  #components=list(),
  #provides=c("logic", "dialog"), 
  pluginmap=list(name="Export Table / CSV files", hierarchy=list("file", "export")),
  #dependencies=plugin.dependencies, 
  tests=FALSE, 
  edit=FALSE, 
  load=TRUE, 
  show=TRUE
)

})
