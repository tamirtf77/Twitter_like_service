/*
 * File:   twitServer.cpp
 * Author: tamirtf77
 *
 */
#include "twitServer.h"

twitServer::Data::Data(int socket) : _socket(socket){}

void twitServer::Data::clearAll()
{
    _followers.clear();
}

twitServer::followers& twitServer::Data::getFollowers()
{
    return _followers;
}

twitServer::followers& twitServer::Data::getBlocks()
{
    return _blocks;
}

int twitServer::Data::getSocket()
{
    return _socket;
}

/* Makes an upper copy of a string. */
std::string twitServer::upperCopy(const std::string& str) const
{
    std::string temp = str;
    std::transform(temp.begin(), temp.end(), temp.begin(), ::toupper);
    return temp;
}

/* Makes an lower copy of a string. */
std::string twitServer::lowerCopy(const std::string& str) const
{
    std::string temp = str;
    std::transform(temp.begin(), temp.end(), temp.begin(), ::tolower);
    return temp;
}

/* Writes an entry to the log. */
int twitServer::writeToLog(const std::string& logEntry)
{
    if (_file.is_open() == false)
    {
        return FAILURE;
    }
    _file << logEntry;
    if (_file.fail())
    {
        return FAILURE;
    }
    return SUCCESS;
}

/* Resets the message i.e. put space in every char of it. */
void twitServer::resetMessage(char* msg) const
{
    for (int i = 0; i < PACKAGE_SIZE; i++)
    {
        *(msg+i) = CHAR_SPACE;
    }
    *(msg + PACKAGE_SIZE-1) = CHAR_FINISH;
}

/* Fills the message with command (the client to know which command it gets) and message.*/
void twitServer::fillMessage(char* msg, const char* command,const std::string & other) const
{
    strncpy(msg,command,strlen(command));
    strncpy(msg + strlen(command)+1,other.c_str(),other.size());
}

/* Sends a message to the server. */
int twitServer::sendMessage(const int& socketfd,const char* msg) const
{
    if (send(socketfd, msg, PACKAGE_SIZE, 0) == FAILURE)
    {
        if (send(socketfd, msg, PACKAGE_SIZE, 0) == FAILURE) // second chance
        {
        	return FAILURE;
        }
    }
    return SUCCESS;
}

/* Creates and sends message.*/
int twitServer::createAndSendMessage(const int& socketfd,char command[],const std::string& other) const
{
    char* msg = new char[PACKAGE_SIZE];
    resetMessage(msg);
    fillMessage(msg,command,other);
    int retVal = sendMessage(socketfd,msg);
    delete[] (msg);
    return retVal;
}

/* Gets the time of broadcast the message i.e. the time the server got the message. */
void twitServer::getTimeOfBrodcast(tm*& ltm) const
{
    time_t now = time(0);
    ltm = localtime(&now);
}

/* Adds a time (hour and minute) to the message. */
void twitServer::addingTimeToMessage(std::string& message,const tm* ltm) const
{
    std::string s;
    std::stringstream out;
    out << ltm->tm_hour;
    s = out.str();
    if (s.length() == 1) // the hours is 0 - 9
    {
    	s = PREFIX_FOR_TIME + s;
    }
    message += s;
    out.str(std::string()); // clears the contents of a stringstream.
    out << ltm->tm_min;
    s = out.str();
    if (s.length() == 1) // the minutes is 0 - 9
    {
    	s = PREFIX_FOR_TIME + s;
    }
    message += (COLON + s + COLON + COLON);
}

/* Creates a direct message for sending to the suppose client. */
void twitServer::createDirectMessageToSend(std::string& dm,std::string& message,const std::string& clientName,
                               const std::string& someClientName,const tm* ltm) const
{
    addingTimeToMessage(dm,ltm);
    dm += (clientName + SHTRUDEL + someClientName + SEPARATOR + message);
}

