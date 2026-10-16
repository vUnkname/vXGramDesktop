// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#pragma once

#include "ayu/features/translator/implementations/http.h"

namespace Ayu::Translator {

// Tencent TranSmart web API.
class TranSmartTranslator final : public HttpTranslator
{
	Q_OBJECT

public:
	static TranSmartTranslator &instance();

	[[nodiscard]] QPointer<QNetworkReply> startSingleTranslation(
		const MultiThreadArgs &args
	) override;

private:
	explicit TranSmartTranslator(QObject *parent = nullptr)
		: HttpTranslator(parent) {
	}
};

}