// rkqwebview.js - This file is part of the RKWard project. Created: Sat Mar 14 2026
// SPDX-FileCopyrightText: 2026 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
// SPDX-FileContributor: The RKWard Team <rkward@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

function __rkward_debounce(fun) {
	let to = null;
	return function() {
		clearTimeout(to);
		to = setTimeout(function() { fun(); }, 20);
	}
}

document.onselectionchange = __rkward_debounce(function() {
	__rkward_sendMessage("selChanged", { sel: document.getSelection().toString() });
});

document.onscroll = __rkward_debounce(function() {
	__rkward_sendMessage("scroll", { x: window.scrollX, y: window.scrollY });
});

function __rkward_sendMessage(msg, args) {
	if (__rkward.readyState == WebSocket.CONNECTING) {
		setTimeout(__rkward_sendMessage.bind(null, msg, args), 10);
	} else {
		__rkward.send(JSON.stringify({msg, args}));
	}
}

document.addEventListener('click', e => {
	const origin = e.target.closest('a');
	if (origin && origin.href.startswith('#')) {
		__rkward_sendMessage("pageInternalNav", { url: origin.href });
		e.preventDefault();
	}
});

document.addEventListener('contextmenu', e => {
	let params = { x: e.screenX, y: e.screenY };
	try {
		params.url = e.target.closest('a').href;
	} catch {}
	try {
		params.src = e.target.closest('img').src;
	} catch {}
	__rkward_sendMessage("contextMenu", params);
	e.preventDefault();
});
