#include "Log.hxx"
#include "Resolved.hxx"
#include "lib/dbus/AppendIter.hxx"
#include "lib/dbus/Message.hxx"
#include "lib/dbus/ReadIter.hxx"
#include "lib/dbus/Types.hxx"
#include "util/Domain.hxx"
using namespace ODBus;

static constexpr Domain resolved_domain("resolved");

ResolvedHelper::ResolvedHelper(EventLoop &_loop, const char *name,
			       const char *service_type, unsigned port) noexcept
: dbus_glue(_loop) {
	auto &connection = dbus_glue->GetConnection();
	Message msg = Message::NewMethodCall(
		"org.freedesktop.resolve1", "/org/freedesktop/resolve1",
		"org.freedesktop.resolve1.Manager", "RegisterService");
	AppendMessageIter(*msg.Get())
		.Append("mpd")		// id
		.Append(name)		// name_template
		.Append(service_type)	// type
		.Append((uint16_t)port) // service_port
		.Append((uint16_t)0)	// service_priority
		.Append((uint16_t)0)	// service_weight
		.AppendEmptyArray<ArrayTypeTraits<DictEntryTypeTraits<
			StringTypeTraits,
			ArrayTypeTraits<ByteTypeTraits>>>>(); // txt_datas

	register_service_request.Send(connection, *msg.Get(), [this](Message reply) {
		const char *error_name = reply.GetErrorName();
		if (error_name != NULL && reply.IsError(error_name)) {
			// TODO: where is this logged to? never shows up
			FmtError(resolved_domain, "Could not register DNS-SD service: {}",
				 error_name);
		} else {
			service_path = ReadMessageIter(*reply.Get()).GetString();
		}
	});
}

ResolvedHelper::~ResolvedHelper() noexcept {
	if (service_path != NULL) {
		auto &connection = dbus_glue->GetConnection();
		Message msg = Message::NewMethodCall(
			"org.freedesktop.resolve1", service_path,
			"org.freedesktop.resolve1.DnssdService", "Unregister");

		AsyncRequest unregister_service_request;
		unregister_service_request.Send(
			connection, *msg.Get(), [](Message reply) {
				const char *error_name = reply.GetErrorName();
				if (error_name != NULL && reply.IsError(error_name)) {
					FmtError(
						resolved_domain,
						"Could not unregister DNS-SD service: {}",
						error_name);
				}
			});
	}
}

std::unique_ptr<ResolvedHelper> ResolvedInit(EventLoop &loop, const char *name,
					     const char *service_type, unsigned port) {
	return std::make_unique<ResolvedHelper>(loop, name, service_type, port);
}

