#pragma once

#include <utility>
#include <type_traits>

namespace cppflow::composition
{
	template <typename... OpTs>
	struct flow_t: std::remove_cvref_t<OpTs>...
	{
		using std::remove_cvref_t<OpTs>::operator()...;

		template <typename OpT>
		auto operator+ (OpT&& op) &
		{
			return flow_t<std::remove_cvref_t<OpTs>..., OpT>{ static_cast<std::remove_cvref_t<OpTs>&>(*this)..., std::forward<OpT>(op) };
		}

		template <typename OpT>
		auto operator+ (OpT&& op) const &
		{
			return flow_t<std::remove_cvref_t<OpTs>..., OpT>{ static_cast<const std::remove_cvref_t<OpTs>&>(*this)..., std::forward<OpT>(op) };
		}

		template <typename OpT>
		auto operator+ (OpT&& op) &&
		{
			return flow_t<std::remove_cvref_t<OpTs>..., OpT>{ static_cast<std::remove_cvref_t<OpTs>&&>(*this)..., std::forward<OpT>(op) };
		}
	};

	static inline constexpr flow_t<> flow{};

	template <typename... OpTs>
	struct reflow_t
	{
		template <typename OpT>
		auto operator+ (OpT&& op) &
		{
			return reflow_t<OpTs..., OpT>{ .caller= flow_t<OpTs..., OpT>{ static_cast<OpTs&>(caller)..., std::forward<OpT>(op) } };
		}

		template <typename OpT>
		auto operator+ (OpT&& op) const &
		{
			return reflow_t<OpTs..., OpT>{ .caller= flow_t<OpTs..., OpT>{ static_cast<const OpTs&>(caller)..., std::forward<OpT>(op) } };
		}

		template <typename OpT>
		auto operator+ (OpT&& op) &&
		{
			return reflow_t<OpTs..., OpT>{ .caller= flow_t<OpTs..., OpT>{ static_cast<OpTs&&>(caller)..., std::forward<OpT>(op) } };
		}

		template <typename... ArgsTs>
		decltype(auto) operator() (ArgsTs&&... args) & requires std::is_invocable_v<flow_t<OpTs...>&, reflow_t&, ArgsTs&&...>
		{
			return caller(static_cast<reflow_t&>(*this), std::forward<ArgsTs>(args)...);
		}

		template <typename... ArgsTs>
		decltype(auto) operator() (ArgsTs&&... args) const & requires std::is_invocable_v<const flow_t<OpTs...>&, const reflow_t&, ArgsTs&&...>
		{
			return caller(static_cast<const reflow_t&>(*this), std::forward<ArgsTs>(args)...);
		}

		template <typename... ArgsTs>
		decltype(auto) operator() (ArgsTs&&... args) && requires std::is_invocable_v<flow_t<OpTs...>&&, reflow_t&&, ArgsTs&&...>
		{
			return caller(static_cast<reflow_t&&>(*this), std::forward<ArgsTs>(args)...);
		}

		[[no_unique_address]]
		flow_t<OpTs...> caller;
	};
	template <typename... OpTs>
	reflow_t(OpTs...) -> reflow_t<std::remove_cvref_t<OpTs>...>;

	static inline constexpr reflow_t<> reflow{};
}