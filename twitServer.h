/*
 * File:   twitServer.h
 * Author: tamirtf77
 *
 */

#ifndef TWITSERVER_H
#define	TWITSERVER_H

#include <set>
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <map>
#include <sstream>
#include <algorithm>
#include <vector>

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctime>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#define NUM_SERVER_ARGUMENTS 2
#define FAILURE_EXIT 1
#define SUCCESS 0
#define FAILURE -1
#define LOG_FILE "twitServer.log"
#define FINISH 1
#define UNFINISH 2
#define POSSIBLE_CONNECTIONS 100
#define STDIN 0
#define PACKAGE_SIZE 250

#define ERROR "Error: "
#define ILLEGAL_NUM_OF_ARGUMENTS "Error: the number of arguments should be 1"
#define LOG_ERROR "Error: the log file could not be open"
#define DIGITS "0123456789"
#define PORT_ILLEGAL "Error: the port number should be natural number"
#define PORT_AT_LEAST "Error: the port number should be greater than 1024"
#define CLIENT_COMMAND_ILLEGAL "'s command is illegal - does not follow the protocol"
#define SOCKET_ERROR "Error: could not open socket stream"
#define BIND_ERROR "Error: could not allocate a network address"
#define LISTEN_ERROR "Error: could not listening for clients requests"
#define SYSTEM_ERROR "Error: system problem"
#define CRASHED " crashed"
#define RECEIVE_ERROR "Error: an error occurred in receiving a message from "
#define SPACE " "
#define CHAR_SPACE ' '
#define CHAR_FINISH '\0'
#define MIN_PRESERVED_PORT 0
#define MAX_PRESERVED_PORT 1024
#define SEND_ERROR "Error: could not send a message to "
#define USER_ILLEGAL_COMMAND "Error: your command is illegal"
#define NOT_EXIST_TO_SEND " does not exist"
#define TRY "a client tries to connect with name "
#define ALREADY " which is already in use"
#define UNAUTHORIZED_LOG " made unauthorized access - does not follow the protocol"
#define COULD_NOT_DM "could not receive a direct message from "
#define DUE_BLOCK "due to a block"
#define CLIENT_NOT_INTRODUCED "a client name which did not manage to introduce himself"
#define SERVER_START "server start"
#define SERVER_EXIT "server finish"

#define EMPTY ""
#define BEGIN "BEGIN"
#define WELCOME "WELCOME"
#define NAME_USED "NAME_USED"
#define UNAUTHORIZED "Error: unauthorized access"
#define ILLEGAL_COMMAND "Error: illegal command"
#define NOT_EXIST "NOT_EXIST"
#define EXIT "EXIT"
#define FOLLOW "FOLLOW"
#define UNFOLLOW "UNFOLLOW"
#define TWIT "TWIT"
#define TWIT_FROM "TWIT_FROM"
#define DM "DM"
#define DM_FROM "DM_FROM"
#define BLOCK "BLOCK"
#define WHO "WHO"
#define WHO_FINISH "WHO_FINISH"

#define CONNECTED "connected"
#define DISCONNECTED "disconnected"
#define FOLLOWED "followed"
#define UNFOLLOWED "unfollowed"
#define TWITTED "twitted"
#define DM_NAME "direct messaged"
#define BLOCKED "blocked"
#define WHO_LOG "Who"

#define DOT "."
#define NEW_LINE "\n"
#define TAB "\t"
#define COLON ":"
#define SHTRUDEL "@"
#define SEPARATOR "--"
#define DIVIDING_LINE " - "
#define PREFIX_FOR_TIME "0"

class twitServer {
public:
    int initTwitServer(const std::string& portNum);
    
private:
    std::ofstream _file;
    int _listener;
    struct sockaddr_in _my_addr;
    std::map<int,std::string> _sockets; // the key is the socket number, the value is the client name.
    bool _finish;
    std::set<int> fdsToRemove;

    typedef std::set<std::string> followers;
    typedef std::set<std::string>::const_iterator setConstIter;

    class Data
    {
        
    public:
        Data(int socket);
        void clearAll();
        followers& getFollowers();
        followers& getBlocks();
        int getSocket();
    private:
        int _socket;
        followers _followers;
        followers _blocks;
    };

