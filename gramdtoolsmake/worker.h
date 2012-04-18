/*
 * worker.h
 *
 *  Created on: Apr 5, 2012
 *      Author: asanoki
 */

#ifndef WORKER_H_
#define WORKER_H_

#include <string>
#include <sstream>
#include <boost/unordered_map.hpp>
#include "reader.h"
#include "filter.h"
#include "storage.h"
#include "storage_impl.h"
#include "../common/log.h"

namespace jovislab {
namespace gramd {
namespace tools {
namespace make {

class Worker {
private:
	Reader &m_reader;
	Filter &m_filter;
	Storage &m_storage;
	int m_rank;
	int m_id;
	char *m_add_char_buffer;
public:
	Worker(int id, int rank, Reader &reader, Storage &storage, Filter &filter) :
			m_id(id), m_rank(rank), m_reader(reader), m_storage(storage), m_filter(
					filter) {
		m_add_char_buffer = new char[rank * 16 + 1];
	}
	Worker(const Worker &worker) : m_id(worker.m_id), m_rank(worker.m_rank), m_reader(worker.m_reader),
			m_storage(worker.m_storage), m_filter(worker.m_filter) {
		m_add_char_buffer = new char[m_rank * 16 + 1];
	}
	~Worker() {
		delete []m_add_char_buffer;
	}
	void run() {
		std::string raw_line;
		std::wstring line;
		std::wstring filtered_line;
		std::vector<std::string> results_list;
		DEBUG("Worker[" << m_id << "]::run: Begin.");
		do {
			DEBUG("Worker[" << m_id << "]::run: Loop iteration.");
			results_list.clear();
			raw_line = m_reader.readLine();
			wchar_t *wchar_buffer = new wchar_t[raw_line.length() + 1];
			encoding::importAsUtf8(wchar_buffer, raw_line.c_str(),
				raw_line.length() + 1 /* Check off by one for \0 */,
				raw_line.length() + 1);
			line = wchar_buffer;
			delete[] wchar_buffer;
			filtered_line = m_filter.process(line);
			DEBUG("Worker[" << m_id << "]::run: Collecting n-grams...");
			for (int index = 0; index < (int)filtered_line.length() - m_rank + 1; index++) {
				std::wstring gram = line.substr(index, m_rank);
				encoding::exportAsUtf8(m_add_char_buffer, gram.c_str(), m_rank * 16 + 1, gram.length() + 1);
				results_list.push_back(m_add_char_buffer);
			}
			DEBUG("Worker[" << m_id << "]::run: Collected. Storing...");
			m_storage.add(results_list);
			DEBUG("Worker[" << m_id << "]::run: End of loop iteration.");
		} while (!raw_line.empty());
	}
	void operator()() {
		run();
	}
};

}
}
}
}

#endif /* WORKER_H_ */
