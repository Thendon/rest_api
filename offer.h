//******************************************************************
//Offer & OfferManager
//Florian Herrmann
//Steffen Adreessen
//******************************************************************

#pragma once

#include <json.hpp>
#include <ctime>
#include "DatabaseManager.h"

using json = nlohmann::json;
using namespace std;

class User;

class Offer
{
public:
	Offer(unsigned int id, string titel, int amount, time_t created, time_t duration, User* creator);
	~Offer() {};
	json toJSON(bool db = false);
	string toString();
	bool take( unsigned int amount );
	int getAmount() { return this->amount; }
	void saveToDB();

	unsigned int id;
	string titel;
	User* creator;
private:
	time_t created;
	time_t duration;
	int amount;
};

class OfferManager
{
public:
	static OfferManager* instance();
	~OfferManager();
	json getAvaliable();
	string toString();
	string tableCreationString();
	json getOffersOf(User * user);
	Offer* getOffer(unsigned int id);
	void addOffer(Offer* offer);
	void addOffer(string titel, int amount, time_t duration, User* creator);
	bool acceptOffer(unsigned int id, unsigned int amount);
	bool fetchData();
private:
	OfferManager() {};
	static OfferManager* offerManager;
	unsigned int ids;
	vector< Offer* > offers;
};