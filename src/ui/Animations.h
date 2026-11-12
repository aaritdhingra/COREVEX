#pragma once
// ============================================================================
// SECTION 12 — ANIMATIONS (sidebar slide, AI panel slide, page fade)
// ----------------------------------------------------------------------------
// Kyun:
//   Pehle sidebar / AI panel ekdum se gayab ya aa jaate the, isliye
//   "half screen / full screen" switch jhatka lagta tha. Ab width
//   animation se slide hota hai (250ms, ease-in-out), aur sidebar
//   pages fade-in hote hain (200ms).
// Kaise kaam karta hai:
//   Widget ki maximumWidth ko animate karte hain (layout ko pata
//   chalta hai, isliye browser area bhi smoothly badhta/ghatta hai).
//   Animation object ko parent widget par rakhte hain taaki wo
//   animation ke dauran delete na ho.
// ============================================================================
#include <QWidget>

namespace corevex {

// Sidebar ko 0 <-> fullWidth tak slide karo. Returns true if animation started.
void animateWidth(QWidget* widget, int fromWidth, int toWidth, int durationMs = 250);

// Page ko 0.0 -> 1.0 opacity fade-in do (sirf sidebar pages par use karo,
// WebEngine page par nahi, kyunki wo heavy hota hai).
void fadeInPage(QWidget* page, int durationMs = 200);

}  // namespace corevex
