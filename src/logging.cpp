#include <emb_lin_util/logging.hpp>

namespace emb_lin_util
{
	void set_default_logger(const std::shared_ptr<spdlog::logger>& glb_logger_mt)
	{
		spdlog::set_default_logger( glb_logger_mt );
	}
}