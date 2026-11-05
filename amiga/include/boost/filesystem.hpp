/* The part of Boost.Filesystem Aleph One uses (Files/FileHandler.cpp,
 * XML/Plugins.cpp), on AmigaDOS. Boost.Filesystem is a compiled library
 * that has no AmigaOS port; everything else Aleph One takes from Boost is
 * header-only. Paths are AmigaDOS names ("PROGDIR:Data/Map"); the calls are
 * in amiga/amiga_fs.cpp. */
#ifndef ALEPHONE_AMIGA_BOOST_FILESYSTEM_HPP
#define ALEPHONE_AMIGA_BOOST_FILESYSTEM_HPP

#include <string>
#include <ctime>
#include <memory>
#include <boost/system/error_code.hpp>

namespace boost {
namespace filesystem {

class path {
public:
	typedef char value_type;
	typedef std::string string_type;
	path() {}
	path(const std::string& s) : s_(s) {}
	path(const char* s) : s_(s) {}
	const std::string& native() const { return s_; }
	const char* c_str() const { return s_.c_str(); }
	std::string string() const { return s_; }
	bool empty() const { return s_.empty(); }
	/* the last part of an AmigaDOS name: after the last '/' or ':' */
	path filename() const {
		std::string::size_type p = s_.find_last_of("/:");
		return p == std::string::npos ? path(s_) : path(s_.substr(p + 1));
	}
	friend bool operator==(const path& a, const path& b) { return a.s_ == b.s_; }
	friend bool operator!=(const path& a, const path& b) { return a.s_ != b.s_; }
private:
	std::string s_;
};

enum file_type {
	status_error, file_not_found, regular_file, directory_file,
	symlink_file, block_file, character_file, fifo_file, socket_file,
	type_unknown
};

class file_status {
public:
	explicit file_status(file_type t = status_error) : t_(t) {}
	file_type type() const { return t_; }
private:
	file_type t_;
};

class directory_entry {
public:
	directory_entry() {}
	directory_entry(const filesystem::path& p, file_type t) : p_(p), t_(t) {}
	const filesystem::path& path() const { return p_; }
	file_status status() const { return file_status(t_); }
	file_status status(system::error_code& ec) const { ec.clear(); return file_status(t_); }
private:
	filesystem::path p_;
	file_type t_ = status_error;
};

struct amiga_dir_state;

/* Reads the whole drawer when it is made (Lock/Examine/ExNext), so no
 * lock stays open while the caller works through the entries. */
class directory_iterator {
public:
	directory_iterator() {}
	directory_iterator(const path& p, system::error_code& ec);
	directory_iterator& increment(system::error_code& ec);
	directory_iterator& operator++() { system::error_code ec; return increment(ec); }
	const directory_entry& operator*() const;
	const directory_entry* operator->() const { return &**this; }
	friend bool operator==(const directory_iterator& a, const directory_iterator& b) { return a.at_end() == b.at_end(); }
	friend bool operator!=(const directory_iterator& a, const directory_iterator& b) { return !(a == b); }
private:
	bool at_end() const;
	std::shared_ptr<amiga_dir_state> st_;
};

bool create_directory(const path& p, system::error_code& ec);
bool is_directory(const path& p, system::error_code& ec);
std::time_t last_write_time(const path& p, system::error_code& ec);
bool remove(const path& p, system::error_code& ec);
void rename(const path& from, const path& to, system::error_code& ec);
path unique_path(const path& model);

} // namespace filesystem
} // namespace boost

#endif
