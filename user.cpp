#include "user.h"
#include <string>

user::user(std::string ulogin, bool leech, bool protect, unsigned int ukarmatmp) : login(ulogin), leechstatus(leech), protect_ip(protect), karmatmp(ukarmatmp) {
	stats.leeching = 0;
	stats.seeding = 0;
}

std::string user::get_login() {
	return login;
}

bool user::is_protected() {
	return protect_ip;
}

void user::set_protected(bool status) {
	protect_ip = status;
}

bool user::can_leech() {
	return leechstatus;
}

void user::set_leechstatus(bool status) {
	leechstatus = status;
}

// Stats methods
unsigned int user::get_leeching() {
	return stats.leeching;
}

unsigned int user::get_seeding() {
	return stats.seeding;
}

void user::decr_leeching() {
	stats.leeching--;
}

void user::decr_seeding() {
	stats.seeding--;
}

void user::incr_leeching() {
	stats.leeching++;
}

void user::incr_seeding() {
	stats.seeding++;
}
