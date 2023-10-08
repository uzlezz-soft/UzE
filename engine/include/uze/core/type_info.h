#pragma once

#include "uze/common.h"
#include <refl.hpp>
#include <unordered_map>
#include <functional>

namespace uze
{

	enum class UZE TypeMemberType
	{
		Unknown, Int, Float, Double, String, ObjectRef
	};

	struct UZE TypeMember
	{
		std::string name;
		TypeMemberType type;
	};

	struct UZE TypeInfo
	{
		std::string name;
		std::vector<TypeInfo*> parents;
		std::vector<TypeMember> members;
	};

	class UZE Object
	{
	public:

		virtual const TypeInfo& getTypeInfo() const;

		virtual ~Object() = default;

	};

}

REFL_AUTO(type(uze::Object));

#define UZE_OBJECT(T) \
	public: using ThisT = T; \
	virtual const TypeInfo& getTypeInfo() const override;

#define UZE_REFLECT(T, ...) \
	UZE_EXPAND(REFL_AUTO(__VA_ARGS__)) \
	const ::uze::TypeInfo& ::T::getTypeInfo() const \
	{ \
		static TypeInfo* ti = Registry::getTypeInfo( \
			::refl::reflect<T>().name.c_str()); \
		return *ti; \
	} \
	namespace UZE_CONCAT(Z_, UZE_CONCAT(__COUNTER__, _UZE_REFLECTION))\
	{ \
		struct Z_register \
		{ \
			Z_register() { ::uze::Registry::registerType<T>(); }\
		}; \
		static Z_register UZE_CONCAT(type_register, __COUNTER__){}; \
	}


namespace uze
{

	class UZE Registry
	{
	public:

		static void init();

		template <class T>
		constexpr static void registerType()
		{
			get().registerImpl([]()
			{
				refl::type_descriptor<T> td = refl::reflect<T>();

				auto ti = new TypeInfo();
				ti->name = td.name.c_str();

				if constexpr (td.declared_bases.size)
				{
					for_each(reflect_types(td.declared_bases), [&](auto t)
					{
						auto pti = getTypeInfo(t.name.c_str());
						if (!pti)
						{
							logWarn(fmt::format("Type `{}` inherits from `{}`, but it's isn't registered",
								ti->name, t.name.c_str()));
							return;
						}

						ti->parents.push_back(pti);
					});
				}

				for_each(td.members, [&](auto member)
				{
					if constexpr (is_readable(member))
					{
						ti->members.push_back({ get_display_name(member), TypeMemberType::Unknown });
					}
				});

				get().addType(ti);
			});
		}

		static TypeInfo* getTypeInfo(std::string_view name);

	private:

		std::unordered_map<std::string, std::shared_ptr<TypeInfo>> m_types;

		void addType(TypeInfo* ti);
		void registerImpl(const std::function<void()>& func);

		static Registry& get();
		static std::vector<std::function<void()>>& getRegisterQueue();
		static void logWarn(std::string_view str);

	};

}