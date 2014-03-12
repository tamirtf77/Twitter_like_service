/*
 * File:   twitClient.cpp
 * Authors: tamirtf77
 *
 */

#include "twitClient.h"


twitClient::Session::Session():_previousSendMessage(EMPTY),_previousReceiveMessage(EMPTY),
              _introductionOver(false),_sessionOver(false){}

void twitClient::Session::setPreviousReceiveMessage(const std::string& command)
{
    _previousReceiveMessage = command;
}

void twitClient::Session::setPreviousSendMessage(const std::string& command)
{
    _previousSendMessage = command;
}

void twitClient::Session::messageToReceive(std::set<std::string>& receive)
{
    if (_introductionOver == false)
    {
        introductionMessageToReceive(receive);
        return;
    }
    receiveMessageAccordingToCommand(receive);
}

void twitClient::Session::introductionMessageToReceive(std::set<std::string>& receive)
{
    if (previousSendReceive(BEGIN,EMPTY))
    {
        receive.insert(WELCOME);
        receive.insert(NAME_USED);
        receive.insert(EXIT);
        receive.insert(TWIT_FROM);
        receive.insert(DM_FROM);
        _introductionOver = true;
    }
}

void twitClient::Session::receiveMessageAccordingToCommand(std::set<std::string>& receive) const
{
    if (_previousSendMessage == BEGIN)
    {
        receive.insert(EXIT);
        receive.insert(TWIT_FROM);
        receive.insert(DM_FROM);
        return;
    }

    if ( (_previousSendMessage == FOLLOW) || ((_previousSendMessage == UNFOLLOW)) ||
         ((_previousSendMessage == DM)) || (_previousSendMessage == BLOCK) )
    {
        receive.insert(NOT_EXIST);
        receive.insert(EXIT);
        receive.insert(TWIT_FROM);
        receive.insert(DM_FROM);
        return;
    }
    if (_previousSendMessage == TWIT)
    {
        receive.insert(EXIT);
        receive.insert(TWIT_FROM);
        receive.insert(DM_FROM);
        return;
    }
    if (_previousSendMessage == WHO)
    {
        receive.insert(WHO);
        receive.insert(WHO_FINISH);
        receive.insert(EXIT);
        receive.insert(TWIT_FROM);
        receive.insert(DM_FROM);
        return;
    }
    if (_previousSendMessage == EXIT)
    {
        receive.insert(EXIT);
        receive.insert(TWIT_FROM);
        receive.insert(DM_FROM);
        return;
    }
}

void twitClient::Session::insertAll(std::set<std::string>& send) const
{
    send.insert(EXIT);
    send.insert(FOLLOW);
    send.insert(UNFOLLOW);
    send.insert(TWIT);
    send.insert(DM);
    send.insert(BLOCK);
    send.insert(WHO);
}

bool twitClient::Session::previousSendReceive(const std::string& send, const std::string& receive) const
{
    if ((_previousSendMessage == send) && (_previousReceiveMessage == receive))
    {
        return true;
    }
    return false;
}

/* Checks whether the client name is legal.
   If the client name is legal, it returns SUCCESS, otherwise return FAILURE.*/
int twitClient::checkClientName(const std::string& clientName) const
{
    if (clientName.size() == 0)
    {
        std::cout << NAME_IS_NULL << std::endl;
        return FAILURE;
    }
    if (clientName.size() > MAXIMUM_CHARS)
    {
        std::cout << NAME_LONG << std::endl;
        return FAILURE;
    }
    if (clientName.find(SPACE) != std::string::npos)
    {
        std::cout << ILLEGAL_SPACE << std::endl;
        return FAILURE;
    }
    if (clientName.find(SHTRUDEL) != std::string::npos)
    {
        std::cout << ILLEGAL_SHTRUDEL << std::endl;
        return FAILURE;
    }
    return SUCCESS;
}

