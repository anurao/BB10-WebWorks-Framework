#ifndef PIM_CALENDAR_ACCT_FOLDER_MGR_HPP_
#define PIM_CALENDAR_ACCT_FOLDER_MGR_HPP_

#include <json/value.h>
#include <pthread.h>
#include <string>
#include <map>
#include <bb/pim/account/Account>
#include <bb/pim/account/AccountService>
#include <bb/pim/calendar/CalendarFolder>
#include <bb/pim/calendar/CalendarService>

//#include "service_provider.hpp"

namespace bbpim = bb::pim::calendar;
namespace bbpimAccount = bb::pim::account;
/*
struct AccountInfo {
	bool supportsInfiniteRecurrence;
	bool supportsParticipants;
	bool supportsMessaging;
	bool enterprise;
};
*/
class AccountFolderManager {
public:
	AccountFolderManager(/*ServiceProvider provider*/);
	bbpimAccount::Account GetAccount(bbpim::AccountId accountId);
	bbpimAccount::Account GetDefaultAccount();
	QList<bbpimAccount::Account> GetAccounts();
	bbpim::CalendarFolder GetFolder(bbpim::AccountId accountId, bbpim::FolderId folderId);
	bbpim::CalendarFolder GetDefaultFolder();
	QList<bbpim::CalendarFolder> GetFolders();
	QList<bbpim::CalendarFolder> GetFoldersForAccount(bbpim::AccountId accountId);
	bool IsDefaultFolder(const bbpim::CalendarFolder& folder);
	Json::Value GetFolderJson(const bbpim::CalendarFolder& folder, bool skipDefaultCheck = false);
	Json::Value GetAccountJson(const bbpimAccount::Account& account);
	static std::string GetFolderKey(const bbpim::AccountId accountId, const bbpim::FolderId);

private:
	bbpim::CalendarService* getCalendarService();
	bbpimAccount::AccountService* getAccountService();
	static std::string intToStr(const int val);
	void fetchAccounts();
	void fetchFolders();
	void fetchDefaultAccount();
	void fetchDefaultFolder();

	bbpim::CalendarService* m_calendarService;
	bbpimAccount::AccountService* m_accountService;

	std::map<std::string, bbpim::CalendarFolder> m_foldersMap;
	std::map<bbpim::AccountId, bbpimAccount::Account> m_accountsMap;
	//std::map<bbpim::AccountId, AccountInfo> m_accountInfoMap;
	bbpimAccount::Account m_defaultAccount;
	bbpim::CalendarFolder m_defaultFolder;
	//ServiceProvider m_provider;
	static pthread_mutex_t m_lock;
};

#endif // PIM_CALENDAR_ACCT_FOLDER_MGR_HPP_