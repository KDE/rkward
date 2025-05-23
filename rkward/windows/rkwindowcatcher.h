/*
rwindowcatcher.h - This file is part of the RKWard project. Created: Wed May 4 2005
SPDX-FileCopyrightText: 2005-2020 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKWINDOWCATCHER_H
#define RKWINDOWCATCHER_H

//#define DISABLE_RKWINDOWCATCHER
#ifndef DISABLE_RKWINDOWCATCHER

#	include <QMap>
#	include <QTimer>
#	include <qwidget.h>

#	include <netwm_def.h>
class RKMDIWindow;

/** This is a simple helper class helping in catching R X11 device windows. The start () and stop () functions are called from RInterface, and then this class takes care of handling those.

The main difficulty to overcome in this context, is to find out, when an R X11 window is created, and what is its X Window id. The notes below are some thoughts on that matter. Probably mostly obsolete, now (the current approach is basically Plan C, and seems to work ok), but maybe Plans A or B or something similar will become necessary some day:

Catch R X11 device windows
- Plan A:
    - initialization function seems to be in_do_X11
    - it might be possible to put a wrapper around this using R_setX11Routines
    - this wrapper could watch the list of devices (curDevice, numDevices), see also addDevice to find out how the list is kept internally
    - if a new device gets added grab its winId and capture
- Plan B:
    - it looks like there's no way to get access to R_setX11Routines or at least the needed struct R_X11Routines. (?)
    - the level above that seems to be do_X11
    - maybe we can modify the mapping from .Internal (X11) to do_X11 and insert wrapper from Plan A -> R_FunTab
    - proceed like in Plan A
    - less preferable as C-plugins might be able to call do_X11 directly (can they?)
- Plan C:
    - modify at R level (override X11 ())
        - notify app right before device is created
        - notify app right after device is created
    - least preferable solution as we can not be sure we catch every use.
        - but definitely most. This is dispatched via CurrentDevice ()->options("device"), and then evaluated in R_GlobalEnv
- remaining problem: how to get the window id given the device id?
    - http://tronche.com/gui/x/xlib/events/window-state-change/create.html#XCreateWindowEvent
    - for active / inactive: XPropertyEvent WM_NAME
    - we may catch this using KApplication::x11EventFilter
    - event filter should only be active during the wrapper (Plan A-C)
    - event filter should probably do some sanity checking
    - this should give us the window id corresponding to the x11-call

@author Thomas Friedrichsmeier
*/
class RKWindowCatcher : public QObject {
	Q_OBJECT
  public:
	/** ctor. Probably you'll only ever need one instance of RKWindowCatcher. */
	RKWindowCatcher();
	/** dtor */
	~RKWindowCatcher();

	/** call this function to start looking out for new R X11 device windows.
	@param prev_cur_device the device number that was active before a new device window was (potentially) created */
	void start(int prev_cur_device);
	/** end looking out for new R X11 windows. If a new window was in fact created, this is captured by creating an RKCaughtX11Window
	@param new_cur_device the new active device number, i.e. the device number of the created window */
	void stop(int new_cur_device);
	/** called from the R backend when the device history needs to be updated
	@param params the serialized parameters as supplied from R */
	void updateHistory(QStringList params);
	/** Kill an R device
	@param device_number R device number of the device to kil */
	void killDevice(int device_number);
	/** watch the given window for changes in its WM_NAME property (i.e. changes in caption), or deletion. When a change is detected, the watcher will be notified (setCaption() or deleteLater())
	    WARNING: Remember to call unregisterNameWatcher, when watcher is deleted! */
	void registerWatcher(WId watched, RKMDIWindow *watcher);
	/** remove a watch created with registerNameWatcher */
	void unregisterWatcher(WId watched);
	static RKWindowCatcher *instance();
	static void discardInstance() {
		delete _instance;
		_instance = nullptr;
	};

  private:
	void pollWatchedWindowStates();
	QTimer poll_timer;
	WId createdWindow();
	int last_cur_device;
	QList<WId> windows_before_add;
	static RKWindowCatcher *_instance;
	QMap<WId, RKMDIWindow *> watchers_list;
};

#	include "rkmdiwindow.h"

#	include <QHash>

