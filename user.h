//******************************************************************
//User & UserManager
//Florian Herrmann
//Steffen Adreessen
//******************************************************************

#pragma once

#include <json.hpp>
#include <boost/crc.hpp>
#include <ctime>
#include <json.hpp>
#include "offer.h"
#include "DatabaseManager.h"

#define REST_TIMEOUT 30000

using json = nlohmann::json;
using namespace std;
using namespace boost;

typedef int sessionID;

class User
{
public:
	User(unsigned int id, string name, int password);
	User(unsigned int id, string name, int password, string firstname, string lastname, time_t birthdate, string address, string city, string phone);
	~User() {};
	void setDetails(string firstname, string lastname, time_t birthdate, string address, string city, string phone);
	bool checkPassword(int checksum, int key);
	string toString();
	json toJSON( bool db = false );
	void addOffer( string titel, unsigned int amount, time_t duration );
	void saveToDB();

	unsigned int id;
	string username;
	time_t lastAction;
private:
	bool online;
	int password;

	string firstname;
	string lastname;
	time_t birthdate;
	string address;
	string city;
	string phone;
	vector< Offer > offers;
};

class UserManager
{
public:
	static UserManager* instance();
	~UserManager();
	int getLoginKey(string name);
	User* getUser(string name);
	User* getUser(int userID);
	User* session(sessionID sessionID);
	void removeSessionsOf(User* user);
	sessionID login(string name, int checksum);
	void logout(sessionID sessionID);
	bool reg(string name, string password, string firstname, string lastname, time_t birthdate, string address, string city, string phone);
	void addUser(User* user);
	string tableCreationString();
	string toString();
	bool fetchData();
	map<sessionID, User*>* getActiveSessions() { return &activeSessions; }
private:
	int toCRC(string str);
	UserManager() {};

	unsigned int userIDs = 0;
	sessionID sessionIDs = 0;
	static UserManager* userManager;
	vector< User* > users;
	map< User*, int > keyChain;
	map< sessionID, User* > activeSessions;
};