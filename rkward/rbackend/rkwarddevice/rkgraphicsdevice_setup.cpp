/*
rkgraphicsdevice_setup - This file is part of RKWard (https://rkward.kde.org). Created: Mon Mar 18 2013
SPDX-FileCopyrightText: 2013-2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
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

#include "../rkrapi.h"

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
		RFn::Rf_error("Invalid width or height: (%g, %g)", width, height);
	}
	RKGraphicsDeviceDesc *desc = new RKGraphicsDeviceDesc;
	desc->width = width;
	desc->height = height;

	if (RFn::R_GE_getVersion() != R_GE_version) {
		if (RFn::R_GE_getVersion() < 14) {
			RKRBackend::this_pointer->graphicsEngineMismatchMessage(R_GE_version, RFn::R_GE_getVersion());
			RFn::Rf_error("Graphics version mismatch");
		} else {
			// All other cases currently thought ot be acceptible?
			// GE version 17 adds capabilities(), so higher GE version should no longer be a problem.
			// Lower GE version should be ok (down to 14, ATM), because we simply behave as though that was the case.
			RK_DEBUG(RBACKEND, DL_WARNING, "GE version compile time: %d, GE version runtime %d", R_GE_version, RFn::R_GE_getVersion());
		}
	}
	RFn::R_CheckDeviceAvailable();
	pDevDesc dev;
	{
		RKRSupport::InterruptSuspension susp;
		/* Allocate and initialize the device driver data */
		const size_t allocsize = sizeof(DevDesc) + 256; // deliberately overallocating, in case later version of R try to write something, here
		dev = (pDevDesc) RFn::R_chk_calloc(1, allocsize);
		// NOTE: The call to RKGraphicsDeviceBackendTransmitter::instance(), here is important beyond error checking. It might *create* the instance and connection, if this is the first use.
		if (!(dev && RKGraphicsDeviceBackendTransmitter::instance () && desc->init (dev, pointsize, family, bg))) {
			RFn::R_chk_free(dev);
			delete(desc);
			desc = nullptr;
		} else {
			desc->devnum = 0;  // graphics engine will send an Activate-event, before we were even
			                   // able to see our own devnum and call RKD_Create. Therefore, initialize
			                   // devnum to 0, so as not to confuse the frontend
			desc->id = id++;   // extra identifier to make sure, R and the frontend are really talking about the same device
			                   // in case of potentially out-of-sync operations (notably RKDAdjustSize)
			pGEDevDesc gdd = RFn::GEcreateDevDesc(dev);
			gdd->displayList = ROb(R_NilValue);
			RFn::GEaddDevice2(gdd, "RKGraphicsDevice");
		}
	};

	if (desc) {
		desc->devnum = RFn::Rf_curDevice();
		RKD_Create(desc->width, desc->height, dev, title, antialias, desc->id);
	} else {
		RFn::Rf_error("unable to start device");
	}
}

SEXP RKStartGraphicsDevice (SEXP width, SEXP height, SEXP pointsize, SEXP family, SEXP bg, SEXP title, SEXP antialias) {
	RKStartGraphicsDevice(RFn::Rf_asReal(width), RFn::Rf_asReal(height), RFn::Rf_asReal(pointsize), RKRSupport::SEXPToStringList(family), RFn::R_GE_str2col(RFn::R_CHAR(RFn::Rf_asChar(bg))), RFn::R_CHAR(RFn::Rf_asChar(title)), RFn::Rf_asLogical(antialias));
	return ROb(R_NilValue);
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
	// NOTE: We need both a compiletime and a runtime check, in order to support running with an R older than what was used at compile time
	if (RFn::R_GE_getVersion() >=  15) {
		// patterns and gradients
		dev->setPattern = RKD_SetPattern;
		dev->releasePattern = RKD_ReleasePattern;
		// clipping paths
		dev->setClipPath = RKD_SetClipPath;
		dev->releaseClipPath = RKD_ReleaseClipPath;
		// masks
		dev->setMask = RKD_SetMask;
		dev->releaseMask = RKD_ReleaseMask;
		dev->deviceVersion = qMin(qMin(15, R_GE_version), RFn::R_GE_getVersion());
		dev->deviceClip = TRUE; // for now
	}
#endif

#if R_VERSION >= R_Version (4, 2, 0)
	if (RFn::R_GE_getVersion() >=  16) {
		// groups
		dev->defineGroup = RKD_DefineGroup;
		dev->useGroup = RKD_UseGroup;
		dev->releaseGroup = RKD_ReleaseGroup;

		// stroked / filled paths
		dev->stroke = RKD_Stroke;
		dev->fill = RKD_Fill;
		dev->fillStroke = RKD_FillStroke;
	}
#endif
	return true;
}