    struct lexIsLess
    {
        bool operator() ( std::string const & a, std::string const & b ) const
        {
            std::string aa = a; std::string bb = b;
            std::transform(aa.begin(), aa.end(), aa.begin(), ::tolower);
            std::transform(bb.begin(), bb.end(), bb.begin(), ::tolower);
            return ( std::lexicographical_compare( aa.begin(), aa.end(), bb.begin(), bb.end() ) );
        }
    };

    // The key is a client name, the value is Data of a client name.
    std::map<std::string,Data,lexIsLess> _clients;
    typedef std::map<std::string,Data,lexIsLess> clientsMap;
    typedef clientsMap::iterator mapClientIter;
    typedef clientsMap::const_iterator mapClientConstIter;

    struct orderSet
    {
        bool operator() (const int &a, const int &b) const
        {
            return (a > b);
        }
    };


    
    std::string upperCopy(const std::string& str) const;

    std::string lowerCopy(const std::string& str) const;

    int writeToLog(const std::string& logEntry);

    void resetMessage(char* msg) const;

    void fillMessage(char* msg, const char* command,const std::string & other) const;

    int sendMessage(const int& socketfd,const char* msg) const;

    int createAndSendMessage(const int& socketfd,char command[],const std::string& other) const;

    void getTimeOfBrodcast(tm*& ltm) const;

    void addingTimeToMessage(std::string& message,const tm* ltm) const;

    void createDirectMessageToSend(std::string& dm,std::string& message,const std::string& clientName,
                               const std::string& someClientName,const tm* ltm) const;

    void createTwitToSend(std::string& twit,const std::string& message,const std::string& clientName,
                      const tm* ltm) const;

    void createMessageNotExist(const std::string& someClientName, std::string& message) const;

    void createStartFinishEntryLog(std::string& str,const std::string& startFinish) const;

    void createLogEntry1(const std::string& clientName,const std::string& wordLink,
                     const std::string& message, std::string& logEntry) const;

    void createLogEntry2(const std::string& clientName, const std::string& someClientName,
                     const std::string& wordLink, std::string& logEntry) const;

    void createLogEntryError(const std::string& someClientName,const std::string& errorDetails,
                         std::string& logEntry) const;

    void closeAllConnections();

    void deleteClientName(const std::string& clientName,clientsMap& aMap) const;

    void performDisconnect(const int& socketfd,const std::string& clientName);

    void performIllegalCommand(const int& socketfd,const std::string& clientName);

    void performNotExist(const int& socketfd,const std::string& someClientName,std::string logEntry);

    bool performFollow(const int& socketfd,const std::string& clientName, const std::string& someClientName);

    bool performUnfollow(const int& socketfd,const std::string& clientName, const std::string& someClientName);

    bool performTwit(const std::string& clientName, std::string& message,const tm* ltm);

    void separateByDelimeter(const std::string& str,std::vector<std::string>& tokens,
                         const std::string& delimiters) const;

    bool performDirectMessage(const int& socketfd,const std::string& clientName,
                          const std::string& SomeClientNameAndMessage,const tm* ltm);

    bool performBlock(const int& socketfd,const std::string& clientName, const std::string& someClientName);
    void performWho(const int& socketfd,const std::string& clientName);

    bool welcomeNewClient(const int& socketfd,const std::vector<std::string>& tokens);

    void concatMessage(std::string& msg, const std::vector<std::string>& tokens) const;

    void performClientCommand(const int& socketfd,const std::vector<std::string>& tokens,
                          const tm* ltm);

    void splitMessage(std::string& msg,std::vector<std::string>& tokens) const;

    bool advanceSession(int socketfd,char* buf,const tm* ltm);

    int performUserCommand(const std::vector<std::string>& tokens) const;

    void splitUserLine(const std::string& line, std::stringstream& stringStream,
               std::vector<std::string>& tokens) const;

    bool receiveCommandFromUser();

    void listening();

    int convertPortNum(const std::string& portNum) const;

    int openLogFile();

    void createSocketAddress(const int& portNum,struct sockaddr_in& my_addr) const;
    
    int openListeningSocket(int portNum);
};

#endif	/* TWITSERVER_H */