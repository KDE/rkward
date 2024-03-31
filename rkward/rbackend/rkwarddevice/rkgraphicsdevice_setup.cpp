/*
rkgraphicsdevice_setup - This file is part of RKWard (https://rkward.kde.org). Created: Mon Mar 18 2013
SPDX-FileCopyrightText: 2013-2021 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

/******************************* ACKNOWLEDGEMENT ***************************
 * 
 * Much of the code in this file is based on, or even copied from package qtutils, version 0.1-3
 * by Deepayan Sarkar. Package qtutils is available from http://qtinterfaces.r-forge.r-project.org
 * under GNU LPGL 2 or later.
 * 
 ***************************************************************************/

#include "../rkrsupport.h"
#include "../rkrbackend.h"

#ifdef TRUE
#	undef TRUE
#endif
#ifdef FALSE
#	undef FALSE
#endif
#define R_USE_PROTOTPYES 1

#include <R_ext/GraphicsEngine.h>
#include <Rversion.h>

// rcolor typedef added in R 3.0.0
#ifndef rcolor
#define rcolor unsigned int
#endif

struct RKGraphicsDeviceDesc {
	bool init (pDevDesc dev, double pointsize, const QStringList &family, rcolor bg);
	int devnum;
	quint32 id;
	double width, height;
	int dpix, dpiy;
	QString getFontFamily (bool symbolfont) const {
		if (symbolfont) return default_symbol_family;
		return default_family;
	}
	QString default_family;
	QString default_symbol_family;
	pDevDesc rdevdesc;
};

#include "rkgraphicsdevice_stubs.cpp"

// No, I do not really understand what this is for.
// Mostly trying to mimick the X11 device's behavior, here.
#define RKGD_DPI 72.0

void RKStartGraphicsDevice (double width, double height, double pointsize, const QStringList &family, rcolor bg, const char* title, bool antialias) {
	static quint32 id = 0;
	if (width <= 0 || height <= 0) {
		Rf_error ("Invalid width or height: (%g, %g)", width, height);
	}
	RKGraphicsDeviceDesc *desc = new RKGraphicsDeviceDesc;
	desc->width = width;
	desc->height = height;

	if (R_GE_getVersion() != R_GE_version) {
		RKRBackend::this_pointer->graphicsEngineMismatchMessage(R_GE_version, R_GE_getVersion());
		Rf_error("Graphics version mismatch");
	}
	R_CheckDeviceAvailable ();
	pDevDesc dev;
	BEGIN_SUSPEND_INTERRUPTS {
		/* Allocate and initialize the device driver data */
		dev = (pDevDesc) R_Calloc(1, DevDesc);
		// NOTE: The call to RKGraphicsDeviceBackendTransmitter::instance(), here is important beyond error checking. It might *create* the instance and connection, if this is the first use.
		if (!(dev && RKGraphicsDeviceBackendTransmitter::instance () && desc->init (dev, pointsize, family, bg))) {
			R_Free (dev);
			delete (desc);
			desc = nullptr;
		} else {
			desc->devnum = 0;  // graphics engine will send an Activate-event, before we were even
			                   // able to see our own devnum and call RKD_Create. Therefore, initialize
			                   // devnum to 0, so as not to confuse the frontend
			desc->id = id++;   // extra identifier to make sure, R and the frontend are really talking about the same device
			                   // in case of potentially out-of-sync operations (notably RKDAdjustSize)
			pGEDevDesc gdd = GEcreateDevDesc(dev);
			gdd->displayList = R_NilValue;
			GEaddDevice2(gdd, "RKGraphicsDevice");
		}
	} END_SUSPEND_INTERRUPTS;

	if (desc) {
		desc->devnum = curDevice ();
		RKD_Create (desc->width, desc->height, dev, title, antialias, desc->id);
	} else {
		Rf_error("unable to start device");
	}
}

SEXP RKStartGraphicsDevice (SEXP width, SEXP height, SEXP pointsize, SEXP family, SEXP bg, SEXP title, SEXP antialias) {
	RKStartGraphicsDevice (Rf_asReal (width), Rf_asReal (height), Rf_asReal (pointsize), RKRSupport::SEXPToStringList (family), R_GE_str2col (CHAR(Rf_asChar(bg))), CHAR(Rf_asChar(title)), Rf_asLogical (antialias));
	return R_NilValue;
}

