//******************************************************************
//RESTManager
//Florian Herrmann
//Steffen Adreessen
//******************************************************************

#pragma once

#include <json.hpp>
#include <iostream>
#include <stdio.h>
#include <restbed>
#include "user.h"
#include "offer.h"
#include "simplelogger.hpp"

using namespace restbed;
using namespace std;
using json = nlohmann::json;
typedef void(*routeHandler)(const json*, json*);

class RESTManager
{
public:
	static RESTManager* instance();
	static multimap<string, string> generateDefaultHeader(const int length);
	void host_REST();
	void shutdown_REST();
	~RESTManager() {};

	map< string, routeHandler > handlerMap;
private:
	RESTManager() {};
	void add_path(string name, routeHandler handler, Service * service);

	static RESTManager* restManager;
	bool restRunning = false;
	Service *restService;
};

void login_handler(const json * req, json * res);
void logout_handler(const json * req, json * res);
void register_handler(const json * req, json * res);
void details_handler(const json * req, json * res);
void addOffer_handler(const json * req, json * res);
void takeOffer_handler(const json * req, json * res);
void offersAvaliable_handler(const json * req, json * res);
void offersUser_handler(const json * req, json * res);
void fetch_handler(const std::shared_ptr<Session> session, const Bytes & body);
void post_handler(const std::shared_ptr<Session> session);
void options_handler(const std::shared_ptr<Session> session);