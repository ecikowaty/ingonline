#include <QRegExp>
#include <QMessageAuthenticationCode>
#include <QtNetwork/QNetworkCookie>
#include <QtNetwork/QNetworkCookieJar>
#include <QUrlQuery>
#include <QApplication>
#include <QDebug>
#include <QWebView>
#include <QWebFrame>
#include <QWebSettings>
#include <QFile>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QtXml/QDomDocument>
#include <QtXml/QDomNodeList>

#include "AccountInfoProvider.hpp"

namespace ingonline
{

	namespace
	{
		const QString ACCOUNT_NUMBER = "82105015591000009041544249";

		const QString BALANCE_PAGE = "https://online.ingbank.pl/mobi/account/accountInfo.html?new=true";

		const QString HISTORY_PAGE = "https://online.ingbank.pl/mobi/account/transhistory.html?new=true";

		const QByteArray COOKIE_CONTENT = "rc=2; IBLogin=krzpot2039; Expires=Thu, 01 Dec 1994 16:00:00 GMT; __gfp_64b=UvlgYl.2PeoAU_pYQd_3tXKHxx4cbKS.HxSQwH7PlpT.77; __utma=254365776.390859550.1349794927.1372155659.1373980249.127; __utmc=254365776; __utmz=254365776.1349794927.1.1.utmcsr=(direct)|utmccn=(direct)|utmcmd=(none); LtpaToken2=mbxz//dgFj17s6enAvxrNQhzJGOC7lgk5XMl11WMwfATCsAgcP5EVjGPcUJOVPEZuQVBdGEl5MDkHm31QnDhqvzTbf+YlzxvM00CjgdMTmOKZhtayrgA6Y+moy/W7zzjAcoultLvfkRzEzKbTiKZSNYLTHkhAWYyajJTfSWnWy9QzO1213vu016uKEDby1w+HJDh0i6Unyqm5XUp5yuqKjfoD1RNWm3r8lsIBPFbRETGR7fPi3GZxVuKQ+uYVdNx46FqcyFe0uKQok9qzk+wy8fM1PZWu5QcWUvqYnISH1WppEEWQTGRhqhCEVjtIF2X3rXSy1+g/WeocnJCBnJecttH5CblqGloqPrH7CLSo1hboAiD6EEiP8ASi552T9cOzULaMC1z+9BV4eYO7e8uhjAHbunbJXB5qxmceImBPBICT+IEBWXS4OSpodZLcqx7tMN/A8tR+9TAF/QqyVRi+8LVxPHRQOE+zL8+/NtcSxn0XofGgPAuym24peaQixNKrPStZH9zapcTYAGyammMp9bVnvslXSBTeKSnRttKYxpDzlXEStyQVBUZGXZ+M1Fkt2Nbyqk7ZwRBAvmu/oLmpg==; JSESSIONID=00009XxhjxLOYCZv951BTC4i-Wv:roz_lx11ci_wolf6";

		QString findHmacKey(const QString& httpReply)
		{
			QRegExp hmacFunctionInvocation("hex_hmac_sha1\\(\"[0-9]{3,}");

			if (hmacFunctionInvocation.indexIn(httpReply) != -1)
			{
				QRegExp keyValue("[0-9]{3,}");
				if (keyValue.indexIn(hmacFunctionInvocation.cap(0)) != -1)
				{
					return keyValue.cap(0);
				}
			}

			return "";
		}

		QString generatePasswordChars(const QString& httpReply)
		{
			QString passwordChars;
			QRegExp passwordCharsInput("passwordChars\\[[0-9][0-9]?\\] = \".\";");

			int pos = 0;
			while ((pos = passwordCharsInput.indexIn(httpReply, pos)) != -1)
			{
				pos += passwordCharsInput.matchedLength();

				QString value = passwordCharsInput.cap(0);
				passwordChars.append(value.at(value.size() - 3));
			}

			return passwordChars;
		}

