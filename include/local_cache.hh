#ifndef LOCAL_CACHE
#define LOCAL_CACHE
#include "c++/7/experimental/filesystem"
namespace fs = std::experimental::filesystem;

static const std::string baseDir = "/dev/shm/";
#include "stages.hh"
class local_cache{


public:
	local_cache(std::string instrument_name, std::string hash){

		// define a temp dir in RAM
		std::string dirName = baseDir + instrument_name + "/";
		_p       = dirName;
		fs::create_directories(_p);
		std::string hash_string = hash;
		_p /= hash_string;
		_p += ".tgz";


	};

	inline bool isOK(void){
		if (!fs::exists(_p) && (fs::exists(_p.parent_path() / _p.stem()))) {
			std::cerr << "[ERROR] Sim dir already exists but not TGZ, "
				"please clean up"
					  << std::endl;
			return false;
		}
		return true;
	}

	inline bool is_done(void){
		return fs::exists(_p);
	}

	inline fs::path path(void){ return _p;};

	inline void save_request(std::string sim_request){
		// print the request json in the directory
		std::ofstream request_dump_file(
			(_p.parent_path() / _p.stem()).string() + "/request.json");
		request_dump_file << sim_request << std::endl;
	}

	inline void save_tgz(void){
		system((std::string("tar -cz -C ") + _p.parent_path().string() +
				" " + _p.stem().string() + " > " + _p.string()).c_str());
	}

	inline void save_stage(size_t is, std::string hash){
		// create a directory for the request at stage 1
		fs::path mcpl_path = _p.parent_path() / "MCPL" / stages[is] / hash;
		fs::create_directories(mcpl_path.parent_path());
		
		// copy the request json
		mcpl_path += ".json";
		fs::copy(_p.parent_path() / _p.stem() / "request.json",
				 mcpl_path);
		
		// move the mcpl file and rename it
		mcpl_path.replace_extension(".mcpl.gz");
		fs::rename(_p.parent_path() / _p.stem() /
				   (std::string(stages[is]) + ".mcpl.gz"),
				   mcpl_path);
		//}
	}

private:
	fs::path _p;
};

#endif
