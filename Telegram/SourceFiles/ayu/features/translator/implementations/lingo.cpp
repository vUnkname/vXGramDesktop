// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/features/translator/implementations/lingo.h"

#include <QtCore/QDateTime>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QStringList>

namespace Ayu::Translator {
namespace {

constexpr auto kUrl = "https://api.interpreter.caiyunai.com/v1/translator";
constexpr auto kToken = "token 9sdftiq37bnv410eon2l";
constexpr auto kUserAgent = "Mozilla/5.0 (iPhone; CPU iPhone OS 10_0 like Mac OS X) AppleWebKit/602.1.38 (KHTML, like Gecko) Version/10.0 Mobile/14A5297c Safari/602.1";

} // namespace

LingoTranslator &LingoTranslator::instance() {
	static LingoTranslator inst;
	return inst;
}

QPointer<QNetworkReply> LingoTranslator::startSingleTranslation(
		const MultiThreadArgs &args) {
	const auto onSuccess = args.onSuccess;
	const auto onFail = args.onFail;
	const auto to = args.parsedData.toLang.trimmed();
	auto from = args.parsedData.fromLang.trimmed();
	if (from.isEmpty()) {
		from = QStringLiteral("auto");
	}
	const auto query = prepareText(args.parsedData.text);

	static const auto supported = QStringList{
		u"zh"_q, u"en"_q, u"es"_q, u"fr"_q, u"ja"_q, u"ru"_q
	};
	if (query.isEmpty() || !languageInList(to, supported)) {
		if (onFail) onFail();
		return nullptr;
	}

	auto source = QJsonArray();
	for (const auto &line : query.split('\n')) {
		source.append(line);
	}
	auto payload = QJsonObject();
	payload.insert(u"source"_q, source);
	payload.insert(u"trans_type"_q, from + u"2"_q + to);
	payload.insert(
		u"request_id"_q,
		QString::number(QDateTime::currentMSecsSinceEpoch()));
	payload.insert(u"detect"_q, true);

	auto request = QNetworkRequest(QUrl(QString::fromLatin1(kUrl)));
	request.setRawHeader("X-Authorization", kToken);
	request.setRawHeader("User-Agent", kUserAgent);
	request.setHeader(
		QNetworkRequest::ContentTypeHeader,
		u"application/json"_q);

	return sendHttp(
		_nam,
		request,
		QJsonDocument(payload).toJson(QJsonDocument::Compact),
		true,
		15000,
		[=](int status, const QByteArray &body) {
			if (!httpOk(status)) {
				if (onFail) onFail();
				return;
			}
			const auto doc = QJsonDocument::fromJson(body);
			const auto target = doc.object().value(u"target"_q).toArray();
			if (target.isEmpty()) {
				if (onFail) onFail();
				return;
			}
			auto lines = QStringList();
			for (const auto &line : target) {
				auto text = line.toString();
				if (text == QString(QChar(0xD835))) {
					text.clear();
				}
				lines.push_back(text);
			}
			if (onSuccess) onSuccess(finishText(lines.join('\n')));
		});
}

}