		QVector<int> findRequestedLetters(const QString& httpReply)
		{
			QDomDocument html;
			html.setContent(httpReply);
			QDomNodeList inputNodes = html.documentElement().elementsByTagName("input");

			QRegExp letterId("[0-9]+");
			QVector<int> challenge;
			for (int i = 0; i < inputNodes.size(); ++i)
			{
				const QString attributeName = inputNodes.at(i).toElement().attribute("name");
				if (attributeName.contains("passwd"))
				{
					letterId.indexIn(attributeName);
					challenge.append(letterId.cap().toInt());
				}
			}

			return challenge;
		}
	}


	void AccountInfoProvider::login(const QString& username, const QString& password)
	{
		qDebug() << "logging in";
		emit progressUpdate("Logging in");

		this->username = username;
		this->password = password;

		networkManager.reset(new QNetworkAccessManager);
		networkManager->setCookieJar(new QNetworkCookieJar(networkManager.data()));
		connect(networkManager.data(), SIGNAL(finished(QNetworkReply*)), this, SLOT(onHttpReply(QNetworkReply*)));

		QNetworkRequest request;
		request.setUrl(QUrl("https://online.ingbank.pl/mobi/login.html"));
		request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
		request.setRawHeader("Cookie", COOKIE_CONTENT);

		QUrlQuery login;
		login.addQueryItem("cmd", "passMask");
		login.addQueryItem("login", username);
		login.addQueryItem("action:login", "");

		networkManager->post(request, login.query(QUrl::FullyEncoded).toUtf8());

		state = LOGIN_ENTRED;
	}

	void AccountInfoProvider::getBalance()
	{
		emit progressUpdate("Sending balance request");

		QNetworkRequest request;
		request.setUrl(QUrl(BALANCE_PAGE));
		request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
		request.setHeader(QNetworkRequest::CookieHeader, cookie);

		state = BALANCE_CHECK;
		networkManager->get(request);
	}

	void AccountInfoProvider::getHistory()
	{
		emit progressUpdate("Downloading account history");

		QNetworkRequest request;
		request.setUrl(QUrl(HISTORY_PAGE));
		request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
		request.setHeader(QNetworkRequest::CookieHeader, cookie);

		QUrlQuery postData;
		postData.addQueryItem("account", ACCOUNT_NUMBER);
		postData.addQueryItem("accountValue", "KONTO Direct; 0,00 PLN");
		postData.addQueryItem("step", "next");
		postData.addQueryItem("action:transhistory", "");
		postData.addQueryItem("form", "true");

		state = HISTORY_REQUESTED;
		networkManager->post(request, postData.query(QUrl::FullyEncoded).toUtf8());
	}

