// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#pragma once

#include "ayu/features/translator/implementations/http.h"

namespace Ayu::Translator {

// DeepL: official API when an auth key is set, otherwise the web JSON-RPC
// endpoint (best effort, may be rate limited by DeepL).
class DeepLTranslator final : public HttpTranslator
{
	Q_OBJECT

public:
	static DeepLTranslator &instance();

	[[nodiscard]] int getConcurrencyLimit() const override { return 2; }

	[[nodiscard]] QPointer<QNetworkReply> startSingleTranslation(
		const MultiThreadArgs &args
	) override;

private:
	explicit DeepLTranslator(QObject *parent = nullptr)
		: HttpTranslator(parent) {
	}

	[[nodiscard]] QPointer<QNetworkReply> translateWithKey(
		const MultiThreadArgs &args,
		const QString &key);
	[[nodiscard]] QPointer<QNetworkReply> translateWeb(
		const MultiThreadArgs &args);

	qint64 _rpcId = 0;
};

}
