/* 
 toptoken  
*/

#ifndef _BUBI_SERIALIZER_H_
#define 	_BUBI_SERIALIZER_H_

#include <vector>
#include <memory>
#include <string>

#include "utils.h"

namespace utils{

class Serializer{
public:
	typedef std::shared_ptr <Serializer> pointer;
	
	Serializer (){}
	~Serializer (){}
	Serializer (std::string str);

	std::vector <char>& peek_data ();
	static uint256 get_prefix_hash (const char *ch, int len);
	bool add_raw (const char* ch, int len);
	bool add256 (uint256 &hash);
	uint256 get_sha512_half ();
	bool	add_serializer (Serializer &s);
	std::size_t	peek_data_size ();
	
private:
	std::vector <char> data_;
};

}

#endif
