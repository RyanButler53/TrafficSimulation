/**
 * @file refelectionUtil.hpp
 * @author Ryan Butler
 * @brief 
 * @version 0.1
 * @date 2026-11-21
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#pragma once
#include <meta>
#include <ranges>
#include <print>
#include <span>

namespace equality {

/// @brief Tag struct to indicate that a member will not be used during an equality calculation
struct ExcludeFromEquality {};
constexpr ExcludeFromEquality skip;
 

/**
 * @brief Filters the members of type T to only contain members of type T
 * 
 * @tparam T Type to get comparable members of
 * @return Iterable std::span of reflections of members not annotated with Exclude that are equality comparable
 */
template <typename T>
consteval std::span<const std::meta::info> filterByComparable(){
    constexpr auto ctx = std::meta::access_context::unchecked();
    return std::define_static_array(std::meta::nonstatic_data_members_of(^^T, ctx)
                                    | std::views::filter([](std::meta::info mem) consteval -> bool {
                                        return std::meta::annotations_of_with_type(mem, ^^ExcludeFromEquality).empty(); })
                                    | std::views::filter([](std::meta::info mem) consteval -> bool {
                                        // Substitute the type into the concept in the reflection world
                                        return std::meta::extract<bool>(std::meta::substitute(^^std::equality_comparable, {std::meta::type_of(mem)})); })
    );
}

/**
 * @brief Generic Equality operator for type T. 
 * @note Requires to be a friend function of the class to use
 * unchecked() access context while finding members
 * @tparam T Type to compare
 * @param obj1 
 * @param obj2 
 * @return true if objects are equal, false otherwise
 */
template <typename T>
constexpr bool operatorEquals(const T& obj1, const T& obj2){
    static constexpr auto comparables = filterByComparable<T>();
    template for( constexpr std::meta::info mem : filterByComparable<T>()){
        if (obj1.[:mem:] != obj2.[:mem:]){ return false; }
    }
    return true;
}
} // namespace equality
