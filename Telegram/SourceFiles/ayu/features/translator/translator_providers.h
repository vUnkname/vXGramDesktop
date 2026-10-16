// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#pragma once

#include "ayu/ayu_settings.h"

namespace Ayu::Translator {

struct ProviderEntry {
	TranslationProvider provider = TranslationProvider::Telegram;
	QString name;
};

// Providers that can be picked in settings (platform dependent ones are
// filtered out when unavailable). `forArticles` hides the providers that make
// no sense for Instant View articles (e.g. LLM).
[[nodiscard]] std::vector<ProviderEntry> AvailableProviders(
	bool forArticles = false);

[[nodiscard]] QString ProviderName(TranslationProvider provider);

} // namespace Ayu::Translator
