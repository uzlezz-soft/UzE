#include "uze/core/type_info.h"
#include <vector>

namespace uze
{

	namespace static_object_registration
	{
		struct StaticObjectRegistration
		{
			StaticObjectRegistration()
			{
				Registry::registerType<Object>();
			}
		};
		StaticObjectRegistration object_init{};
	}

	Registry s_registry;
	bool s_initialized{ false };
	std::vector<std::function<void()>> s_register_queue;

	constexpr static LogCategory log_registry { "Registry" };

	const TypeInfo& Object::getTypeInfo() const
	{
		static TypeInfo* ti = Registry::getTypeInfo(refl::reflect<Object>().name.c_str());
		return *ti;
	}

	void Registry::init()
	{
		if (s_initialized) return;

		for (auto& func : getRegisterQueue())
		{
			func();
		}

		s_initialized = true;
	}

	TypeInfo* Registry::getTypeInfo(std::string_view name)
	{
		auto it = get().m_types.find(name.data());
		if (it == get().m_types.end())
			return nullptr;

		return it->second.get();
	}

	void Registry::addType(TypeInfo* ti)
	{
		std::string name = ti->name;
		get().m_types.emplace(name, std::shared_ptr<TypeInfo>(ti));
		uzLog(log_registry, Info, fmt::format("Registered type `{}`", name));
	}

	void Registry::registerImpl(const std::function<void()>& func)
	{
		if (!s_initialized)
		{
			getRegisterQueue().push_back(func);
			return;
		}

		func();
	}

	Registry& Registry::get()
	{
		return s_registry;
	}

	std::vector<std::function<void()>>& Registry::getRegisterQueue()
	{
		return s_register_queue;
	}

	void Registry::logWarn(std::string_view str)
	{
		uzLog(log_registry, Warn, str);
	}
}
