/* 
 toptoken  
*/

#ifndef CONTRACT_MANAGER_H_
#define CONTRACT_MANAGER_H_

#include "v8_contract.h"

namespace toptoken{

	class ContractManager :
		public utils::Singleton<ContractManager>{
		friend class utils::Singleton<ContractManager>;

		utils::Mutex contracts_lock_;
		ContractMap contracts_;
	public:
		ContractManager();
		~ContractManager();

		bool Initialize(int argc, char** argv);
		bool Exit();

		Result Execute(int32_t type, const ContractParameter &paramter);
		bool Query(int32_t type, const ContractParameter &paramter, Json::Value &result);
		bool Cancel(int64_t contract_id);
		Result SourceCodeCheck(int32_t type, const std::string &code, uint32_t ldcontext_stack_size);
		Contract *GetContract(int64_t contract_id);
	};
}
#endif
