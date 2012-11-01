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

#include <bps/bps.h>
#include <json/reader.h>
#include <json/writer.h>
#include <sstream>
#include <string>
#include <errno.h>
#include <sys/pps.h>
#include <sys/select.h>

#include "ids_js.hpp"

extern "C" {
	void getTokenSuccessCB(ids_request_id_t requestId, const char *token, int paramCount, const ids_token_param_t *params, void *cbData) {
		if( cbData ) {
			IDSEXT *request = (IDSEXT *) cbData;
			
			Json::FastWriter writer;
			Json::Value resultJSON;
		
			resultJSON["requestId"] = requestId;
			resultJSON["token"] = token;

			resultJSON["paramCount"] = paramCount;
			Json::Value tokenParams;
			int i;
			for(i = 0; i < paramCount; i++) {
				tokenParams[i]["name"] = params[i].name;
				tokenParams[i]["value"] = params[i].value;
			}
			resultJSON["tokenParams"] = Json::Value(tokenParams);

			std::string resultStr = writer.write(resultJSON);
			request->NotifyEvent(request->getEventId(), writer.write(resultJSON));
		}
	}

	void clearTokenSuccessCB( ids_request_id_t requestId, bool clear, void* cbData ) {
		if( cbData ) {
			IDSEXT *request = (IDSEXT *) cbData;

			Json::FastWriter writer;
			Json::Value resultJSON;
		
			resultJSON["requestId"] = requestId;
			resultJSON["clear"] = clear;

			std::string resultStr = writer.write(resultJSON);

			request->NotifyEvent(request->getEventId(), resultStr.c_str());
		}
	}
	
		void getPropertiesSuccessCB( ids_request_id_t requestId, int propertyCount, const ids_property_t* properties, void* cbData ) {
		if( cbData ) {
			IDSEXT *request = (IDSEXT *) cbData;

			Json::FastWriter writer;
			Json::Value resultJSON;
		
			resultJSON["requestId"] = requestId;
			resultJSON["propertyCount"] = propertyCount;
			Json::Value userProperties;
		
			int i;
			for(i = 0; i < propertyCount; i++) {
				userProperties[i]["uri"] = properties[i].name;
				userProperties[i]["value"] = properties[i].value;
			}

			resultJSON["userProperties"] = Json::Value(userProperties);

			std::string resultStr = writer.write(resultJSON);
			
			request->NotifyEvent(request->getEventId(), resultStr.c_str());
		}
	}


	void failureCB(ids_request_id_t requestId, ids_result_t result, const char *failureInfo, void *cbData) {
		if( cbData ) {
			IDSEXT *request = (IDSEXT *) cbData;

			Json::FastWriter writer;
			Json::Value resultJSON;

			resultJSON["requestId"] = requestId;
			resultJSON["result"] = result;
			resultJSON["failureInfo"] = ( failureInfo ? failureInfo : "" );
			std::string resultStr = writer.write(resultJSON);
			
			request->NotifyEvent(request->getEventId(), resultStr.c_str());
		}
	}
}


IDSEXT::IDSEXT(const std::string& id) : m_id(id)
{
	if( ids_initialize() != IDS_SUCCESS ) {
		fprintf(stderr, "Unable to initialize IDS library\n");
	}
	maxFds = -1;
	providers = NULL;
	
	pthread_create(&m_thread, NULL, idsEventThread, this );
}

std::string IDSEXT::getEventId()
{
	return event_id;
}

ids_provider_mapping* IDSEXT::getProviders()
{
	return providers;
}

char* onGetObjList()
{
    // Return list of classes in the object
    static char name[] = "IDSEXT";
    return name;
}

JSExt* onCreateObject(const std::string& className, const std::string& id)
{
    // Make sure we are creating the right class
    if (className != "IDSEXT") {
        return 0;
    }

    return new IDSEXT(id);
}

ids_provider_mapping* IDSEXT::getProvider(const std::string& provider)
{
	ids_provider_mapping *current = providers;

	while( current != NULL ) {
		if( provider == current->providerName ) {
			return current;
		} else {
			current = providers->next;
		}
	}
	
	return NULL;
}

void IDSEXT::clearProviders(void)
{
	ids_provider_mapping *current = providers;
	
	while( current != NULL ) {
		providers = current->next;
		if( current->providerName ) free( (char *) current->providerName );
		if( current->provider ) free( (void *) current->provider );
		if( current->next ) free( (void *) current->next );
		current = providers;
	}
}