/* Converts the port number from string to integer.
  If the string contains chars which are not digits, it returns FAILURE.
  Otherwise, it returns port as an integer. */
int twitClient::convertPortNum(const std::string& portNum) const
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

/* Creates the socket address of the client. It supports also the string of localhost. */
int twitClient::createSocketAddress(const char* serverAddress, const unsigned int& portVal,struct sockaddr_in& my_addr) const
{
    char localAddressIP[] = LOCAL_HOST_IP;
    std::string serverAdd = std::string(serverAddress);
    int retInet;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(portVal);
    if (serverAdd == LOCAL_HOST)
    {
        retInet = inet_aton(localAddressIP,&(my_addr.sin_addr));
    }
    else if (serverAdd.find_first_not_of(DIGITS_DOT, 0) == std::string::npos)
    {
        retInet = inet_aton(serverAddress,&(my_addr.sin_addr));
    }
    else
    {
        return FAILURE;
    }
    memset(&(my_addr.sin_zero),CHAR_FINISH, 8);
    if (retInet == INET_FAILURE)
    {
        return FAILURE;
    }
    return SUCCESS;
}

/* Resets the message i.e. put space in every char of it. */
void twitClient::resetMessage(char* msg) const
{
    for (int i = 0; i < PACKAGE_SIZE; i++)
    {
        *(msg+i) = CHAR_SPACE;
    }
    *(msg + PACKAGE_SIZE-1) = CHAR_FINISH;
}

/* Truncates the message in case it is longer than MAXIMUM_CHARS_MESSAGE. */
void twitClient::truncateMessage(std::string& message) const
{
    if (message.size() > MAXIMUM_CHARS_MESSAGE)
    {
        message.resize(MAXIMUM_CHARS_MESSAGE);
    }
}
/* Fills the message with command (the server to know which command it gets) and message.*/
void twitClient::fillMessage(char* msg, const char* command, const std::string& other) const
{
    strncpy(msg,command,strlen(command));
    strncpy(msg + strlen(command)+1,other.c_str(),other.size());
}

/* Sends a message to the server. */
void twitClient::sendMessage(const char* msg)
{
    if (send(_socketfd, msg, PACKAGE_SIZE, 0) == FAILURE)
    {
        if (send(_socketfd, msg, PACKAGE_SIZE, 0) == FAILURE) // second chance
        {
            std::cout << SEND_ERROR << std::endl;
            _send = FAILURE;
            return;
        }
    }
    _send = SUCCESS;
    return;
}

/* Creates and sends message.*/
void twitClient::createAndSendMessage(char command[],const std::string& other)
{
    char* msg = new char[PACKAGE_SIZE];
    resetMessage(msg);
    fillMessage(msg,command,other);
    sendMessage(msg);
    delete[] (msg);
}

/* Makes an upper copy of a string. */
std::string twitClient::upperCopy(const std::string& str) const
{
    std::string temp = str;
    std::transform(temp.begin(), temp.end(), temp.begin(), ::toupper);
    return temp;
}

/* Splits the input line from the user to tokens. */
void twitClient::splitLine(std::string& line,std::vector<std::string>& tokens) const
{
    std::string::size_type pos1,pos2;
    std::string token;
    while (line.size() > 0)
    {
        pos1 = line.find_first_not_of(SPACE, 0);
        if (pos1 == std::string::npos)
        {
            break;
        }
        pos2 = line.find_first_of(SPACE, pos1);
        if (pos2 == std::string::npos)
        {
            pos2 = line.size();
        }
        token = line.substr(0,pos2);
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
        line = line.substr(pos2,line.size() - pos2);
    }
}

/* Concats a message. */
void twitClient::concatMessage(std::string& msg, const std::vector<std::string>& tokens) const
{
    msg.assign(EMPTY);
    for (unsigned int i = 1; i < tokens.size() - 1; i++)
    {
        msg.append(tokens.at(i));
    }
    msg.append(tokens.at(tokens.size()-1));
}

