/*
 * reader.h
 *
 *  Created on: Apr 5, 2012
 *      Author: asanoki
 */

#ifndef READER_H_
#define READER_H_

#include <fstream>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/algorithm/string.hpp>
#include "../common/boost_extras/wprogress.h"
#include "../common/encoding.h"
#include "../common/log.h"

namespace jovislab {
namespace gramd {
namespace tools {
namespace make {

class Reader {
private:
	std::vector<std::string> m_filenames;
	std::vector<std::string>::iterator m_iterator;
	boost::shared_ptr<boost::wprogress_display> m_progress_display;
	std::ifstream m_data;
	size_t m_input_size;
	int m_data_pos;
	int m_data_pos_local;
	boost::mutex m_mutex;
	int open(std::string filename) {
		m_data.open(filename.c_str(), std::ios::in);
		m_data.imbue(std::locale("C"));
		if (!m_data.good())
			return -1;
		return 0;
	}
public:
	Reader() {
		m_input_size = 0;
		m_data_pos = 0;
	}
	Reader(std::vector<std::string> filenames) :
			m_filenames(filenames) {
		m_input_size = 0;
		m_data_pos = 0;
	}
	int initialize(std::vector<std::string> filenames) {
		m_filenames = filenames;
		return initialize();
	}
	int initialize() {
		for (std::vector<std::string>::iterator it = m_filenames.begin();
				it != m_filenames.end(); it++) {
			m_data.open(it->c_str(), std::ios::in);
			m_data.imbue(std::locale("C"));
			if (!m_data.good())
				return -1;
			m_data.seekg(0, std::ios::end);
			m_input_size += m_data.tellg();
			m_data.seekg(0, std::ios::beg);
			m_data.close();
		}
		m_data_pos = 0;
		m_data_pos_local = 0;
		m_iterator = m_filenames.begin();
		open(*m_iterator);
		m_iterator++;
		return 0;
	}
	std::string readLine() {
		boost::mutex::scoped_lock lock(m_mutex);
		DEBUG("Reader::readLine: Begin.");
		std::string raw_line;
		std::wstring line;

		if (m_progress_display.get() == NULL) {
			m_progress_display.reset(
					new boost::wprogress_display(m_input_size));
		}

		if (m_data.eof()) {
			m_data.close();
			if (m_iterator == m_filenames.end()) {
				DEBUG("Reader::readLine: End. No more data.");
				return "";
			}
			m_iterator++;
			open(*m_iterator);
		}

		while (raw_line.empty() && !m_data.eof()) {
			DEBUG("Reader::readLine: Reading line...");
			std::getline(m_data, raw_line);
			m_data_pos_local += raw_line.length();
			m_data_pos += raw_line.length();
			if (m_data_pos_local > 10000) {
				m_progress_display.get()->operator +=(m_data_pos_local);
				m_data_pos_local = 0;
			}
			boost::algorithm::trim(raw_line);
		}

		DEBUG("Reader::readLine: End. Returning a line.");
		return raw_line;
	}
	void close() {
//		m_progress_display.reset();
		m_progress_display.reset();//new boost::progress_display(m_input_size));
	}
};

}
}
}
}

#endif /* READER_H_ */