std::string IDSEXT::InvokeMethod(const std::string& command)
{
    int index = command.find_first_of(" ");

    string strCommand = command.substr(0, index);
    string strParam = command.substr(index + 1, command.length());

    Json::Reader reader;
    Json::Value obj;

    if (strCommand == "getVersion") {
		return GetVersion();
    } else if (strCommand == "registerProvider") {
		return RegisterProvider(strParam);
    } else if (strCommand == "setOption") {
        // parse the JSON
        bool parse = reader.parse(strParam, obj);

        if (!parse) {
            //fprintf(stderr, "%s\n", "error parsing\n");
            return "unable to parse options";
        }

        int option = obj["option"].asInt();
        const std::string value = obj["value"].asString();

		return( SetOption(option, value) );
	} else if (strCommand == "getToken") {

        // parse the JSON
        bool parse = reader.parse(strParam, obj);

        if (!parse) {
            //fprintf(stderr, "%s", "error parsing\n");
            return "unable to parse options";
        }
		event_id = obj["_eventId"].asString();
		std::string provider = obj["provider"].asString();
        std::string tokenType = obj["tokenType"].asString();
        const std::string appliesTo = obj["appliesTo"].asString();

		GetToken(provider, tokenType, appliesTo);
	} else if (strCommand == "clearToken") {
	        // parse the JSON
        bool parse = reader.parse(strParam, obj);

        if (!parse) {
            //fprintf(stderr, "%s", "error parsing\n");
            return "unable to parse options";
        }
		event_id = obj["_eventId"].asString();
		std::string provider = obj["provider"].asString();
        std::string tokenType = obj["tokenType"].asString();
        const std::string appliesTo = obj["appliesTo"].asString();

		ClearToken(provider, tokenType, appliesTo);
	} else if (strCommand == "getProperties") {
        // parse the JSON
        bool parse = reader.parse(strParam, obj);
        if (!parse) {
            //fprintf(stderr, "%s", "error parsing\n");
            return "unable to parse options";
        }
		event_id = obj["_eventId"].asString();
		std::string provider = obj["provider"].asString();
        int numProps = obj["numProps"].asInt();
        const std::string userProps = obj["userProperties"].asString();
		GetProperties(provider, numProps, userProps.c_str());
	}

    return "";
}

bool IDSEXT::CanDelete()
{
    return true;
}

// Notifies JavaScript of an event
void IDSEXT::NotifyEvent(const std::string& eventId, const std::string& event)
{
    std::string eventString = m_id + " ";
    eventString.append(eventId);
    eventString.append(" ");
    eventString.append(event);
    SendPluginEvent(eventString.c_str(), m_pContext);
}

std::string IDSEXT::GetVersion()
{
	ostringstream ver;
	ver << ids_get_version();
	return( ver.str() );
}

std::string IDSEXT::RegisterProvider(const std::string& providerName)
{
	Json::FastWriter writer;
	Json::Value resultJSON;
	
	
	ids_provider_mapping *registeredItem = (ids_provider_mapping *) malloc( sizeof( ids_provider_mapping ) );
	if( !registeredItem ) {
		fprintf(stderr, "Unable to register IDS provider - malloc error\n");
		return "";
	}
	
	registeredItem->providerName = strdup(providerName.c_str());
	resultJSON["result"] = ids_register_provider( registeredItem->providerName, &registeredItem->provider, &registeredItem->providerFd );
	if( (ids_result_t) resultJSON["result"].asInt() == IDS_SUCCESS ) {
		registeredItem->next = providers;
		providers = registeredItem;
		
		// Signal the select in our event thread to break to add a new provider
		write( fileIds[1], "%i\n", registeredItem->providerFd);
		FD_SET( registeredItem->providerFd, &tRfd );

		if( registeredItem->providerFd > maxFds ) maxFds = registeredItem->providerFd;
	} else {
		resultJSON["errno"] = errno;
	}



	std::string resultStr = writer.write(resultJSON);

	return( resultStr.c_str() );
}

std::string IDSEXT::SetOption(int option, const std::string& value)
{
	Json::FastWriter writer;
	Json::Value resultJSON;

	resultJSON["result"] = ids_set_option( (ids_option_t) option, value.c_str() );
	if( (ids_result_t) resultJSON["result"].asInt() != IDS_SUCCESS ) {
		resultJSON["errno"] = errno;
	}


	std::string resultStr = writer.write(resultJSON);

	return( resultStr.c_str());
}

