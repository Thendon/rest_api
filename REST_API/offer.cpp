
#include "stdafx.h"
#include "offer.h"
#include "user.h"

#define TABLE_OFFERS "offers"

Offer::Offer(unsigned int id, string titel, int amount, time_t created, time_t duration, User* creator)
{
	this->id = id;
	this->titel = titel;
	this->amount = amount;
	this->created = created;
	this->duration = duration;
	this->creator = creator;
}

string Offer::toString()
{
	string ret = "";
	ret += to_string(this->id) + " ";
	ret += this->titel + " ";
	ret += this->creator->username + " ";
	return ret;
}

bool Offer::take(unsigned int amount)
{
	if (this->amount < (int)amount)
		return false;

	this->amount -= amount;
	return true;
}

void Offer::saveToDB()
{
	DatabaseManager::instance()->save(TABLE_OFFERS, this->toJSON( true ));
}

json Offer::toJSON( bool db )
{
	json ret = json({
		{ "id", this->id },
		{ "titel", this->titel },
		{ "amount", this->amount },
		{ "created", this->created },
		{ "duration", this->duration },
	});

	if (db)
		ret["userID"] = this->creator->id;
	else
		ret["creator"] = this->creator->username;

	return ret;
}

//**********************************

OfferManager* OfferManager::offerManager;

OfferManager* OfferManager::instance()
{
	if (!OfferManager::offerManager)
		OfferManager::offerManager = new OfferManager();

	return OfferManager::offerManager;
}

OfferManager::~OfferManager()
{
	for (vector< Offer* >::iterator offer = this->offers.begin(); offer != this->offers.end(); ++offer)
		delete (*offer);
}

json OfferManager::getAvaliable()
{
	json ret = json::array();
	for (vector< Offer* >::iterator offer = this->offers.begin(); offer != this->offers.end(); ++offer) {
		if ((*offer)->getAmount() <= 0)
			continue;

		ret.push_back((*offer)->toJSON());
	}
	return ret;
}

string OfferManager::toString()
{
	string ret;
	for (vector< Offer* >::iterator offer = offers.begin(); offer != offers.end(); ++offer) {
		ret.append((*offer)->toString() + "\n");
	}
	return ret;
}

string OfferManager::tableCreationString()
{
	string queryString;
	queryString += "CREATE TABLE offers(";
	queryString += "id INT NOT NULL,";
	queryString += "titel VARCHAR(100),";
	queryString += "amount INT,";
	queryString += "created INT,";
	queryString += "duration INT,";
	queryString += "userID INT,";
	queryString += "PRIMARY KEY (id))";
	return queryString;
}

bool OfferManager::fetchData()
{
	DatabaseManager* databaseManager = DatabaseManager::instance();

	if (!databaseManager->tableCheck(TABLE_OFFERS))
	{
		std::cout << "[WARNING] table OFFERS not found, generating ... \n";
		if (!databaseManager->execute(this->tableCreationString()))
			return false;
	}

	json res = json::array();
	if (!databaseManager->executeQuery("SELECT * from offers", &res))
		return false;

	for (json::iterator row = res.begin(); row != res.end(); ++row)
	{
		int offerID = (*row).at("id").get<int>();
		string titel = (*row).at("titel").get<string>();
		int amount = (*row).at("amount").get<int>();
		int created = (*row).at("created").get<int>();
		int duration = (*row).at("duration").get<int>();
		int userID = (*row).at("userID").get<int>();

		this->addOffer( new Offer( offerID, titel, amount, created, duration, UserManager::instance()->getUser(userID)) );
	}

	return true;
}

json OfferManager::getOffersOf(User* user) 
{
	json ret = json::array();
	for (vector< Offer* >::iterator offer = this->offers.begin(); offer != this->offers.end(); ++offer) {
		if ((*offer)->creator != user)
			continue;

		ret.push_back((*offer)->toJSON());
	}

	return ret;
}

Offer* OfferManager::getOffer(unsigned int id)
{
	for (vector< Offer* >::iterator offer = this->offers.begin(); offer != this->offers.end(); ++offer) {
		if ((*offer)->id == id)
			return (*offer);
	}
	return NULL;
}

void OfferManager::addOffer(Offer* offer)
{
	//always keep ids the highest
	if (offer->id >= this->ids)
		this->ids = offer->id + 1;

	this->offers.push_back(offer);
}

void OfferManager::addOffer(string titel, int amount, time_t duration, User * creator)
{
	Offer* offer = new Offer(this->ids, titel, amount, time(0), duration, creator);
	offer->saveToDB();
	this->addOffer(offer);
}

bool OfferManager::acceptOffer(unsigned int id, unsigned int amount)
{
	Offer* offer = this->getOffer(id);
	if (!offer)
		return false;

	if (!offer->take(amount))
		return false;

	offer->saveToDB();

	return true;
}
