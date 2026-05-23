#pragma once

#include <list>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Editor {

	namespace ValuesProviderDetail {

		template <class T>
		std::vector<std::decay_t<T>> ToVector(std::vector<T>&& c) {
			using E = std::decay_t<T>;
			return std::vector<E>(std::make_move_iterator(c.begin()), std::make_move_iterator(c.end()));
		}

		template <class T>
		std::vector<T> ToVector(const std::vector<T>& c) {
			return std::vector<T>(c.begin(), c.end());
		}

		template <class T>
		std::vector<std::decay_t<T>> ToVector(std::list<T>&& c) {
			using E = std::decay_t<T>;
			return std::vector<E>(std::make_move_iterator(c.begin()), std::make_move_iterator(c.end()));
		}

		template <class T>
		std::vector<T> ToVector(const std::list<T>& c) {
			return std::vector<T>(c.begin(), c.end());
		}

		template <class T, class Cmp, class Alloc>
		std::vector<T> ToVector(const std::set<T, Cmp, Alloc>& c) {
			return std::vector<T>(c.begin(), c.end());
		}

		template <class T, class Hash, class Eq, class Alloc>
		std::vector<T> ToVector(const std::unordered_set<T, Hash, Eq, Alloc>& c) {
			return std::vector<T>(c.begin(), c.end());
		}

	} // namespace ValuesProviderDetail

	namespace ValuesProviders {

		[[nodiscard]] std::vector<std::string> GetTextures();

	} // namespace ValuesProviders

} // namespace Editor
