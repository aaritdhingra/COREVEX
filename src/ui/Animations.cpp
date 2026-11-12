// ============================================================================
// SECTION 12 (impl) — ANIMATIONS
// ============================================================================
#include "Animations.h"

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>

namespace corevex {

void animateWidth(QWidget* widget, int fromWidth, int toWidth, int durationMs) {
    if (!widget) return;
    auto* anim = new QPropertyAnimation(widget, "maximumWidth", widget);
    anim->setDuration(durationMs);
    anim->setStartValue(fromWidth);
    anim->setEndValue(toWidth);
    anim->setEasingCurve(QEasingCurve::InOutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void fadeInPage(QWidget* page, int durationMs) {
    Q_UNUSED(durationMs);
    // v3.3 FIX — animation band: fast hover/click par opacity effect stack
    // ho jaata tha aur page gayab dikhta tha. Ab sirf purana effect hatao
    // aur page ko poori tarah visible rakho. Stability > animation.
    if (!page) return;
    if (page->graphicsEffect()) {
        page->setGraphicsEffect(nullptr);
    }
    page->setVisible(true);
}

}  // namespace corevex
