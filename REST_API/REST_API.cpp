//******************************************************************
//REST_API
//Florian Herrmann
//Steffen Adreessen
//******************************************************************

#include "stdafx.h"
#include <stdio.h>
#include <thread>
#include <iostream>
#include "user.h" //differentiate Manager
#include "offer.h" //differentiate Manager
#include "DatabaseManager.h"
#include "RESTManager.h"

#define SCAN_SPEED 1000

bool load_data();
bool main_menu();

int main(int argc, char** argv)
{
	RESTManager* restManager = RESTManager::instance();
	//Setting random seed (for key generation)
	srand((unsigned int)time(nullptr));
	//Load data
	if (!load_data()) {
		cin.get();
		return 1;
	}
	//Boot REST Server
	thread(&RESTManager::host_REST, restManager).detach();
	//Open main menu
	while (main_menu());

	//Wait for shutdown of server & close
	thread wait(&RESTManager::shutdown_REST, restManager);
	wait.join();
    return 0;
}

//*************************************
// Inits & Menus
//*************************************

bool load_data()
{
	//Check MySQL status
	if (!DatabaseManager::instance()->connect("tcp://127.0.0.1:3306", "food", "fooood", "food"))
		return false;

	if (!UserManager::instance()->fetchData())
		return false;

	if (!OfferManager::instance()->fetchData())
		return false;

	return true;
}

bool main_menu() {
	string input;
	cout << "Commands => -E-xit || Printer => -H-elp -U-ser -O-ffers A-active" << "\n";
	cin >> input;
	if (input == "E") {
		return false;
	}
	if (input == "H") {
		return true;
	}
	if (input == "U") {
		cout << UserManager::instance()->toString() << "\n";
	}
	if (input == "O") {
		cout << OfferManager::instance()->toString() << "\n";
	}
	if (input == "A") {
		map<sessionID, User*>* sessions = UserManager::instance()->getActiveSessions();
		for(auto it = sessions->begin(); it != sessions->end(); it++)
		{
			if (!it->second)
				continue;

			cout << it->second->id << " " << it->second->username << " session: " << it->first << "\n";
		}
	}
	return true;
}