// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#pragma once

#include "ayu/features/translator/implementations/base.h"

#include <functional>
#include <QtCore/QByteArray>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

namespace Ayu::Translator {

// `status` is the HTTP status code, or 0 on a network-level failure.
using HttpDone = std::function<void(int status, const QByteArray &body)>;

// Sends the request with a timeout. The reply is deleted after `done` returns.
QPointer<QNetworkReply> sendHttp(
	QNetworkAccessManager &nam,
	const QNetworkRequest &request,
	const QByteArray &body,
	bool post,
	int timeoutMs,
	HttpDone done);

// Returns true if `done` was fired with a 2xx status.
[[nodiscard]] inline bool httpOk(int status) {
	return (status >= 200) && (status < 300);
}

// Generic "is this language supported" check (case-insensitive).
[[nodiscard]] bool languageInList(const QString &lang, const QStringList &list);

// Prepares text for sending (HTML when wrapping is enabled).
[[nodiscard]] QString prepareText(const TextWithEntities &text);

// Converts a translated string back into text with entities.
[[nodiscard]] TextWithEntities finishText(const QString &translated);

// Base for engines which talk to an HTTP API.
class HttpTranslator : public MultiThreadTranslator
{
	Q_OBJECT

public:
	explicit HttpTranslator(QObject *parent = nullptr)
		: MultiThreadTranslator(parent) {
	}

protected:
	QNetworkAccessManager _nam;
};

} // namespace Ayu::Translator
