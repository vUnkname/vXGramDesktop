// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/features/translator/implementations/deepl.h"

#include "ayu/ayu_settings.h"
#include "base/random.h"

#include <QtCore/QDateTime>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QUrlQuery>

namespace Ayu::Translator {
namespace {

constexpr auto kFreeUrl = "https://api-free.deepl.com/v2/translate";
constexpr auto kProUrl = "https://api.deepl.com/v2/translate";
constexpr auto kWebUrl = "https://www2.deepl.com/jsonrpc?client=chrome-extension,1.28.0";

[[nodiscard]] qint64 TimestampFor(const QString &text) {
	const auto now = QDateTime::currentMSecsSinceEpoch();
	const auto count = text.count(QChar('i'));
	if (!count) {
		return now;
	}
	return now - (now % (count + 1)) + (count + 1);
}

} // namespace

DeepLTranslator &DeepLTranslator::instance() {
	static DeepLTranslator inst;
	return inst;
}

QPointer<QNetworkReply> DeepLTranslator::startSingleTranslation(
		const MultiThreadArgs &args) {
	const auto key = AyuSettings::getInstance().deepLTranslateKey().trimmed();
	return key.isEmpty() ? translateWeb(args) : translateWithKey(args, key);
}

QPointer<QNetworkReply> DeepLTranslator::translateWithKey(
		const MultiThreadArgs &args,
		const QString &key) {
	const auto onSuccess = args.onSuccess;
	const auto onFail = args.onFail;
	const auto to = args.parsedData.toLang.trimmed().toUpper();
	const auto from = args.parsedData.fromLang.trimmed();
	const auto query = prepareText(args.parsedData.text);
	if (query.isEmpty() || to.isEmpty()) {
		if (onFail) onFail();
		return nullptr;
	}

	auto form = QUrlQuery();
	form.addQueryItem(u"text"_q, query);
	form.addQueryItem(u"target_lang"_q, to);
	if (!from.isEmpty() && (from != u"auto"_q)) {
		form.addQueryItem(u"source_lang"_q, from.toUpper());
	}

	const auto url = key.endsWith(u":fx"_q) ? kFreeUrl : kProUrl;
	auto request = QNetworkRequest(QUrl(QString::fromLatin1(url)));
	request.setRawHeader(
		"Authorization",
		QByteArray("DeepL-Auth-Key ") + key.toUtf8());
	request.setHeader(
		QNetworkRequest::ContentTypeHeader,
		u"application/x-www-form-urlencoded"_q);

	return sendHttp(
		_nam,
		request,
		form.toString(QUrl::FullyEncoded).toUtf8(),
		true,
		20000,
		[=](int status, const QByteArray &body) {
			if (!httpOk(status)) {
				if (onFail) onFail();
				return;
			}
			const auto list = QJsonDocument::fromJson(body).object()
				.value(u"translations"_q).toArray();
			if (list.isEmpty()) {
				if (onFail) onFail();
				return;
			}
			const auto text = list.at(0).toObject()
				.value(u"text"_q).toString();
			if (onSuccess) onSuccess(finishText(text));
		});
}

QPointer<QNetworkReply> DeepLTranslator::translateWeb(
		const MultiThreadArgs &args) {
	const auto onSuccess = args.onSuccess;
	const auto onFail = args.onFail;
	const auto to = args.parsedData.toLang.trimmed().toUpper();
	const auto query = prepareText(args.parsedData.text);
	if (query.isEmpty() || to.isEmpty()) {
		if (onFail) onFail();
		return nullptr;
	}

	if (!_rpcId) {
		_rpcId = (qint64(base::RandomIndex(99999)) + 8300000) * 1000;
	}
	const auto id = ++_rpcId;

	auto lang = QJsonObject();
	lang.insert(u"source_lang_user_selected"_q, u"auto"_q);
	lang.insert(u"target_lang"_q, to);
	auto textItem = QJsonObject();
	textItem.insert(u"text"_q, query);
	textItem.insert(u"requestAlternatives"_q, 0);
	auto params = QJsonObject();
	params.insert(u"splitting"_q, u"newlines"_q);
	params.insert(u"lang"_q, lang);
	params.insert(u"texts"_q, QJsonArray{ textItem });
	params.insert(u"timestamp"_q, TimestampFor(query));
	auto payload = QJsonObject();
	payload.insert(u"jsonrpc"_q, u"2.0"_q);
	payload.insert(u"method"_q, u"LMT_handle_texts"_q);
	payload.insert(u"params"_q, params);
	payload.insert(u"id"_q, id);

	auto json = QJsonDocument(payload).toJson(QJsonDocument::Compact);
	if ((((id + 3) % 13) == 0) || (((id + 5) % 29) == 0)) {
		json.replace("\"method\":\"", "\"method\" : \"");
	}

	auto request = QNetworkRequest(QUrl(QString::fromLatin1(kWebUrl)));
	request.setHeader(
		QNetworkRequest::ContentTypeHeader,
		u"application/json"_q);
	request.setRawHeader("Accept", "*/*");
	request.setRawHeader("x-app-os-name", "iOS");
	request.setRawHeader("x-app-os-version", "16.3.0");
	request.setRawHeader("x-app-device", "iPhone13,2");
	request.setRawHeader("x-app-build", "510265");
	request.setRawHeader("x-app-version", "2.9.1");
	request.setRawHeader("User-Agent", "DeepL/1627620 CFNetwork/1403.2 Darwin/22.1.0");

	return sendHttp(
		_nam,
		request,
		json,
		true,
		20000,
		[=](int status, const QByteArray &body) {
			if (!httpOk(status)) {
				if (onFail) onFail();
				return;
			}
			const auto result = QJsonDocument::fromJson(body).object()
				.value(u"result"_q).toObject();
			const auto texts = result.value(u"texts"_q).toArray();
			if (texts.isEmpty()) {
				if (onFail) onFail();
				return;
			}
			const auto text = texts.at(0).toObject()
				.value(u"text"_q).toString();
			if (text.isEmpty()) {
				if (onFail) onFail();
				return;
			}
			if (onSuccess) onSuccess(finishText(text));
		});
}

}
