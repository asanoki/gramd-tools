/*
 * filter.h
 *
 *  Created on: Apr 10, 2012
 *      Author: asanoki
 */

#ifndef FILTER_H_
#define FILTER_H_

#include <fstream>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include "../common/encoding.h"
#include "../common/log.h"

class Filter {
private:
	std::string m_filename;
	std::ifstream m_data;
	std::vector<boost::wregex> m_regexes;
	boost::mutex m_mutex;
public:
	Filter(const std::string filename) :
			m_filename(filename) {

	}
	Filter() {
		m_filename = "";
	}
	int initialize(const std::string filename) {
		m_filename = filename;
		return initialize();
	}
	int initialize() {
		m_regexes.clear();
		m_data.open(m_filename.c_str(), std::ios::in);
		m_data.imbue(std::locale("C"));
		if (!m_data.good())
			return -1;
		std::string raw_line;
		std::wstring line;
		int line_value;
		while (!m_data.eof()) {
			raw_line.clear();
			while (raw_line.empty() && !m_data.eof()) {
				std::getline(m_data, raw_line);
			}
			boost::algorithm::trim(raw_line);
			if (raw_line.empty()) {
				continue;
			}
			if (raw_line[0] == '#') {
				continue;
			}
			wchar_t *wchar_buffer = new wchar_t[raw_line.length() + 1];
			encoding::importAsUtf8(wchar_buffer, raw_line.c_str(),
					raw_line.length() + 1/* Check off by one for \0 */,
					raw_line.length() + 1);
			line = wchar_buffer;
			m_regexes.push_back(boost::wregex(line));
			delete[] wchar_buffer;
		}
		m_data.close();
		return 0;
	}
	std::wstring process(const std::wstring &input) {
		if (m_regexes.size() == 0)
			return input;
		boost::wsmatch what;
		for (std::vector<boost::wregex>::iterator it = m_regexes.begin();
				it != m_regexes.end(); it++) {
			if (boost::regex_match(input, what, *it)) {
				if (what[1].matched) {
					std::wstring result(what[1].first, what[1].second);
					return result;
				}
			}
		}
		return L"";
	}
};

#endif /* FILTER_H_ */