void* IDSEXT::idsEventThread(void *args)
{
	IDSEXT *myidsext = (IDSEXT *) args;

	pipe(myidsext->fileIds);
	if( myidsext->maxFds < myidsext->fileIds[0] ) myidsext->maxFds = myidsext->fileIds[0];

	FD_ZERO((fd_set *) &myidsext->tRfd);
	FD_SET( myidsext->fileIds[0], &myidsext->tRfd );
	
	ids_provider_mapping* current;
	int selectRes = 0;

	char *pcBuff = NULL;
	pcBuff = (char *) malloc( 12 * sizeof(char) );
	if( pcBuff ) {
		while(1) {
			selectRes = select( myidsext->maxFds + 1, (fd_set *)&myidsext->tRfd, 0, 0, NULL );
	
			if(FD_ISSET( myidsext->fileIds[0], &myidsext->tRfd) > 0) {
				// We've added another provider - nothing to do but clear the pipe
				read(myidsext->fileIds[0], pcBuff, 64 * sizeof(char));
			}
	
			if( selectRes > 0 ) {
				current = myidsext->providers;
				while( current != NULL ) {
					ids_process_msg( current->providerFd );
					current = current->next;			
				}
			}
		}
	if( pcBuff ) free( pcBuff );
	}

	return NULL;
}


std::string IDSEXT::GetToken(const std::string& provider, const std::string& tokenType, const std::string& appliesTo)
{
	ids_request_id_t *getTokenRequestId = NULL;
	ids_provider_mapping *requestProvider = getProvider( provider );

	ids_result_t getTokenResult;
	if( requestProvider ) {
		getTokenResult = ids_get_token( requestProvider->provider, tokenType.c_str(), appliesTo.c_str(), getTokenSuccessCB, failureCB, this, getTokenRequestId);
	} else {
		getTokenResult = ids_get_token( NULL, tokenType.c_str(), appliesTo.c_str(), getTokenSuccessCB, failureCB, this, getTokenRequestId);
	}

	if( getTokenResult != IDS_SUCCESS ) {
		failureCB( (ids_request_id_t)0, IDS_FAILURE, NULL, this );
	}

	return "";
}

std::string IDSEXT::GetProperties(const std::string& provider, int numProps, const char* properties)
{
	char *propList[numProps];
	char *delimited = strdup( properties );
	char *token;
	
	int i = 0;
	while( ( token = strsep( &delimited, "," ) ) ) {
         propList[i] = strdup( token );
         i++;
    }
	ids_request_id_t *getPropertiesRequestId = NULL;
	ids_provider_mapping *requestProvider = getProvider( provider );
	ids_result_t getPropertiesResult;

	if( requestProvider ) {
		getPropertiesResult = ids_get_properties( requestProvider->provider, 0, numProps, (const char **) propList, getPropertiesSuccessCB, failureCB, this, getPropertiesRequestId);
	} else {
		getPropertiesResult = ids_get_properties( NULL, 0, numProps, (const char **) propList, getPropertiesSuccessCB, failureCB, this, getPropertiesRequestId);
	}
	if( getPropertiesResult != IDS_SUCCESS ) {
		failureCB( (ids_request_id_t)0, IDS_FAILURE, NULL, this );
	}


	return "";

}

std::string IDSEXT::ClearToken(const std::string& provider, const std::string& tokenType, const std::string& appliesTo)
{
	ids_request_id_t *clearTokenRequestId = NULL;
	ids_provider_mapping *requestProvider = getProvider( provider );

	ids_result_t clearTokenResult;
	if( requestProvider ) {
		clearTokenResult = ids_clear_token( requestProvider->provider, tokenType.c_str(), appliesTo.c_str(), clearTokenSuccessCB, failureCB, this, clearTokenRequestId);
	} else {
		clearTokenResult = ids_clear_token( NULL, tokenType.c_str(), appliesTo.c_str(), clearTokenSuccessCB, failureCB, this, clearTokenRequestId);
	}

	if( clearTokenResult != IDS_SUCCESS ) {
		failureCB( (ids_request_id_t)0, IDS_FAILURE, NULL, this );
	}

	return "";
}