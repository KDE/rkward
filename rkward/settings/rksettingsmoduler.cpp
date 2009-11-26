/***************************************************************************
                          rksettingsmoduler  -  description
                             -------------------
    begin                : Wed Jul 28 2004
    copyright            : (C) 2004, 2007, 2009 by Thomas Friedrichsmeier
    email                : tfry@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "rksettingsmoduler.h"

#include <klocale.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kinputdialog.h>
#include <knuminput.h>
#include <kfiledialog.h>

#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <QVBoxLayout>
#include <QGridLayout>

#include "../misc/multistringselector.h"
#include "../rbackend/rinterface.h"
#include "../rkglobals.h"
#include "../debug.h"

// static members
QString RKSettingsModuleR::options_outdec;
int RKSettingsModuleR::options_width;
int RKSettingsModuleR::options_warn;
int RKSettingsModuleR::options_warningslength;
int RKSettingsModuleR::options_maxprint;
bool RKSettingsModuleR::options_keepsource;
bool RKSettingsModuleR::options_keepsourcepkgs;
int RKSettingsModuleR::options_expressions;
int RKSettingsModuleR::options_digits;
bool RKSettingsModuleR::options_checkbounds;
QString RKSettingsModuleR::options_printcmd;
QString RKSettingsModuleR::options_editor;
QString RKSettingsModuleR::options_pager;
// static constants
QString RKSettingsModuleR::builtin_editor = "<rkward>";
// session constants
QString RKSettingsModuleR::help_base_url;

RKSettingsModuleR::RKSettingsModuleR (RKSettings *gui, QWidget *parent) : RKSettingsModule(gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	QLabel *label = new QLabel (i18n ("The following settings mostly affect R behavior in the console. It is generally safe to keep these unchanged."), this);
	label->setWordWrap (true);
	main_vbox->addWidget (label);

	QGridLayout *grid = new QGridLayout ();
	main_vbox->addLayout (grid);
	int row = -1;

	// options (warn)
	grid->addWidget (new QLabel (i18n ("Display warnings"), this), ++row, 0);
	warn_input = new QComboBox (this);
	warn_input->setEditable (false);
	warn_input->insertItem (0, i18n ("Suppress warnings"));			// do not change the order of options! See also: applyChanges ()
	warn_input->insertItem (1, i18n ("Print warnings later (default)"));
	warn_input->insertItem (2, i18n ("Print warnings immediately"));
	warn_input->insertItem (3, i18n ("Convert warnings to errors"));
	warn_input->setCurrentIndex (options_warn + 1);
	connect (warn_input, SIGNAL (activated (int)), this, SLOT (boxChanged (int)));
	grid->addWidget (warn_input, row, 1);

	// options (OutDec)
	grid->addWidget (new QLabel (i18n ("Decimal character (only for printing)"), this), ++row, 0);
	outdec_input = new QLineEdit (options_outdec, this);
	outdec_input->setMaxLength (1);
	connect (outdec_input, SIGNAL (textChanged (const QString &)), this, SLOT (textChanged (const QString &)));
	grid->addWidget (outdec_input, row, 1);

	// options (width)
	grid->addWidget (new QLabel (i18n ("Output width (characters)"), this), ++row, 0);
	width_input = new KIntSpinBox (10, 10000, 1, options_width, this);
	connect (width_input, SIGNAL (valueChanged (int)), this, SLOT (boxChanged (int)));
	grid->addWidget (width_input, row, 1);

	// options (max.print)
	grid->addWidget (new QLabel (i18n ("Maximum number of elements shown in print"), this), ++row, 0);
	maxprint_input = new KIntSpinBox (100, INT_MAX, 1, options_maxprint, this);
	connect (maxprint_input, SIGNAL (valueChanged (int)), this, SLOT (boxChanged (int)));
	grid->addWidget (maxprint_input, row, 1);

	// options (warnings.length)
	grid->addWidget (new QLabel (i18n ("Maximum length of warnings/errors to print"), this), ++row, 0);
	warningslength_input = new KIntSpinBox (100, 8192, 1, options_warningslength, this);
	connect (warningslength_input, SIGNAL (valueChanged (int)), this, SLOT (boxChanged (int)));
	grid->addWidget (warningslength_input, row, 1);

	// options (keep.source)
	grid->addWidget (new QLabel (i18n ("Keep comments in functions"), this), ++row, 0);
	keepsource_input = new QComboBox (this);
	keepsource_input->setEditable (false);
	keepsource_input->addItem (i18n ("TRUE (default)"), true);
	keepsource_input->addItem (i18n ("FALSE"), false);
	keepsource_input->setCurrentIndex (options_keepsource ? 0 : 1);
	connect (keepsource_input, SIGNAL (activated (int)), this, SLOT (boxChanged (int)));
	grid->addWidget (keepsource_input, row, 1);

	// options (keep.source.pkgs)
	grid->addWidget (new QLabel (i18n ("Keep comments in packages"), this), ++row, 0);
	keepsourcepkgs_input = new QComboBox (this);
	keepsourcepkgs_input->setEditable (false);
	keepsourcepkgs_input->addItem (i18n ("TRUE"), true);
	keepsourcepkgs_input->addItem (i18n ("FALSE (default)"), false);
	keepsourcepkgs_input->setCurrentIndex (options_keepsourcepkgs ? 0 : 1);
	connect (keepsourcepkgs_input, SIGNAL (activated (int)), this, SLOT (boxChanged (int)));
	grid->addWidget (keepsourcepkgs_input, row, 1);

	// options (expressions)
	grid->addWidget (new QLabel (i18n ("Maximum level of nested expressions"), this), ++row, 0);
	expressions_input = new KIntSpinBox (25, 500000, 1, options_expressions, this);
	connect (expressions_input, SIGNAL (valueChanged (int)), this, SLOT (boxChanged (int)));
	grid->addWidget (expressions_input, row, 1);

	// options (digits)
	grid->addWidget (new QLabel (i18n ("Default decimal precision in print ()"), this), ++row, 0);
	digits_input = new KIntSpinBox (1, 22, 1, options_digits, this);
	connect (digits_input, SIGNAL (valueChanged (int)), this, SLOT (boxChanged (int)));
	grid->addWidget (digits_input, row, 1);

	// options (check.bounds)
	grid->addWidget (new QLabel (i18n ("Check vector bounds (warn)"), this), ++row, 0);
	checkbounds_input = new QComboBox (this);
	checkbounds_input->setEditable (false);
	checkbounds_input->addItem (i18n ("TRUE"), true);
	checkbounds_input->addItem (i18n ("FALSE (default)"), false);
	checkbounds_input->setCurrentIndex (options_checkbounds ? 0 : 1);
	connect (checkbounds_input, SIGNAL (activated (int)), this, SLOT (boxChanged (int)));
	grid->addWidget (checkbounds_input, row, 1);

	grid->addWidget (new QLabel (i18n ("Command used to send files to printer"), this), ++row, 0);
	printcmd_input = new QLineEdit (options_printcmd, this);
	connect (printcmd_input, SIGNAL (textChanged (const QString &)), this, SLOT (textChanged (const QString &)));
	grid->addWidget (printcmd_input, row, 1);

	grid->addWidget (new QLabel (i18n ("Editor command"), this), ++row, 0);
	editor_input = new QComboBox (this);
	editor_input->setEditable (true);
	editor_input->addItem (builtin_editor);
	if (options_editor != builtin_editor) {
		editor_input->addItem (options_editor);
		editor_input->setCurrentIndex (1);
	}
	connect (editor_input, SIGNAL (editTextChanged (const QString &)), this, SLOT (textChanged (const QString &)));
	grid->addWidget (editor_input, row, 1);

	grid->addWidget (new QLabel (i18n ("Pager command"), this), ++row, 0);
	pager_input = new QComboBox (this);
	pager_input->setEditable (true);
	pager_input->addItem (builtin_editor);
	if (options_pager != builtin_editor) {
		pager_input->addItem (options_pager);
		pager_input->setCurrentIndex (1);
	}
	connect (pager_input, SIGNAL (editTextChanged (const QString &)), this, SLOT (textChanged (const QString &)));
	grid->addWidget (pager_input, row, 1);

	main_vbox->addStretch ();
}

RKSettingsModuleR::~RKSettingsModuleR() {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleR::boxChanged (int) {
	RK_TRACE (SETTINGS);
	change ();
}

void RKSettingsModuleR::pathChanged () {
	RK_TRACE (SETTINGS);
	change ();
}

void RKSettingsModuleR::textChanged (const QString &) {
	RK_TRACE (SETTINGS);
	change ();
}

QString RKSettingsModuleR::caption () {
	RK_TRACE (SETTINGS);
	return (i18n ("R-Backend"));
}

bool RKSettingsModuleR::hasChanges () {
	RK_TRACE (SETTINGS);
	return changed;
}

void RKSettingsModuleR::applyChanges () {
	RK_TRACE (SETTINGS);

	options_outdec = outdec_input->text ();
	options_width = width_input->value ();
	options_warn = warn_input->currentIndex () - 1;
	options_warningslength = warningslength_input->value ();
	options_maxprint = maxprint_input->value ();
	options_keepsource = keepsource_input->itemData (keepsource_input->currentIndex ()).toBool ();
	options_keepsourcepkgs = keepsourcepkgs_input->itemData (keepsourcepkgs_input->currentIndex ()).toBool ();
	options_expressions = expressions_input->value ();
	options_digits = digits_input->value ();
	options_checkbounds = checkbounds_input->itemData (checkbounds_input->currentIndex ()).toBool ();
	options_printcmd = printcmd_input->text ();
	options_editor = editor_input->currentText ();
	options_pager = pager_input->currentText ();

// apply run time options in R
	QStringList commands = makeRRunTimeOptionCommands ();
	for (QStringList::const_iterator it = commands.begin (); it != commands.end (); ++it) {
		RKGlobals::rInterface ()->issueCommand (*it, RCommand::App, QString::null, 0, 0, commandChain ());
	}
}

//static
QStringList RKSettingsModuleR::makeRRunTimeOptionCommands () {
	QStringList list;

	QString tf;
	list.append ("options (OutDec=\"" + options_outdec.left (1) + "\")\n");
	list.append ("options (width=" + QString::number (options_width) + ")\n");
	list.append ("options (warn=" + QString::number (options_warn) + ")\n");
	list.append ("options (max.print=" + QString::number (options_maxprint) + ")\n");
	list.append ("options (warnings.length=" + QString::number (options_warningslength) + ")\n");
	if (options_keepsource) tf = "TRUE"; else tf = "FALSE";
	list.append ("options (keep.source=" + tf + ")\n");
	if (options_keepsourcepkgs) tf = "TRUE"; else tf = "FALSE";
	list.append ("options (keep.source.pkgs=" + tf + ")\n");
	list.append ("options (expressions=" + QString::number (options_expressions) + ")\n");
	list.append ("options (digits=" + QString::number (options_digits) + ")\n");
	if (options_checkbounds) tf = "TRUE"; else tf = "FALSE";
	list.append ("options (checkbounds=" + tf + ")\n");
	list.append ("options (printcmd=\"" + options_printcmd + "\")\n");
	if (options_editor == builtin_editor) list.append ("options (editor=rk.edit.files)\n");
	else list.append ("options (editor=\"" + options_editor + "\")\n");
	if (options_pager == builtin_editor) list.append ("options (pager=rk.show.files)\n");
	else list.append ("options (pager=\"" + options_pager + "\")\n");

#warning TODO make the following options configurable
	list.append ("options (device=\"rk.screen.device\")\n");
	// register as interactive
	list.append ("try (deviceIsInteractive(name=\"rk.screen.device\"))\n");
	list.append ("options (help_type=\"html\")\n");		// for R 2.10.0 and above
	list.append ("options (htmlhelp=TRUE); options (chmhelp=FALSE)\n");	// COMPAT: for R 2.9.x and below
	list.append ("options (browser=rk.show.html)\n");

	return list;
}

void RKSettingsModuleR::save (KConfig *config) {
	RK_TRACE (SETTINGS);

	saveSettings (config);
}

void RKSettingsModuleR::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("R Settings");
	cg.writeEntry ("OutDec", options_outdec);
	cg.writeEntry ("width", options_width);
	cg.writeEntry ("warn", options_warn);
	cg.writeEntry ("maxprint", options_maxprint);
	cg.writeEntry ("warnings.length", options_warningslength);
	cg.writeEntry ("keep.source", options_keepsource);
	cg.writeEntry ("keep.source.pkgs", options_keepsourcepkgs);
	cg.writeEntry ("expressions", options_expressions);
	cg.writeEntry ("digits", options_digits);
	cg.writeEntry ("check.bounds", options_checkbounds);
	cg.writeEntry ("printcmd", options_printcmd);
	cg.writeEntry ("editor", options_editor);
	cg.writeEntry ("pager", options_pager);
}

void RKSettingsModuleR::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("R Settings");
	options_outdec = cg.readEntry ("OutDec", ".");
	options_width = cg.readEntry ("width", 80);
	options_warn = cg.readEntry ("warn", 0);
	options_maxprint = cg.readEntry ("max.print", 99999);
	options_warningslength = cg.readEntry ("warnings.length", 1000);
	options_keepsource = cg.readEntry ("keep.source", true);
	options_keepsourcepkgs = cg.readEntry ("keep.source.pkgs", false);
	options_expressions = cg.readEntry ("expressions", 5000);
	options_digits = cg.readEntry ("digits", 7);
	options_checkbounds = cg.readEntry ("check.bounds", false);
	options_printcmd = cg.readEntry ("printcmd", "kprinter");
	options_editor = cg.readEntry ("editor", builtin_editor);
	options_pager = cg.readEntry ("pager", builtin_editor);
}

//#################################################
//############### RKSettingsModuleRPackages ################
//#################################################

// static members
QStringList RKSettingsModuleRPackages::liblocs;
QStringList RKSettingsModuleRPackages::defaultliblocs;
bool RKSettingsModuleRPackages::archive_packages;
QStringList RKSettingsModuleRPackages::package_repositories;
QString RKSettingsModuleRPackages::essential_packages = QString ("base\nmethods\nutils\ngrDevices\ngraphics\nrkward");
int RKSettingsModuleRPackages::cran_mirror_index;
QString RKSettingsModuleRPackages::cran_mirror_url;

RKSettingsModuleRPackages::RKSettingsModuleRPackages (RKSettings *gui, QWidget *parent) : RKSettingsModule(gui, parent) {
	RK_TRACE (SETTINGS);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);

	main_vbox->addSpacing (2*RKGlobals::spacingHint ());

	repository_selector = new MultiStringSelector (i18n ("Package repositories (where libraries are downloaded from)"), this);
	repository_selector->setValues (package_repositories);
	connect (repository_selector, SIGNAL (listChanged ()), this, SLOT (listChanged ()));
	connect (repository_selector, SIGNAL (getNewStrings (QStringList*)), this, SLOT (addRepository (QStringList*)));
	main_vbox->addWidget (repository_selector);

	/** lists created from CRAN_mirrors.csv using CRAN_mirror_list.R: 
	 would ideally like to create this dynamically at build time 
	 in a separate .h file  as a static variable **/
	cran_mirror_list  << "Ask everytime" << "Argentina - Buenos Aires - Patan.com.ar" << "Argentina - Mendoza - CONICET" << "Australia - Melbourne - University of Melbourne" << "Austria - Wien - Wirtschaftsuniversitaet Wien" << "Belgium - Antwerp - K.U.Leuven Association" << "Brazil (PR) - Curitiba - Universidade Federal do Parana" << "Brazil (RJ) - Rio de Janeiro - Oswaldo Cruz Foundation" << "Brazil (SP 1) - Sao Paulo - University of Sao Paulo" << "Brazil (SP 2) - Piracicaba - University of Sao Paulo" << "Canada (BC) - Burnaby - Simon Fraser University" << "Canada (ON) - Toronto - University of Toronto" << "Canada (QC) - Montreal - iWeb" << "Chile - Santiago - Pontificia Universidad Catolica de Chile" << "China (Beijing) - Bejing - CTEX.ORG" << "China - Hong Kong - GeoExpat.Com" << "Colombia - Bogota - National University of Colombia" << "Croatia - Zagreb - Rudjer Boskovic Institute" << "Denmark - Aalborg - dotsrc.org" << "France - Toulouse - CICT" << "France - Lyon - Dept. of Biometry & Evol. Biology" << "France - Paris - Miroir-Francais" << "Germany - Berlin - Softliste.de" << "Germany - Goettingen - GWDG Goettingen" << "Germany - Hannover - Opensource Mirror Project" << "Germany - Muenchen - Rakanu.com" << "Germany - Wiesbaden - Yalwa GmbH" << "Iran - Mashhad - Ferdowsi University of Mashhad" << "Ireland - Dublin - HEAnet" << "Italy - Milano - Garr Mirror" << "Italy - Padua - University of Padua" << "Italy - Palermo - Universita degli Studi di Palermo" << "Japan - Aizu - University of Aizu" << "Japan - Hyogo - Hyogo University of Teacher Education" << "Japan - Tokyo - University of Tokyo" << "Japan - Tsukuba - University of Tsukuba" << "Korea - Seoul - Seoul National University" << "Mexico - Cuernavaca - Universidad Autonoma del Estado de Morelos" << "Netherlands (Amsterdam 2) - Amsterdam - Nedmirror" << "Netherlands - Utrecht - Utrecht University" << "New Zealand - Auckland - University of Auckland" << "Norway - Bergen - University of Bergen" << "Poland - Oswiecim - Piotrkosoft - Data Storage Center" << "Poland - Wroclaw - University of Wroclaw" << "Portugal - Porto - Universidade do Porto" << "Russia - Moscow - GIS-Lab.info" << "Singapore 1 - Singapore - National University of Singapore" << "Singapore 2 - Singapore - National University of Singapore" << "Slovakia - Bratislava - FYXM.net" << "South Africa - Grahamstown - Rhodes University" << "Spain - Madrid - Spanish National Research Network" << "Sweden - Uppsala - Swedish University Computer Network" << "Switzerland - Zuerich - ETH Zuerich" << "Taiwan - Taichung - Providence University" << "Taiwan (Taipeh) - Taipei - National Taiwan University" << "Thailand - Bangkok - Kapook.com" << "UK - Bristol - University of Bristol" << "USA (AZ) - Scottsdale - opensourceresources.org" << "USA (CA 1) - Berkeley - University of California" << "USA (CA 2) - Los Angeles - University of California" << "USA (IA) - Ames - Iowa State University" << "USA (MD) - Bethesda - National Cancer Institute" << "USA (MI) - Houghton - Michigan Technological University" << "USA (MO) - St. Louis - Washington University" << "USA (NC) - Chapel Hill - University of North Carolina" << "USA (OH) - Cleveland - Case Western Reserve University" << "USA (PA 1) - Pittsburgh - Statlib" << "USA (PA 2) - Pittsburgh - Hoobly Classifieds" << "USA (TN) - Knoxville - University of Tennessee" << "USA (TX 1) - San Antonio - Revolution Computing" << "USA (TX 2) - Dallas - CyberUse.com" << "USA (TX 3) - Houston - sixsigmaonline.org" << "USA (WA) - Seattle - Fred Hutchinson Cancer Research Center";
	cran_url_list << "@CRAN@" << "http://cran.patan.com.ar/" << "http://mirror.cricyt.edu.ar/r/" << "http://cran.ms.unimelb.edu.au/" << "http://cran.at.r-project.org/" << "http://www.freestatistics.org/cran/" << "http://cran.br.r-project.org/" << "http://cran.fiocruz.br/" << "http://www.vps.fmvz.usp.br/CRAN/" << "http://brieger.esalq.usp.br/CRAN/" << "http://cran.stat.sfu.ca/" << "http://probability.ca/cran/" << "http://cran.parentinginformed.com/" << "http://dirichlet.mat.puc.cl/" << "http://ftp.ctex.org/mirrors/CRAN/" << "http://mirrors.geoexpat.com/cran/" << "http://www.laqee.unal.edu.co/CRAN/" << "http://imago.irb.hr/r/" << "http://cran.dk.r-project.org/" << "http://cran.fr.r-project.org/" << "http://cran.univ-lyon1.fr/" << "http://cran.miroir-francais.fr/" << "http://mirrors.softliste.de/cran/" << "http://ftp5.gwdg.de/pub/misc/cran/" << "http://cran.mirroring.de" << "http://cran.rakanu.com/" << "http://ftp.yalwa.org/cran/" << "http://cran.um.ac.ir/" << "http://ftp.heanet.ie/mirrors/cran.r-project.org/" << "http://rm.mirror.garr.it/mirrors/CRAN/" << "http://cran.stat.unipd.it/" << "http://dssm.unipa.it/CRAN/" << "ftp://ftp.u-aizu.ac.jp/pub/lang/R/CRAN" << "http://essrc.hyogo-u.ac.jp/cran/" << "ftp://ftp.ecc.u-tokyo.ac.jp/CRAN/" << "http://cran.md.tsukuba.ac.jp/" << "http://bibs.snu.ac.kr/R/" << "http://www2.uaem.mx/r-mirror/" << "http://cran.nedmirror.nl/" << "http://cran-mirror.cs.uu.nl/" << "http://cran.stat.auckland.ac.nz/" << "http://cran.ii.uib.no/" << "http://piotrkosoft.net/pub/mirrors/CRAN/" << "http://r.meteo.uni.wroc.pl/" << "http://cran.pt.r-project.org/" << "http://cran.gis-lab.info/" << "http://cran.bic.nus.edu.sg/" << "http://cran.stat.nus.edu.sg/" << "http://cran.fyxm.net/" << "http://cran.za.r-project.org/" << "http://cran.es.r-project.org/" << "http://ftp.sunet.se/pub/lang/CRAN/" << "http://cran.ch.r-project.org/" << "http://cran.cs.pu.edu.tw/" << "http://cran.csie.ntu.edu.tw/" << "http://mirror.kapook.com/cran/" << "http://cran.uk.r-project.org/" << "http://cran.opensourceresources.org/" << "http://cran.cnr.Berkeley.edu" << "http://cran.stat.ucla.edu/" << "http://streaming.stat.iastate.edu/CRAN/" << "http://watson.nci.nih.gov/cran_mirror/" << "http://cran.mtu.edu/" << "http://cran.wustl.edu/" << "http://www.ibiblio.org/pub/languages/R/CRAN/" << "http://cran.case.edu/" << "http://lib.stat.cmu.edu/R/CRAN/" << "http://cran.mirrors.hoobly.com" << "http://mira.sunsite.utk.edu/CRAN/" << "http://www.revolution-computing.com/cran/" << "http://www.cyberuse.com/cran/" << "http://cran.sixsigmaonline.org/" << "http://cran.fhcrc.org/";
	main_vbox->addWidget (new QLabel (i18n ("CRAN download mirror:"), this));
	cran_mirrors = new QComboBox (this);
	cran_mirrors->setEditable (false);
	for (int i = 0; i < cran_mirror_list.size(); ++i)
		cran_mirrors->insertItem (i, i18n (cran_mirror_list.at(i).toLocal8Bit().constData()));
	cran_mirrors->setCurrentIndex (cran_mirror_index);
	connect (cran_mirrors, SIGNAL (activated (int)), this, SLOT (boxChanged (int)));
	main_vbox->addWidget (cran_mirrors);

	archive_packages_box = new QCheckBox (i18n ("Archive downloaded packages"), this);
	archive_packages_box->setChecked (archive_packages);
	connect (archive_packages_box, SIGNAL (stateChanged (int)), this, SLOT (boxChanged (int)));
	main_vbox->addWidget (archive_packages_box);	

	main_vbox->addStretch ();

	libloc_selector = new MultiStringSelector (i18n ("R Library locations  (where libraries get installed to, locally)"), this);
	libloc_selector->setValues (liblocs);
	connect (libloc_selector, SIGNAL (listChanged ()), this, SLOT (listChanged ()));
	connect (libloc_selector, SIGNAL (getNewStrings (QStringList*)), this, SLOT (addLibLoc (QStringList*)));
	main_vbox->addWidget (libloc_selector);
	QLabel *label = new QLabel (i18n ("Note: The startup defaults will always be used in addition to the locations you specify in this list"), this);
	main_vbox->addWidget (label);

	main_vbox->addStretch ();
}

