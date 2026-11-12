#pragma once
// ============================================================================
// SECTION 7 — AI SETTINGS DIALOG (Settings page ka kaam karta hua hissa)
// ----------------------------------------------------------------------------
// Kya karta hai:
//   User ko AI provider chunne deta hai:
//     * Local Ollama (default) — base URL http://127.0.0.1:11434/...,
//       model (jaise llama3), koi API key nahi.
//     * API (OpenAI-compatible) — apni base URL, model aur API key.
//   "Test Connection" button batata hai ki endpoint zinda hai ya nahi,
//   bina chat bheje. Save karne par QSettings me save hota hai.
// Purane code se farq:
//   Settings page pehle ek stub tha ("Awaiting data binding"), isliye
//   provider badla hi nahi ja sakta tha. Ab ye dialog asli hai aur
//   Sidebar ke header aur Settings page, dono se khulta hai.
// ============================================================================
#include <QDialog>

namespace corevex {

class AiSettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit AiSettingsDialog(QWidget* parent = nullptr);
};

}  // namespace corevex
