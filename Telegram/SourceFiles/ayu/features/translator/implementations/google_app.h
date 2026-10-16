// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#pragma once

#include "ayu/features/translator/implementations/http.h"

namespace Ayu::Translator {

// Google dispatcher (mirrors Android `GoogleAppTranslator`):
// - "Google Translate experimental" on  -> translate-pa web endpoint;
// - experimental off + Cloud API key    -> Google Cloud Translation v2;
// - experimental off, no key            -> legacy translate_a/single.
class GoogleAppTranslator final : public HttpTranslator
{
	Q_OBJECT

public:
	static GoogleAppTranslator &instance();

	[[nodiscard]] QPointer<QNetworkReply> startSingleTranslation(
		const MultiThreadArgs &args
	) override;

private:
	explicit GoogleAppTranslator(QObject *parent = nullptr)
		: HttpTranslator(parent) {
	}

	[[nodiscard]] QPointer<QNetworkReply> translateCloud(
		const MultiThreadArgs &args,
		const QString &key);
	[[nodiscard]] QPointer<QNetworkReply> translateLegacy(
		const MultiThreadArgs &args);
};

}
