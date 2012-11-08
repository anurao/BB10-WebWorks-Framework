/*
 * Copyright 2012 Research In Motion Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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

#include "service_provider.hpp"
#include "thread_sync.hpp"

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
class AccountFolderManager : public ThreadSync {
public:
	AccountFolderManager(ServiceProvider& provider);
	bbpimAccount::Account GetAccount(bbpim::AccountId accountId, bool fresh = true);
	bbpimAccount::Account GetDefaultAccount(bool fresh = true);
	QList<bbpimAccount::Account> GetAccounts(bool fresh = true);
	bbpim::CalendarFolder GetFolder(bbpim::AccountId accountId, bbpim::FolderId folderId, bool fresh = true);
	bbpim::CalendarFolder GetDefaultFolder(bool fresh = true);
	QList<bbpim::CalendarFolder> GetFolders(bool fresh = true);
	QList<bbpim::CalendarFolder> GetFoldersForAccount(bbpim::AccountId accountId, bool fresh = true);
	bool IsDefaultFolder(const bbpim::CalendarFolder& folder, bool fresh = true);
	Json::Value GetFolderJson(const bbpim::CalendarFolder& folder, bool skipDefaultCheck = false, bool fresh = true);
	Json::Value GetAccountJson(const bbpimAccount::Account& account, bool fresh = true);
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
	ServiceProvider& m_provider;
};

#endif // PIM_CALENDAR_ACCT_FOLDER_MGR_HPP_
