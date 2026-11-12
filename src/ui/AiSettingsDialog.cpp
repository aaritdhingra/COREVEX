// ============================================================================
// SECTION 7 (impl) — AI SETTINGS DIALOG
// ============================================================================
#include "AiSettingsDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QVBoxLayout>

#include "../ai/AiClient.h"

namespace corevex {

AiSettingsDialog::AiSettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(QStringLiteral("AI Settings — COREVEX"));
    resize(520, 420);
    setMinimumSize(520, 400);

    QSettings settings(QStringLiteral("COREVEX"), QStringLiteral("Browser"));
    auto* layout = new QVBoxLayout(this);
    auto* form = new QFormLayout();

    // --- 7A. Fields (QSettings se load, pehle wale defaults ke saath) --
    auto* provider = new QComboBox(this);
    provider->addItems({QStringLiteral("local"), QStringLiteral("api"), QStringLiteral("gemini")});
    provider->setCurrentText(settings.value(QStringLiteral("ai/provider"), QStringLiteral("local")).toString());
    auto* baseUrl = new QLineEdit(settings.value(QStringLiteral("ai/baseUrl"), QStringLiteral("http://127.0.0.1:11434/api/chat")).toString(), this);
    // Model: dropdown (preset) + custom text — Gemini jaisa picker.
    auto* modelCombo = new QComboBox(this);
    modelCombo->setEditable(true);
    const QString curModel = settings.value(QStringLiteral("ai/model"), QStringLiteral("llama3")).toString();
    modelCombo->setCurrentText(curModel);
    auto* apiKey = new QLineEdit(settings.value(QStringLiteral("ai/apiKey"), QString()).toString(), this);
    apiKey->setEchoMode(QLineEdit::Password);
    apiKey->setPlaceholderText(QStringLiteral("API / Gemini mode me chahiye, Local me nahi"));

    form->addRow(QStringLiteral("Provider:"), provider);
    form->addRow(QStringLiteral("Base URL:"), baseUrl);
    form->addRow(QStringLiteral("Model:"), modelCombo);
    form->addRow(QStringLiteral("API key:"), apiKey);
    layout->addLayout(form);

    // --- 7B. Provider badalte hi sensible defaults bharna --------------
    auto fillModels = [=](const QString& p) {
        // Glitch fix: provider badalte hi model ko pehle preset par mat
        // phenko agar user ka model us provider ki list me hai to.
        const QString keep = modelCombo->currentText();
        modelCombo->clear();
        modelCombo->addItems(aiModelsForProvider(p));
        if (!keep.isEmpty() && aiModelsForProvider(p).contains(keep)) modelCombo->setCurrentText(keep);
        else modelCombo->setCurrentText(modelCombo->itemText(0));
    };
    fillModels(provider->currentText());
    modelCombo->setCurrentText(curModel);
    connect(provider, &QComboBox::currentTextChanged, this, [=](const QString& p) {
        if (p == QStringLiteral("local")) {
            baseUrl->setText(QStringLiteral("http://127.0.0.1:11434/api/chat"));
        } else if (p == QStringLiteral("gemini")) {
            baseUrl->setText(QStringLiteral("https://generativelanguage.googleapis.com/v1beta/models"));
        } else if (baseUrl->text().contains(QStringLiteral("127.0.0.1")) || baseUrl->text().contains(QStringLiteral("generativelanguage"))) {
            baseUrl->setText(QStringLiteral("https://api.openai.com/v1/chat/completions"));
        }
        fillModels(p);
    });

    // --- 7C. Test + Save ------------------------------------------------
    auto* testBtn = new QPushButton(QStringLiteral("Test Connection"), this);
    layout->addWidget(testBtn);
    connect(testBtn, &QPushButton::clicked, this, [=] {
        // Client dialog ka child hai, isliye async test ke dauraan
        // zinda rahega; result aate hi message box dikhega.
        auto* client = new AiClient(this);
        AiConfig cfg;
        cfg.provider = provider->currentText();
        cfg.baseUrl = baseUrl->text().trimmed();
        cfg.model = modelCombo->currentText().trimmed();
        cfg.apiKey = apiKey->text();
        client->setConfig(cfg);
        testBtn->setEnabled(false);
        testBtn->setText(QStringLiteral("Testing…"));
        client->testConnection([this, testBtn](bool ok, const QString& msg) {
            testBtn->setEnabled(true);
            testBtn->setText(QStringLiteral("Test Connection"));
            if (ok) QMessageBox::information(this, QStringLiteral("Test"), msg);
            else QMessageBox::warning(this, QStringLiteral("Test"), msg);
        });
    });

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::accepted, this, [=] {
        // NOTE: Constructor wala QSettings yahan capture nahi kar sakte —
        // QSettings copy nahi hota (copy constructor deleted hai, isliye
        // build me C2280 error aata tha), aur constructor ke baad wo
        // stack variable khatam bhi ho jaata hai. Isliye Save par naya
        // QSettings banakar save karo — yehi sahi tareeka hai.
        QSettings saveSettings(QStringLiteral("COREVEX"), QStringLiteral("Browser"));
        saveSettings.setValue(QStringLiteral("ai/provider"), provider->currentText());
        saveSettings.setValue(QStringLiteral("ai/baseUrl"), baseUrl->text().trimmed());
        saveSettings.setValue(QStringLiteral("ai/model"), modelCombo->currentText().trimmed());
        if (!apiKey->text().isEmpty())
            saveSettings.setValue(QStringLiteral("ai/apiKey"), apiKey->text());
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    layout->addWidget(new QLabel(
        QStringLiteral("Local mode me data laptop se bahar nahi jaata. "
                       "API / Gemini mode me sirf tumhara prompt provider ko jaata hai. "
                       "API key isi dialog me save hoti hai, chat me kabhi dikhti nahi."), this));
}

}  // namespace corevex
