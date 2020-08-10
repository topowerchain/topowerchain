/* 
 toptoken  
*/

#include "serializer.h"

#include <openssl/sha.h>

namespace utils{


Serializer::Serializer (std::string str){
	std::size_t sz = str.length ();
	for (size_t i = 0; i < sz; i++){
		data_.push_back (str[i]);
	}
}

std::vector <char>&
Serializer::peek_data (){
	return data_;
}

uint256
Serializer::get_prefix_hash (const char* ch, int len){
	uint256 j[2];
	SHA512_CTX  ctx;
	SHA512_Init (&ctx);
	SHA512_Update (&ctx, ch, len);
	SHA512_Final (reinterpret_cast <unsigned char *> (&j[0]), &ctx);
	return j[0];
}

bool
Serializer::add_serializer (Serializer &s){
	std::vector <char>& vt = s.peek_data ();
	add_raw (&(*(vt.begin())), vt.size());
	return true;
}

std::size_t
Serializer::peek_data_size (){
	return data_.size ();
}
bool
Serializer::add_raw (const char *ch, int len){
	for (int i=0; i<len; i++){
		data_.push_back (ch[i]);
	}
	return true;
}

bool
Serializer::add256 (uint256 &hash){
	data_.insert (data_.end(), hash.begin(),hash.end());
	return true;
}

uint256
Serializer::get_sha512_half (){
	uint256 j[2];
	SHA512 ( reinterpret_cast<unsigned char*>(&(data_.front())), data_.size(), reinterpret_cast <unsigned char *>(&j[0]) );
	return j[0];
}

}
