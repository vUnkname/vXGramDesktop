// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/features/translator/implementations/google_app.h"

#include "ayu/ayu_settings.h"
#include "ayu/features/translator/implementations/google.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>

namespace Ayu::Translator {
namespace {

constexpr auto kCloudUrl = "https://translation.googleapis.com/language/translate/v2";
constexpr auto kLegacyUrl = "https://translate.google.com/translate_a/single";
constexpr auto kLegacyUserAgent = "GoogleTranslate/6.14.0.04.343003216 (Linux; U; Android 10; Redmi K20 Pro)";

[[nodiscard]] QString FromLang(const MultiThreadArgs &args) {
	const auto from = args.parsedData.fromLang.trimmed();
	return from.isEmpty() ? QStringLiteral("auto") : from;
}

} // namespace

GoogleAppTranslator &GoogleAppTranslator::instance() {
	static GoogleAppTranslator inst;
	return inst;
}

QPointer<QNetworkReply> GoogleAppTranslator::startSingleTranslation(
		const MultiThreadArgs &args) {
	const auto &settings = AyuSettings::getInstance();
	if (settings.googleTranslateExp()) {
		return GoogleTranslator::instance().startSingleTranslation(args);
	}
	const auto key = settings.googleCloudTranslateKey().trimmed();
	if (!key.isEmpty()) {
		return translateCloud(args, key);
	}
	return translateLegacy(args);
}

QPointer<QNetworkReply> GoogleAppTranslator::translateCloud(
		const MultiThreadArgs &args,
		const QString &key) {
	const auto onSuccess = args.onSuccess;
	const auto onFail = args.onFail;
	const auto to = args.parsedData.toLang.trimmed();
	const auto from = FromLang(args);
	const auto query = prepareText(args.parsedData.text);
	if (query.isEmpty() || to.isEmpty()) {
		if (onFail) onFail();
		return nullptr;
	}

	auto form = QUrlQuery();
	form.addQueryItem(u"q"_q, query);
	form.addQueryItem(u"target"_q, to);
	form.addQueryItem(u"format"_q, u"text"_q);
	form.addQueryItem(u"key"_q, key);
	if (from != u"auto"_q) {
		form.addQueryItem(u"source"_q, from);
	}

	auto request = QNetworkRequest(QUrl(QString::fromLatin1(kCloudUrl)));
	request.setHeader(
		QNetworkRequest::ContentTypeHeader,
		u"application/x-www-form-urlencoded"_q);

	return sendHttp(
		_nam,
		request,
		form.toString(QUrl::FullyEncoded).toUtf8(),
		true,
		15000,
		[=](int status, const QByteArray &body) {
			if (!httpOk(status)) {
				if (onFail) onFail();
				return;
			}
			const auto data = QJsonDocument::fromJson(body).object()
				.value(u"data"_q).toObject();
			const auto list = data.value(u"translations"_q).toArray();
			if (list.isEmpty()) {
				if (onFail) onFail();
				return;
			}
			const auto text = list.at(0).toObject()
				.value(u"translatedText"_q).toString();
			if (onSuccess) onSuccess(finishText(text));
		});
}

QPointer<QNetworkReply> GoogleAppTranslator::translateLegacy(
		const MultiThreadArgs &args) {
	const auto onSuccess = args.onSuccess;
	const auto onFail = args.onFail;
	const auto to = args.parsedData.toLang.trimmed();
	const auto query = prepareText(args.parsedData.text);
	if (query.isEmpty() || to.isEmpty()) {
		if (onFail) onFail();
		return nullptr;
	}

	auto url = QUrl(QString::fromLatin1(kLegacyUrl));
	auto params = QUrlQuery();
	params.addQueryItem(u"dj"_q, u"1"_q);
	params.addQueryItem(u"q"_q, query);
	params.addQueryItem(u"sl"_q, u"auto"_q);
	params.addQueryItem(u"tl"_q, to);
	params.addQueryItem(u"ie"_q, u"UTF-8"_q);
	params.addQueryItem(u"oe"_q, u"UTF-8"_q);
	params.addQueryItem(u"client"_q, u"at"_q);
	params.addQueryItem(u"dt"_q, u"t"_q);
	params.addQueryItem(u"otf"_q, u"2"_q);
	url.setQuery(params);

	auto request = QNetworkRequest(url);
	request.setRawHeader("User-Agent", kLegacyUserAgent);

	return sendHttp(
		_nam,
		request,
		QByteArray(),
		false,
		15000,
		[=](int status, const QByteArray &body) {
			if (!httpOk(status)) {
				if (onFail) onFail();
				return;
			}
			const auto sentences = QJsonDocument::fromJson(body).object()
				.value(u"sentences"_q).toArray();
			auto text = QString();
			for (const auto &sentence : sentences) {
				text += sentence.toObject().value(u"trans"_q).toString();
			}
			if (text.isEmpty()) {
				if (onFail) onFail();
				return;
			}
			if (onSuccess) onSuccess(finishText(text));
		});
}

}