/* Creates a twit for sending to the suppose clients. */
void twitServer::createTwitToSend(std::string& twit,const std::string& message,const std::string& clientName,
                      const tm* ltm) const
{
    addingTimeToMessage(twit,ltm);
    twit += (clientName + DIVIDING_LINE + message);
}

/* Creates a message of not exist. */
void twitServer::createMessageNotExist(const std::string& someClientName, std::string& message) const
{
    message += (ERROR + someClientName + NOT_EXIST);
}

/* Creates an entry log for the start and finish of the server */
void twitServer::createStartFinishEntryLog(std::string& str,const std::string& startFinish) const
{
    tm* ltm = NULL;
    getTimeOfBrodcast(ltm);
    addingTimeToMessage(str,ltm);
    str += startFinish;
    str += DOT;
    str += NEW_LINE;
}

/* Creates an entry log. */
void twitServer::createLogEntry1(const std::string& clientName,const std::string& wordLink,
                     const std::string& message, std::string& logEntry) const
{
    logEntry += (clientName + TAB + SPACE + wordLink);
    if (message != EMPTY)
    {
        logEntry += message;
    }
    logEntry += DOT;
    logEntry += NEW_LINE;
}

/* Creates an entry log. */
void twitServer::createLogEntry2(const std::string& clientName, const std::string& someClientName,
                     const std::string& wordLink, std::string& logEntry) const
{
    logEntry += (clientName + TAB + SPACE + wordLink + SPACE + someClientName + DOT + NEW_LINE);

}

/* Creates an entry log of error. */
void twitServer::createLogEntryError(const std::string& someClientName,const std::string& errorDetails,
                         std::string& logEntry) const
{
    logEntry += (ERROR + someClientName + errorDetails + NEW_LINE);
}

/* Close all the connections with clients. */
void twitServer::closeAllConnections()
{
    char* msg = new char[PACKAGE_SIZE];
    resetMessage(msg);
    char command[] = EMPTY;
    std::string logEntry = EMPTY;

    fillMessage(msg,command,EXIT);
    for(std::map<int,std::string>::const_iterator it = _sockets.begin(); it != _sockets.end(); ++it)
    {
        sendMessage(it->first,msg);
        close(it->first);
        _clients.find(it->second)->second.clearAll();
        createLogEntry1(it->second,DISCONNECTED,EMPTY,logEntry);
        writeToLog(logEntry);
        logEntry = EMPTY;
    }
    delete[] (msg);
    close(_listener);
    close(STDIN);
    fflush(STDIN);
}

/* Delete a client name from all the data structures */
void twitServer::deleteClientName(const std::string& clientName,clientsMap& aMap) const
{
    aMap.find(clientName)->second.clearAll();
    aMap.erase(clientName);
    for(mapClientIter it = aMap.begin(); it != aMap.end(); ++it)
    {
        it->second.getFollowers().erase(lowerCopy(clientName));
        it->second.getBlocks().erase(lowerCopy(clientName));
    }
}

/* Performs disconnect of a client. */
void twitServer::performDisconnect(const int& socketfd,const std::string& clientName)
{
    std::string logEntry = EMPTY;
    if (clientName != EMPTY)
    {
    	createLogEntry1(clientName,DISCONNECTED,EMPTY,logEntry);
    }
    else
    {
    	createLogEntry1(CLIENT_NOT_INTRODUCED,DISCONNECTED,EMPTY,logEntry);
    }

    writeToLog(logEntry);
    deleteClientName(clientName,_clients);
    close(socketfd);
    _sockets.erase(socketfd);
}