	void AccountInfoProvider::onHttpReply(QNetworkReply *reply)
	{
		qDebug() << "got reply" << reply->bytesAvailable();
		cookie = reply->header(QNetworkRequest::CookieHeader);

		if(reply->error() != QNetworkReply::NoError)
		{
			qCritical() << tr("Error while downloading information!\n%1").arg(reply->errorString());
			return;
		}

		if (state == LOGIN_ENTRED)
		{
			QString httpReplyContent(reply->readAll());
			emit dataReceived(httpReplyContent);

			QString hmacKey = findHmacKey(httpReplyContent);
			QString passwordChars = generatePasswordChars(httpReplyContent);
			QVector<int>&& passwordChallenge = findRequestedLetters(httpReplyContent);

			qDebug() << "hmac key:" << hmacKey;
			qDebug() << "password chars:" << passwordChars;
			qDebug() << "password challenge:" << passwordChallenge;

			QNetworkRequest request;
			request.setUrl(QUrl("https://online.ingbank.pl/mobi/j_security_check"));
			request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
			request.setHeader(QNetworkRequest::CookieHeader, cookie);

			for (int requestedLetter : passwordChallenge)
			{
				passwordChars[requestedLetter] = password[requestedLetter];
			}

			QFile sha1Javascript(QApplication::applicationDirPath() + "/sha1.html");
			if (sha1Javascript.open(QIODevice::ReadOnly))
			{
				QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
				QWebView webView;
				webView.setHtml(sha1Javascript.readAll());

				QString hmacFormat("hex_hmac_sha1('%1', '%2');");
				QVariant result = webView.page()->mainFrame()->evaluateJavaScript(hmacFormat.arg(hmacKey, passwordChars));
				qDebug() << "hmac value:" << result.toString();

				QUrlQuery login;
				login.addQueryItem("j_username", username);
				login.addQueryItem("j_password1", "e36e1a6b89a6cbfb851d85e78c5527457ce7d024");
				login.addQueryItem("j_password2", result.toString());
				login.addQueryItem("j_password3", "8fb79f14a041123f99f3954da128202d7c8c8b0a");

				networkManager->post(request, login.query(QUrl::FullyEncoded).toUtf8());
			}

			state = PASSWORD_ENTRED;
		}
		else if (state == PASSWORD_ENTRED)
		{
			qDebug() << "location:" << reply->rawHeader("Location");

			if (!reply->rawHeader("Location").contains("err"))
			{
				state = LOGIN_SUCCESSFULL;

				qDebug() << "login successfull";

				emit loginSuccessfull();
				emit progressUpdate("Login successfull");
			}
			else
			{
				qCritical() << "login error";
				state = LOGIN_UNSUCCESSFULL;

				emit progressUpdate("Login unsuccessfull");
			}
		}
		else if (state == BALANCE_CHECK)
		{
			qDebug() << "on balance check";
			emit progressUpdate("Checking balance");

			QDomDocument html;
			html.setContent(reply->readAll());

			QStringList accountsBalance;
			QDomNodeList optionNodes = html.documentElement().elementsByTagName("option");
			for (int i = 0; i < optionNodes.size(); ++i)
			{
				const QString balanceValue = optionNodes.at(i).toElement().firstChild().nodeValue();

				if (!balanceValue.contains("Stara"))
				{
					accountsBalance << balanceValue.split("; ");
				}
			}

			state = IDLE;
			emit balanceDataUpdated(accountsBalance);
		}
		else if (state == HISTORY_REQUESTED)
		{
			qDebug() << "history requested";

			QNetworkRequest request;
			request.setUrl(QUrl(reply->rawHeader("Location")));
			request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");
			request.setHeader(QNetworkRequest::CookieHeader, cookie);

			state = AWAITING_HISTORY_PAGE;
			networkManager->get(request);
		}
		else if (state == AWAITING_HISTORY_PAGE)
		{
			emit progressUpdate("Account history downloaded");
			qDebug() << "history reply received:";

			QString httpReply = reply->readAll();

			httpReply.remove(QRegExp("<[^>]*>"));
			httpReply.replace(QRegExp("\\s{2,}"), "\n");

			QTextStream text(&httpReply);

			QStringList historyEntries;
			while (true)
			{

				QString line = text.readLine();
				if (line.isNull())
				{
					break;
				}

				if (line == "Data transakcji:")
				{
					historyEntries << text.readLine();
				}
				else if (line == "Opis:")
				{
					historyEntries << text.readLine();
				}
				else if (line == "Kwota:")
				{
					QString value;

					line = text.readLine();
					if (line == "-")
					{
						value = "-" + text.readLine();
					}
					else
					{
						value = line;
					}

					value.replace(",", ".");
					value.remove(QRegExp("\\s*"));

					historyEntries << value;
				}
			}

			if (historyEntries.size() % 3 != 0)
			{
				qWarning() << "wrong stringEntries size";
			}

			emit historyDataReceived(historyEntries);

			qDebug() << "done";
		}
	}

	void AccountInfoProvider::onHttpError(QNetworkReply::NetworkError error)
	{
		emit progressUpdate("HTTP error");
		qCritical() << "network error:" << error;
	}

}
