// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#pragma once

#include <functional>
#include <QtCore/QPointer>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

namespace Ayu::Translator::Llm {

template <typename T>
struct Response {
	T data;
	QString error;
	qint64 durationMs = 0;
	int httpCode = 0;

	[[nodiscard]] bool ok() const {
		return error.isEmpty();
	}
};

// Minimal OpenAI-compatible client (Android `OpenAICompatClient`).
class Client final
{
public:
	static Client &instance();

	// POST {baseUrl}/chat/completions, returns message content.
	QPointer<QNetworkReply> chatCompletions(
		const QString &baseUrl,
		const QString &apiKey,
		const QByteArray &requestJson,
		std::function<void(Response<QString>)> done,
		int timeoutMs = 60000);

	// GET {baseUrl}/models
	void fetchModels(
		const QString &baseUrl,
		const QString &apiKey,
		std::function<void(Response<QStringList>)> done);

	// Sends a tiny request to verify credentials and model.
	void testChatCompletions(
		const QString &baseUrl,
		const QString &apiKey,
		const QString &model,
		std::function<void(Response<QString>)> done);

private:
	Client() = default;

	QNetworkAccessManager _nam;
};

} // namespace Ayu::Translator::Llm