/* Performs illegal command i.e. a mock client does not follow the protocol. */
void twitServer::performIllegalCommand(const int& socketfd,const std::string& clientName)
{
    std::string logEntry = EMPTY;
    std::string clientNameCopy = clientName;
    char command[] = ILLEGAL_COMMAND;
    int sendVal = createAndSendMessage(socketfd,command,EMPTY);

    createLogEntryError(clientName,CLIENT_COMMAND_ILLEGAL,logEntry);
    writeToLog(logEntry);
    std::cout << (ERROR + clientName + CLIENT_COMMAND_ILLEGAL) << std::endl;
    logEntry = EMPTY;

    if (sendVal == FAILURE)
    {
    	std::cout << SEND_ERROR << clientName << std::endl;
    	writeToLog((SEND_ERROR + clientName + NEW_LINE));
    	fdsToRemove.insert(socketfd);
    }

    createLogEntry1(clientNameCopy,DISCONNECTED,EMPTY,logEntry);
    writeToLog(logEntry);
    deleteClientName(clientName,_clients);
    close(socketfd);
    _sockets.erase(socketfd);
}

/* Performs not exist i.e. sends a message to a client X in case the client name Y,
 * it wants to follow/unfollow/block/dm does not exist. */
void twitServer::performNotExist(const int& socketfd,const std::string& someClientName,std::string logEntry)
{
    createLogEntryError(someClientName,NOT_EXIST_TO_SEND,logEntry);
    writeToLog(logEntry);

    std::string temp = logEntry;
    logEntry = NOT_EXIST;
    logEntry += SPACE;
    logEntry += temp;
    logEntry = logEntry.substr(0,logEntry.size()-1);

    char command[] = EMPTY;
    if (createAndSendMessage(socketfd,command,logEntry) == FAILURE)
    {
    	std::cout << SEND_ERROR << someClientName << std::endl;
    	writeToLog((SEND_ERROR + someClientName + NEW_LINE));
    	fdsToRemove.insert(socketfd);
    	performDisconnect(socketfd,_sockets.find(socketfd)->second);
    }
}

/* Performs follow command of a client.*/
bool twitServer::performFollow(const int& socketfd,const std::string& clientName, const std::string& someClientName)
{
    std::string logEntry = EMPTY;
    mapClientIter it;
    if ( (it = _clients.find(someClientName)) != _clients.end())
    {
        createLogEntry2(clientName,it->first,FOLLOWED,logEntry);
        writeToLog(logEntry);
        it->second.getFollowers().insert(lowerCopy(clientName));
        it->second.getBlocks().erase(lowerCopy(clientName));
        return true;
    }
    createLogEntry2(clientName,someClientName,FOLLOWED,logEntry);
    writeToLog(logEntry);
    logEntry = EMPTY;
    performNotExist(socketfd,someClientName,logEntry);
    return false;
}

/* Performs unfollow command of a client.*/
bool twitServer::performUnfollow(const int& socketfd,const std::string& clientName, const std::string& someClientName)
{
    std::string logEntry = EMPTY;
    mapClientIter it;
    if ( (it = _clients.find(someClientName)) != _clients.end())
    {
        createLogEntry2(clientName,it->first,UNFOLLOWED,logEntry);
        writeToLog(logEntry);
        it->second.getFollowers().erase(lowerCopy(clientName));
        return true;
    }
    createLogEntry2(clientName,someClientName,UNFOLLOWED,logEntry);
    writeToLog(logEntry);
    logEntry = EMPTY;
    performNotExist(socketfd,someClientName,logEntry);
    return false;
}

