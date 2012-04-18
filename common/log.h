/*
 * log.h
 *
 *  Created on: Mar 31, 2012
 *      Author: asanoki
 */

#ifndef LOG_H_
#define LOG_H_

#include <iostream>
#include <ostream>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/thread/mutex.hpp>

#define LOG_LOCK boost::mutex::scoped_lock lock(Log::m_mutex)
#define LOG_MESSAGE(a, b) { LOG_LOCK; Log::a << b << std::endl; }

#ifndef NDEBUG
#	define DEBUG(a) LOG_MESSAGE(debug, a)
#else
#	define DEBUG(a)
#endif

#define INFO(a) LOG_MESSAGE(info, a)
#define MESSAGE(a) LOG_MESSAGE(message, a)
#define NOTICE(a) LOG_MESSAGE(notice, a)
#define WARNING(a) LOG_MESSAGE(warning, a)
#define ERROR(a) LOG_MESSAGE(error, a)

namespace jovislab {

class Log;

typedef std::wostream& (*manip1)(std::wostream&);
typedef std::basic_ios<std::wostream::char_type, std::ostream::traits_type> ios_type;
typedef ios_type& (*manip2)(ios_type&);
typedef std::ios_base& (*manip3)(std::ios_base&);

class Log {
private:
	bool m_enabled;
	std::wostream &m_stream;

public:
	static Log debug;
	static Log info;
	static Log message;
	static Log notice;
	static Log warning;
	static Log error;
	static boost::mutex m_mutex;

	void enable() {
		m_enabled = true;
	}

	void disable() {
		m_enabled = false;
	}

	bool enabled() {
		return m_enabled;
	}

	inline Log &operator<<(manip1 pf) {
		if (m_enabled) {
			m_stream << pf;
		}
		return *this;
	}

	inline Log &operator<<(manip2 pf) {
		if (m_enabled) {
			m_stream << pf;
		}
		return *this;
	}

	inline Log &operator<<(manip3 pf) {
		if (m_enabled) {
			m_stream << pf;
		}
		return *this;
	}

	std::wostream &stream() {
		return m_stream;
	}

	Log(std::wostream &stream) : m_stream(stream) {
	}
};

template<typename T>
inline Log &operator<<(Log &l, const T &t) {
	if (l.enabled()) {
		l.stream() << t;
	}
	return l;
}

}

#endif /* LOG_H_ */
