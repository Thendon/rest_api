#include "stdafx.h"
#include "RESTManager.h"

RESTManager* RESTManager::restManager;

RESTManager* RESTManager::instance()
{
	if (!RESTManager::restManager)
		RESTManager::restManager = new RESTManager();

	return RESTManager::restManager;
}

void RESTManager::host_REST()
{
	auto settings = make_shared< Settings >();
	settings->set_port(1337);
	//settings->set_default_header("Connection", "close");
	settings->set_default_headers({
		{ "Connection", "close" },
		{ "Access-Control-Allow-Origin", "*" },
		{ "Access-Control-Allow-Methods", "GET, POST" },
		{ "Access-Control-Allow-Headers", "Content-Type, Content-Length" },
		{ "Access-Control-Allow-Credentials", "true" }
	});

	restService = new Service();
	restService->set_logger(make_shared< SimpleLogger >());
	add_path("login", login_handler, restService);
	add_path("logout", logout_handler, restService);
	add_path("register", register_handler, restService);
	add_path("userDetails", details_handler, restService);
	add_path("offersAvaliable", offersAvaliable_handler, restService);
	add_path("offersUser", offersUser_handler, restService);
	add_path("addOffer", addOffer_handler, restService);
	add_path("takeOffer", takeOffer_handler, restService);

	restRunning = true;
	printf("starting web server ...\n");
	restService->start(settings);
	restRunning = false;
	delete restService;
}

void RESTManager::shutdown_REST()
{
	try {
		while (true) {
			if (!restRunning)
			{
				break;
			}

			try {
				printf("stopping web server ...\n");
				restService->stop();
			}
			catch (...) {
				printf("stopping error\n");
			}
			//this_thread::sleep_for(chrono::milliseconds(SCAN_SPEED));
		}
	}
	catch (std::exception e)
	{
		printf("%s\n", e.what());
	}
}

multimap< string, string > RESTManager::generateDefaultHeader(const int length)
{
	multimap< string, string > headers;
	headers.insert({ "Content-Type", "application/json" });
	headers.insert({ "Content-Length", to_string(length) });
	return headers;
}

void RESTManager::add_path(string name, routeHandler handler, Service* service)
{
	cout << "adding path " << name << "...\n";

	auto ressource = make_shared< Resource >();
	ressource->set_path(name);
	ressource->set_method_handler("POST", post_handler);
	ressource->set_method_handler("OPTIONS", options_handler);

	handlerMap[name] = handler;

	service->publish(ressource);
}

void login_handler(const json* req, json* res)
{
	UserManager* userManager = UserManager::instance();

	string name = req->at("username").get<string>();
	if (req->at("init").get<bool>())
	{
		int key = userManager->getLoginKey(name);

		if (key == -1)
			(*res)["error"] = "key not found";
		else
			(*res)["key"] = key;

		return;
	}

	int key = req->at("checksum").get<int>();
	sessionID SID = userManager->login(name, key);
	if (SID == -1)
		(*res)["error"] = "password incorrect";
	else {
		(*res)["sessionID"] = SID;
		(*res)["username"] = name;
	}
}

void logout_handler(const json* req, json* res)
{
	sessionID SID = req->at("sessionID").get<int>();
	UserManager::instance()->logout(SID);
}

void register_handler(const json* req, json* res)
{
	string username = req->at("username").get<string>();
	string pass = req->at("pass").get<string>();
	string firstname = req->at("firstname").get<string>();
	string lastname = req->at("lastname").get<string>();
	time_t birthdate = req->at("birthdate").get<int>();
	string address = req->at("address").get<string>();
	string city = req->at("city").get<string>();
	string phone = req->at("phone").get<string>();

	if (!UserManager::instance()->reg(username, pass, firstname, lastname, birthdate, address, city, phone))
		(*res)["error"] = "username already taken!";
}

void details_handler(const json* req, json* res)
{
	sessionID sID = req->at("sessionID").get<int>();
	if (!UserManager::instance()->session(sID))
	{
		(*res)["error"] = "unauthed";
		return;
	}

	string username = req->at("username").get<string>();

	User* user = UserManager::instance()->getUser(username);
	if (!user)
	{
		(*res)["error"] = "user not found!";
		return;
	}

	(*res)["details"] = user->toJSON();
}

void addOffer_handler(const json* req, json* res)
{
	sessionID sID = req->at("sessionID").get<int>();
	User* user = UserManager::instance()->session(sID);
	if (!user)
	{
		(*res)["error"] = "unauthed";
		return;
	}

	string titel = req->at("titel").get<string>();
	int amount = req->at("amount").get<int>();
	time_t duration = req->at("duration").get<int>();

	if (amount <= 0)
	{
		(*res)["error"] = "amount has to be at least 1";
		return;
	}

	if (duration <= time(0))
	{
		(*res)["error"] = "duration too low (nicht sicher ob das schon sinn macht duration: " + to_string(duration) + " < time: " + to_string(time(0)) + ")";
		return;
	}

	user->addOffer(titel, amount, duration);
}

void takeOffer_handler(const json* req, json* res)
{
	sessionID sID = req->at("sessionID").get<int>();
	if (!UserManager::instance()->session(sID))
	{
		(*res)["error"] = "unauthed";
		return;
	}

	int id = req->at("id").get<int>();
	int amount = req->at("amount").get<int>();

	if (amount <= 0)
	{
		(*res)["error"] = "amount has to be at least 1";
		return;
	}

	if (!OfferManager::instance()->acceptOffer(id, amount))
		(*res)["error"] = "offer / amount not avaliable";
}

void offersAvaliable_handler(const json* req, json* res)
{
	sessionID sID = req->at("sessionID").get<int>();
	if (!UserManager::instance()->session(sID))
	{
		(*res)["error"] = "unauthed";
		return;
	}

	(*res)["offers"] = OfferManager::instance()->getAvaliable();
}

void offersUser_handler(const json* req, json* res)
{
	sessionID sID = req->at("sessionID").get<int>();
	if (!UserManager::instance()->session(sID))
	{
		(*res)["error"] = "unauthed";
		return;
	}

	string username = req->at("username").get<string>();
	User* user = UserManager::instance()->getUser(username);
	if (!user)
	{
		(*res)["error"] = "username not found";
		return;
	}

	(*res)["offers"] = OfferManager::instance()->getOffersOf(user);
}

void fetch_handler(const std::shared_ptr< Session > session, const Bytes & body)
{
	json res;
	try {
		string path = session->get_request()->get_path().erase(0, 1);
		routeHandler func = RESTManager::instance()->handlerMap[path];
		if (!func)
			throw invalid_argument("(fatal) no route handler found!");

		json req = json::parse(string(body.begin(), body.end()));
		func(&req, &res);
		res["success"] = true;
	}
	catch (const std::exception& e) {
		printf("error: %s\n", e.what());
		res["success"] = false;
	}

	string ret = res.dump();
	multimap< string, string > headers = RESTManager::generateDefaultHeader((int)ret.length());
	session->close(OK, ret, headers);
}

void post_handler(const std::shared_ptr< Session > session)
{
	const auto request = session->get_request();
	int content_length = request->get_header("Content-Length", 0);

	session->fetch(content_length, fetch_handler);
}

void options_handler(const std::shared_ptr< Session > session)
{
	session->close(OK);
}