RKSettingsModuleRPackages::~RKSettingsModuleRPackages () {
	RK_TRACE (SETTINGS);
}

void RKSettingsModuleRPackages::listChanged () {
	RK_TRACE (SETTINGS);
	change ();
}

void RKSettingsModuleRPackages::boxChanged (int) {
	RK_TRACE (SETTINGS);
	change ();
}

void RKSettingsModuleRPackages::addLibLoc (QStringList *string_list) {
	RK_TRACE (SETTINGS);
	QString new_string = KFileDialog::getExistingDirectory (KUrl (), this, i18n ("Add R Library Directory"));
	if (!new_string.isEmpty ()) {
		(*string_list).append (new_string);
	}
}

void RKSettingsModuleRPackages::addRepository (QStringList *string_list) {
	RK_TRACE (SETTINGS);
	QString new_string = KInputDialog::getText (i18n ("Add repository"), i18n ("Add URL of new repository\n(Enter \"@CRAN@\" for the standard CRAN-mirror)"), QString::null, 0, this);
	(*string_list).append (new_string);
}

QString RKSettingsModuleRPackages::caption () {
	RK_TRACE (SETTINGS);
	return (i18n ("R-Packages"));
}

bool RKSettingsModuleRPackages::hasChanges () {
	RK_TRACE (SETTINGS);
	return changed;
}

