// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#pragma once

#include "ayu/features/translator/implementations/http.h"

namespace Ayu::Translator {

// bing.com/translator web endpoint (Android `MicrosoftTranslator`).
class MicrosoftTranslator final : public HttpTranslator
{
	Q_OBJECT

public:
	static MicrosoftTranslator &instance();

	[[nodiscard]] QPointer<QNetworkReply> startSingleTranslation(
		const MultiThreadArgs &args
	) override;

private:
	explicit MicrosoftTranslator(QObject *parent = nullptr)
		: HttpTranslator(parent) {
	}

	[[nodiscard]] bool configExpired() const;
	[[nodiscard]] QPointer<QNetworkReply> fetchConfig(
		Fn<void(bool)> done);
	[[nodiscard]] QPointer<QNetworkReply> perform(
		const MultiThreadArgs &args,
		const QString &from,
		const QString &to,
		const QString &query);

	QString _ig;
	QString _iid;
	QString _key;
	QString _token;
	qint64 _tokenTs = 0;
	qint64 _tokenInterval = 0;
	int _count = 0;
};

// Edge browser translator endpoint (Android `RealMicrosoftTranslator`).
class RealMicrosoftTranslator final : public HttpTranslator
{
	Q_OBJECT

public:
	static RealMicrosoftTranslator &instance();

	[[nodiscard]] QPointer<QNetworkReply> startSingleTranslation(
		const MultiThreadArgs &args
	) override;

private:
	explicit RealMicrosoftTranslator(QObject *parent = nullptr)
		: HttpTranslator(parent) {
	}

	[[nodiscard]] QPointer<QNetworkReply> perform(
		const MultiThreadArgs &args,
		const QString &from,
		const QString &to,
		const QString &query);

	QString _token;
	qint64 _tokenTs = 0;
};

}
