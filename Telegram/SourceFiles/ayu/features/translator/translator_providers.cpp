// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/features/translator/translator_providers.h"

#include "platform/platform_translate_provider.h"
#include "base/platform/base_platform_info.h"

namespace Ayu::Translator {
namespace {

[[nodiscard]] QString NativeProviderName() {
	if constexpr (Platform::IsMac()) {
		return QString("macOS");
	} else if constexpr (Platform::IsWindows()) {
		return QString("Windows");
	} else {
		return QString("Linux");
	}
}

} // namespace

std::vector<ProviderEntry> AvailableProviders(bool forArticles) {
	auto result = std::vector<ProviderEntry>{
		{ TranslationProvider::Google, QString("Google Translate") },
		{ TranslationProvider::Yandex, QString("Yandex Translate") },
		{ TranslationProvider::Lingo, QString("LingoCloud Translate") },
		{ TranslationProvider::Microsoft, QString("Bing Translator") },
		{ TranslationProvider::RealMicrosoft, QString("Microsoft Translator") },
		{ TranslationProvider::DeepL, QString("DeepL Translate") },
		{ TranslationProvider::Telegram, QString("Telegram API") },
		{ TranslationProvider::TranSmart, QString("TranSmart Translator") },
	};
	if (!forArticles) {
		result.push_back({
			TranslationProvider::LLM,
			QString("AI Translator"),
		});
	}
	if (Platform::IsTranslateProviderAvailable()) {
		result.push_back({
			TranslationProvider::Native,
			NativeProviderName(),
		});
	}
	return result;
}

QString ProviderName(TranslationProvider provider) {
	for (const auto &entry : AvailableProviders(false)) {
		if (entry.provider == provider) {
			return entry.name;
		}
	}
	return QString("Telegram");
}

} // namespace Ayu::Translator
