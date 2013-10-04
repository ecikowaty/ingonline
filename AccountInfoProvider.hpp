#ifndef ACCOUNTINFOPROVIDER_HPP
#define ACCOUNTINFOPROVIDER_HPP

#include <QVariant>
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
		HISTORY_REQUESTED,
		AWAITING_HISTORY_PAGE,
		READ_ERROR,
		IDLE
	};

	class INGONLINESHARED_EXPORT AccountInfoProvider: public QObject
	{
		Q_OBJECT

	public:
		typedef QScopedPointer<AccountInfoProvider> Ptr;

		void login(const QString& username, const QString& password);
		void logout();

	public slots:
		void getBalance();
		void getHistory();
		void onHttpReply(QNetworkReply* reply);
		void onHttpError(QNetworkReply::NetworkError error);

	signals:
		void balanceDataUpdated(const QStringList& data);
		void historyDataReceived(const QStringList& data);
		void loginSuccessfull();
		void loginUnsuccessfull();
		void dataReceived(const QString& data);
		void passwordReady(const QString& data);

		void unexpectedError(const QString& message);
		void progressUpdate(const QString& info);

	private:
		QScopedPointer<QNetworkAccessManager>	networkManager;

		LoginState	state = ENTERING_LOGIN;
		QString		username;
		QString		password;

		QVariant	cookie;
	};

}

#endif // ACCOUNTINFOPROVIDER_HPP