/* Performs twit command of a client.*/
bool twitServer::performTwit(const std::string& clientName, std::string& message,const tm* ltm)
{
    std::string logEntry = EMPTY, twit = EMPTY;
    if (upperCopy(message) == TWIT) // it is an empty message
    {
    	message = EMPTY;
    }
    logEntry += (clientName + TAB + SPACE + TWITTED + SPACE + message + DOT + NEW_LINE);
    writeToLog(logEntry);
    logEntry = EMPTY;
    createTwitToSend(twit,message,clientName,ltm);
    char* msg = new char[PACKAGE_SIZE];
    resetMessage(msg);
    char command[] = TWIT_FROM;
    fillMessage(msg,command,twit);
    if (_clients.find(clientName)->second.getFollowers().empty() == false)
    {
        mapClientIter itFollowers = _clients.find(clientName);
        for (setConstIter itFollowersSet = itFollowers->second.getFollowers().begin(); itFollowersSet != itFollowers->second.getFollowers().end(); ++itFollowersSet)
        {
            if (sendMessage(_clients.find(*itFollowersSet)->second.getSocket(),msg) == FAILURE)
            {
            	std::cout << SEND_ERROR << _clients.find(*itFollowersSet)->first << std::endl;
            	writeToLog((SEND_ERROR + _clients.find(*itFollowersSet)->first + NEW_LINE));
            	fdsToRemove.insert(_clients.find(*itFollowersSet)->second.getSocket());
            	performDisconnect(_clients.find(*itFollowersSet)->second.getSocket(),_clients.find(*itFollowersSet)->first);
            }
        }
    }
    delete[] (msg);
    return false;
}

/* Separates a string by a delimiter.*/
void twitServer::separateByDelimeter(const std::string& str,std::vector<std::string>& tokens,
                         const std::string& delimiters = SHTRUDEL) const
{
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);
    tokens.push_back(str.substr(lastPos, pos - lastPos));
    tokens.push_back(str.substr(pos - lastPos+1,str.size() - 1));
}

/* Performs direct message command of a client.*/
bool twitServer::performDirectMessage(const int& socketfd,const std::string& clientName,
                          const std::string& SomeClientNameAndMessage,const tm* ltm)
{
    std::string logEntry = EMPTY;
    std::vector<std::string> tokens;
    separateByDelimeter(SomeClientNameAndMessage,tokens);
    std::string someClientName = tokens.at(0), message = tokens.at(1);
    mapClientIter it1;
    if ( (it1 = _clients.find(someClientName)) != _clients.end())
    {
        logEntry += (clientName + TAB + SPACE + DM_NAME + SPACE + it1->first + TAB + SPACE + message + DOT + NEW_LINE);
        writeToLog(logEntry);
        logEntry = EMPTY;
        setConstIter it2 = _clients.find(clientName)->second.getBlocks().begin();
        if ((it2 != _clients.find(clientName)->second.getBlocks().end()) && (it2->find(lowerCopy(someClientName)) != it2->npos))
    	{
            logEntry += (it1->first + TAB + SPACE + COULD_NOT_DM + clientName + SPACE + DUE_BLOCK + DOT + NEW_LINE);
            writeToLog(logEntry);
            return false;
    	}
    	else
    	{
            std::string dm;
            createDirectMessageToSend(dm,message,clientName,it1->first,ltm);
            char command[] = DM_FROM;
            if (createAndSendMessage(it1->second.getSocket(),command,dm) == FAILURE)
            {
            	std::cout << SEND_ERROR << it1->first << std::endl;
            	writeToLog((SEND_ERROR + it1->first + NEW_LINE));
            	fdsToRemove.insert(it1->second.getSocket());
            	performDisconnect(it1->second.getSocket(),it1->first);
            }
            return true;
    	}
    }
    logEntry += (clientName + TAB + SPACE + DM_NAME + SPACE + someClientName + TAB + SPACE + message + DOT + NEW_LINE);
    writeToLog(logEntry);
    logEntry = EMPTY;
    performNotExist(socketfd,someClientName,logEntry);
    return false;
}

/* Performs block command of a client.*/
bool twitServer::performBlock(const int& socketfd,const std::string& clientName, const std::string& someClientName)
{
    std::string logEntry = EMPTY;
    mapClientIter it;
    if ( (it = _clients.find(someClientName)) != _clients.end())
    {
        createLogEntry2(clientName,it->first,BLOCKED,logEntry);
        writeToLog(logEntry);
        it->second.getBlocks().insert(lowerCopy(clientName));
        it->second.getFollowers().erase(lowerCopy(clientName));
        return true;
    }
    createLogEntry2(clientName,someClientName,BLOCKED,logEntry);
    writeToLog(logEntry);
    logEntry = EMPTY;
    performNotExist(socketfd,someClientName,logEntry);
    return false;
}

