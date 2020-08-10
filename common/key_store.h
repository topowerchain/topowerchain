/* 
 toptoken  
*/

#ifndef KEYSTORE_H_
#define KEYSTORE_H_

namespace toptoken {
	class KeyStore {
	public:
		KeyStore();
		~KeyStore();

		bool Generate(const std::string &password, Json::Value &key_store, std::string &new_priv_key);  //If new_private_key is empty, create a new private key.
		bool From(const Json::Value &key_store, const std::string &password, std::string &priv_key);
	};
}

#endif
