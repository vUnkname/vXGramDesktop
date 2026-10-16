// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/features/translator/implementations/transmart.h"

#include "base/random.h"

#include <QtCore/QDateTime>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QStringList>
#include <QtCore/QUuid>

namespace Ayu::Translator {
namespace {

constexpr auto kUrl = "https://transmart.qq.com/api/imt";
constexpr auto kUserAgent = "Mozilla/5.0 (iPhone; CPU iPhone OS 10_0 like Mac OS X) AppleWebKit/602.1.38 (KHTML, like Gecko) Version/10.0 Mobile/14A5297c Safari/602.1";

[[nodiscard]] const QStringList &TargetLanguages() {
	static const auto result = QStringList{
		u"ar"_q, u"fr"_q, u"fil"_q, u"lo"_q, u"ja"_q, u"it"_q, u"hi"_q,
		u"id"_q, u"vi"_q, u"de"_q, u"km"_q, u"ms"_q, u"th"_q, u"tr"_q,
		u"zh"_q, u"ru"_q, u"ko"_q, u"pt"_q, u"es"_q,
	};
	return result;
}

[[nodiscard]] QString ClientKey() {
	const auto version = QStringLiteral("%1.%2.%3").arg(
		100 + base::RandomIndex(17)).arg(
		base::RandomIndex(20)).arg(
		base::RandomIndex(20));
	const auto os = base::RandomIndex(2) ? u"Windows"_q : u"Mac OS"_q;
	return QStringLiteral("browser-chrome-%1-%2-%3-%4").arg(
		version,
		os,
		QUuid::createUuid().toString(QUuid::WithoutBraces),
		QString::number(QDateTime::currentMSecsSinceEpoch()));
}

} // namespace

TranSmartTranslator &TranSmartTranslator::instance() {
	static TranSmartTranslator inst;
	return inst;
}

QPointer<QNetworkReply> TranSmartTranslator::startSingleTranslation(
		const MultiThreadArgs &args) {
	const auto onSuccess = args.onSuccess;
	const auto onFail = args.onFail;
	const auto to = args.parsedData.toLang.trimmed();
	const auto from = args.parsedData.fromLang.trimmed();
	const auto query = prepareText(args.parsedData.text);

	if (query.isEmpty() || !languageInList(to, TargetLanguages())) {
		if (onFail) onFail();
		return nullptr;
	}

	auto lines = QJsonArray();
	for (const auto &line : query.split('\n')) {
		lines.append(line);
	}
	auto header = QJsonObject();
	header.insert(u"client_key"_q, ClientKey());
	header.insert(u"fn"_q, u"auto_translation"_q);
	header.insert(u"session"_q, QString());
	header.insert(u"user"_q, QString());
	auto source = QJsonObject();
	source.insert(
		u"lang"_q,
		languageInList(from, TargetLanguages()) ? from : u"en"_q);
	source.insert(u"text_list"_q, lines);
	auto target = QJsonObject();
	target.insert(u"lang"_q, to);
	auto payload = QJsonObject();
	payload.insert(u"header"_q, header);
	payload.insert(u"source"_q, source);
	payload.insert(u"target"_q, target);
	payload.insert(u"model_category"_q, u"normal"_q);
	payload.insert(u"text_domain"_q, QString());
	payload.insert(u"type"_q, u"plain"_q);

	auto request = QNetworkRequest(QUrl(QString::fromLatin1(kUrl)));
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
			const auto list
				= doc.object().value(u"auto_translation"_q).toArray();
			if (list.isEmpty()) {
				if (onFail) onFail();
				return;
			}
			auto result = QStringList();
			for (const auto &line : list) {
				result.push_back(line.toString());
			}
			if (onSuccess) onSuccess(finishText(result.join('\n')));
		});
}

}