/* Performs who command of a client.*/
void twitServer::performWho(const int& socketfd,const std::string& clientName)
{
    std::string logEntry = EMPTY;
    char command[] = WHO, commandFinish[] = WHO_FINISH;
    int sendVal = SUCCESS;

    createLogEntry1(clientName,WHO_LOG,EMPTY,logEntry);
    writeToLog(logEntry);

    for(mapClientConstIter it = _clients.begin(); (it != _clients.end()) && (sendVal == SUCCESS); ++it)
    {
    	if ((sendVal = createAndSendMessage(socketfd,command,it->first)) == FAILURE)
    	{
        	std::cout << SEND_ERROR << clientName << std::endl;
        	writeToLog((SEND_ERROR + clientName + NEW_LINE));
        	fdsToRemove.insert(socketfd);
        	performDisconnect(socketfd,clientName);
        	sendVal = FAILURE;
    	}
    }
    if (sendVal == SUCCESS)
    {
    	if (createAndSendMessage(socketfd,commandFinish,EMPTY) == FAILURE)
    	{
        	std::cout << SEND_ERROR << clientName << std::endl;
        	writeToLog((SEND_ERROR + clientName + NEW_LINE));
        	fdsToRemove.insert(socketfd);
        	performDisconnect(socketfd,clientName);
    	}
    }
}

/* Welcomes a new client, i.e. sends it WELCOME in case his name does not exist in the system.
 * If his name exist it sends NOT_EXIST.
 * In case it is a mock client, it sends UNAUTHORIZED.*/
bool twitServer::welcomeNewClient(const int& socketfd,const std::vector<std::string>& tokens)
{
    std::string logEntry = EMPTY;
    if (tokens.at(0) == BEGIN)
    {
        if (_clients.find(tokens.at(1)) == _clients.end())
        {
            _sockets.find(socketfd)->second = tokens.at(1);
            _clients.insert(std::make_pair(tokens.at(1),Data(socketfd)));
            createLogEntry1(tokens.at(1),CONNECTED,EMPTY,logEntry);
            writeToLog(logEntry);
            char command[] = WELCOME;
            if (createAndSendMessage(socketfd,command,EMPTY) == FAILURE)
            {
            	std::cout << SEND_ERROR << tokens.at(1) << std::endl;
            	writeToLog((SEND_ERROR + tokens.at(1) + NEW_LINE));
            	fdsToRemove.insert(socketfd);
            	performDisconnect(socketfd,tokens.at(1));
            }
            return true;
        }
        else
        {
            char command[] = NAME_USED;
            createLogEntryError((TRY + _clients.find(tokens.at(1))->first),ALREADY,logEntry);
            std::cout << (ERROR + (TRY + _clients.find(tokens.at(1))->first) + ALREADY) << std::endl;
            writeToLog(logEntry);
            createAndSendMessage(socketfd,command,EMPTY);
            close(socketfd);
            _sockets.erase(socketfd);

        }
    }
    else
    {
        char command[] = UNAUTHORIZED;
        createAndSendMessage(socketfd,command,EMPTY);
        close(socketfd);
        _sockets.erase(socketfd);
        createLogEntryError(tokens.at(1),UNAUTHORIZED_LOG,logEntry);
        std::cout << (ERROR + tokens.at(1) + UNAUTHORIZED_LOG) << std::endl;
        writeToLog(logEntry);
    }
    return false;
}

/* Concats a message. */
void twitServer::concatMessage(std::string& msg, const std::vector<std::string>& tokens) const
{
    msg.assign(EMPTY);
    for (unsigned int i = 1; i < tokens.size() - 1; i++)
    {
        msg.append(tokens.at(i));
    }
    msg.append(tokens.at(tokens.size()-1));
}