class RKCaughtX11WindowPart;
class KToggleAction;
class QAction;
class KSelectAction;
class QXEmbedCopy;
class QScrollArea;
class RKProgressControl;
class QX11EmbedContainer;
class QWinHost;
class RKGraphicsDevice;

/** An R onscreen graphics device window managed by rkward. Currently, this can be X11 devices (on X11), Windows devices (on Windows), and
 RK devices (anywhere). */
class RKCaughtX11Window : public RKMDIWindow {
	Q_OBJECT
  public:
	/** ctor
	@param window_to_embed the Window id of the R X11 device window to embed
	@param device_number the device number corresponding to that window */
	RKCaughtX11Window(QWindow *window_to_embed, int device_number);
	RKCaughtX11Window(RKGraphicsDevice *rkward_device, int device_number);
	/** dtor */
	~RKCaughtX11Window() override;

	/** reimplemented from RKMDIWindow to switch to fixed size mode, and disable the dynamic_size_action */
	void prepareToBeAttached() override;
	/** see prepareToBeAttached (). Reenable the dynamic_size_action */
	void prepareToBeDetached() override;
	/** returns the window corresponding the to given R device number (or 0 if no such window exists) */
	static RKCaughtX11Window *getWindow(int device_number) { return device_windows.value(device_number); };
	void updateHistoryActions(int history_length, int position, const QStringList &labels);
  public Q_SLOTS:
	void deviceInteractive(bool interactive, const QString &prompt);

	/** Fixed size action was (potentially) toggled. Update to the new state */
	void fixedSizeToggled();
	/** Switch to fixed size mode, and set size1 (currently 500*500) */
	void setFixedSize1();
	/** Switch to fixed size mode, and set size2 (currently 1000*1000) */
	void setFixedSize2();
	/** Switch to fixed size mode, and set size3 (currently 2000*2000) */
	void setFixedSize3();
	/** Switch to fixed size mode, and set user specified size (size read from a dialog) */
	void setFixedSizeManual();

	void activateDevice();
	void copyDeviceToOutput();
	void printDevice();
	void copyDeviceToRObject();
	void duplicateDevice();
	void stopInteraction();

	/** history navigation */
	void firstPlot();
	void previousPlot();
	void nextPlot();
	void lastPlot();
	void gotoPlot(int index);
	void forceAppendCurrentPlot();
	void removeCurrentPlot();
	void clearHistory();
	void showPlotInfo();

	/** reimplemented to keep window alive while saving history */
	bool close(RKMDIWindow::CloseWindowMode ask_save) override;
	void setKilledInR() { killed_in_r = true; };
	void setWindowStyleHint(const QString &hint) override;
  private Q_SLOTS:
	void doEmbed();

  private:
	void forceClose();
	void commonInit(int device_number);
	void commonClose(bool in_destructor);
	friend class RKCaughtX11WindowPart; // needs access to the actions
	int device_number;
	bool killed_in_r;
	bool close_attempted;
	QWidget *xembed_container;
	QScrollArea *scroll_widget;
	RKProgressControl *error_dialog;

	static QHash<int, RKCaughtX11Window *> device_windows;
	QWindow *embedded;
	bool embedding_complete;
	QWidget *capture; // The captured window (0, if using an rk native device)
	RKGraphicsDevice *rk_native_device;

	bool dynamicSize() const;
	KToggleAction *dynamic_size_action;
	QAction *plot_prev_action;
	QAction *plot_next_action;
	QAction *plot_first_action;
	QAction *plot_last_action;
	QAction *plot_force_append_action;
	QAction *plot_remove_action;
	QAction *plot_clear_history_action;
	QAction *plot_properties_action;
	KSelectAction *plot_list_action;
	QAction *stop_interaction;

	QList<QAction *> actions_not_for_preview;
	int history_length;
	int history_position;
};

/** Provides a KPart interface for RKCaughtX11Window. */
class RKCaughtX11WindowPart : public KParts::Part {
  public:
	/** constructor.
	@param window The RKCatehdX11Window for this part */
	explicit RKCaughtX11WindowPart(RKCaughtX11Window *window);
	/** destructor */
	~RKCaughtX11WindowPart();

  private:
	RKCaughtX11Window *window;
};

#endif // DISABLE_RKWINDOWCATCHER
#endif
