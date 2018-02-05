
#include "stdafx.h"
#include "user.h"

#define TABLE_USERS "users"

User::User(unsigned int id, string username, int password)
{
	this->id = id;
	this->username = username;
	this->password = password;
}

User::User(unsigned int id, string username, int password, string firstname, string lastname, time_t birthdate, string address, string city, string phone)
{
	this->id = id;
	this->username = username;
	this->password = password;
	this->firstname = firstname;
	this->lastname = lastname;
	this->birthdate = birthdate;
	this->address = address;
	this->city = city;
	this->phone = phone;
}

void User::setDetails(string firstname, string lastname, time_t birthdate, string address, string city, string phone)
{
	this->firstname = firstname;
	this->lastname = lastname;
	this->birthdate = birthdate;
	this->address = address;
	this->city = city;
	this->phone = phone;
}

bool User::checkPassword(int checksum, int key)
{
	if (this->password * key != checksum)
		return false;

	return true;
}

string User::toString()
{
	string ret = "";
	ret += to_string(this->id) + " ";
	ret += this->username + " ";
	ret += to_string(this->password) + " ";
	ret += this->firstname + " ";
	ret += this->lastname + " ";
	ret += this->address + " ";
	ret += this->city + " ";
	ret += this->phone + " ";
	return ret;
}

json User::toJSON( bool db )
{
	json ret = json({
		{ "id", this->id },
		{ "username", this->username },
		{ "firstname", this->firstname },
		{ "lastname", this->lastname },
		{ "address", this->address },
		{ "city", this->city },
		{ "phone", this->phone },
		{ "birthdate", this->birthdate }
	});

	if (db)
		ret["password"] = this->password;
	
	return ret;
}

void User::addOffer(string titel, unsigned int amount, time_t duration)
{
	OfferManager::instance()->addOffer(titel, amount, duration, this);
}

void User::saveToDB()
{
	DatabaseManager::instance()->save(TABLE_USERS, this->toJSON(true));
}

//******************************************************

UserManager* UserManager::userManager;

UserManager::~UserManager()
{
	for (vector< User* >::iterator user = users.begin(); user != users.end(); ++user)
		delete (*user);
};

UserManager* UserManager::instance()
{
	if (!UserManager::userManager)
		UserManager::userManager = new UserManager();

	return UserManager::userManager;
}

int UserManager::getLoginKey(string username)
{
	User* user = this->getUser(username);
	//user doesnt excist -> return errorcode -1
	if (!user)
		return -1;

	//generate random key
	int key = rand() % 1000 + 1234; //key range = 1234 - (1234+1000) => 1234 - 2234
									//save random key in chain
	this->keyChain[user] = key;

	return key;
}

User* UserManager::getUser(string username)
{
	for (vector< User* >::iterator user = users.begin(); user != users.end(); ++user) {
		if ((*user)->username == username)
			return (*user);
	}
	return NULL;
}

User* UserManager::getUser(int userID)
{
	for (vector< User* >::iterator user = users.begin(); user != users.end(); ++user) {
		if ((*user)->id == userID)
			return (*user);
	}
	return NULL;
}

User* UserManager::session(sessionID sessionID)
{
	User* user = activeSessions[sessionID];
	
	if(!user)
		return NULL;

	//if timed out loggin user out and removing his sessionID
	if (user->lastAction + REST_TIMEOUT < time(0))
	{
		cout << user->username << " timed out! (" << user->lastAction + REST_TIMEOUT << " < " << time(0) << ")\n";
		this->logout(sessionID);
		return NULL;
	}
	//refresh action time & return user
	user->lastAction = time(0);
	
	return user;
}

void UserManager::removeSessionsOf(User * user)
{
	for (auto it = activeSessions.begin(); it != activeSessions.end(); it++) 
	{
		if (it->second != user)
			continue;
		
		activeSessions.erase(it);
	}
}

sessionID UserManager::login(string username, int checksum)
{
	User* user = this->getUser(username);
	//check if user is in login process / even excisting
	if(!user || !keyChain[user])
		return -1;

	//remove key from keyChain
	int key = keyChain[user];
	keyChain.erase(user);

	//Check if password is correct
	if(!user->checkPassword(checksum, key))
		return -1;

	this->removeSessionsOf(user);
	activeSessions[sessionIDs] = user;
	user->lastAction = time(0);
	return sessionIDs++;
}

void UserManager::logout(sessionID SID)
{
	this->activeSessions.erase(SID);
}

bool UserManager::reg(string username, string password, string firstname, string lastname, time_t birthdate, string address, string city, string phone)
{
	if (this->getUser(username))
		return false;

	User* user = new User(this->userIDs, username, toCRC(password), firstname, lastname, birthdate, address, city, phone);
	user->saveToDB();
	this->addUser(user);

	return true;
}

void UserManager::addUser(User* user)
{
	//always keep ids the highest
	if (user->id >= this->userIDs)
		this->userIDs = user->id + 1;

	this->users.push_back(user);
}

string UserManager::tableCreationString()
{
	string queryString;
	queryString += "CREATE TABLE users(";
	queryString += "id INT NOT NULL,";
	queryString += "username VARCHAR(100),";
	queryString += "password INT,";
	queryString += "firstname VARCHAR(100),";
	queryString += "lastname VARCHAR(100),";
	queryString += "birthdate INT,";
	queryString += "address VARCHAR(100),";
	queryString += "city VARCHAR(100),";
	queryString += "phone VARCHAR(100),";
	queryString += "PRIMARY KEY (id))";
	return queryString;
}

bool UserManager::fetchData()
{
	DatabaseManager* databaseManager = DatabaseManager::instance();
	if (!databaseManager->tableCheck(TABLE_USERS))
	{
		std::cout << "[WARNING] table USERS not found, generating ... \n";
		if (!databaseManager->execute(this->tableCreationString()))
			return false;
	}

	json res = json::array();
	if (!databaseManager->executeQuery("SELECT * from users", &res))
		return false;

	for (json::iterator row = res.begin(); row != res.end(); ++row)
	{
		int userID = (*row).at("id").get<int>();
		string username = (*row).at("username").get<string>();
		int password = (*row).at("password").get<int>();
		string firstname = (*row).at("firstname").get<string>();
		string lastname = (*row).at("lastname").get<string>();
		time_t birthdate = (*row).at("birthdate").get<int>();
		string address = (*row).at("address").get<string>();
		string city = (*row).at("city").get<string>();
		string phone = (*row).at("phone").get<string>();

		this->addUser(new User(userID, username, password, firstname, lastname, birthdate, address, city, phone));
	}

	return true;
}

string UserManager::toString()
{
	string ret;
	for (vector< User* >::iterator user = users.begin(); user != users.end(); ++user) {
		ret.append((*user)->toString() + "\n");
	}
	return ret;
}

int UserManager::toCRC(string str)
{
	crc_32_type result;
	result.process_bytes(str.data(), str.length());
	return result.checksum();
}