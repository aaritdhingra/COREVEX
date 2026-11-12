// ============================================================================
// SECTION 0 — APP ENTRY POINT
// ----------------------------------------------------------------------------
// Kya karta hai:
//   QApplication banata hai, COREVEX theme (QSS) load karta hai,
//   window kholta hai, app chalata hai.
// Pehle QSS kabhi load hi nahi hota tha, isliye Pages ke buttons
// default grey dikhte the. Ab loadApplicationStyleSheet() pehle
// chalega, phir window banegi — taaki har page/dialog sleek dikhe.
// Kya nahi karta (purana bug fix):
//   Koi hardcoded path set nahi karta (pehle QDir::setCurrent("E:/...")
//   tha). Assets ka path utilities/Assets.h resolver dhoondhta hai.
// ============================================================================
#include <QApplication>

#include "../utilities/Assets.h"
#include "CorevexWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("COREVEX"));
    app.setOrganizationName(QStringLiteral("COREVEX"));
    corevex::loadApplicationStyleSheet();
    CorevexWindow w;
    w.show();
    return app.exec();
}