/* Performs disconnect from the server. */
void twitClient::performDisconnect(const std::vector<std::string>& tokens)
{
    char command[] = EXIT;
    createAndSendMessage(command,EMPTY);
    _session.setPreviousSendMessage(upperCopy(tokens.at(0)));
}

/* Performs follow command.*/
void twitClient::performFollow(const std::vector<std::string>& tokens)
{
    char command[] = FOLLOW;
    if (checkClientName(tokens.at(1)) == SUCCESS)
    {
        createAndSendMessage(command,tokens.at(1));
        _session.setPreviousSendMessage(upperCopy(tokens.at(0)));
    }
}

/* Performs unfollow command.*/
void twitClient::performUnfollow(const std::vector<std::string>& tokens)
{
    char command[] = UNFOLLOW;
    if (checkClientName(tokens.at(1)) == SUCCESS)
    {
        createAndSendMessage(command,tokens.at(1));
        _session.setPreviousSendMessage(upperCopy(tokens.at(0)));
    }
}

/* Performs twit command.*/
void twitClient::performTwit(const std::vector<std::string>& tokens)
{
    std::string tMessage = EMPTY;
    concatMessage(tMessage,tokens);
    truncateMessage(tMessage);
    if (upperCopy(tMessage) == TWIT) // it is an empty message
    {
    	tMessage = EMPTY;
    }

    char command[] = TWIT;
    createAndSendMessage(command,tMessage);
    _session.setPreviousSendMessage(upperCopy(tokens.at(0)));
}

/* Separates a string by a delimiter.*/
void twitClient::separateByDelimiter(const std::string& str,std::vector<std::string>& tokens,
                         const std::string& delimiters = SHTRUDEL) const
{
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);
    tokens.push_back(str.substr(lastPos, pos - lastPos));
    tokens.push_back(str.substr(pos - lastPos+1,str.size() - 1));
}

/* Performs direct message command.*/
void twitClient::performDirectMessage(const std::vector<std::string>& tokens)
{
    std::vector<std::string> tokensTemp;
    std::string::size_type pos;
    if (tokens.size() == 1)
    {
    	std::cout << DM_ERROR << std::endl;
    	return;
    }
    pos = tokens.at(1).find_first_of(SHTRUDEL,0);
    if ( (pos == 0) || (pos == std::string::npos) )
    {
        std::cout << DM_ERROR << std::endl;
    }
    else
    {
        separateByDelimiter(tokens.at(1),tokensTemp);
        std::string temp = tokensTemp.at(0), tMessage = EMPTY;
        for (unsigned int i = 2; i < tokens.size() - 1; i++)
        {
            tMessage.append(tokens.at(i));
        }
        if ((tokens.size() - 1) >= 2)
        {
            tMessage.append(tokens.at(tokens.size()-1));
        }
        tMessage = tokensTemp.at(1) + tMessage;
        truncateMessage(tMessage);
        temp += (SHTRUDEL + tMessage);
        char command[] = DM;
        if (checkClientName(tokensTemp.at(0)) == SUCCESS)
        {
            createAndSendMessage(command,temp);
            _session.setPreviousSendMessage(upperCopy(tokens.at(0)));
        }
    }
}

/* Performs block command.*/
void twitClient::performBlock(const std::vector<std::string>& tokens)
{
    char command[] = BLOCK;
    if (checkClientName(tokens.at(1)) == SUCCESS)
    {
        createAndSendMessage(command,tokens.at(1));
        _session.setPreviousSendMessage(upperCopy(tokens.at(0)));
    }
}

/* Performs who command.*/
void twitClient::performWho(const std::vector<std::string>& tokens)
{
    char command[] = WHO;
    createAndSendMessage(command,EMPTY);
    _session.setPreviousSendMessage(upperCopy(tokens.at(0)));
}

