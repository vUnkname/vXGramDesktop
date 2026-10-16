// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/features/translator/implementations/http.h"

#include "ayu/features/translator/html_parser.h"

#include <QtCore/QStringList>
#include <QtCore/QTimer>

namespace Ayu::Translator {

QPointer<QNetworkReply> sendHttp(
		QNetworkAccessManager &nam,
		const QNetworkRequest &request,
		const QByteArray &body,
		bool post,
		int timeoutMs,
		HttpDone done) {
	QPointer<QNetworkReply> reply = post
		? nam.post(request, body)
		: nam.get(request);

	const auto timer = new QTimer(reply);
	timer->setSingleShot(true);
	timer->setInterval(timeoutMs);
	QObject::connect(timer, &QTimer::timeout, reply, [reply] {
		if (reply && reply->isRunning()) {
			reply->abort();
		}
	});
	timer->start();

	QObject::connect(reply, &QNetworkReply::finished, reply, [
		reply,
		timer,
		done = std::move(done)
	] {
		if (!reply) {
			return;
		}
		timer->stop();
		const auto status = reply->attribute(
			QNetworkRequest::HttpStatusCodeAttribute).toInt();
		const auto failed = (reply->error() != QNetworkReply::NoError)
			&& (status == 0);
		const auto data = reply->readAll();
		reply->deleteLater();
		if (done) {
			done(failed ? 0 : status, data);
		}
	});
	return reply;
}

bool languageInList(const QString &lang, const QStringList &list) {
	for (const auto &item : list) {
		if (!item.compare(lang, Qt::CaseInsensitive)) {
			return true;
		}
	}
	return false;
}

QString prepareText(const TextWithEntities &text) {
	return shouldWrapInHtml() ? Html::entitiesToHtml(text) : text.text;
}

TextWithEntities finishText(const QString &translated) {
	return shouldWrapInHtml()
		? Html::htmlToEntities(translated)
		: TextWithEntities{ translated };
}

} // namespace Ayu::Translator
