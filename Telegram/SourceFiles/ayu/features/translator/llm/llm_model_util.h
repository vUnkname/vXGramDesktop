// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#pragma once

#include <QtCore/QJsonObject>
#include <QtCore/QString>
#include <QtCore/QStringList>

namespace Ayu::Translator::Llm::ModelUtil {

[[nodiscard]] QString BaseModelName(const QString &model);
[[nodiscard]] bool IsGPT5(const QString &model);
[[nodiscard]] bool IsGemma4(const QString &model);
[[nodiscard]] bool IsGemini3(const QString &model);
[[nodiscard]] bool IsGeminiLegacy(const QString &model);
[[nodiscard]] bool IsDeepSeekV4(const QString &model);
[[nodiscard]] bool IsReasoning(const QString &model);
[[nodiscard]] bool IsOpenaiCompatibleReasoning(const QString &model);
[[nodiscard]] QString ReasoningEffort(const QString &model);

// Adds provider specific parameters which turn "thinking" off.
void ApplyReasoningParameters(
	QJsonObject &request,
	const QString &baseUrl,
	const QString &model);

[[nodiscard]] bool SupportsTemperature(const QString &model);
[[nodiscard]] QStringList StripModelsPrefix(const QStringList &models);
[[nodiscard]] bool IsOpenRouterFreeModel(const QString &model);
[[nodiscard]] QString SanitizeResponse(
	const QString &model,
	const QString &content);

// Base URL helpers (Android `LlmUrlNormalizer`).
[[nodiscard]] QString NormalizeBaseUrl(const QString &url);
[[nodiscard]] bool IsValidBaseUrl(const QString &url);

// Strips <think>...</think> blocks (Android `ReasoningContentFilter`).
[[nodiscard]] QString StripReasoningMarkup(const QString &text);

} // namespace Ayu::Translator::Llm::ModelUtil