/* Performs command according to the user input line.
   If the user command is EXIT then it returns FINISH.
   Otherwise, it returns UNFINISH. */
int twitClient::performCommand(const std::vector<std::string>& tokens)
{
    std::string command = upperCopy(tokens.at(0));
    if (command == TWIT)
    {
        performTwit(tokens);
    }
    else if (command == DM)
    {
        performDirectMessage(tokens);
    }
    else if (tokens.size() == 1)
    {
        if (command == EXIT)
        {
            performDisconnect(tokens);
            return FINISH;
        }
        else if (command == WHO)
        {
            performWho(tokens);
        }
        else
        {
            std::cout << USER_ILLEGAL_COMMAND << std::endl;
        }
    }

    else if (tokens.size() == 2)
    {
        if (command == FOLLOW)
        {
            performFollow(tokens);
        }
        else if (command == UNFOLLOW)
        {
            performUnfollow(tokens);
        }
        else if (command == BLOCK)
        {
            performBlock(tokens);
        }
        else
        {
            std::cout << USER_ILLEGAL_COMMAND << std::endl;
        }
    }
    else
    {
        std::cout << USER_ILLEGAL_COMMAND << std::endl;
    }
    return UNFINISH;
}

/* Gets commands from the user. */
bool twitClient::getCommands()
{
    std::string line = EMPTY;
    std::stringstream stringStream;
    std::vector<std::string> tokens;
    std::getline(std::cin,line);
    splitLine(line,tokens);
    int finishVal = performCommand(tokens);
    stringStream.clear();
    tokens.clear();

    if (finishVal == FINISH)
    {
        return true;
    }
    return false;
}

/* Prints a message according to a command from the server and/or
 * the contents of DM or TWIT of other user.*/
void twitClient::messageToPrint(const std::vector<std::string>& tokens,
                    std::vector<std::string>& clientsNames) const
{
    std::string msg;
    concatMessage(msg,tokens);
    if (tokens.at(0) == WELCOME) { std::cout << CONNECT_SUCCESS_PRINT << std::endl; }
    else if(tokens.at(0) == NAME_USED) { std::cout << NAME_USED_PRINT << std::endl; }
    else if(tokens.at(0) == EXIT) { std::cout << SERVER_EXIT_PRINT << std::endl;}

    else if( (tokens.at(0) == NOT_EXIST) || (tokens.at(0) == TWIT_FROM) ||
             (tokens.at(0) == DM_FROM) )
    {
        std::cout << msg << std::endl;
    }
    else if (tokens.at(0) == WHO_FINISH)
    {
        for(unsigned int i = 0; i < clientsNames.size()-1; i++)
        {
            std::cout << clientsNames.at(i) << TAB << SPACE;
        }
        std::cout << clientsNames.at(clientsNames.size()-1) << std::endl;
        clientsNames.clear();
    }
}

/* Receives a message from the server. */
int twitClient::receiveFromServer(char* buf,std::vector<std::string>& clientsNames)
{
    std::string msg;
    std::vector<std::string> tokens;
    msg.assign(buf);
    splitLine(msg,tokens);
    std::set<std::string> receiveOptions;
    _session.messageToReceive(receiveOptions);
    if (receiveOptions.find(tokens.at(0)) == receiveOptions.end())
    {
        std::cout << SERVER_ERROR_PRINT << std::endl;
        return FAILURE_EXIT;
    }
    messageToPrint(tokens,clientsNames);
    if (tokens.at(0) == WHO)
    {
        clientsNames.push_back(tokens.at(1));
    }
    _session.setPreviousReceiveMessage(tokens.at(0));
    if (tokens.at(0) == NAME_USED)
    {
    	return FAILURE_EXIT;
    }
    if (tokens.at(0) == EXIT)
    {
    	return SUCCESS_EXIT;
    }
    return NOT_EXIT;
}

