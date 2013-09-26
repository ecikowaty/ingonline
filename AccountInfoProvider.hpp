#ifndef ACCOUNTINFOPROVIDER_HPP
#define ACCOUNTINFOPROVIDER_HPP

#include <QObject>
#include <QScopedPointer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QScopedPointer>
#include "ingonline_global.hpp"

class QStateMachine;

namespace ingonline
{
	enum LoginState
	{
		ENTERING_LOGIN,
		LOGIN_ENTRED,
		PASSWORD_ENTRED,
		LOGIN_SUCCESSFULL,
		LOGIN_UNSUCCESSFULL,
		BALANCE_CHECK,
		BALANCE_UPDATED
	};

	class INGONLINESHARED_EXPORT AccountInfoProvider: public QObject
	{
		Q_OBJECT

	public:
		typedef QScopedPointer<AccountInfoProvider> Ptr;

		void login(const QString& username, const QString& password);
		void logout();
		void getBalance();

	public slots:
		void onHttpReply(QNetworkReply* reply);
		void onHttpError(QNetworkReply::NetworkError error);

	signals:
		void balanceDataUpdated(const QStringList& data);
		void loginSuccessfull();
		void loginUnsuccessfull();
		void dataReceived(const QString& data);
		void passwordReady(const QString& data);

	private:
		QScopedPointer<QNetworkAccessManager>	networkManager;

		LoginState	state = ENTERING_LOGIN;
		QString		password;
	};

}

#endif // ACCOUNTINFOPROVIDER_HPP
