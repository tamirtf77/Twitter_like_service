/*
 * File:   twitClient.h
 * Author: tamirtf77
 * 
 */

#ifndef TWITCLIENT_H
#define	TWITCLIENT_H

#include <sstream>
#include <vector>
#include <algorithm>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <set>
#include <unistd.h>
#include <signal.h>

#include <cstdlib>
#include <string>
#include <iostream>
#include <string.h>

#define NUM_SERVER_ARGUMENTS 4
#define SUCCESS_EXIT 0
#define FAILURE_EXIT 1
#define NOT_EXIT 2
#define SUCCESS 0
#define FAILURE -1
#define FINISH 1
#define UNFINISH 2
#define PACKAGE_SIZE 250
#define STDIN 0
#define LOCAL_HOST "localhost"
#define LOCAL_HOST_IP "127.0.0.1"
#define INET_FAILURE 0

#define ERROR "Error: "
#define ILLEGAL_NUM_OF_ARGUMENTS "Error: the number of arguments should be 3"
#define NAME_IS_NULL "Error: the client name is null"
#define MAXIMUM_CHARS 30
#define MAXIMUM_CHARS_MESSAGE 140
#define MIN_PRESERVED_PORT 0
#define MAX_PRESERVED_PORT 1024
#define DIGITS "0123456789"
#define DIGITS_DOT "0123456789."
#define PORT_ILLEGAL "Error: the port number should be natural number"
#define PORT_AT_LEAST "Error: the port number should be greater than 1024"
#define NAME_LONG "Error: the client name should be no longer than 30 characters"
#define ILLEGAL_SPACE "Error: the client name should not contains white-space"
#define ILLEGAL_SHTRUDEL "Error: the client name should not contains @"

#define USER_ILLEGAL_COMMAND "Error: your command is illegal"
#define SOCKET_ERROR "Error: could not open socket stream"
#define CONNECT_ERROR "Error: could not connect to the server - wrong port"
#define SEND_ERROR "Error: could not send a message"
#define DM_ERROR "Error: DM arguments format is <SOME CLIENT NAME>@<Message>"
#define SYSTEM_ERROR "Error: system problem"
#define ADDRESS_ERROR "Error: wrong address"

#define CONNECT_SUCCESS_PRINT "Connected successfully"
#define SERVER_EXIT_PRINT "Server disconnected"
#define SERVER_CRASHED "Server crashed"
#define RECEIVE_ERROR "Error: an error occurred in receiving a message"
#define NAME_USED_PRINT "Error: your client name is already in use"
#define UNAUTHORIZED_PRINT "Error: unauthorized access"
#define NOT_EXIST_PRINT " does not exist"
#define SERVER_ERROR_PRINT "Error: the server sent an unexpected message, it might be a mock server"

#define EMPTY ""
#define BEGIN "BEGIN"
#define WELCOME "WELCOME"
#define NAME_USED "NAME_USED"
#define UNAUTHORIZED "UNAUTHORIZED"
#define ILLEGAL_COMMAND "ILLEGAL_COMMAND"
#define NOT_EXIST "NOT_EXIST"
#define EXIT "EXIT"
#define FOLLOW "FOLLOW"
#define UNFOLLOW "UNFOLLOW"
#define TWIT "TWIT"
#define DM "DM"
#define BLOCK "BLOCK"
#define WHO "WHO"
#define WHO_FINISH "WHO_FINISH"
#define TWIT_FROM "TWIT_FROM"
#define DM_FROM "DM_FROM"

#define SHTRUDEL "@"
#define SPACE " "
#define TAB "\t"
#define CHAR_SPACE ' '
#define CHAR_FINISH '\0'

class twitClient {

public:
    int initTwitClient(const std::string& clientName, const char* serverAddress,
                   const std::string& portNum);
    void sendReceive();
    
private:

    /* An inner class which tells which command the client should receive in any time
        of the session with the server. */
    class Session
    {
    public:
        Session();
        void setPreviousReceiveMessage(const std::string& command);
        void setPreviousSendMessage(const std::string& command);
        void messageToReceive(std::set<std::string>& receive);
        
    private:
        void introductionMessageToReceive(std::set<std::string>& receive);
        void receiveMessageAccordingToCommand(std::set<std::string>& receive) const;
        void insertAll(std::set<std::string>& send) const;
        bool previousSendReceive(const std::string& send, const std::string& receive) const;
        std::string _previousSendMessage, _previousReceiveMessage;
        bool _introductionOver,_sessionOver;
    };

    int _socketfd;
    Session _session;
    bool _finish;
    int _send;

    int checkClientName(const std::string& clientName) const;
    int convertPortNum(const std::string& portNum) const;
    int createSocketAddress(const char* serverAddress, const unsigned int& portVal,struct sockaddr_in& my_addr) const;
    void resetMessage(char* msg) const;
    void truncateMessage(std::string& message) const;
    void fillMessage(char* msg, const char* command, const std::string& other) const;
    void sendMessage(const char* msg);
    void createAndSendMessage(char command[],const std::string& other);
    std::string upperCopy(const std::string& str) const;
    void splitLine(std::string& line,std::vector<std::string>& tokens) const;
    void concatMessage(std::string& msg, const std::vector<std::string>& tokens) const;
    void performDisconnect(const std::vector<std::string>& tokens);
    void performFollow(const std::vector<std::string>& tokens);
    void performUnfollow(const std::vector<std::string>& tokens);
    void performTwit(const std::vector<std::string>& tokens);
    void separateByDelimiter(const std::string& str,std::vector<std::string>& tokens,
                         const std::string& delimiters) const;
    void performDirectMessage(const std::vector<std::string>& tokens);
    void performBlock(const std::vector<std::string>& tokens);
    void performWho(const std::vector<std::string>& tokens);
    int performCommand(const std::vector<std::string>& tokens);
    bool getCommands();
    void messageToPrint(const std::vector<std::string>& tokens,
                    std::vector<std::string>& clientsNames) const;
    int receiveFromServer(char* buf,std::vector<std::string>& clientsNames);
};

#endif	/* TWITCLIENT_H */

