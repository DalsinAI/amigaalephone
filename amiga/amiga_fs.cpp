/* amiga/include/boost/filesystem.hpp's calls, on dos.library. */
#include <boost/filesystem.hpp>
#include <vector>
#include <cerrno>
#include <cstdlib>
#include <cstring>

#include <exec/types.h>
#include <dos/dos.h>
#include <proto/dos.h>

namespace boost {
namespace filesystem {

namespace {

void set_error(system::error_code& ec, int e)
{
	ec.assign(e, system::generic_category());
}

/* IoErr() to errno, for FileHandler.cpp's messages */
int dos_errno(LONG ioerr)
{
	switch (ioerr) {
	case ERROR_OBJECT_NOT_FOUND:
	case ERROR_DIR_NOT_FOUND:
	case ERROR_DEVICE_NOT_MOUNTED:
		return ENOENT;
	case ERROR_OBJECT_EXISTS:
		return EEXIST;
	case ERROR_OBJECT_IN_USE:
		return EBUSY;
	case ERROR_DISK_WRITE_PROTECTED:
	case ERROR_WRITE_PROTECTED:
	case ERROR_DELETE_PROTECTED:
	case ERROR_READ_PROTECTED:
		return EACCES;
	case ERROR_DISK_FULL:
		return ENOSPC;
	case ERROR_DIRECTORY_NOT_EMPTY:
		return ENOTEMPTY;
	case ERROR_NO_FREE_STORE:
		return ENOMEM;
	default:
		return EIO;
	}
}

/* AmigaDOS counts from 1 January 1978; Unix from 1970 (2922 days). */
std::time_t to_time(const struct DateStamp& ds)
{
	return (std::time_t)((ds.ds_Days + 2922L) * 86400L + ds.ds_Minute * 60L + ds.ds_Tick / TICKS_PER_SECOND);
}

std::string join(const std::string& dir, const char* name)
{
	if (dir.empty())
		return name;
	char last = dir[dir.size() - 1];
	if (last == ':' || last == '/')
		return dir + name;
	return dir + "/" + name;
}

/* Examine one object; false (and ec) when it isn't there */
bool examine(const path& p, struct FileInfoBlock* fib, system::error_code& ec)
{
	BPTR lock = Lock((CONST_STRPTR)p.c_str(), ACCESS_READ);
	if (!lock) {
		set_error(ec, dos_errno(IoErr()));
		return false;
	}
	BOOL ok = Examine(lock, fib);
	LONG err = ok ? 0 : IoErr();
	UnLock(lock);
	if (!ok) {
		set_error(ec, dos_errno(err));
		return false;
	}
	ec.clear();
	return true;
}

} // namespace

struct amiga_dir_state {
	std::vector<directory_entry> entries;
	size_t pos = 0;
};

directory_iterator::directory_iterator(const path& p, system::error_code& ec)
{
	ec.clear();
	BPTR lock = Lock((CONST_STRPTR)p.c_str(), ACCESS_READ);
	if (!lock) {
		set_error(ec, dos_errno(IoErr()));
		return;
	}
	struct FileInfoBlock* fib = (struct FileInfoBlock*)AllocDosObject(DOS_FIB, NULL);
	if (!fib) {
		UnLock(lock);
		set_error(ec, ENOMEM);
		return;
	}
	std::shared_ptr<amiga_dir_state> st = std::make_shared<amiga_dir_state>();
	if (Examine(lock, fib) && fib->fib_DirEntryType > 0) {
		while (ExNext(lock, fib)) {
			/* a soft link (ST_SOFTLINK) is skipped as a special file */
			file_type t = fib->fib_DirEntryType == ST_SOFTLINK ? symlink_file
			            : fib->fib_DirEntryType > 0 ? directory_file : regular_file;
			st->entries.emplace_back(path(join(p.native(), (const char*)fib->fib_FileName)), t);
		}
		if (IoErr() != ERROR_NO_MORE_ENTRIES)
			set_error(ec, dos_errno(IoErr()));
	} else {
		set_error(ec, ENOTDIR);
	}
	FreeDosObject(DOS_FIB, fib);
	UnLock(lock);
	if (!ec && !st->entries.empty())
		st_ = st;
}

directory_iterator& directory_iterator::increment(system::error_code& ec)
{
	ec.clear();
	if (st_ && ++st_->pos >= st_->entries.size())
		st_.reset();
	return *this;
}

const directory_entry& directory_iterator::operator*() const
{
	return st_->entries[st_->pos];
}

bool directory_iterator::at_end() const
{
	return !st_;
}

bool create_directory(const path& p, system::error_code& ec)
{
	BPTR lock = CreateDir((CONST_STRPTR)p.c_str());
	if (lock) {
		UnLock(lock);
		ec.clear();
		return true;
	}
	LONG err = IoErr();
	if (err == ERROR_OBJECT_EXISTS) {
		/* as Boost: an existing drawer is no error, just not made */
		if (is_directory(p, ec))
			return false;
		set_error(ec, EEXIST);
		return false;
	}
	set_error(ec, dos_errno(err));
	return false;
}

bool is_directory(const path& p, system::error_code& ec)
{
	struct FileInfoBlock* fib = (struct FileInfoBlock*)AllocDosObject(DOS_FIB, NULL);
	if (!fib) {
		set_error(ec, ENOMEM);
		return false;
	}
	bool dir = false;
	if (examine(p, fib, ec))
		dir = fib->fib_DirEntryType > 0 && fib->fib_DirEntryType != ST_SOFTLINK;
	else if (ec.value() == ENOENT)
		ec.clear(); /* as Boost: a missing file is simply not a directory */
	FreeDosObject(DOS_FIB, fib);
	return dir;
}

std::time_t last_write_time(const path& p, system::error_code& ec)
{
	struct FileInfoBlock* fib = (struct FileInfoBlock*)AllocDosObject(DOS_FIB, NULL);
	if (!fib) {
		set_error(ec, ENOMEM);
		return (std::time_t)-1;
	}
	std::time_t t = (std::time_t)-1;
	if (examine(p, fib, ec))
		t = to_time(fib->fib_Date);
	FreeDosObject(DOS_FIB, fib);
	return t;
}

bool remove(const path& p, system::error_code& ec)
{
	if (DeleteFile((CONST_STRPTR)p.c_str())) {
		ec.clear();
		return true;
	}
	LONG err = IoErr();
	if (err == ERROR_OBJECT_NOT_FOUND) {
		ec.clear(); /* as Boost: nothing to remove is no error */
		return false;
	}
	set_error(ec, dos_errno(err));
	return false;
}

void rename(const path& from, const path& to, system::error_code& ec)
{
	ec.clear();
	if (Rename((CONST_STRPTR)from.c_str(), (CONST_STRPTR)to.c_str()))
		return;
	LONG err = IoErr();
	if (err == ERROR_OBJECT_EXISTS) {
		/* POSIX rename replaces the target (saved games and preferences are
		 * written to a temporary name and renamed over the old file) */
		DeleteFile((CONST_STRPTR)to.c_str());
		if (Rename((CONST_STRPTR)from.c_str(), (CONST_STRPTR)to.c_str()))
			return;
		err = IoErr();
	}
	set_error(ec, dos_errno(err));
}

path unique_path(const path& model)
{
	static const char hex[] = "0123456789abcdef";
	std::string s = model.native();
	for (char& c : s)
		if (c == '%')
			c = hex[std::rand() & 15];
	return path(s);
}

} // namespace filesystem
} // namespace boost
