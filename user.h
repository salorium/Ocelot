#include <string>
#include <mutex>
class user {
	private:
		std::string login;
		bool leechstatus;
		bool protect_ip;
		struct {
			unsigned int leeching;
			unsigned int seeding;
		} stats;
	public:
        unsigned karmatmp;
        std::mutex mutex;
		user(std::string ulogin, bool leech, bool protect, unsigned int ukarmatmp);
		std::string get_login();
		bool is_protected();
		void set_protected(bool status);
		bool can_leech();
		void set_leechstatus(bool status);
		void decr_leeching();
		void decr_seeding();
		void incr_leeching();
		void incr_seeding();
		unsigned int get_leeching();
		unsigned int get_seeding();
};
