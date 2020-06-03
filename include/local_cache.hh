#ifndef LOCAL_CACHE
#define LOCAL_CACHE
#include "c++/7/experimental/filesystem"
namespace fs = std::experimental::filesystem;

#include "stages.hh"
#include <fstream>
#include <iostream>

namespace panosc
{
static const std::string baseDir = "/dev/shm/";

class local_cache
{

	public:
	local_cache(std::string instrument_name, std::string hash)
	{

		// define a temp dir in RAM
		std::string dirName = baseDir + instrument_name + "/";
		_p                  = dirName;
		fs::create_directories(_p);
		_p /= hash;
		//	_p += ".tgz";
	};

	// in the form of baseDir/instrument_name/hash/
	inline fs::path output_dir(void) const { return _p; };

	inline bool isOK(void) const
	{
		if (!fs::exists(path_tgz()) && fs::exists(output_dir())) {
			std::cerr << "[ERROR] Sim dir already exists but not TGZ, "
			             "please clean up"
			          << std::endl;
			return false;
		}
		return true;
	}

	inline bool is_done(void) const { return fs::exists(path_tgz()); }

	inline fs::path path_tgz(void) const
	{
		auto p = _p;
		p += ".tgz";
		return p;
	};

	inline void save_request(std::string sim_request)
	{
		// print the request json in the directory
		std::ofstream request_dump_file((output_dir()).string() + "/request.json");
		request_dump_file << sim_request << std::endl;
	}

	inline void save_tgz(void)
	{
		system((std::string("tar -cz -C ") + _p.parent_path().string() + " " + _p.stem().string() +
		        " > " + path_tgz().string())
		           .c_str());
	}

	inline void save_stage(stage_t is, std::string hash) const
	{
		auto stage_name = stages.at(is);
		auto l          = output_dir() / stage_name;
		l += ".mcpl.gz";

		// it is possible that the INSTRUMENT does not have implemented a MCPL output for all the
		// stages
		if (fs::exists(l) == false) {
			std::cerr << "[WARNING] file: " << l << " does not exists" << std::endl;
			return;
		}

		// strip the hash, add MCPL, add stage_name, add the hash
		fs::path mcpl_path = _p.parent_path() / "MCPL" / stage_name / hash;
		fs::create_directories(mcpl_path.parent_path());

		// copy the request json
		mcpl_path += ".json";
		fs::copy(output_dir() / "request.json", mcpl_path);

		// move the mcpl file and rename it
		mcpl_path.replace_extension(".mcpl.gz");
		fs::rename(l, mcpl_path);
		//}
	}

	inline std::pair<size_t, std::string> get_stage(std::vector<std::string> hashes) const
	{
		std::string mcpl_filename;
		stage_t     istage = sSOURCE;
		// check here if any MCPL exists for one of the stages
		// stages are ordered from the detector to the source
		for (istage = 0; istage < stages.size() and mcpl_filename.empty();) {
			const auto &stage_hash = hashes[istage];
			const auto &sp         = stage_path(istage, stage_hash);
			std::cout << "[INFO] check if MCPL file for stage " << istage
			          << " exists\n       check existence of file" << sp << std::endl;
			if (fs::exists(sp)) {
				mcpl_filename = sp.parent_path() / sp.stem();
				mcpl_filename += ".mcpl.gz";
				std::cout << "    -> file found" << std::endl;
			} else
				++istage;
		}
		if (mcpl_filename.empty()) {
			std::cout << "    -> file NOT found" << std::endl;
		}
		if (mcpl_filename.empty())
			istage = stages.size() - 1;

		return std::pair<size_t, std::string>(istage, mcpl_filename);
	}

	private:
	fs::path _p;

	inline fs::path stage_path(stage_t istage, std::string stage_hash) const
	{
		// const auto stage_name = stages[istage];

		fs::path sp(_p.parent_path() / "MCPL" / stages.at(istage) / stage_hash);
		sp += ".json";
		return sp;
	}
};
} // namespace panosc
#endif
