#pragma once
// ============================================================================
// SECTION 6 — AI CLIENT (Local LLM default + API option)
// ----------------------------------------------------------------------------
// Kya karta hai:
//   Sidebar chat ko ek hi interface se do tareekon se chalata hai:
//   (1) LOCAL (default): Ollama, http://127.0.0.1:11434 — isme koi
//       API key nahi chahiye, model local machine par chalta hai.
//   (2) API (OpenAI-compatible): /chat/completions — user apni API key,
//       base URL aur model Settings me daalta hai.
//   (3) GEMINI (Google): generativelanguage.googleapis.com — Gemini ka
//       apna format hai (OpenAI jaisa nahi), isliye alag se handle hota
//       hai. User baad me apni Gemini API key daalega, tab ye chalega.
// Kaise pehchanta hai kaunsa format:
//   baseUrl ke aakhir se: "/api/generate" -> Ollama generate,
//   "/api/chat" -> Ollama chat, "/chat/completions" -> OpenAI format,
//   warna Local mode me "/api/generate" joda jaata hai.
// Purane code se farq:
//   Purana code sirf ek hi format (Ollama generate, prompt field)
//   bhejta tha, bina history ke, aur Settings page stub tha — user
//   provider badal hi nahi sakta tha. Ab history bheji jaati hai
//   (chat bot jaisa normal conversation), aur provider Settings
//   dialog se badla ja sakta hai (Section 7).
// Privacy:
//   Local mode me prompt laptop se bahar nahi jaata. API mode me
//   sirf wahi prompt jaata hai jo user ne sidebar me likha hai.
// ============================================================================
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QVector>
#include <functional>

namespace corevex {

struct AiConfig {
    // "local" / "api" / "gemini". Default local hai.
    QString provider = QStringLiteral("local");
    QString baseUrl = QStringLiteral("http://127.0.0.1:11434/api/chat");  // v3.1: /api/chat, /api/generate se cleaner chat milta hai, glitch kam
    QString model = QStringLiteral("llama3");
    QString apiKey;  // Sirf API / Gemini mode me use hota hai.
    bool isLocal() const { return provider == QStringLiteral("local"); }
    bool isGemini() const { return provider == QStringLiteral("gemini") || baseUrl.contains(QStringLiteral("generativelanguage.googleapis.com")); }
};
// Gemini-style model picker ke liye preset lists (panel me dropdown).
QStringList aiModelsForProvider(const QString& provider);
// Chat bubble ke liye markdown-lite -> HTML (bold, code, lists, headings).
QString aiMarkdownToHtml(const QString& markdown);

struct AiChatMessage { QString role; QString content; };

class AiClient : public QObject {
    Q_OBJECT
public:
    explicit AiClient(QObject* parent = nullptr);

    // Config Settings dialog se aati hai (QSettings me saved).
    void setConfig(const AiConfig& config) { config_ = config; }
    AiConfig config() const { return config_; }

    // Ek chat turn: history + naya user message -> assistant reply.
    // onDone(reply) ya onError(message) me se ek hi call hoga.
    void chat(const QVector<AiChatMessage>& history, const QString& userText,
              std::function<void(const QString&)> onDone,
              std::function<void(const QString&)> onError);

    // Local Ollama chal raha hai ya nahi, ye check karta hai.
    // API/Gemini mode me sirf config complete hai ya nahi batata hai.
    void testConnection(std::function<void(bool, const QString&)> onResult);

    // Chal rahi chat ko roko (Stop button ke liye).
    void cancel();

private:
    AiConfig config_;
    QNetworkAccessManager net_;
    QNetworkReply* currentReply_ = nullptr;  // global Qt type, corevex:: nahi
};

}  // namespace corevex
