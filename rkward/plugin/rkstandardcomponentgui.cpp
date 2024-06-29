/*
rkstandardcomponentgui - This file is part of RKWard (https://rkward.kde.org). Created: Sun Mar 19 2006
SPDX-FileCopyrightText: 2006-2012 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkstandardcomponentgui.h"

#include <KLocalizedString>
#include <kactioncollection.h>

#include <QApplication>
#include <QTimer>
#include <QPushButton>
#include <QLabel>
#include <QCloseEvent>
#include <QCheckBox>
#include <QSplitter>
#include <QHBoxLayout>
#include <QToolButton>
#include <QAction>
#include <QUrl>

#include "rkcomponentmap.h"
#include "../misc/rkcommonfunctions.h"
#include "../misc/rkstandardicons.h"
#include "../misc/rkxmlguipreviewarea.h"
#include "../misc/rkstyle.h"
#include "../windows/rkworkplace.h"
#include "../windows/rkcommandeditorwindow.h"
#include "../rbackend/rkrinterface.h"
#include "../rkward.h"
#include "../settings/rksettingsmoduleplugins.h"

#include "../debug.h"

class RKExtensionSplitter : public QSplitter {
public:
	explicit RKExtensionSplitter (QWidget *window, QWidget *parent, QWidget *main_area, QWidget *extension_area) : QSplitter (parent) {
		RK_TRACE (PLUGIN);
		RKExtensionSplitter::window = window;
		extension = extension_area;
		addWidget (main_area);          // Right now, this class supports only left -> right and top -> bottom layout
		addWidget (extension_area);
		setStretchFactor (0, 0);        // When resizing the window, *and* the extension (preview) is visible, effectively resize the extension. Dialog area can be resized via splitter.
		setStretchFactor (1, 1);
		setChildrenCollapsible (false); // It's just too difficult to make this consistent, esp. for shrinking the dialog would _also_ be expected to collapse widgets. Besides, this makes it
	                                    // easier to keep track of which expansions are currently visible.
	}

	void setExtensionVisible (bool new_visible) {
		RK_TRACE (PLUGIN);
		if (extension->isVisible () == new_visible) return;

		int size_change = 0;
		QList<int> sizes = QSplitter::sizes ();
		if (new_visible) {
			int s = defaultExtensionSize ();
			if (s < 80) s = 80;
			size_change = s;
			extension->show ();
			// HACK, see below
			window->setMinimumSize (QSize (0, 0));
			sizes[1] = s;
		} else {
			saveSize ();
			int s = sizes[1];
			size_change = -s;
			extension->hide ();
			// HACK: I don't understand why, but somehow with nested splitters, the minimum size is calculated _as if_ the extension was still visible.
			//       this results in the resize(), below to result in a larger than expected size. The dialog does not shrink back to it's original size. Qt 4.8.5.
			window->setMinimumSize (widget (0)->minimumSizeHint ());
			sizes[1] = 0;
		}
		setSizes (sizes);

		if (QSplitter::window ()->isVisible ()) {
			QRect boundary = screen() ? screen()->availableGeometry() : QApplication::primaryScreen()->availableGeometry();
			int new_width = window->width ();
			int new_height = window->height ();
			int new_x = window->x ();
			int new_y = window->y ();
			if (orientation () == Qt::Horizontal) {
				new_width = qMin (boundary.width (), new_width + size_change);      // no wider than screen
				if (new_width + new_x > boundary.right ()) {                        // don't leave screen to the right
					new_x = boundary.right () - new_width;
				}
			} else {
				new_height = qMin (boundary.height (), new_height + size_change);   // see above
				if (new_height + new_y > boundary.bottom ()) {
					new_y = boundary.bottom () - new_height;
				}
			}
			// Ok, I can't find a way to make resize+move work with a single operation. Doing it in two operations carries the danger that the WM will interfere, though, so
			// the order is shrink+move for hiding the preview pane, but move+grow for showing the preview pane.
			if (new_visible) window->move (new_x, new_y);
			window->resize (new_width, new_height);
			if (!new_visible) window->move (new_x, new_y);
		}
	};

	void saveSize() {
		if (extension->isVisible()) {
			if (orientation() == Qt::Horizontal) RKSettingsModulePlugins::setDefaultSidePreviewWidth(sizes().at(1));
			else RKSettingsModulePlugins::setDefaultCodeHeight(sizes().at(1));
		}
	}

	int defaultExtensionSize () {
		return ((orientation () == Qt::Horizontal) ? RKSettingsModulePlugins::defaultSidePreviewWidth () : RKSettingsModulePlugins::defaultCodeHeight ());
	}

	bool isExtensionVisible () const { return extension->isVisible (); };
private:
	QWidget *window;
	QWidget *extension;
};

/////////////////////////////////////// RKStandardComponentGUI ////////////////////////////////////////////////

RKStandardComponentGUI::RKStandardComponentGUI (RKStandardComponent *component, RKComponentPropertyCode *code_property, bool enslaved) :
	code_display_visibility (this, false, false)
{
	RK_TRACE (PLUGIN);

	toggle_code_box = nullptr;
	hsplitter = nullptr;
	vsplitter = nullptr;
	code_display = nullptr;

	RKStandardComponentGUI::component = component;
	RKStandardComponentGUI::code_property = code_property;
	connect (code_property, &RKComponentPropertyBase::valueChanged, this, &RKStandardComponentGUI::codeChanged);
	connect (RKWardMainWindow::getMain(), &RKWardMainWindow::aboutToQuitRKWard, this, &RKStandardComponentGUI::cancel);

	RKStandardComponentGUI::enslaved = enslaved;

	// code update timer
	code_update_timer = new QTimer (this);
	code_update_timer->setSingleShot (true);
	connect (code_update_timer, &QTimer::timeout, this, &RKStandardComponentGUI::updateCodeNow);

	if (!enslaved) {
		// code display
		RKXMLGUIPreviewArea *area = addDockedPreview(&code_display_visibility, i18n("Code Preview"), nullptr, true);
		code_display = new RKCommandEditorWindow(nullptr, QUrl(), QString(), RKCommandEditorFlags::DefaultToRHighlighting);
		code_display->setReadOnly (true);
		code_display_visibility.setBoolValue (!enslaved && RKSettingsModulePlugins::showCodeByDefault ());
		area->setWindow(code_display);

		KActionCollection *action_collection = new KActionCollection (this);
		action_collection->addAction(KStandardAction::Copy, "copy", this, &RKStandardComponentGUI::copyCode);
	}
}

RKStandardComponentGUI::~RKStandardComponentGUI () {
	RK_TRACE (PLUGIN);
}

void RKStandardComponentGUI::createDialog (bool switchable) {
	RK_TRACE (PLUGIN);

	QWidget *central_widget = new QWidget ();
	QHBoxLayout *hbox = new QHBoxLayout (central_widget);

	// build standard elements
	main_widget = new QWidget (central_widget);
	QVBoxLayout* main_widget_layout = new QVBoxLayout (main_widget);
	main_widget_layout->setContentsMargins(0, 0, 0, 0);
	hbox->addWidget (main_widget);

	// lines
	QFrame *line = new QFrame (central_widget);
	line->setFrameShape (QFrame::VLine);
	line->setFrameShadow (QFrame::Plain);	
	hbox->addWidget (line);

	// buttons
	QVBoxLayout* vbox = new QVBoxLayout ();
	hbox->addLayout (vbox);
	vbox->setContentsMargins (0, 0, 0, 0);
	vbox->setSpacing (RKStyle::spacingHint ());
	ok_button = new QPushButton (i18n ("Submit"), central_widget);
	connect (ok_button, &QPushButton::clicked, this, &RKStandardComponentGUI::ok);
	vbox->addWidget (ok_button);
	if (enslaved) ok_button->hide ();

	cancel_button = new QPushButton (i18n ("Close"), central_widget);
	connect (cancel_button, &QPushButton::clicked, this, &RKStandardComponentGUI::cancel);
	vbox->addWidget (cancel_button);
	auto_close_box = new QCheckBox (i18n ("Auto close"), central_widget);
	auto_close_box->setChecked (true);
	vbox->addWidget (auto_close_box);
	if (enslaved) auto_close_box->hide ();
	vbox->addStretch (1);
	
	help_button = new QPushButton (i18n ("Help"), central_widget);
	help_button->setEnabled (component->haveHelp ());
	connect (help_button, &QPushButton::clicked, this, &RKStandardComponentGUI::help);
	vbox->addWidget (help_button);
	
	if (switchable && (!enslaved)) {
		switch_button = new QPushButton (i18n ("Use Wizard"), central_widget);
		connect (switch_button, &QPushButton::clicked, this, &RKStandardComponentGUI::switchInterface);
		vbox->addWidget (switch_button);
	}
	vbox->addStretch (2);

	custom_preview_buttons_area = new QWidget ();
	QVBoxLayout *dummy = new QVBoxLayout (custom_preview_buttons_area);
	dummy->setContentsMargins (0, 0, 0, 0);
	vbox->addWidget (custom_preview_buttons_area);

	toggle_code_box = new QCheckBox (i18n ("Code Preview"), central_widget);
	toggle_code_box->setChecked (code_display_visibility.boolValue ());
	connect (toggle_code_box, &QCheckBox::clicked, this, &RKStandardComponentGUI::toggleCode);
	vbox->addWidget (toggle_code_box);
	if (enslaved) toggle_code_box->hide ();

	hpreview_area = new QSplitter (this);
	hpreview_area->setOrientation (Qt::Vertical);
	hpreview_area->setChildrenCollapsible (false);
	vpreview_area = new QWidget (this);
	new QVBoxLayout (vpreview_area);

	hsplitter = new RKExtensionSplitter (this, this, central_widget, hpreview_area);
	hsplitter->setOrientation (Qt::Horizontal);
	vsplitter = new RKExtensionSplitter (this, this, hsplitter, vpreview_area);
	vsplitter->setOrientation (Qt::Vertical);

	QVBoxLayout *main_vbox = new QVBoxLayout (this);
	main_vbox->setContentsMargins (0, 0, 0, 0);
	main_vbox->addWidget (vsplitter);
}

void RKStandardComponentGUI::finalize () {
	RK_TRACE (PLUGIN);

	bool any_hpreview_visible = false;
	bool any_vpreview_visible = RKSettingsModulePlugins::showCodeByDefault ();
	for (int i = 0; i < previews.size (); ++i) {
		// Add preview to splitter. Also add a title bar to each preview.
		previews[i].widget = previews[i].preview_area;
		connect (previews[i].preview_area, &RKXMLGUIPreviewArea::previewClosed, this, &RKStandardComponentGUI::previewCloseButtonClicked);
		connect (previews[i].controller, &RKComponentPropertyBase::valueChanged, this, &RKStandardComponentGUI::previewVisibilityChanged);
		if (!(previews[i].controller->boolValue ())) previews[i].widget->hide ();
		else {
			if (previews[i].position == Qt::Horizontal) any_hpreview_visible = true;
			else any_vpreview_visible = true;
		}
		if (previews[i].position == Qt::Horizontal) hpreview_area->insertWidget (hpreview_area->count () - 1, previews[i].widget);
		else vpreview_area->layout ()->addWidget (previews[i].widget);
	}

	if (any_hpreview_visible) {
		hpreview_area->setMinimumWidth (hsplitter->defaultExtensionSize ());  // enforce minimum, here to achieve sane size on show. Will be cleared directly after show.
	} else {
		hpreview_area->hide ();
	}
	if (any_vpreview_visible) {
		vpreview_area->setMinimumHeight (vsplitter->defaultExtensionSize ());  // see above
	} else {
		vpreview_area->hide ();
	}

	// (With Qt 5.4.1) it seems *very* important to call this before the first show. Otherwise we will see strange placement issues, esp. in multi-monitor setup.
	// Yes, that's right. placement, not initial size. For whatever reason.
	main_widget->updateGeometry ();
}

RKXMLGUIPreviewArea* RKStandardComponentGUI::addDockedPreview(RKComponentPropertyBool *controller, const QString& label, RKPreviewManager *manager, bool bottom) {
	RK_TRACE (PLUGIN);

	RKXMLGUIPreviewArea *area = new RKXMLGUIPreviewArea(label, nullptr, manager);
	PreviewArea parea;
	parea.preview_area = area;
	parea.widget = area;   // may be replaced by a wrapper in "finalize"
	parea.controller = controller;
	parea.position = bottom ? Qt::Vertical : Qt::Horizontal;
	previews.insert (0, parea);

	if (manager) {
		RKWorkplace::mainWorkplace()->registerNamedWindow(manager->previewId(), this, area);
	}
	return area;
};

void RKStandardComponentGUI::showEvent (QShowEvent *e) {
	RK_TRACE (PLUGIN);

	QWidget::showEvent (e);

	// HACK: Workaround for this issue (caused by a mysterious Qt bug, apparently): https://mail.kde.org/pipermail/rkward-devel/2011-June/002831.html
	QSize min = minimumSize ();
	if ((min.width () < 50) || (min.height () < 50)) min = sizeHint ();
	setMinimumSize (min.expandedTo (QSize (50, 50)));

	if (toggle_code_box) {	// this is a dialog, not  wizard
		QTimer::singleShot(0, this, &RKStandardComponentGUI::doPostShowCleanup);
	}
}

void RKStandardComponentGUI::ok () {
	RK_TRACE (PLUGIN);

	RK_ASSERT (code_property->isValid ());

	QString command = "local({\n";
	command.append (code_property->preprocess ());
	command.append (code_property->calculate ());
	command.append (code_property->printout ());
	command.append ("})\n");
	RInterface::issueCommand (new RCommand (command, RCommand::Plugin | RCommand::CCOutput | RCommand::ObjectListUpdate), component->commandChain ());

	// re-run link
	// This should be run in a separate command, in case the above command bails out with an error. Even in that case, the re-run link should be printed.
	command.clear ();
	RKComponentHandle *handle = RKComponentMap::getComponentHandle (component->getId ());
	if (handle && handle->isAccessible ()) {
		RKComponent::PropertyValueMap map;
		component->serializeState (&map);
		command.append (".rk.rerun.plugin.link(plugin=\"" + component->getId () + "\", settings=\"" + RKCommonFunctions::escape (RKComponent::valueMapToString (map)) + "\", label=\"" + i18n ("Run again") + "\")\n");
		// NOTE: the serialized state is quote-escape *again* for passing to R.
	}
	// separator line
	command.append (".rk.make.hr()\n");
	RInterface::issueCommand (new RCommand (command, RCommand::Plugin | RCommand::ObjectListUpdate | RCommand::Silent), component->commandChain ());

	if (auto_close_box->isChecked ()) cancel ();
}

void RKStandardComponentGUI::cancel () {
	RK_TRACE (PLUGIN);

	if (!enslaved && toggle_code_box && hsplitter) {  // A top-level dialog-style UI. Save state of preview areas
		RKSettingsModulePlugins::setShowCodeByDefault (code_display_visibility.boolValue ());
		hsplitter->saveSize ();
		vsplitter->saveSize ();
	}
	hide ();
	if (!enslaved) {
		component->deleteLater ();
	}
}

void RKStandardComponentGUI::doPostShowCleanup () {
	RK_TRACE (PLUGIN);

	if (toggle_code_box) {  // is Dialog
		hpreview_area->setMinimumWidth (80);
		vpreview_area->setMinimumHeight (40);
		previewVisibilityChanged(nullptr);
	}
}


void RKStandardComponentGUI::toggleCode () {
	RK_TRACE (PLUGIN);
	RK_ASSERT (toggle_code_box);

	if (code_display_visibility.boolValue () != toggle_code_box->isChecked ()) {
		code_display_visibility.setBoolValue (toggle_code_box->isChecked ());
	}
	updateCode ();
}

void RKStandardComponentGUI::previewCloseButtonClicked (RKXMLGUIPreviewArea *area) {
	RK_TRACE (PLUGIN);

	RK_ASSERT (hsplitter);  // is a dialog
	for (int i = 0; i < previews.size (); ++i) {
		if (area == previews[i].preview_area) {
			previews[i].controller->setBoolValue (false);
			if (i == previews.size () - 1) toggle_code_box->setChecked (false);
			return;
		}
	}

	RK_ASSERT (false);
}

void RKStandardComponentGUI::previewVisibilityChanged (RKComponentPropertyBase*) {
	RK_TRACE (PLUGIN);

	if (!isVisible ()) return;
	if (!hsplitter) return;

	bool new_h_visible = false;
	bool new_v_visible = false;
	// which previews are active?
	for (int i = 0; i < previews.size (); ++i) {
		previews[i].widget->setVisible (previews[i].controller->boolValue ());
		if (previews[i].controller->boolValue ()) {
			if (previews[i].position == Qt::Horizontal) new_h_visible = true;
			else new_v_visible = true;
		}
	}
	hsplitter->setExtensionVisible (new_h_visible);
	vsplitter->setExtensionVisible (new_v_visible);
}

void RKStandardComponentGUI::copyCode () {
	RK_TRACE (PLUGIN);

	code_display->copy ();
}

void RKStandardComponentGUI::help () {
	RK_TRACE (PLUGIN);

	QString path = component->getId ().split ("::").join ("/");
	RKWorkplace::mainWorkplace ()->openHelpWindow (QUrl ("rkward://component/" + path));
}

void RKStandardComponentGUI::closeEvent (QCloseEvent *e) {
	RK_TRACE (PLUGIN);

	e->accept ();
	cancel ();
}

void RKStandardComponentGUI::enableSubmit (bool enable) {
	RK_TRACE (PLUGIN);

	ok_button->setEnabled (enable);
}

void RKStandardComponentGUI::codeChanged (RKComponentPropertyBase *) {
	RK_TRACE (PLUGIN);

	updateCode ();
}

void RKStandardComponentGUI::updateCode () {
	RK_TRACE (PLUGIN);

	if (!code_display_visibility.boolValue ()) return;
	code_update_timer->start (0);
}

void RKStandardComponentGUI::updateCodeNow () {
	RK_TRACE (PLUGIN);
	static bool was_valid = false;
	if (was_valid) code_display->saveScrollPosition ();

	if (!code_property->isValid ()) {
		code_display->setText (i18n ("Processing. Please wait"));
		RK_DEBUG (PLUGIN, DL_DEBUG, "code not ready to be displayed: pre %d, cal %d, pri %d", !code_property->preprocess ().isNull (), !code_property->calculate ().isNull (), !code_property->printout ().isNull ());
		was_valid = false;
	} else {
		code_display->setText ("local({\n" + code_property->preprocess () + code_property->calculate () + code_property->printout () + "})\n");
		code_display->restoreScrollPosition ();
		was_valid = true;
	}
}

///////////////////////////////// RKStandardComponentWizard /////////////////////////////////

RKStandardComponentWizard::RKStandardComponentWizard (RKStandardComponent *component, RKComponentPropertyCode *code_property, bool enslaved) : RKStandardComponentGUI (component, code_property, enslaved) {
	RK_TRACE (PLUGIN);

	submit_enabled = false;
	is_switchable = false;
}

RKStandardComponentWizard::~RKStandardComponentWizard () {
	RK_TRACE (PLUGIN);
}

void RKStandardComponentWizard::updateCode () {
	RK_TRACE (PLUGIN);

	updateState ();
}

void RKStandardComponentWizard::createWizard (bool switchable) {
	RK_TRACE (PLUGIN);

	is_switchable = switchable;
	// create main layout and stack
	QGridLayout *main_grid = new QGridLayout (this);
	main_widget = stack = new RKStandardComponentStack (this);
	main_grid->addWidget (stack, 0, 0, 1, 4);

	// build standard elements
	// lines
	QFrame *line = new QFrame (main_widget);
	line->setFrameShape (QFrame::HLine);
	line->setFrameShadow (QFrame::Plain);
	main_grid->addWidget (line, 1, 0, 1, 4);

	// buttons
	cancel_button = new QPushButton (i18n ("Cancel"), this);
	main_grid->addWidget (cancel_button, 2, 0, Qt::AlignLeft);
	if (enslaved) cancel_button->hide ();
	help_button = new QPushButton (i18n ("Help"), this);
	main_grid->addWidget (help_button, 2, 1, Qt::AlignLeft);
	prev_button = new QPushButton (QString (), this);
	prev_button->setEnabled (false);
	main_grid->addWidget (prev_button, 2, 2, Qt::AlignRight);
	next_button = new QPushButton (QString (), this);
	main_grid->addWidget (next_button, 2, 3, Qt::AlignRight);
	connect (next_button, &QPushButton::clicked, this, &RKStandardComponentWizard::next);
	connect (prev_button, &QPushButton::clicked, this, &RKStandardComponentWizard::prev);
	connect (cancel_button, &QPushButton::clicked, this, &RKStandardComponentWizard::cancel);
	connect (help_button, &QPushButton::clicked, this, &RKStandardComponentWizard::help);

	// dummy:
	auto_close_box = new QCheckBox(this);
	auto_close_box->setChecked (true);
	auto_close_box->hide ();
}

void RKStandardComponentWizard::finalize () {
	RK_TRACE (PLUGIN);

	if (!enslaved) {
		// build the last page
		RKComponent *last_page = stack->addPage (component);
		QVBoxLayout *vbox = new QVBoxLayout (last_page);
		vbox->setContentsMargins (0, 0, 0, 0);
		if (previews.size () < 2) {
			RK_ASSERT (previews.size () == 1);
			vbox->addWidget (RKCommonFunctions::wordWrappedLabel (i18n ("Below you can preview the R commands corresponding to the settings you made. Click 'Submit' to run the commands.")));
			vbox->addWidget (code_display);
		} else {
			vbox->addWidget (RKCommonFunctions::wordWrappedLabel (i18n ("Below you can preview the result of your settings, and the R commands to be run. Click 'Submit' to run the commands.")));
			QTabWidget *previews_widget = new QTabWidget (last_page);
			vbox->addWidget (previews_widget);
			for (int i = 0; i < previews.size (); ++i) {
				previews_widget->addTab (previews[i].widget, previews[i].preview_area->label ());
			}
		}
	}

	stack->goToFirstPage ();
	updateState ();
}

void RKStandardComponentWizard::next () {
	RK_TRACE (PLUGIN);

	RK_ASSERT (stack->currentPageSatisfied ());

	if (stack->havePage (true)) {
		stack->movePage (true);
		updateState ();
	} else {
		if (enslaved) cancel ();
		else ok ();
	}
}

void RKStandardComponentWizard::prev () {
	RK_TRACE (PLUGIN);

	if (enslaved) {
		if (stack->havePage (false)) {
			stack->movePage (false);
			updateState ();
		} else {
			cancel ();
		}
	} else {
		if (stack->havePage (false)) {
			stack->movePage (false);
			updateState ();
		} else if (is_switchable) {
			switchInterface ();
		} else {
			RK_ASSERT (false);
		}
	}
}

void RKStandardComponentWizard::updateState () {
	RK_TRACE (PLUGIN);

	if (enslaved) {
		if (stack->havePage (true)) {		// not on last page
			next_button->setText (i18n ("Next >"));
			next_button->setEnabled (stack->currentPageSatisfied ());
		} else {			// on last page
			// do code update when on last page
			next_button->setText (i18n ("Done"));
			next_button->setEnabled (stack->currentPageSatisfied ());
		}

		prev_button->setEnabled (true);
		if (stack->havePage (false)) {		// not on first page
			prev_button->setText (i18n ("< Back"));
		} else {
			prev_button->setText (i18n ("Close"));
		}
		return;
	}

	if (stack->havePage (true)) {		// not on last page
		next_button->setText (i18n ("Next >"));
		next_button->setEnabled (stack->currentPageSatisfied ());
	} else {			// on last page
		// do code update when on last page
		if (!stack->havePage (true)) code_update_timer->start (0);
		next_button->setText (i18n ("Submit"));
		next_button->setEnabled (submit_enabled);
	}

	if (stack->havePage (false) || (!is_switchable)) {		// not on first page
		prev_button->setText (i18n ("< Back"));
		prev_button->setEnabled (stack->havePage (false));
	} else {
		prev_button->setText (i18n ("Use Dialog"));
		prev_button->setEnabled (true);
	}
}

void RKStandardComponentWizard::enableSubmit (bool enable) {
	RK_TRACE (PLUGIN);

	submit_enabled = enable;
}



//////////////////////////////// RKStandardComponentStack ////////////////////////////////////

RKStandardComponentStack::RKStandardComponentStack (QWidget *parent) : QStackedWidget (parent) {
	RK_TRACE (PLUGIN);
	// We want minimumExpanding, intially, as it makes better use of available space when first showing the window ...
	setSizePolicy (QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

RKStandardComponentStack::~RKStandardComponentStack () {
	RK_TRACE (PLUGIN);

	for (int i = 0; i < pages.count (); ++i) {
		delete (pages[i]);
	}
}

bool RKStandardComponentStack::havePage (bool next) {
	RK_TRACE (PLUGIN);

	if (next) {
		return (nextVisiblePage () <= (count () - 1));
	} else {
		return (previousVisiblePage () >= 0);
	}
}

void RKStandardComponentStack::movePage (bool next) {
	RK_TRACE (PLUGIN);

	int id;
	if (next) {
		id = nextVisiblePage ();
		if (id < (count() - 1)) {
			// ... but on the last (preview) page, minimumExpanding causes nasty layout fluctuation (https://bugs.kde.org/show_bug.cgi?id=425885)
			// Not quite sure, where the bug is at, here, but this papers over the worst symptoms.
			setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);
		}
	} else {
		id = previousVisiblePage ();
	}

	if (id < 0) {
		RK_ASSERT (false);
		return;
	}

	setCurrentIndex (id);
}

bool RKStandardComponentStack::currentPageSatisfied () {
	RK_TRACE (PLUGIN);

	PageDef* current_page = pages.value (currentIndex ());
	if (!current_page) {
		RK_ASSERT (false);
		return false;
	}

	for (int i = 0; i < current_page->page_components.size (); ++i) {
		if (!((current_page->page_components[i])->isSatisfied ())) {
			return false;
		}
	}

	return true;
}

void RKStandardComponentStack::goToFirstPage () {
	RK_TRACE (PLUGIN);

	setCurrentIndex (0);
}

RKComponent *RKStandardComponentStack::addPage (RKComponent *parent) {
	RK_TRACE (PLUGIN);

	PageDef *def = new PageDef;
	QWidget *wrap = new QWidget ();
	new QVBoxLayout (wrap);
	def->page = new RKComponent (parent, wrap);
	wrap->layout ()->addWidget (def->page);
	setCurrentIndex (addWidget (wrap));
	pages.append (def);

	return def->page;
}

void RKStandardComponentStack::addComponentToCurrentPage (RKComponent *component) {
	RK_TRACE (PLUGIN);

	PageDef* current_page = pages.value (currentIndex ());
	if (!current_page) {
		RK_ASSERT (false);
		return;
	}

	current_page->page_components.append (component);
}

int RKStandardComponentStack::previousVisiblePage () {
	RK_TRACE (PLUGIN);

	int prev_page = currentIndex () - 1;
	while (prev_page >= 0) {
		if (pages[prev_page]->page->visibilityProperty ()->boolValue ()) return prev_page;
		--prev_page;
	}

	return prev_page;
}

int RKStandardComponentStack::nextVisiblePage () {
	RK_TRACE (PLUGIN);

	int next_page = currentIndex () + 1;
	while (next_page <= (count ()-1)) {
		if (pages[next_page]->page->visibilityProperty ()->boolValue ()) return next_page;
		++next_page;
	}

	return next_page;
}

