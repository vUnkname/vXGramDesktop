// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#pragma once

#include "ayu/features/translator/implementations/http.h"

namespace Ayu::Translator {

// "AI Translator": any OpenAI-compatible chat completions endpoint.
// Rate limits are retried with backoff by `MultiThreadTranslator`,
// other failures fall back to Google (same as the Android client).
class LLMTranslator final : public HttpTranslator
{
	Q_OBJECT

public:
	static LLMTranslator &instance();

	[[nodiscard]] int getConcurrencyLimit() const override { return 3; }

	[[nodiscard]] QPointer<QNetworkReply> startSingleTranslation(
		const MultiThreadArgs &args
	) override;

private:
	explicit LLMTranslator(QObject *parent = nullptr)
		: HttpTranslator(parent) {
	}

	[[nodiscard]] QString nextApiKey(int preset);

	int _keyPreset = -1;
	QString _keyCache;
	QStringList _keys;
	int _keyIndex = 0;
};

}
