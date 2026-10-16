// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/features/translator/llm/llm_client.h"

#include "lang_auto.h"
#include "ayu/features/translator/implementations/http.h"
#include "ayu/features/translator/llm/llm_model_util.h"

#include <QtCore/QDateTime>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QUrl>

namespace Ayu::Translator::Llm {
namespace {

[[nodiscard]] QString Truncate(const QByteArray &body) {
	constexpr auto kLimit = 4096;
	const auto text = QString::fromUtf8(body);
	return (text.size() <= kLimit)
		? text
		: (text.left(kLimit) + QStringLiteral("\n(truncated)"));
}

[[nodiscard]] QString HttpError(int code, const QByteArray &body) {
	return QStringLiteral("HTTP %1 : %2").arg(code).arg(Truncate(body));
}

[[nodiscard]] QString Validate(const QString &baseUrl, const QString &key) {
	if (baseUrl.trimmed().isEmpty()) {
		return QStringLiteral("Empty base URL");
	}
	const auto trimmed = key.trimmed();
	if (trimmed.isEmpty()) {
		return tr::ayu_LlmApiKeyNotSet(tr::now);
	}
	if (trimmed.contains('\n') || trimmed.contains('\r')) {
		return QStringLiteral("Invalid API key");
	}
	return {};
}

[[nodiscard]] QNetworkRequest MakeRequest(
		const QString &url,
		const QString &apiKey) {
	auto request = QNetworkRequest(QUrl(url));
	request.setRawHeader(
		"Authorization",
		QByteArray("Bearer ") + apiKey.trimmed().toUtf8());
	request.setHeader(
		QNetworkRequest::ContentTypeHeader,
		QStringLiteral("application/json"));
	return request;
}

[[nodiscard]] QString ParseFirstMessageContent(const QByteArray &body) {
	const auto choices = QJsonDocument::fromJson(body).object()
		.value(QStringLiteral("choices")).toArray();
	if (choices.isEmpty()) {
		return {};
	}
	return choices.at(0).toObject()
		.value(QStringLiteral("message")).toObject()
		.value(QStringLiteral("content")).toString();
}

void ExtractIds(const QJsonArray &array, QStringList &out) {
	for (const auto &item : array) {
		auto id = QString();
		if (item.isObject()) {
			id = item.toObject().value(QStringLiteral("id")).toString();
		} else if (item.isString()) {
			id = item.toString();
		}
		id = id.trimmed();
		if (!id.isEmpty() && !out.contains(id)) {
			out.push_back(id);
		}
	}
}

[[nodiscard]] QStringList ParseModelIds(const QByteArray &body) {
	auto result = QStringList();
	const auto doc = QJsonDocument::fromJson(body);
	if (doc.isArray()) {
		ExtractIds(doc.array(), result);
	} else if (doc.isObject()) {
		const auto object = doc.object();
		const auto data = object.value(QStringLiteral("data"));
		const auto models = object.value(QStringLiteral("models"));
		if (data.isArray()) {
			ExtractIds(data.toArray(), result);
		} else if (models.isArray()) {
			ExtractIds(models.toArray(), result);
		} else if (data.isObject()) {
			const auto id = data.toObject().value(QStringLiteral("id"))
				.toString().trimmed();
			if (!id.isEmpty()) {
				result.push_back(id);
			}
		}
	}
	return result;
}

} // namespace

Client &Client::instance() {
	static Client result;
	return result;
}

QPointer<QNetworkReply> Client::chatCompletions(
		const QString &baseUrl,
		const QString &apiKey,
		const QByteArray &requestJson,
		std::function<void(Response<QString>)> done,
		int timeoutMs) {
	if (const auto error = Validate(baseUrl, apiKey); !error.isEmpty()) {
		done(Response<QString>{ .error = error });
		return nullptr;
	}
	const auto started = QDateTime::currentMSecsSinceEpoch();
	return sendHttp(
		_nam,
		MakeRequest(baseUrl.trimmed() + u"/chat/completions"_q, apiKey),
		requestJson,
		true,
		timeoutMs,
		[=](int status, const QByteArray &body) {
			const auto duration = QDateTime::currentMSecsSinceEpoch() - started;
			if (!status) {
				done({
					.error = QStringLiteral("Network error"),
					.durationMs = duration,
				});
			} else if (!httpOk(status)) {
				done({
					.error = HttpError(status, body),
					.durationMs = duration,
					.httpCode = status,
				});
			} else {
				const auto content = ParseFirstMessageContent(body).trimmed();
				if (content.isEmpty()) {
					done({
						.error = QStringLiteral("Empty content: ")
							+ Truncate(body),
						.durationMs = duration,
						.httpCode = status,
					});
				} else {
					done({
						.data = content,
						.durationMs = duration,
						.httpCode = status,
					});
				}
			}
		});
}

void Client::fetchModels(
		const QString &baseUrl,
		const QString &apiKey,
		std::function<void(Response<QStringList>)> done) {
	if (const auto error = Validate(baseUrl, apiKey); !error.isEmpty()) {
		done(Response<QStringList>{ .error = error });
		return;
	}
	const auto url = baseUrl.trimmed();
	const auto started = QDateTime::currentMSecsSinceEpoch();
	[[maybe_unused]] const auto reply = sendHttp(
		_nam,
		MakeRequest(url + u"/models"_q, apiKey),
		QByteArray(),
		false,
		20000,
		[=](int status, const QByteArray &body) {
			const auto duration = QDateTime::currentMSecsSinceEpoch() - started;
			if (!status) {
				done({
					.error = QStringLiteral("Network error"),
					.durationMs = duration,
				});
				return;
			} else if (!httpOk(status)) {
				done({
					.error = HttpError(status, body),
					.durationMs = duration,
					.httpCode = status,
				});
				return;
			}
			auto models = ParseModelIds(body);
			if (url.contains(
					QStringLiteral("generativelanguage.googleapis.com"),
					Qt::CaseInsensitive)) {
				models = ModelUtil::StripModelsPrefix(models);
			}
			if (models.isEmpty()) {
				done({
					.error = QStringLiteral("No models found: ")
						+ Truncate(body),
					.durationMs = duration,
					.httpCode = status,
				});
				return;
			}
			done({
				.data = models,
				.durationMs = duration,
				.httpCode = status,
			});
		});
}

void Client::testChatCompletions(
		const QString &baseUrl,
		const QString &apiKey,
		const QString &model,
		std::function<void(Response<QString>)> done) {
	const auto modelName = model.trimmed();
	if (modelName.isEmpty()) {
		done(Response<QString>{ .error = QStringLiteral("Model is empty") });
		return;
	}
	auto message = QJsonObject();
	message.insert(QStringLiteral("role"), QStringLiteral("user"));
	message.insert(
		QStringLiteral("content"),
		QStringLiteral("This is a test. Reply with a single word: OK"));
	auto request = QJsonObject();
	request.insert(QStringLiteral("model"), modelName);
	request.insert(QStringLiteral("messages"), QJsonArray{ message });
	ModelUtil::ApplyReasoningParameters(request, baseUrl, modelName);

	[[maybe_unused]] const auto reply = chatCompletions(
		baseUrl,
		apiKey,
		QJsonDocument(request).toJson(QJsonDocument::Compact),
		[=](Response<QString> response) {
			if (response.ok()) {
				response.data = ModelUtil::SanitizeResponse(
					modelName,
					response.data);
			}
			done(std::move(response));
		},
		20000);
}

} // namespace Ayu::Translator::Llm
