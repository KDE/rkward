/***************************************************************************
                          rkmodificationtracker  -  description
                             -------------------
    begin                : Tue Aug 31 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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
#include "rkmodificationtracker.h"

#include <kmessagebox.h>
#include <klocale.h>

#include "../rkglobals.h"
#include "../rkeditormanager.h"
#include "../dataeditor/rkeditor.h"
#include "rcontainerobject.h"

#include "../debug.h"

RKModificationTracker::RKModificationTracker (QObject *parent) : QObject (parent) {
	RK_TRACE (OBJECTS);
}


RKModificationTracker::~RKModificationTracker () {
	RK_TRACE (OBJECTS);
}

void RKModificationTracker::removeObject (RObject *object, RKEditor *editor, bool removed_in_workspace) {
	RK_TRACE (OBJECTS);
// TODO: allow more than one editor per object
	RKEditor *ed = object->objectOpened ();
	RK_ASSERT (!((editor) && (!ed)));
	
	if (removed_in_workspace) {
		if (ed) {
			if (KMessageBox::questionYesNo (0, i18n ("The object '") + object->getFullName () + i18n ("' was removed in the R workspace, but is currently opened for editing. Do you want to restore the object?"), i18n ("Restore object?")) == KMessageBox::Yes) {
				if (removed_in_workspace) ed->restoreObject (object);
				return;
			}
		}
	} else {
		if (editor || ed) {
			if (KMessageBox::questionYesNo (0, i18n ("Do you really want to remove the object '") + object->getFullName () + i18n ("'? The object is currently opened for editing, it will be removed in the editor, too. There's no way to get it back."), i18n ("Remove object?")) != KMessageBox::Yes) {
				return;
			}
		} else {
			// TODO: check for other editors editing this object
			if (KMessageBox::questionYesNo (0, i18n ("Do you really want to remove the object '") + object->getFullName () + i18n ("'? There's no way to get it back."), i18n ("Remove object?")) != KMessageBox::Yes) {
				return;
			}
		}
	}
	
	if (ed) ed->removeObject (object);
	emit (objectRemoved (object));
	object->remove ();
}

void RKModificationTracker::renameObject (RObject *object, const QString &new_name) {
	RK_TRACE (OBJECTS);
// TODO: allow more than one editor per object
// TODO: find out, whether new object-name is valid
	RKEditor *ed = object->objectOpened ();

	object->rename (new_name);

// since we may end up with a different name that originally requested, we propagate the change also to the original editor
	if (ed) ed->renameObject (object);
	emit (objectPropertiesChanged (object));
}

void RKModificationTracker::addObject (RObject *object, RKEditor *editor) {
	RK_TRACE (OBJECTS);
// TODO: allow more than one editor per object
	RKEditor *ed = 0;
	if (object->getContainer ()) ed = object->getContainer ()->objectOpened ();
	RK_ASSERT (!((editor) && (!ed)));
	
	if (ed) {
		if (ed != editor) {
			ed->addObject (object);
		}
	}
	emit (objectAdded (object));
}

void RKModificationTracker::objectMetaChanged (RObject *object) {
	RK_TRACE (OBJECTS);
// TODO: allow more than one editor per object
	RKEditor *ed = object->objectOpened ();
	
	if (ed) {
		ed->updateObjectMeta (object);
	}
	emit (objectPropertiesChanged (object));
}

void RKModificationTracker::objectDataChanged (RObject *object, RObject::ChangeSet *changes) {
	RK_TRACE (OBJECTS);
// TODO: allow more than one editor per object
	RKEditor *ed = object->objectOpened ();

	if (ed) {
		ed->updateObjectData (object, changes);
	}

	delete changes;
}

#include "rkmodificationtracker.moc"