/* Performs a client command. */
void twitServer::performClientCommand(const int& socketfd,const std::vector<std::string>& tokens,
                          const tm* ltm)
{
    if (tokens.at(0) == TWIT)
    {
        std::string msg = EMPTY;
        concatMessage(msg,tokens);
        performTwit(_sockets.find(socketfd)->second,msg,ltm);
    }
    else if (tokens.at(0) == DM)
    {
        std::string msg = EMPTY;
        concatMessage(msg,tokens);
        performDirectMessage(socketfd,_sockets.find(socketfd)->second,msg,ltm);
    }
    else if (tokens.size() == 1)
    {
        if (tokens.at(0) == EXIT)
        {
            performDisconnect(socketfd,_sockets.find(socketfd)->second);
        }
        else if (tokens.at(0) == WHO)
        {
            performWho(socketfd,_sockets.find(socketfd)->second);
        }
        else
        {
            performIllegalCommand(socketfd,_sockets.find(socketfd)->second);
        }
    }

    else if (tokens.size() == 2)
    {
        if (tokens.at(0) == FOLLOW)
        {
            performFollow(socketfd,_sockets.find(socketfd)->second,tokens.at(1));
        }
        else if (tokens.at(0) == UNFOLLOW)
        {
            performUnfollow(socketfd,_sockets.find(socketfd)->second,tokens.at(1));
        }
        else if (tokens.at(0) == BLOCK)
        {
            performBlock(socketfd,_sockets.find(socketfd)->second,tokens.at(1));
        }
        else
        {
            performIllegalCommand(socketfd,_sockets.find(socketfd)->second);
        }
    }
    else
    {
        performIllegalCommand(socketfd,_sockets.find(socketfd)->second);
    }
}

/* Splits the input line from the user to tokens. */
void twitServer::splitMessage(std::string& msg,std::vector<std::string>& tokens) const
{
    std::string::size_type pos1,pos2;
    std::string token;
    while (msg.size() > 0)
    {
        pos1 = msg.find_first_not_of(SPACE, 0);
        if (pos1 == std::string::npos)
        {
            break;
        }
        pos2 = msg.find_first_of(SPACE, pos1);
        if (pos2 == std::string::npos)
        {
            pos2 = msg.size();
        }
        token = msg.substr(0,pos2);
        if (tokens.size() == 0)
        {
        	token.erase(std::remove_if(token.begin(), token.end(), isspace), token.end());
        }
        if (tokens.size() == 1)
        {
        	if (upperCopy(tokens.at(0)) != TWIT)
        	{
        		token.erase(std::remove_if(token.begin(), token.end(), isspace), token.end());
        	}
        	else
        	{
        		token = token.substr(1,token.size() - 1);
        	}
        }
        tokens.push_back(token);
        msg = msg.substr(pos2,msg.size() - pos2);
    }
}

/* Advances in a session with a client i.e. perform his command. */
bool twitServer::advanceSession(int socketfd,char* buf,const tm* ltm)
{
    std::string msg, clientName;
    std::vector<std::string> tokens;
    msg.assign(buf);
    splitMessage(msg,tokens);
    if (_sockets.find(socketfd)->second == EMPTY) // new client.
    {
        return welcomeNewClient(socketfd,tokens);
    }
    // not new client which has already got a legal access to the server.
    clientName = _sockets.find(socketfd)->second;

    performClientCommand(socketfd,tokens,ltm);
    if (_clients.find(clientName) != _clients.end())
    {
            return true;
    }
    else // the socket was closed and the client has been deleted.
    {
            return false;
    }
}

/* Performs command according to the user input line.
   If the user command is EXIT then it returns FINISH.
   Otherwise, it returns UNFINISH. */
