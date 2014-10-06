#include <iostream>
#include <map>
#include <sstream>
#include "ocelot.h"
#include "misc_functions.h"
#include "report.h"
#include "response.h"
#include "user.h"

std::string report(params_type &params, user_list &users_list) {
	std::stringstream output;
	std::string action = params["get"];
	if (action == "") {
		output << "Invalid action\n";
	} else if (action == "stats") {
		time_t uptime = time(NULL) - stats.start_time;
		int up_d = uptime / 86400;
		uptime -= up_d * 86400;
		int up_h = uptime / 3600;
		uptime -= up_h * 3600;
		int up_m = uptime / 60;
		int up_s = uptime - up_m * 60;
		std::string up_ht = up_h <= 9 ? '0' + std::to_string(up_h) : std::to_string(up_h);
		std::string up_mt = up_m <= 9 ? '0' + std::to_string(up_m) : std::to_string(up_m);
		std::string up_st = up_s <= 9 ? '0' + std::to_string(up_s) : std::to_string(up_s);
        double nbt = static_cast<double>(stats.nbtorrents);
        double nbts6 = static_cast<double>(stats.nbtorrentsseederipv6);
        double nbts4 = static_cast<double>(stats.nbtorrentsseederipv4);
		output << "Uptime: " << up_d << " days, " << up_ht << ':' << up_mt << ':' << up_st << '\n'
		<< stats.opened_connections << " connections opened\n"
		<< stats.open_connections << " open connections\n"
		<< stats.connection_rate << " connections/s\n"
		<< stats.succ_announcements << " successful announcements\n"
		<< (stats.announcements - stats.succ_announcements) << " failed announcements\n"
		<< stats.scrapes << " scrapes\n"
		<< stats.leechers << " leechers tracked\n"
        << stats.leechersipv4 << " leechers ipv4 tracked\n"
        << stats.leechersipv6 << " leechers ipv6 tracked\n"
		<< stats.seeders << " seeders tracked\n"
        << stats.seedersipv4 << " seeders ipv4 tracked\n"
        << stats.seedersipv6 << " seeders ipv6 tracked\n"
        << stats.nbtorrents << " torrents\n"
        << stats.nbtorrentsseederipv4 << " torrent with seeders ipv4\n"
        << stats.nbtorrentsseederipv6 << " torrent with seeders ipv6\n"
        << (nbts6 / nbt *100.00 )<< " % torrent with seeders ipv6\n"
        << (nbts4 / nbt *100.00 )<< " % torrent with seeders ipv4\n"
        << stats.bytes_read << " bytes read\n"
		<< stats.bytes_written << " bytes written\n";
	} else if (action == "user") {
		std::string key = params["key"];
		if (key == "") {
			output << "Invalid action\n";
		} else {
			user_list::const_iterator u = users_list.find(key);
			if (u != users_list.end()) {
				output << u->second->get_leeching() << " leeching\n"
				<< u->second->get_seeding() << " seeding\n";
			}
		}
	} else {
		output << "Invalid action\n";
	}
	output << "success";
	return response(output.str(), false, false);
}
