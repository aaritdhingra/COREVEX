// ============================================================================
// SECTION 6 (impl) — AI CLIENT
// ============================================================================
#include "AiClient.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

namespace corevex {

QStringList aiModelsForProvider(const QString& provider) {
    if (provider == QStringLiteral("gemini"))
        return {QStringLiteral("gemini-2.0-flash"), QStringLiteral("gemini-2.5-flash"), QStringLiteral("gemini-1.5-flash"), QStringLiteral("gemini-1.5-pro")};
    if (provider == QStringLiteral("api"))
        return {QStringLiteral("gpt-4o-mini"), QStringLiteral("gpt-4o"), QStringLiteral("gpt-3.5-turbo")};
    return {QStringLiteral("llama3"), QStringLiteral("llama3.1"), QStringLiteral("qwen2"), QStringLiteral("mistral"), QStringLiteral("phi3")};
}

QString aiMarkdownToHtml(const QString& markdown) {
    QString s = markdown;
    s.replace('&', QStringLiteral("&amp;"));
    s.replace('<', QStringLiteral("&lt;"));
    s.replace('>', QStringLiteral("&gt;"));
    // Code blocks ```...``` pehle, placeholder token se bachao.
    QStringList blocks;
    int bi = 0;
    while (true) {
        const int a = s.indexOf(QStringLiteral("```"));
        if (a < 0) break;
        const int b = s.indexOf(QStringLiteral("```"), a + 3);
        if (b < 0) break;
        QString code = s.mid(a + 3, b - (a + 3));
        const int nl = code.indexOf('\n');
        if (nl >= 0 && nl < 20 && !code.left(nl).contains(' ')) code = code.mid(nl + 1);
        blocks.append(QStringLiteral("<pre style='background:#0B0C0F;border:1px solid #2A2D35;border-radius:8px;padding:10px;white-space:pre-wrap;'>") + code.trimmed() + QStringLiteral("</pre>"));
        s = s.left(a) + QStringLiteral("@@BLOCK%1@@").arg(bi++) + s.mid(b + 3);
    }
    // Bold **...** — manual scan, regex group ki zaroorat nahi.
    QString out;
    for (int i = 0; i < s.size();) {
        if (s.mid(i, 2) == QStringLiteral("**")) {
            const int e = s.indexOf(QStringLiteral("**"), i + 2);
            if (e > 0) { out += QStringLiteral("<b>") + s.mid(i + 2, e - (i + 2)) + QStringLiteral("</b>"); i = e + 2; continue; }
        }
        if (s[i] == '`') {
            const int e = s.indexOf('`', i + 1);
            if (e > 0) { out += QStringLiteral("<code style='background:#0B0C0F;padding:2px 5px;border-radius:4px;'>") + s.mid(i + 1, e - (i + 1)) + QStringLiteral("</code>"); i = e + 1; continue; }
        }
        out += s[i]; ++i;
    }
    s = out;
    QStringList lines = s.split('\n');
    for (int i = 0; i < lines.size(); ++i) {
        QString l = lines[i];
        if (l.startsWith(QStringLiteral("### "))) l = QStringLiteral("<b>") + l.mid(4) + QStringLiteral("</b>");
        else if (l.startsWith(QStringLiteral("## "))) l = QStringLiteral("<b style='font-size:15px;'>") + l.mid(3) + QStringLiteral("</b>");
        else if (l.startsWith(QStringLiteral("# "))) l = QStringLiteral("<b style='font-size:17px;'>") + l.mid(2) + QStringLiteral("</b>");
        else if (l.startsWith(QStringLiteral("- ")) || l.startsWith(QStringLiteral("* "))) l = QStringLiteral("&bull; ") + l.mid(2);
        lines[i] = l;
    }
    s = lines.join(QStringLiteral("<br/>"));
    for (int i = 0; i < blocks.size(); ++i)
        s.replace(QStringLiteral("@@BLOCK") + QString::number(i) + QStringLiteral("@@"), blocks[i]);
    return s;
}

AiClient::AiClient(QObject* parent) : QObject(parent) {}

void AiClient::cancel() {
    if (currentReply_) { currentReply_->abort(); currentReply_ = nullptr; }
}

void AiClient::chat(const QVector<AiChatMessage>& history, const QString& userText,
                    std::function<void(const QString&)> onDone,
                    std::function<void(const QString&)> onError) {
    const QString url = config_.baseUrl.trimmed();
    if (url.isEmpty() || config_.model.isEmpty()) {
        if (onError) onError(QStringLiteral("AI Settings me base URL aur model set karo."));
        return;
    }

    QJsonObject body;
    // NOTE: QNetworkRequest req(QUrl(url)) mat likhna — C++ isko function
    // declaration samajh leta hai (most vexing parse), isliye build me
    // "left of .setHeader must have class" error aata tha. setUrl() safe hai.
    QNetworkRequest req;
    req.setUrl(QUrl(url));
    req.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    if (!config_.apiKey.isEmpty())
        req.setRawHeader("Authorization", ("Bearer " + config_.apiKey).toUtf8());

    // --- 6A. Format selection ------------------------------------------------
    // Gemini ka format alag hai: contents[] me user/model roles, parts[] me text.
    // URL: .../models/{model}:generateContent?key={apiKey}
    QString geminiUrl;
    if (config_.isGemini()) {
        QString base = url;
        if (base.endsWith(QStringLiteral("/models"))) base += QStringLiteral("/") + config_.model + QStringLiteral(":generateContent");
        else if (!base.contains(QStringLiteral(":generateContent"))) base += QStringLiteral(":generateContent");
        geminiUrl = base;
        if (!config_.apiKey.isEmpty()) {
            geminiUrl += (geminiUrl.contains('?') ? QStringLiteral("&") : QStringLiteral("?")) + QStringLiteral("key=") + QString::fromUtf8(QUrl::toPercentEncoding(config_.apiKey));
        }
        // Gemini me Authorization Bearer nahi, key query me jaati hai — isliye
        // upar wala Bearer header yahan hata do (warna Google 400 de sakta hai).
        req.setRawHeader("Authorization", QByteArray());
        req.setUrl(QUrl(geminiUrl));
        QJsonArray contents;
        for (const auto& m : history) {
            QJsonObject part; part[QStringLiteral("text")] = m.content;
            QJsonObject entry; entry[QStringLiteral("role")] = (m.role == QStringLiteral("assistant") ? QStringLiteral("model") : QStringLiteral("user"));
            entry[QStringLiteral("parts")] = QJsonArray{part};
            contents.append(entry);
        }
        QJsonObject upart; upart[QStringLiteral("text")] = userText;
        QJsonObject uentry; uentry[QStringLiteral("role")] = QStringLiteral("user"); uentry[QStringLiteral("parts")] = QJsonArray{upart};
        contents.append(uentry);
        body[QStringLiteral("contents")] = contents;
        { QJsonObject spart; spart[QStringLiteral("text")] = QStringLiteral("You are COREVEX AI, a helpful assistant. Answer clearly and concisely."); QJsonObject sys; sys[QStringLiteral("parts")] = QJsonArray{spart}; body[QStringLiteral("systemInstruction")] = sys; }
        QJsonObject gen; gen[QStringLiteral("temperature")] = 0.7; body[QStringLiteral("generationConfig")] = gen;
    } else if (url.contains(QStringLiteral("/chat/completions"))) {
        // OpenAI-compatible: messages array + system prompt (glitch kam, tone consistent).
        QJsonArray messages;
        { QJsonObject sys; sys[QStringLiteral("role")] = QStringLiteral("system"); sys[QStringLiteral("content")] = QStringLiteral("You are COREVEX AI, a helpful assistant. Answer clearly and concisely, in the user's language. Use markdown for structure when helpful."); messages.append(sys); }
        for (const auto& m : history) {
            QJsonObject o; o[QStringLiteral("role")] = m.role; o[QStringLiteral("content")] = m.content; messages.append(o);
        }
        QJsonObject user; user[QStringLiteral("role")] = QStringLiteral("user"); user[QStringLiteral("content")] = userText; messages.append(user);
        body[QStringLiteral("model")] = config_.model;
        body[QStringLiteral("messages")] = messages;
        body[QStringLiteral("stream")] = false;
    } else if (url.contains(QStringLiteral("/api/chat"))) {
        // Ollama chat: messages array + system prompt.
        QJsonArray messages;
        { QJsonObject sys; sys[QStringLiteral("role")] = QStringLiteral("system"); sys[QStringLiteral("content")] = QStringLiteral("You are COREVEX AI, a helpful assistant. Answer clearly and concisely, in the user's language."); messages.append(sys); }
        for (const auto& m : history) {
            QJsonObject o; o[QStringLiteral("role")] = m.role; o[QStringLiteral("content")] = m.content; messages.append(o);
        }
        QJsonObject user; user[QStringLiteral("role")] = QStringLiteral("user"); user[QStringLiteral("content")] = userText; messages.append(user);
        body[QStringLiteral("model")] = config_.model;
        body[QStringLiteral("messages")] = messages;
        body[QStringLiteral("stream")] = false;
    } else {
        // Ollama generate (purana saved endpoint, ab bhi support hai):
        // System instruction pehle, phir conversation — bina iske model
        // history ko aage badhata hua glitched output deta tha.
        QString prompt = QStringLiteral("System: You are COREVEX AI, a helpful assistant. Answer clearly and concisely.\n");
        for (const auto& m : history) prompt += m.role + QStringLiteral(": ") + m.content + QStringLiteral("\n");
        prompt += QStringLiteral("user: ") + userText + QStringLiteral("\nassistant:");
        body[QStringLiteral("model")] = config_.model;
        body[QStringLiteral("prompt")] = prompt;
        body[QStringLiteral("stream")] = false;
    }

    // Timeout: Local Ollama pehli baar model load karne me slow hota hai
    // (60-90s tak), isliye local ke liye 90s, API/Gemini ke liye 30s.
    // Timeout par reply abort hoga aur finished handler clear error dega.
    auto* reply = net_.post(req, QJsonDocument(body).toJson());
    currentReply_ = reply;
    QTimer::singleShot(config_.isLocal() ? 90000 : 30000, this, [this, reply] {
        if (currentReply_ == reply) reply->abort();
    });
    const bool isGeminiReq = config_.isGemini();
    connect(reply, &QNetworkReply::finished, this, [this, reply, onDone, onError, url, isGeminiReq] {
        if (currentReply_ == reply) currentReply_ = nullptr;
        if (reply->error() != QNetworkReply::NoError) {
            const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            QString msg = reply->errorString();
            if (reply->error() == QNetworkReply::OperationCanceledError) msg = QStringLiteral("Stopped by user.");
            if (status == 401) msg = QStringLiteral("API key galat hai (401). Settings me key check karo.");
            else if (url.contains(QStringLiteral("127.0.0.1")) || url.contains(QStringLiteral("localhost")))
                msg = QStringLiteral("Local LLM nahi mila. Ollama chal raha hai? (ollama serve + ollama pull llama3) — ya Settings me API mode chuno.");
            if (onError) onError(msg);
            reply->deleteLater();
            return;
        }
        const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        const QJsonObject obj = doc.object();
        QString text;
        if (isGeminiReq) {
            const QJsonArray cands = obj.value(QStringLiteral("candidates")).toArray();
            if (!cands.isEmpty()) {
                const QJsonArray parts = cands.first().toObject().value(QStringLiteral("content")).toObject().value(QStringLiteral("parts")).toArray();
                for (const auto& pv : parts) text += pv.toObject().value(QStringLiteral("text")).toString();
            }
            if (text.isEmpty() && obj.contains(QStringLiteral("error")))
                text = obj.value(QStringLiteral("error")).toObject().value(QStringLiteral("message")).toString();
        } else if (url.contains(QStringLiteral("/chat/completions"))) {
            const QJsonArray choices = obj.value(QStringLiteral("choices")).toArray();
            if (!choices.isEmpty())
                text = choices.first().toObject().value(QStringLiteral("message")).toObject().value(QStringLiteral("content")).toString();
        } else if (url.contains(QStringLiteral("/api/chat"))) {
            text = obj.value(QStringLiteral("message")).toObject().value(QStringLiteral("content")).toString();
        } else {
            text = obj.value(QStringLiteral("response")).toString();
        }
        if (onDone) onDone(text.isEmpty() ? QStringLiteral("AI se khaali jawab aaya. Model/endpoint check karo.") : text);
        reply->deleteLater();
    });
}

void AiClient::testConnection(std::function<void(bool, const QString&)> onResult) {
    if (config_.isGemini()) {
        const bool ok = !config_.model.isEmpty() && !config_.apiKey.isEmpty();
        if (onResult) onResult(ok, ok ? QStringLiteral("Gemini config complete hai (key + model).") : QStringLiteral("Gemini ke liye model aur API key dono chahiye."));
        return;
    }
    if (config_.isLocal()) {
        // Ollama tags endpoint: server zinda hai ya nahi, bina model
        // chalaye pata chal jaata hai.
        QString base = config_.baseUrl;
        const int idx = base.indexOf(QStringLiteral("/api/"));
        if (idx >= 0) base = base.left(idx);
        QNetworkRequest req;
        req.setUrl(QUrl(base + QStringLiteral("/api/tags")));
        auto* reply = net_.get(req);
        connect(reply, &QNetworkReply::finished, this, [reply, onResult] {
            const bool ok = reply->error() == QNetworkReply::NoError;
            if (onResult) onResult(ok, ok ? QStringLiteral("Local Ollama mil gaya.") : QStringLiteral("Local Ollama nahi mila: ollama serve chalao."));
            reply->deleteLater();
        });
    } else {
        const bool ok = !config_.baseUrl.isEmpty() && !config_.model.isEmpty() && !config_.apiKey.isEmpty();
        if (onResult) onResult(ok, ok ? QStringLiteral("API config complete hai.") : QStringLiteral("API mode me base URL, model aur API key teeno chahiye."));
    }
}

}  // namespace corevex
