#pragma once

#include "Engine/Core/AssetManager.h"
#include "Engine/Core/ContentPaths.h"
#include "Engine/Core/MainContext.h"

#include <filesystem>
#include <memory>
#include <string>

namespace Engine {

	[[nodiscard]] inline std::filesystem::path NormalizeAssetRelativePath(std::filesystem::path path) {
		if (path.empty()) {
			return {};
		}
		std::error_code ec;
		const std::filesystem::path contentRoot = ContentRoot();
		if (path.is_relative()) {
			return path.lexically_normal();
		}
		std::filesystem::path rel = std::filesystem::relative(path, contentRoot, ec);
		if (!ec && !rel.empty()) {
			return rel.lexically_normal();
		}
		return path.lexically_normal();
	}

} // namespace Engine

/// Serializable reference to an on-disk asset by content-relative path.
/// Resolved only through `Engine::MainContext` asset manager (not by loading files directly).
template <typename TAsset>
class AssetRef
{
public:
	[[nodiscard]] const std::filesystem::path& GetPath() const {
		return _path;
	}

	[[nodiscard]] std::string GetPathString() const {
		return _path.generic_string();
	}

	void SetPath(std::filesystem::path path) {
		_path = Engine::NormalizeAssetRelativePath(std::move(path));
		_cached.reset();
	}

	void SetPath(std::string path) {
		SetPath(std::filesystem::path{std::move(path)});
	}

	void Clear() {
		_path.clear();
		_cached.reset();
	}

	[[nodiscard]] explicit operator bool() const {
		return !_path.empty();
	}

	[[nodiscard]] std::shared_ptr<TAsset> Get() const {
		if (_path.empty()) {
			_cached.reset();
			return nullptr;
		}
		if (auto locked = _cached.lock()) {
			return locked;
		}
		auto manager = Engine::MainContext::GetInstance().GetAssetManager();
		if (!manager) {
			return nullptr;
		}
		auto loaded = manager->GetOrLoad<TAsset>(_path);
		_cached = loaded;
		return loaded;
	}

private:
	std::filesystem::path _path;
	mutable std::weak_ptr<TAsset> _cached{};
};