int twitServer::performUserCommand(const std::vector<std::string>& tokens) const
{
    if ((tokens.size() == 1) && (upperCopy(tokens.at(0)) == EXIT))
    {
        return FINISH;
    }
    std::cout << USER_ILLEGAL_COMMAND << std::endl;
    return UNFINISH;
}

/* Splits the input line from the user to tokens. */
void twitServer::splitUserLine(const std::string& line, std::stringstream& stringStream,
               std::vector<std::string>& tokens) const
{
    std::string token;
    stringStream << line;
    while (stringStream >> token)
    {
        tokens.push_back(token);
    }
}

/* Gets commands from the user. */
bool twitServer::receiveCommandFromUser()
{
    std::string line = EMPTY;
    std::stringstream stringStream;
    std::vector<std::string> tokens;
    int finishVal = UNFINISH;

    std::getline(std::cin,line);
    splitUserLine(line,stringStream,tokens);
    finishVal = performUserCommand(tokens);

    stringStream.clear();
    tokens.clear();
    if (finishVal == FINISH)
    {
        closeAllConnections();
        std::string logEntry = EMPTY;
        createStartFinishEntryLog(logEntry, SERVER_EXIT);
        writeToLog(logEntry);
        return true;
    }
    return false;
}

/*The main loop - receives messages from clients or input the user. */
void twitServer::listening()
{
    fd_set readfds,writefds,master;
    std::set<int,orderSet> fds;
    int newfd, maxfd, bytesReceive;
    char buf[PACKAGE_SIZE];
    tm* ltm = NULL;

    fds.insert(_listener);
    fds.insert(STDIN);
    maxfd = *fds.begin();

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&master);
    FD_SET(_listener,&master);
    FD_SET(STDIN,&master);

    std::string serverStart = EMPTY;
    createStartFinishEntryLog(serverStart,SERVER_START);
    writeToLog(serverStart);

    while(true)
    {
        readfds = master;
        select(maxfd + 1, &readfds,&writefds, NULL, NULL);
        for (int i = 0; (i <= maxfd) && (_finish == false); i++)
        {
            if (FD_ISSET(i, &readfds))
            {
                if (i == STDIN)
                {
                    if (receiveCommandFromUser() == true)
                    {
                        _finish = true;
                    }
                }
                else if (i == _listener)
                {
                    struct sockaddr_in their_addr; // connector's address information
                    socklen_t sin_size = sizeof(struct sockaddr_in);
                    if ((newfd = accept(_listener, (struct sockaddr *)&their_addr,&sin_size)) != FAILURE)
                    {
                        _sockets.insert(std::make_pair(newfd,EMPTY));
                        FD_SET(newfd, &master);
                        fds.insert(newfd);
                        maxfd = *fds.begin();
                    }
                }
                else
                {
                    resetMessage(buf);
                    bytesReceive = recv(i, buf, sizeof(buf), 0);
                    getTimeOfBrodcast(ltm);
                    if (bytesReceive <= 0)
                    {
                    	std::string logEntry = EMPTY, clientName = _sockets.find(i)->second;
                    	if (clientName == EMPTY)
                    	{
                    		clientName = CLIENT_NOT_INTRODUCED;
                    	}
						if (bytesReceive == 0)
						{
							createLogEntryError(clientName,CRASHED,logEntry);
							std::cout << ERROR << clientName << CRASHED << std::endl;
						}
						if (bytesReceive < 0)
						{
							logEntry = (RECEIVE_ERROR + clientName + NEW_LINE);
							std::cout << RECEIVE_ERROR << clientName << std::endl;
						}
						writeToLog(logEntry);
                        performDisconnect(i,_sockets.find(i)->second);
                        FD_CLR(i,&master);
                        fds.erase(i);
                        maxfd = *fds.begin();
                    }
                    else if (advanceSession(i,buf,ltm) == false)
                    {
                        FD_CLR(i,&master);
                        fds.erase(i);
                        maxfd = *fds.begin();
                    }
                    for (std::set<int>::const_iterator it = fdsToRemove.begin(); it != fdsToRemove.end(); ++it)
                    {
                        FD_CLR(*it,&master);
                        fds.erase(*it);
                        maxfd = *fds.begin();
                    }
                    fdsToRemove.clear();
                    ltm = NULL;

                }
            }
        }
        if (_finish == true)
        {
            break;
        }
    }
    _file.close();
}

