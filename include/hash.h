#ifndef HASH_CLASS
#define HASH_CLASS

#include <gcrypt.h>

#define SHA256_SIZE 32

class hash{

		private:

	char digest[SHA256_SIZE];

public:
	hash(std::string message)
		{
			gcry_md_hd_t handle = NULL;
			gcry_error_t  error;
	
			error = gcry_md_open(&handle,GCRY_MD_SHA256, GCRY_MD_FLAG_SECURE) ;
			if(error)
			{
				printf(" The Error : gcry_md_open computehmacSHA256 %s \n", gcry_strerror(error));
				return;
			}
			error = gcry_md_enable (handle, GCRY_MD_SHA256);
			if(error)
			{
				printf(" The Error : gcry_md_enable computehmacSHA256 %s \n", gcry_strerror(error));
				return;
			}
			
			int hashSize = gcry_md_get_algo_dlen(GCRY_MD_SHA256);
			assert(hashSize==SHA256_SIZE);
			
			gcry_md_write (handle, message.c_str(), message.size()-1);
			gcry_md_final (handle);
			char* d = (char *)(gcry_md_read(handle, GCRY_MD_SHA256));
			for(size_t i=0; i < SHA256_SIZE; ++i){
				digest[i] = d[i];
			}
			gcry_md_close(handle);  
			
		}

	std::string operator()(void) const{
		std::stringstream ret;
		for (size_t i=0; i < SHA256_SIZE; ++i){
			ret <<std::showbase <<  std::hex  << std::setw(2) <<  std::nouppercase  << (int)digest[i];
		}
		return ret.str();
	};
	
};
#endif
