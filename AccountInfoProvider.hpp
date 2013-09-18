#ifndef ACCOUNTINFOPROVIDER_HPP
#define ACCOUNTINFOPROVIDER_HPP

#include <QObject>
#include <QScopedPointer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include "ingonline_global.hpp"

namespace ingonline
{

	class INGONLINESHARED_EXPORT AccountInfoProvider: public QObject
	{
		Q_OBJECT

	public:
		typedef QScopedPointer<AccountInfoProvider> Ptr;

		AccountInfoProvider();

		void login(const QString& username);
		void password(const QString& password);

	public slots:
		void readReply(QNetworkReply* reply);
		void handleError(QNetworkReply::NetworkError error);

	signals:
		void dataReceived(const QString& data);

	private:
		QNetworkAccessManager networkManager;
	};

}

#endif // ACCOUNTINFOPROVIDER_HPP
