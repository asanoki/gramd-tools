/*
 * storage_impl.h
 *
 *  Created on: Apr 18, 2012
 *      Author: asanoki
 */

#ifndef STORAGE_IMPL_H_
#define STORAGE_IMPL_H_

#include <string>
#include "storage.h"

namespace jovislab {
namespace gramd {
namespace tools {
namespace make {

class Storage: public StorageEssentialBase {
public:
//	enum EntryMode {
//		EM_ASCII = 1, EM_UCS2 = 2, EM_UCS4 = 4
//	};
	enum EntrySize {
		ES_ANY = 0,
		ES_2 = 2,
		ES_4 = 4,
		ES_6 = 6,
		ES_8 = 8,
		ES_10 = 10,
		ES_12 = 12,
		ES_14 = 14,
		ES_16 = 16,
		ES_24 = 24,
		ES_32 = 32,
		ES_40 = 40,
		ES_48 = 48,
		ES_56 = 56,
		ES_64 = 64
	};
private:
	int m_rank;
//	enum EntryMode m_mode;
	enum EntrySize m_size;

	ConstantStorage<char, 2> m_storage_2;
	ConstantStorage<char, 4> m_storage_4;
	ConstantStorage<char, 6> m_storage_6;
	ConstantStorage<char, 8> m_storage_8;
	ConstantStorage<char, 10> m_storage_10;
	ConstantStorage<char, 12> m_storage_12;
	ConstantStorage<char, 14> m_storage_14;
	ConstantStorage<char, 16> m_storage_16;
	ConstantStorage<char, 24> m_storage_24;
	ConstantStorage<char, 32> m_storage_32;
	ConstantStorage<char, 40> m_storage_40;
	ConstantStorage<char, 48> m_storage_48;
	ConstantStorage<char, 56> m_storage_56;
	ConstantStorage<char, 64> m_storage_64;
	DynamicStorage<char> m_storage_any;
	StorageBase *m_current_storage;

public:
	Storage(int rank, enum EntrySize size) :
			m_rank(rank), m_size(size) {
	}

	Storage() {
	}

	virtual ~Storage() {
	}

	int initialize(int rank, enum EntrySize size) {
		m_rank = rank;
		m_size = size;
		return initialize();
	}

	int initialize() {
		if (m_size == ES_2) {
			m_current_storage = &m_storage_2;
		} else if (m_size == ES_4) {
			m_current_storage = &m_storage_4;
		} else if (m_size == ES_6) {
			m_current_storage = &m_storage_6;
		} else if (m_size == ES_8) {
			m_current_storage = &m_storage_8;
		} else if (m_size == ES_10) {
			m_current_storage = &m_storage_10;
		} else if (m_size == ES_12) {
			m_current_storage = &m_storage_12;
		} else if (m_size == ES_14) {
			m_current_storage = &m_storage_14;
		} else if (m_size == ES_16) {
			m_current_storage = &m_storage_16;
		} else if (m_size == ES_24) {
			m_current_storage = &m_storage_24;
		} else if (m_size == ES_32) {
			m_current_storage = &m_storage_32;
		} else if (m_size == ES_40) {
			m_current_storage = &m_storage_40;
		} else if (m_size == ES_48) {
			m_current_storage = &m_storage_48;
		} else if (m_size == ES_56) {
			m_current_storage = &m_storage_56;
		} else if (m_size == ES_64) {
			m_current_storage = &m_storage_64;
		} else if (m_size == ES_64) {
			m_current_storage = &m_storage_64;
		} else if (m_size == ES_ANY) {
			m_current_storage = &m_storage_any;
		} else {
			return -1;
		}
		return m_current_storage->initialize(m_rank);
	}

	void add(std::vector<std::string> &grams) {
		m_current_storage->add(grams);
	}

	int saveText(const std::string filename) {
		return m_current_storage->saveText(filename);
	}
};

}
}
}
}

#endif /* STORAGE_IMPL_H_ */
