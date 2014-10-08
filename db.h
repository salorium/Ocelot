#ifndef OCELOT_DB_H
#define OCELOT_DB_H
#pragma GCC visibility push(default)
#include <mysql++/mysql++.h>
#include <string>
#include <unordered_map>
#include <queue>
#include <mutex>

class mysql {
	private:
		mysqlpp::Connection conn;
		std::string update_user_buffer;
		std::string update_torrent_buffer;
		std::string update_heavy_peer_buffer;
		std::string update_light_peer_buffer;
		std::string update_snatch_buffer;
		std::string update_token_buffer;

		std::queue<std::string> user_queue;
		std::queue<std::string> torrent_queue;
		std::queue<std::string> peer_queue;
		std::queue<std::string> snatch_queue;
		std::queue<std::string> token_queue;

		std::string db, server, db_user, pw;
		bool u_active, t_active, p_active, s_active, tok_active;

		// These locks prevent more than one thread from reading/writing the buffers.
		// These should be held for the minimum time possible.
		std::mutex user_queue_lock;
		std::mutex torrent_buffer_lock;
		std::mutex torrent_queue_lock;
		std::mutex peer_queue_lock;
		std::mutex snatch_queue_lock;
		std::mutex token_queue_lock;

		void do_flush_users();
		void do_flush_torrents();
		void do_flush_snatches();
		void do_flush_peers();
		void do_flush_tokens();

		void flush_users();
		void flush_torrents();
		void flush_snatches();
		void flush_peers();
		void flush_tokens();
		void clear_peer_data();

	public:
		bool verbose_flush;

		mysql(std::string mysql_db, std::string mysql_host, std::string username, std::string password);
		bool connected();
		void load_torrents(torrent_list &torrents);
		void load_users(user_list &users);
		void load_tokens(torrent_list &torrents);
		void load_whitelist(std::vector<std::string> &whitelist);

		void record_user(std::string &record,std::string &ulogin); // (id,uploaded_change,downloaded_change)
		void record_torrent(std::string &record); // (id,seeders,leechers,snatched_change,balance)
		void record_snatch(std::string &record, std::string &ip); // (uid,fid,tstamp)
		void record_peer(std::string &record, std::string &ip, std::string &peer_id, std::string &useragent,std::string &ulogin); // (uid,fid,active,peerid,useragent,ip,uploaded,downloaded,upspeed,downspeed,left,timespent,announces,tstamp)
		void record_peer(std::string &record, std::string &peer_id); // (fid,peerid,timespent,announces,tstamp)
		void record_token(std::string &record);

		void flush();

		bool all_clear();

		std::mutex torrent_list_mutex;
};

#pragma GCC visibility pop
#endif
