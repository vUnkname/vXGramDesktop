// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/features/translator/implementations/microsoft.h"

#include <QtCore/QDateTime>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QRegularExpression>
#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>

namespace Ayu::Translator {
namespace {

constexpr auto kUserAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/122.0.0.0 Safari/537.36 Edg/122.0.0.0";
constexpr auto kBingPage = "https://www.bing.com/translator";
constexpr auto kBingApi = "https://www.bing.com/ttranslatev3";
constexpr auto kEdgeAuth = "https://edge.microsoft.com/translate/auth";
constexpr auto kEdgeApi = "https://api.cognitive.microsofttranslator.com/translate";
constexpr auto kEdgeTokenExpiry = qint64(10 * 60 * 1000);
constexpr auto kTextLimit = 3000;

[[nodiscard]] const QStringList &TargetLanguages() {
	static const auto result = QStringList{
		u"af"_q, u"sq"_q, u"am"_q, u"ar"_q, u"az"_q, u"bn"_q, u"bs"_q,
		u"bg"_q, u"ca"_q, u"zh-Hans"_q, u"zh-Hant"_q, u"hr"_q, u"cs"_q,
		u"da"_q, u"nl"_q, u"en"_q, u"et"_q, u"fil"_q, u"fi"_q, u"fr"_q,
		u"de"_q, u"el"_q, u"gu"_q, u"ht"_q, u"he"_q, u"hi"_q, u"hu"_q,
		u"is"_q, u"id"_q, u"it"_q, u"ja"_q, u"kn"_q, u"kk"_q, u"km"_q,
		u"ko"_q, u"ku"_q, u"lo"_q, u"lv"_q, u"lt"_q, u"mg"_q, u"ms"_q,
		u"ml"_q, u"mt"_q, u"mr"_q, u"ne"_q, u"nb"_q, u"fa"_q, u"pl"_q,
		u"pt"_q, u"ro"_q, u"ru"_q, u"sr-Cyrl"_q, u"sr-Latn"_q, u"sk"_q,
		u"sl"_q, u"es"_q, u"sw"_q, u"sv"_q, u"ta"_q, u"te"_q, u"th"_q,
		u"tr"_q, u"uk"_q, u"ur"_q, u"vi"_q,
	};
	return result;
}

[[nodiscard]] QString MapTarget(const QString &to) {
	const auto lower = to.toLower();
	if ((lower == u"zh"_q) || (lower == u"zh-cn"_q)) {
		return u"zh-Hans"_q;
	} else if (lower == u"zh-tw"_q) {
		return u"zh-Hant"_q;
	}
	return to;
}

[[nodiscard]] QString ExtractValue(
		const QString &html,
		const QString &pattern) {
	const auto match = QRegularExpression(pattern).match(html);
	return match.hasMatch() ? match.captured(0) : QString();
}

[[nodiscard]] QString ArrayPart(const QString &array, int index) {
	if (array.size() < 2) {
		return {};
	}
	const auto parts = array.mid(1, array.size() - 2).split(',');
	if (index >= parts.size()) {
		return {};
	}
	return QString(parts[index]).remove('"').trimmed();
}

} // namespace

// ---- Bing ------------------------------------------------------------

MicrosoftTranslator &MicrosoftTranslator::instance() {
	static MicrosoftTranslator inst;
	return inst;
}

bool MicrosoftTranslator::configExpired() const {
	return _ig.isEmpty()
		|| _iid.isEmpty()
		|| _key.isEmpty()
		|| _token.isEmpty()
		|| ((QDateTime::currentMSecsSinceEpoch() - _tokenTs) > _tokenInterval);
}

QPointer<QNetworkReply> MicrosoftTranslator::fetchConfig(Fn<void(bool)> done) {
	auto request = QNetworkRequest(QUrl(QString::fromLatin1(kBingPage)));
	request.setRawHeader("User-Agent", kUserAgent);
	return sendHttp(
		_nam,
		request,
		QByteArray(),
		false,
		10000,
		[=](int status, const QByteArray &body) {
			if (!httpOk(status)) {
				done(false);
				return;
			}
			const auto html = QString::fromUtf8(body);
			_ig = ExtractValue(html, u"(?<=IG:\")[^\"]*"_q);
			_iid = ExtractValue(html, u"(?<=data-iid=\")[^\"]*"_q);
			const auto params = ExtractValue(
				html,
				u"(?<=params_AbusePreventionHelper = )\\[[^\\]]+\\]"_q);
			_key = ArrayPart(params, 0);
			_token = ArrayPart(params, 1);
			_tokenTs = _key.toLongLong();
			_tokenInterval = ArrayPart(params, 2).toLongLong();
			done(!configExpired());
		});
}

QPointer<QNetworkReply> MicrosoftTranslator::perform(
		const MultiThreadArgs &args,
		const QString &from,
		const QString &to,
		const QString &query) {
	const auto onSuccess = args.onSuccess;
	const auto onFail = args.onFail;

	const auto iid = _iid + '.' + QString::number(++_count);
	auto url = QUrl(QString::fromLatin1(kBingApi));
	auto params = QUrlQuery();
	params.addQueryItem(u"isVertical"_q, u"1"_q);
	params.addQueryItem(u"IG"_q, _ig);
	params.addQueryItem(u"IID"_q, iid);
	params.addQueryItem(u"ref"_q, u"TThis"_q);
	params.addQueryItem(u"edgepdftranslator"_q, u"1"_q);
	url.setQuery(params);

	auto form = QUrlQuery();
	form.addQueryItem(u"fromLang"_q, from);
	form.addQueryItem(u"to"_q, to);
	form.addQueryItem(u"text"_q, query);
	form.addQueryItem(u"token"_q, _token);
	form.addQueryItem(u"key"_q, _key);
	form.addQueryItem(u"tryFetchingGenderDebiasedTranslations"_q, u"true"_q);

	auto request = QNetworkRequest(url);
	request.setRawHeader("User-Agent", kUserAgent);
	request.setRawHeader("Referer", kBingPage);
	request.setHeader(
		QNetworkRequest::ContentTypeHeader,
		u"application/x-www-form-urlencoded"_q);

	return sendHttp(
		_nam,
		request,
		form.toString(QUrl::FullyEncoded).toUtf8(),
		true,
		10000,
		[=](int status, const QByteArray &body) {
			if (!httpOk(status)) {
				if (onFail) onFail();
				return;
			}
			const auto root = QJsonDocument::fromJson(body).array();
			const auto translations = root.isEmpty()
				? QJsonArray()
				: root.at(0).toObject().value(u"translations"_q).toArray();
			if (translations.isEmpty()) {
				if (onFail) onFail();
				return;
			}
			const auto text = translations.at(0).toObject()
				.value(u"text"_q).toString();
			if (onSuccess) onSuccess(finishText(text));
		});
}

QPointer<QNetworkReply> MicrosoftTranslator::startSingleTranslation(
		const MultiThreadArgs &args) {
	const auto onFail = args.onFail;
	const auto fromRaw = args.parsedData.fromLang.trimmed();
	const auto from = (fromRaw.isEmpty() || (fromRaw == u"auto"_q))
		? u"auto-detect"_q
		: fromRaw;
	const auto to = MapTarget(args.parsedData.toLang.trimmed());
	const auto query = prepareText(args.parsedData.text);

	if (query.isEmpty()
		|| (query.size() > kTextLimit)
		|| !languageInList(to, TargetLanguages())) {
		if (onFail) onFail();
		return nullptr;
	}
	if (!configExpired()) {
		return perform(args, from, to, query);
	}
	return fetchConfig([=](bool ok) {
		if (!ok) {
			if (onFail) onFail();
			return;
		}
		// The reply of the nested request cannot be tracked here, the
		// batch state guards against late callbacks.
		[[maybe_unused]] const auto reply = perform(args, from, to, query);
	});
}

// ---- Edge ------------------------------------------------------------

RealMicrosoftTranslator &RealMicrosoftTranslator::instance() {
	static RealMicrosoftTranslator inst;
	return inst;
}

QPointer<QNetworkReply> RealMicrosoftTranslator::perform(
		const MultiThreadArgs &args,
		const QString &from,
		const QString &to,
		const QString &query) {
	const auto onSuccess = args.onSuccess;
	const auto onFail = args.onFail;

	auto url = QUrl(QString::fromLatin1(kEdgeApi));
	auto params = QUrlQuery();
	params.addQueryItem(u"api-version"_q, u"3.0"_q);
	params.addQueryItem(u"to"_q, to);
	if (!from.isEmpty()) {
		params.addQueryItem(u"from"_q, from);
	}
	url.setQuery(params);

	auto item = QJsonObject();
	item.insert(u"Text"_q, query);
	const auto body = QJsonDocument(QJsonArray{ item }).toJson(
		QJsonDocument::Compact);

	auto request = QNetworkRequest(url);
	request.setRawHeader("User-Agent", kUserAgent);
	request.setRawHeader("Authorization", "Bearer " + _token.toUtf8());
	request.setHeader(
		QNetworkRequest::ContentTypeHeader,
		u"application/json"_q);

	return sendHttp(
		_nam,
		request,
		body,
		true,
		15000,
		[=](int status, const QByteArray &response) {
			if (!httpOk(status)) {
				if (status == 401) {
					_token.clear();
				}
				if (onFail) onFail();
				return;
			}
			const auto root = QJsonDocument::fromJson(response).array();
			const auto translations = root.isEmpty()
				? QJsonArray()
				: root.at(0).toObject().value(u"translations"_q).toArray();
			if (translations.isEmpty()) {
				if (onFail) onFail();
				return;
			}
			const auto text = translations.at(0).toObject()
				.value(u"text"_q).toString();
			if (onSuccess) onSuccess(finishText(text));
		});
}

QPointer<QNetworkReply> RealMicrosoftTranslator::startSingleTranslation(
		const MultiThreadArgs &args) {
	const auto onFail = args.onFail;
	const auto fromRaw = args.parsedData.fromLang.trimmed();
	const auto from = (fromRaw == u"auto"_q) ? QString() : fromRaw;
	const auto to = MapTarget(args.parsedData.toLang.trimmed());
	const auto query = prepareText(args.parsedData.text);

	if (query.isEmpty() || !languageInList(to, TargetLanguages())) {
		if (onFail) onFail();
		return nullptr;
	}
	const auto now = QDateTime::currentMSecsSinceEpoch();
	if (!_token.isEmpty() && ((now - _tokenTs) < kEdgeTokenExpiry)) {
		return perform(args, from, to, query);
	}

	auto request = QNetworkRequest(QUrl(QString::fromLatin1(kEdgeAuth)));
	request.setRawHeader("User-Agent", kUserAgent);
	return sendHttp(
		_nam,
		request,
		QByteArray(),
		false,
		10000,
		[=](int status, const QByteArray &body) {
			if (!httpOk(status) || body.trimmed().isEmpty()) {
				if (onFail) onFail();
				return;
			}
			_token = QString::fromUtf8(body).trimmed();
			_tokenTs = QDateTime::currentMSecsSinceEpoch();
			[[maybe_unused]] const auto reply = perform(args, from, to, query);
		});
}

}
