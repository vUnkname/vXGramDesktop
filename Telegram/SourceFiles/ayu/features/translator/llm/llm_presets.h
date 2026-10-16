// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#pragma once

#include <QtCore/QString>
#include <QtCore/QStringList>

namespace Ayu::Translator::Llm {

// Mirrors Android `LlmPresetRegistry`.
enum Preset {
	Custom = 0,
	OpenAI = 1,
	Gemini = 2,
	Groq = 3,
	DeepSeek = 4,
	XAI = 5,
	Cerebras = 6,
	OllamaCloud = 7,
	OpenRouter = 8,
	VercelAiGateway = 9,
};

constexpr auto kPresetCount = 10;

[[nodiscard]] QString PresetName(int preset);
[[nodiscard]] QString PresetBaseUrl(int preset);
[[nodiscard]] QString PresetDefaultModel(int preset);
// Page where the user can obtain an API key for the preset.
[[nodiscard]] QString PresetKeyUrl(int preset);

} // namespace Ayu::Translator::Llm