/* Converts the port number from string to integer.
  If the string contains chars which are not digits, it returns FAILURE.
  Otherwise, it returns port as an integer. */
int twitServer::convertPortNum(const std::string& portNum) const
{
    if (portNum.find_first_not_of(DIGITS) != std::string::npos)
    {
        std::cout << PORT_ILLEGAL << std::endl;
        return FAILURE;
    }
    std::stringstream strValue;
    unsigned int intValue;
    strValue << portNum;
    strValue >> intValue;
    return intValue;
}

/* Opens the log file and appends .*/
int twitServer::openLogFile()
{
    _file.open(LOG_FILE, std::ios::out| std::ios::app);
    if ( (_file.is_open() == false) || (_file.fail()) )
    {
        std::cout << LOG_ERROR << std::endl;
        return FAILURE;
    }
    _file.clear();
    return SUCCESS;
}

/* Creates the socket address of the client. */
void twitServer::createSocketAddress(const int& portNum,struct sockaddr_in& my_addr) const
{
    my_addr.sin_family = AF_INET;// host byte order
    my_addr.sin_port = htons(portNum); //
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // use my IP address
    memset(&(my_addr.sin_zero), CHAR_FINISH, 8); // zero the rest of the struct
}

/* Opens a socket for listening to clients requests in a specific port.*/
int twitServer::openListeningSocket(int portNum)
{
    createSocketAddress(portNum,_my_addr);
    if ( (_listener = socket(PF_INET, SOCK_STREAM, 0)) == FAILURE)
    {
        std::cout << SOCKET_ERROR << std::endl;
        return FAILURE;
    }
    int yes = 1;
    if (setsockopt(_listener, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == FAILURE)
    {
        std::cout << BIND_ERROR << std::endl;
        return FAILURE;
    }
    if (bind(_listener, (struct sockaddr *)&_my_addr, sizeof(struct sockaddr)) == FAILURE)
    {
        std::cout << BIND_ERROR << std::endl;
        return FAILURE;
    }
    if (listen(_listener,POSSIBLE_CONNECTIONS) == FAILURE)
    {
        std::cout << LISTEN_ERROR << std::endl;
        return FAILURE;
    }
    return SUCCESS;
}

/* Inits the server.
   If the initialization went successfully, it returns SUCEESS.
   Otherwise, it returns FAILURE. */
int twitServer::initTwitServer(const std::string& portNum)
{
    unsigned int port;
    _finish = false;
    if ( (int)(port = convertPortNum(portNum)) == FAILURE)
    {
        return FAILURE;
    }
    if ( (port >= MIN_PRESERVED_PORT) && (port <= MAX_PRESERVED_PORT) )
    {
    	std::cout << PORT_AT_LEAST << std::endl;
    	return FAILURE;
    }
    if (openLogFile() == FAILURE)
    {
        close(_listener);
        return FAILURE;
    }
    if (openListeningSocket(port) == FAILURE)
    {
        close(_listener);
        _file.close();
        return FAILURE;
    }
    listening();
    return SUCCESS;
}

int main(int argc, char** argv)
{
    if (argc != NUM_SERVER_ARGUMENTS)
    {
        std::cout << ILLEGAL_NUM_OF_ARGUMENTS << std::endl;
        exit(FAILURE_EXIT);
    }
    else
    {
        std::string port;
        port.assign(argv[1],strlen(argv[1]));
        twitServer* server = new twitServer();
        if (server->initTwitServer(port) == FAILURE)
        {
            exit(FAILURE_EXIT);
        }
    }
    return 0;
}