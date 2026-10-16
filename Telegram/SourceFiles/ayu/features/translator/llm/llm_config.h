// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#pragma once

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <rpl/producer.h>

namespace Ayu::Translator::Llm::Config {

// Selected preset (Android `llmProviderPreset`).
[[nodiscard]] int CurrentPreset();

// Saved / effective model of the preset.
[[nodiscard]] QString SavedModel(int preset);
void SetSavedModel(int preset, const QString &model);
[[nodiscard]] QString EffectiveModel(int preset);
[[nodiscard]] rpl::producer<QString> EffectiveModelValue(int preset);
// Emits whenever model / preset / url change (for UI dependent rows).
[[nodiscard]] rpl::producer<QString> CurrentEffectiveModelValue();

// URL used for requests (custom preset uses the user URL).
[[nodiscard]] QString EffectiveBaseUrl(int preset);
void SetCustomBaseUrl(const QString &url);

// API keys of the preset, comma separated, as saved.
[[nodiscard]] QString ApiKeys(int preset);
void SetApiKeys(int preset, const QString &keys);
[[nodiscard]] rpl::producer<QString> ApiKeysValue(int preset);
[[nodiscard]] QStringList ApiKeyList(int preset);
[[nodiscard]] QString FirstApiKey(int preset);

// True when the selected preset has at least one API key.
[[nodiscard]] bool IsAvailable();

[[nodiscard]] int ContextMessageLimit();

} // namespace Ayu::Translator::Llm::Config
