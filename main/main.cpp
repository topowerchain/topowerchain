/* 
 toptoken  
*/

#include <utils/headers.h>
#include <common/general.h>
#include <common/storage.h>
#include <common/private_key.h>
#include <common/argument.h>
#include <common/daemon.h>
#include <overlay/peer_manager.h>
#include <ledger/ledger_manager.h>
#include <consensus/consensus_manager.h>
#include <glue/glue_manager.h>
#include <api/web_server.h>
#include <api/websocket_server.h>
#include <api/console.h>
#include <contract/contract_manager.h>
#include <monitor/monitor_manager.h>
#include "configure.h"

void SaveWSPort();
void RunLoop();
int main(int argc, char *argv[]){

#ifdef WIN32
	_set_output_format(_TWO_DIGIT_EXPONENT);
#else
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	size_t stacksize = 0;
	int ret = pthread_attr_getstacksize(&attr, &stacksize);
	if (ret != 0) {
		printf("get stacksize error!:%d\n", (int)stacksize);
		return -1;
	}

	if (stacksize <= 2 * 1024 * 1024)
	{
		stacksize = 2 * 1024 * 1024;

		pthread_attr_t object_attr;
		pthread_attr_init(&object_attr);
		ret = pthread_attr_setstacksize(&object_attr, stacksize);
		if (ret != 0) {
			printf("set main stacksize error!:%d\n", (int)stacksize);
			return -1;
		}
	}
#endif

	utils::SetExceptionHandle();
	utils::Thread::SetCurrentThreadName("toptoken-thread");

	utils::Daemon::InitInstance();
	utils::net::Initialize();
	utils::Timer::InitInstance();
	toptoken::Configure::InitInstance();
	toptoken::Storage::InitInstance();
	toptoken::Global::InitInstance();
	toptoken::SlowTimer::InitInstance();
	utils::Logger::InitInstance();
	toptoken::Console::InitInstance();
	toptoken::PeerManager::InitInstance();
	toptoken::LedgerManager::InitInstance();
	toptoken::ConsensusManager::InitInstance();
	toptoken::GlueManager::InitInstance();
	toptoken::WebSocketServer::InitInstance();
	toptoken::WebServer::InitInstance();
	toptoken::MonitorManager::InitInstance();
	toptoken::ContractManager::InitInstance();

	toptoken::Argument arg;
	if (arg.Parse(argc, argv)){
		return 1;
	}

	do {
		utils::ObjectExit object_exit;
		toptoken::InstallSignal();

		if (arg.console_){
			arg.log_dest_ = utils::LOG_DEST_FILE; //Cancel the std output
			toptoken::Console &console = toptoken::Console::Instance();
			console.Initialize();
			object_exit.Push(std::bind(&toptoken::Console::Exit, &console));
		}

		srand((uint32_t)time(NULL));
		toptoken::StatusModule::modules_status_ = new Json::Value;
#ifndef OS_MAC
		utils::Daemon &daemon = utils::Daemon::Instance();
		if (!toptoken::g_enable_ || !daemon.Initialize((int32_t)1234))
		{
			LOG_STD_ERRNO("Failed to initialize daemon", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}
		object_exit.Push(std::bind(&utils::Daemon::Exit, &daemon));
#endif

		toptoken::Configure &config = toptoken::Configure::Instance();
		std::string config_path = toptoken::General::CONFIG_FILE;
		if (!utils::File::IsAbsolute(config_path)){
			config_path = utils::String::Format("%s/%s", utils::File::GetBinHome().c_str(), config_path.c_str());
		}

		if (!config.Load(config_path)){
			LOG_STD_ERRNO("Failed to load configuration", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}

		std::string log_path = config.logger_configure_.path_;
		if (!utils::File::IsAbsolute(log_path)){
			log_path = utils::String::Format("%s/%s", utils::File::GetBinHome().c_str(), log_path.c_str());
		}
		const toptoken::LoggerConfigure &logger_config = toptoken::Configure::Instance().logger_configure_;
		utils::Logger &logger = utils::Logger::Instance();
		logger.SetCapacity(logger_config.time_capacity_, logger_config.size_capacity_);
		logger.SetExpireDays(logger_config.expire_days_);
		if (!toptoken::g_enable_ || !logger.Initialize((utils::LogDest)(arg.log_dest_ >= 0 ? arg.log_dest_ : logger_config.dest_),
			(utils::LogLevel)logger_config.level_, log_path, true)){
			LOG_STD_ERR("Failed to initialize logger");
			break;
		}
		object_exit.Push(std::bind(&utils::Logger::Exit, &logger));
		LOG_INFO("Initialized daemon successfully");
		LOG_INFO("Loaded configure successfully");
		LOG_INFO("Initialized logger successfully");

		// end run command
		toptoken::Storage &storage = toptoken::Storage::Instance();
		LOG_INFO("The path of the database is as follows: keyvalue(%s),account(%s),ledger(%s)", 
			config.db_configure_.keyvalue_db_path_.c_str(),
			config.db_configure_.account_db_path_.c_str(),
			config.db_configure_.ledger_db_path_.c_str());

		if (!toptoken::g_enable_ || !storage.Initialize(config.db_configure_, arg.drop_db_)) {
			LOG_ERROR("Failed to initialize database");
			break;
		}
		object_exit.Push(std::bind(&toptoken::Storage::Exit, &storage));
		LOG_INFO("Initialized database successfully");

		if (arg.drop_db_) {
			LOG_INFO("Droped database successfully");
			return 1;
		} 
		
		if ( arg.clear_consensus_status_ ){
			toptoken::Pbft::ClearStatus();
			LOG_INFO("Cleared consensus status successfully");
			return 1;
		}

		if (arg.clear_peer_addresses_) {
			toptoken::KeyValueDb *db = toptoken::Storage::Instance().keyvalue_db();
			db->Put(toptoken::General::PEERS_TABLE, "");
			LOG_INFO("Cleared peer addresss list successfully");
			return 1;
		} 

		if (arg.create_hardfork_) {
			toptoken::LedgerManager &ledgermanger = toptoken::LedgerManager::Instance();
			if (!ledgermanger.Initialize()) {
				LOG_ERROR("Failed to initialize legder manger!");
				return -1;
			}
			toptoken::LedgerManager::CreateHardforkLedger();
			return 1;
		}

		toptoken::Global &global = toptoken::Global::Instance();
		if (!toptoken::g_enable_ || !global.Initialize()){
			LOG_ERROR_ERRNO("Failed to initialize global variable", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}
		object_exit.Push(std::bind(&toptoken::Global::Exit, &global));
		LOG_INFO("Initialized global module successfully");

		//Consensus manager must be initialized before ledger manager and glue manager
		toptoken::ConsensusManager &consensus_manager = toptoken::ConsensusManager::Instance();
		if (!toptoken::g_enable_ || !consensus_manager.Initialize(toptoken::Configure::Instance().ledger_configure_.validation_type_)) {
			LOG_ERROR("Failed to initialize consensus manager");
			break;
		}
		object_exit.Push(std::bind(&toptoken::ConsensusManager::Exit, &consensus_manager));
		LOG_INFO("Initialized consensus manager successfully");

		toptoken::LedgerManager &ledgermanger = toptoken::LedgerManager::Instance();
		if (!toptoken::g_enable_ || !ledgermanger.Initialize()) {
			LOG_ERROR("Failed to initialize ledger manager");
			break;
		}
		object_exit.Push(std::bind(&toptoken::LedgerManager::Exit, &ledgermanger));
		LOG_INFO("Initialized ledger successfully");

		toptoken::GlueManager &glue = toptoken::GlueManager::Instance();
		if (!toptoken::g_enable_ || !glue.Initialize()){
			LOG_ERROR("Failed to initialize glue manager");
			break;
		}
		object_exit.Push(std::bind(&toptoken::GlueManager::Exit, &glue));
		LOG_INFO("Initialized glue manager successfully");

		toptoken::PeerManager &p2p = toptoken::PeerManager::Instance();
		if (!toptoken::g_enable_ || !p2p.Initialize(NULL, false)) {
			LOG_ERROR("Failed to initialize peer network");
			break;
		}
		object_exit.Push(std::bind(&toptoken::PeerManager::Exit, &p2p));
		LOG_INFO("Initialized peer network successfully");

		toptoken::SlowTimer &slow_timer = toptoken::SlowTimer::Instance();
		if (!toptoken::g_enable_ || !slow_timer.Initialize(1)){
			LOG_ERROR_ERRNO("Failed to initialize slow timer", STD_ERR_CODE, STD_ERR_DESC);
			break;
		}
		object_exit.Push(std::bind(&toptoken::SlowTimer::Exit, &slow_timer));
		LOG_INFO("Initialized slow timer with " FMT_SIZE " successfully", utils::System::GetCpuCoreCount());

		toptoken::WebSocketServer &ws_server = toptoken::WebSocketServer::Instance();
		if (!toptoken::g_enable_ || !ws_server.Initialize(toptoken::Configure::Instance().wsserver_configure_)) {
			LOG_ERROR("Failed to initialize web server");
			break;
		}
		object_exit.Push(std::bind(&toptoken::WebSocketServer::Exit, &ws_server));
		LOG_INFO("Initialized web server successfully");

		toptoken::WebServer &web_server = toptoken::WebServer::Instance();
		if (!toptoken::g_enable_ || !web_server.Initialize(toptoken::Configure::Instance().webserver_configure_)) {
			LOG_ERROR("Failed to initialize web server");
			break;
		}
		object_exit.Push(std::bind(&toptoken::WebServer::Exit, &web_server));
		LOG_INFO("Initialized web server successfully");

		SaveWSPort();
		
		toptoken::MonitorManager &monitor_manager = toptoken::MonitorManager::Instance();
		if (!toptoken::g_enable_ || !monitor_manager.Initialize()) {
			LOG_ERROR("Failed to initialize monitor manager");
			break;
		}
		object_exit.Push(std::bind(&toptoken::MonitorManager::Exit, &monitor_manager));
		LOG_INFO("Initialized monitor manager successfully");

		toptoken::ContractManager &contract_manager = toptoken::ContractManager::Instance();
		if (!contract_manager.Initialize(argc, argv)){
			LOG_ERROR("Failed to initialize contract manager");
			break;
		}
		object_exit.Push(std::bind(&toptoken::ContractManager::Exit, &contract_manager));
		LOG_INFO("Initialized contract manager successfully");

		toptoken::g_ready_ = true;

		RunLoop();

		LOG_INFO("Process begins to quit...");
		delete toptoken::StatusModule::modules_status_;

	} while (false);

	toptoken::ContractManager::ExitInstance();
	toptoken::SlowTimer::ExitInstance();
	toptoken::GlueManager::ExitInstance();
	toptoken::LedgerManager::ExitInstance();
	toptoken::PeerManager::ExitInstance();
	toptoken::WebSocketServer::ExitInstance();
	toptoken::WebServer::ExitInstance();
	toptoken::MonitorManager::ExitInstance();
	toptoken::Configure::ExitInstance();
	toptoken::Global::ExitInstance();
	toptoken::Storage::ExitInstance();
	utils::Logger::ExitInstance();
	utils::Daemon::ExitInstance();
	
	if (arg.console_ && !toptoken::g_ready_) {
		printf("Initialized failed, please check log for detail\n");
	}
	printf("process exit\n");
}

void RunLoop(){
	int64_t check_module_interval = 5 * utils::MICRO_UNITS_PER_SEC;
	int64_t last_check_module = 0;
	while (toptoken::g_enable_){
		int64_t current_time = utils::Timestamp::HighResolution();

		for (auto item : toptoken::TimerNotify::notifys_){
			item->TimerWrapper(utils::Timestamp::HighResolution());
			if (item->IsExpire(utils::MICRO_UNITS_PER_SEC)){
				LOG_WARN("The execution time(" FMT_I64 " us) for the timer(%s) is expired after 1s elapses", item->GetLastExecuteTime(), item->GetTimerName().c_str());
			}
		}

		utils::Timer::Instance().OnTimer(current_time);
		utils::Logger::Instance().CheckExpiredLog();

		if (current_time - last_check_module > check_module_interval){
			utils::WriteLockGuard guard(toptoken::StatusModule::status_lock_);
			toptoken::StatusModule::GetModulesStatus(*toptoken::StatusModule::modules_status_);
			last_check_module = current_time;
		}

		utils::Sleep(1);
	}
}

void SaveWSPort(){    
    std::string tmp_file = utils::File::GetTempDirectory() +"/toptoken_listen_port";
	Json::Value json_port = Json::Value(Json::objectValue);
	json_port["webserver_port"] = toptoken::WebServer::Instance().GetListenPort();
	json_port["wsserver_port"] = toptoken::WebSocketServer::Instance().GetListenPort();
	utils::File file;
	if (file.Open(tmp_file, utils::File::FILE_M_WRITE | utils::File::FILE_M_TEXT))
	{
		std::string line = json_port.toFastString();
		file.Write(line.c_str(), 1, line.length());
		file.Close();
	}
}