/*The main loop - receives messages from the server or input the user. */
void twitClient::sendReceive()
{
    fd_set readfds,master;
    int bytesReceive,retVal = SUCCESS_EXIT, maxfd = (_socketfd > STDIN)?_socketfd:STDIN;
    char buf[PACKAGE_SIZE];
    std::vector<std::string> clientsNames;
    FD_ZERO(&readfds);
    FD_ZERO(&master);
    FD_SET(_socketfd,&master);
    FD_SET(STDIN,&master);

    if (_send == SUCCESS) // sending BEGIN successfully.
    {
		while(true)
		{
			readfds = master;
			select(maxfd + 1, &readfds,NULL, NULL, NULL);
			for (int i = 0; (i <= maxfd) && (_finish == false); i++)
			{
				if (FD_ISSET(i, &readfds))
				{
					if (i == _socketfd)
					{
						resetMessage(buf);
						bytesReceive = recv(i, buf, sizeof(buf), 0);
						if (bytesReceive <= 0)
						{
							if (bytesReceive == 0)
							{
								std::cout << SERVER_CRASHED << std::endl;
							}
							if (bytesReceive < 0)
							{
								std::cout << RECEIVE_ERROR << std::endl;
								retVal = FAILURE_EXIT;
							}
							_finish = true;
							close(i);
							FD_CLR(i,&master);
						}
						else if ((retVal = receiveFromServer(buf,clientsNames)) != NOT_EXIT)
						{
							_finish = true;
							close(i);
							FD_CLR(i,&master);
						}
					}
					else if (i == STDIN)
					{
						if ((getCommands() == true) || (_send == FAILURE))
						{
							_finish = true;
							close(_socketfd);
							FD_CLR(i,&master);
						}
					}
				}
			}

			if (_finish == true)
			{
				break;
			}
		}
    }
    if (retVal == FAILURE_EXIT)
    {
    	exit(FAILURE_EXIT);
    }
    if (_send == FAILURE)
    {
        exit(FAILURE_EXIT);
    }
}

/* Inits the client.
   If the initialization went successfully, it returns SUCEESS.
   Otherwise, it returns FAILURE. */
int twitClient::initTwitClient(const std::string& clientName, const char* serverAddress,
                   const std::string& portNum)
{
    unsigned int portVal;
    _finish = false;
    if (checkClientName(clientName) == FAILURE)
    {
        return FAILURE;
    }
    if ((int)(portVal = convertPortNum(portNum)) == FAILURE)
    {
        return FAILURE;
    }
    if ( (portVal >= MIN_PRESERVED_PORT) && (portVal <= MAX_PRESERVED_PORT) )
    {
    	std::cout << PORT_AT_LEAST << std::endl;
    	return FAILURE;
    }
    struct sockaddr_in server_addr;
    if (createSocketAddress(serverAddress,portVal,server_addr) == FAILURE)
    {
        std::cout << ADDRESS_ERROR << std::endl;
        return FAILURE;
    }
    if ((_socketfd = socket(PF_INET,SOCK_STREAM,0)) == FAILURE)
    {
    	std::cout << SOCKET_ERROR << std::endl;
    	return FAILURE;
    }
    if (connect(_socketfd,(struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == FAILURE)
    {
    	std::cout << CONNECT_ERROR << std::endl;
    	return FAILURE;
    }
    char command[] = BEGIN;
    createAndSendMessage(command,clientName);
    _session.setPreviousSendMessage(BEGIN);
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
        std::string clientName,serverAddress,serverPort;
        clientName.assign(argv[1],strlen(argv[1]));
        serverAddress.assign(argv[2],strlen(argv[2]));
        serverPort.assign(argv[3],strlen(argv[3]));
        twitClient* client = new twitClient();
        if (client->initTwitClient(clientName,argv[2],serverPort) == FAILURE)
        {
            exit(FAILURE_EXIT);
        }
        client->sendReceive();
    }
    return SUCCESS_EXIT;
}