bool RKGraphicsDeviceDesc::init (pDevDesc dev, double pointsize, const QStringList &family, rcolor bg) {
	default_family = family.value (0, "Helvetica");
	default_symbol_family = family.value (0, "Symbol");
	RKD_QueryResolution (&dpix, &dpiy);
	if (dpix <= 1) dpix = RKGD_DPI;
	if (dpiy <= 1) dpiy = RKGD_DPI;
	width *= dpix;
	height *= dpiy;
//	Rprintf ("dpi: %d * %d, dims: %f * %f\n", dpix, dpiy, width, height);

	dev->deviceSpecific = (void *) this;

	// pointsize?

	/*
	* Initial graphical settings
	*/
	dev->startfont = 1;
	dev->startps = pointsize;
	dev->startcol = R_RGB(0, 0, 0);
	dev->startfill = bg;
	dev->startlty = LTY_SOLID;
	dev->startgamma = 1;
	/*
	* Device physical characteristics
	*/
	dev->left   = dev->clipLeft   = 0;
	dev->right  = dev->clipRight  = width;
	dev->bottom = dev->clipBottom = height;
	dev->top    = dev->clipTop    = 0;
	dev->cra[0] = 0.9 * pointsize * (dpix / RKGD_DPI);
	dev->cra[1] = 1.2 * pointsize * (dpiy / RKGD_DPI);
	dev->xCharOffset = 0.4900;
	dev->yCharOffset = 0.3333;
	dev->yLineBias = 0.2;
	dev->ipr[0] = 1.0 / dpix;
	dev->ipr[1] = 1.0 / dpiy;
	/*
	* Device capabilities
	*/
	dev->canClip = TRUE;
	dev->canHAdj = 2;
	dev->canChangeGamma = FALSE;
	dev->displayListOn = TRUE;

	dev->hasTextUTF8 = TRUE;
	dev->textUTF8 = RKD_TextUTF8;
	dev->strWidthUTF8 = RKD_StrWidthUTF8;
	dev->wantSymbolUTF8 = TRUE;
	dev->useRotatedTextInContour = TRUE;

	dev->haveTransparency = 2;
	dev->haveTransparentBg = 2; // FIXME. Do we really? Check.
	dev->haveRaster = 2;
	dev->haveCapture = 2;
	dev->haveLocator = 2;

	/*
	* Mouse events
	*/
	dev->canGenMouseDown = TRUE;
	dev->canGenMouseMove = TRUE;
	dev->canGenMouseUp = TRUE; 
	dev->canGenKeybd = TRUE;
	dev->canGenIdle = TRUE;

	// gettingEvent; This is set while getGraphicsEvent is actively
	// looking for events
	dev->eventHelper = RKD_EventHelper;
	dev->onExit = RKD_onExit;

	/*
	* Device functions
	*/
	dev->activate = RKD_Activate;
	dev->circle = RKD_Circle;
	dev->clip = RKD_Clip;
	dev->close = RKD_Close;
	dev->deactivate = RKD_Deactivate;
	dev->locator = RKD_Locator;
	dev->line = RKD_Line;
	dev->metricInfo = RKD_MetricInfo;
	dev->mode = RKD_Mode;
	dev->newPage = RKD_NewPage;
	dev->polygon = RKD_Polygon;
	dev->polyline = RKD_Polyline;
	dev->path = RKD_Path;
	dev->rect = RKD_Rect;
	dev->size = RKD_Size;
	// dev->onexit = RKD_OnExit; Called on user interrupts. NULL is OK.
	dev->raster = RKD_Raster;
	dev->cap = RKD_Capture;
	dev->newFrameConfirm = RKD_NewFrameConfirm;
	dev->holdflush = RKD_HoldFlush;

#if R_VERSION >= R_Version (4, 1, 0)
	// patterns and gradients
	dev->setPattern = RKD_SetPattern;
	dev->releasePattern = RKD_ReleasePattern;
	// clipping paths
	dev->setClipPath = RKD_SetClipPath;
	dev->releaseClipPath = RKD_ReleaseClipPath;
	// masks
	dev->setMask = RKD_SetMask;
	dev->releaseMask = RKD_ReleaseMask;
	dev->deviceVersion = qMin(15, R_GE_version);
	dev->deviceClip = TRUE; // for now
#endif

#if R_VERSION >= R_Version (4, 2, 0)
	// groups
	dev->defineGroup = RKD_DefineGroup;
	dev->useGroup = RKD_UseGroup;
	dev->releaseGroup = RKD_ReleaseGroup;

	// stroked / filled paths
	dev->stroke = RKD_Stroke;
	dev->fill = RKD_Fill;
	dev->fillStroke = RKD_FillStroke;
#endif
	return true;
}
