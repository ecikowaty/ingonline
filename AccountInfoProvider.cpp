#include <QtNetwork/QNetworkCookie>
#include <QtNetwork/QNetworkCookieJar>
#include <QUrlQuery>
#include <QDebug>
#include "AccountInfoProvider.hpp"

namespace ingonline
{

	namespace
	{
		bool passSet = false;
		const QByteArray COOKIE_CONTENT = "rc=2; IBLogin=krzpot2039; Expires=Thu, 01 Dec 1994 16:00:00 GMT; __gfp_64b=UvlgYl.2PeoAU_pYQd_3tXKHxx4cbKS.HxSQwH7PlpT.77; __utma=254365776.390859550.1349794927.1372155659.1373980249.127; __utmc=254365776; __utmz=254365776.1349794927.1.1.utmcsr=(direct)|utmccn=(direct)|utmcmd=(none); LtpaToken2=mbxz//dgFj17s6enAvxrNQhzJGOC7lgk5XMl11WMwfATCsAgcP5EVjGPcUJOVPEZuQVBdGEl5MDkHm31QnDhqvzTbf+YlzxvM00CjgdMTmOKZhtayrgA6Y+moy/W7zzjAcoultLvfkRzEzKbTiKZSNYLTHkhAWYyajJTfSWnWy9QzO1213vu016uKEDby1w+HJDh0i6Unyqm5XUp5yuqKjfoD1RNWm3r8lsIBPFbRETGR7fPi3GZxVuKQ+uYVdNx46FqcyFe0uKQok9qzk+wy8fM1PZWu5QcWUvqYnISH1WppEEWQTGRhqhCEVjtIF2X3rXSy1+g/WeocnJCBnJecttH5CblqGloqPrH7CLSo1hboAiD6EEiP8ASi552T9cOzULaMC1z+9BV4eYO7e8uhjAHbunbJXB5qxmceImBPBICT+IEBWXS4OSpodZLcqx7tMN/A8tR+9TAF/QqyVRi+8LVxPHRQOE+zL8+/NtcSxn0XofGgPAuym24peaQixNKrPStZH9zapcTYAGyammMp9bVnvslXSBTeKSnRttKYxpDzlXEStyQVBUZGXZ+M1Fkt2Nbyqk7ZwRBAvmu/oLmpg==; JSESSIONID=00009XxhjxLOYCZv951BTC4i-Wv:roz_lx11ci_wolf6";
	}

	AccountInfoProvider::AccountInfoProvider()
	{
	}

	void AccountInfoProvider::login(const QString& username)
	{
		networkManager.setCookieJar(new QNetworkCookieJar(&networkManager));
		connect(&networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(readReply(QNetworkReply*)));

		QNetworkRequest request;
		request.setUrl(QUrl("https://online.ingbank.pl/mobi/login.html"));
		request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
		request.setRawHeader("Cookie", COOKIE_CONTENT);

		QUrlQuery login;
		login.addQueryItem("cmd", "passMask");
		login.addQueryItem("login", username);
//		login.addQueryItem("action:login", "");

		networkManager.post(request, login.query(QUrl::FullyEncoded).toUtf8());
	}

	void AccountInfoProvider::password(const QString& password)
	{
		qDebug() << "settings password:" << password;

		QList<QNetworkCookie> cookies = networkManager.cookieJar()->cookiesForUrl(QUrl("https://online.ingbank.pl/mobi/login.html"));
		QVariant var;
		var.setValue(cookies);

		QNetworkRequest request;
		request.setUrl(QUrl("https://online.ingbank.pl/mobi/login.html#"));
		request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
		request.setHeader(QNetworkRequest::CookieHeader, var);

		QUrlQuery login;

		networkManager.post(request, login.query(QUrl::FullyEncoded).toUtf8());
	}

	void AccountInfoProvider::readReply(QNetworkReply *reply)
	{
		if(reply->error() != QNetworkReply::NoError)
		{
			qCritical() << tr("Error while downloading information!\n%1").arg(reply->errorString());
			return;
		}

		if (!passSet)
		{
			passSet = true;
		}

		emit dataReceived(QString(reply->readAll()));

		qDebug() << "Got reply"/* << reply->readAll()*/;
	}

	void AccountInfoProvider::handleError(QNetworkReply::NetworkError error)
	{
		qCritical() << "network error:" << error;
	}

}
