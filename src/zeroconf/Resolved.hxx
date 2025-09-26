#include "lib/dbus/AsyncRequest.hxx"
#include "lib/dbus/Glue.hxx"
#include "thread/SafeSingleton.hxx"
#include <memory>
class EventLoop;

class ResolvedHelper final {
private:
	SafeSingleton<ODBus::Glue> dbus_glue;
	ODBus::AsyncRequest register_service_request;
	const char *service_path = NULL;

public:
	ResolvedHelper(EventLoop &_loop, const char *name, const char *service_name,
		       unsigned port) noexcept;

	~ResolvedHelper() noexcept;

	ResolvedHelper(const ResolvedHelper &) = delete;
	ResolvedHelper &operator=(const ResolvedHelper &) = delete;
};

std::unique_ptr<ResolvedHelper> ResolvedInit(EventLoop &event_loop,
					     const char *service_name,
					     const char *service_type, unsigned port);
