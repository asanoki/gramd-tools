/*
 * gramdtoolsmake.cpp
 *
 *  Created on: Apr 5, 2012
 *      Author: asanoki
 */

#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

#include "worker.h"
#include "reader.h"
#include "filter.h"
#include "storage.h"
#include "../common/log.h"

namespace po = boost::program_options;

int main(int argc, char **argv) {
	try {
		Log::error.enable();

		// Initialize
		po::options_description desc;
		desc.add_options()("limit", po::value<int>(), "memory usage threshold, after which cache files will be used, default: 1000000");
		desc.add_options()("rank,r", po::value<int>(), "rank of collected n-grams, default: 3");
		desc.add_options()("threads,j", po::value<int>(), "number of threads, default: 2");
		desc.add_options()("help", "display help message");
		desc.add_options()("verbose,v", "output useful information");
		desc.add_options()("debug,d", "output debug information");
		desc.add_options()("text,t", "store result file using text-mode");
		desc.add_options()("quiet,q", "do not display any startup messages");
		desc.add_options()("filter,F", po::value<std::string>(),
				"filename with regular expressions to be applied on each input line");
		desc.add_options()("locale,l", po::value<std::string>(),
				"define custom locale");
		desc.add_options()("input,I", po::value<std::vector<std::string> >(),
				"input files (--input or -I can be omitted)");
		desc.add_options()("output,O", po::value<std::string>(),
				"Output filename");

		po::positional_options_description p;
		p.add("input", -1);

		po::variables_map vm;
		po::store(
				po::command_line_parser(argc, argv).options(desc).positional(p).run(),
				vm);
		po::notify(vm);

		if (vm.count("help")) {
			std::cout << desc;
			return -1;
		}

		Log::message.enable();
		Log::warning.enable();

		if (vm.count("verbose")) {
			Log::info.enable();
		}

		if (vm.count("debug")) {
			Log::debug.enable();
		}

		if (vm.count("quiet")) {
			Log::debug.disable();
			Log::info.disable();
			Log::message.disable();
			Log::notice.disable();
			Log::warning.disable();
			Log::error.disable();
		}

		// Set dictionary locale
		encoding::setUtf8InternalLocale();

		// Configure global locale
		if (vm.count("locale")) {
			std::string locale_name = vm["locale"].as<std::string>();
			try {
				std::locale::global(std::locale(locale_name.c_str()));
				setlocale(LC_ALL, locale_name.c_str());
			} catch (...) {
				Log::error << "Unable to set locale to: " << locale_name.c_str()
						<< std::endl;
				return -1;
			}
		} else {
			// Default locale
			try {
				std::locale::global(std::locale(""));
				setlocale(LC_ALL, "");
			} catch (...) {
				Log::warning
						<< "Unable to set to system locale. Trying C instead."
						<< std::endl;
				std::locale::global(std::locale("C"));
				setlocale(LC_ALL, "C");
			}
		}

		// Objects
		Reader reader;
		Filter filter;
		Storage storage;
		std::vector<boost::shared_ptr<Worker> > workers;
		boost::thread_group threads;
		int rank = 3;
		int threads_count = 2;
		std::string output_filename;
		bool text_mode = false;

		// Final checks for required fields
		if (!vm.count("input")) {
			std::cout << desc;
			return -1;
		}

		if (!vm.count("output")) {
			std::cout << desc;
			return -1;
		} else {
			output_filename = vm["output"].as<std::string>();
		}

		if (vm.count("rank")) {
			rank = vm["rank"].as<int>();
		}
		storage.initialize(rank);

		if (vm.count("threads")) {
			threads_count = vm["threads"].as<int>();
		}

		if (vm.count("text")) {
			text_mode = true;
		}

		// Read input files
		std::vector<std::string> inputs =
				vm["input"].as<std::vector<std::string> >();
		if (reader.initialize(inputs) < 0) {
			Log::error << "Unable to read input files." << std::endl;
			return -1;
		}

		// Read filter file
		if (vm.count("filter")) {
			if (filter.initialize(vm["filter"].as<std::string>()) < 0) {
				Log::error << "Unable to read the filter file." << std::endl;
				return -1;
			}
		}

		// Construct worker threads
		for (int index = 0; index < threads_count; index++) {
			boost::shared_ptr<Worker> worker(new Worker(index, rank, reader, storage, filter));
			workers.push_back(worker);
			threads.create_thread(*worker);
		}

		// Run threads and wait
		threads.join_all();

		// Close reader (and display summary)
		reader.close();

		Log::message << "Saving output file..." << std::endl;
		// Save storage
		if (!text_mode) {
			if (storage.saveBinary(output_filename) < 0) {
				Log::error << "Unable to write to output file." << std::endl;
				return -1;
			}
		}
		else {
			if (storage.saveText(output_filename) < 0) {
				Log::error << "Unable to write to output file." << std::endl;
				return -1;
			}
		}

		Log::message << "Done." << std::endl;

		return 0;
	} catch (std::exception &e) {
		Log::error << e.what() << std::endl;
		return -1;
	}
}