//static
QStringList RKSettingsModuleRPackages::makeRRunTimeOptionCommands () {
	QStringList list;

// package repositories
	QString command = "options (repos=c (";
	for (QStringList::const_iterator it = package_repositories.begin (); it != package_repositories.end (); ++it) {
		if (it != package_repositories.begin ()) {
			command.append (", ");
		}
		if (*it == "@CRAN@") command.append ("CRAN=\"" + cran_mirror_url + "\""); 
		else command.append ("\"" + *it + "\"");
	}
	list.append (command + "))\n");

// library locations
	command = ".libPaths (unique (c (";
	bool first = true;
	for (QStringList::const_iterator it = liblocs.begin (); it != liblocs.end (); ++it) {
		if (first) first = false;
		else command.append (", ");
		command.append ("\"" + *it + "\"");
	}
	for (QStringList::const_iterator it = defaultliblocs.begin (); it != defaultliblocs.end (); ++it) {
		if (first) first = false;
		else command.append (", ");
		command.append ("\"" + *it + "\"");
	}
	command.append (")))");
	list.append (command);

	return list;
}

void RKSettingsModuleRPackages::applyChanges () {
	RK_TRACE (SETTINGS);

	cran_mirror_index = cran_mirrors->currentIndex ();
	cran_mirror_url = cran_url_list.at(cran_mirror_index).toLocal8Bit().constData();

	archive_packages = archive_packages_box->isChecked ();
	package_repositories = repository_selector->getValues ();
	liblocs = libloc_selector->getValues ();

// apply options in R
	QStringList commands = makeRRunTimeOptionCommands ();
	for (QStringList::const_iterator it = commands.begin (); it != commands.end (); ++it) {
		RKGlobals::rInterface ()->issueCommand (*it, RCommand::App, QString::null, 0, 0, commandChain ());
	}
}

void RKSettingsModuleRPackages::save (KConfig *config) {
	RK_TRACE (SETTINGS);

	saveSettings (config);
}

void RKSettingsModuleRPackages::saveSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("R Settings");
	cg.writeEntry ("CRAN mirror index", cran_mirror_index);
	cg.writeEntry ("archive packages", archive_packages);
	cg.writeEntry ("Repositories", package_repositories);
	cg.writeEntry ("LibraryLocations", liblocs);
}

void RKSettingsModuleRPackages::loadSettings (KConfig *config) {
	RK_TRACE (SETTINGS);

	KConfigGroup cg = config->group ("R Settings");
#warning BUG: loading the mirror index does not set options(repos) for the first run
	cran_mirror_index = cg.readEntry ("CRAN mirror index", 0);
	archive_packages = cg.readEntry ("archive packages", false);
	package_repositories = cg.readEntry ("Repositories", QStringList ("@CRAN@"));

	liblocs = cg.readEntry ("LibraryLocations", QStringList ());
}

#include "rksettingsmoduler.moc"
