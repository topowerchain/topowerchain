/* 
 toptoken  
*/

#include <utils/logger.h>
#include <utils/timestamp.h>
#include <common/general.h>
#include <common/storage.h>
#include <common/private_key.h>
#include <main/configure.h>
#include "consensus_manager.h"

namespace toptoken {
	ConsensusManager::ConsensusManager() {
		check_interval_ = 500 * utils::MICRO_UNITS_PER_MILLI;
		timer_name_ = "Consensus Manager";
	}
	ConsensusManager::~ConsensusManager() {}

	bool ConsensusManager::Initialize(const std::string &validation_type) {
		if (validation_type == "one_node") {
			consensus_ = std::shared_ptr<Consensus>(new OneNode());
			if (!consensus_->Initialize())
				return false;
		}
		else {
			consensus_ = std::shared_ptr<Consensus>(new Pbft());
			if (!consensus_->Initialize())
				return false;
		}

		TimerNotify::RegisterModule(this);
		//StatusModule::RegisterModule(this);
		return true;
	}

	bool ConsensusManager::Exit() {
		return consensus_->Exit();
	}

	std::shared_ptr<Consensus> ConsensusManager::GetConsensus() {
		return consensus_;
	}

	void ConsensusManager::GetModuleStatus(Json::Value &data) {
		data["name"] = "consensus manager";
	}

	void ConsensusManager::OnTimer(int64_t current_time) {
		consensus_->OnTimer(current_time);
	}

	void ConsensusManager::OnSlowTimer(int64_t current_time) {
		consensus_->OnSlowTimer(current_time);
	}
}
