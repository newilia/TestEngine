#include "Engine/Editor/ValuesProviders.h"

#include "Engine/Core/ContentPaths.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <optional>

namespace {

	namespace fs = std::filesystem;

	void AsciiLowerInPlace(std::string& s) {
		for (char& c : s) {
			c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
		}
	}

	bool HasImageExtension(const fs::path& path) {
		std::string ext = path.extension().string();
		AsciiLowerInPlace(ext);
		return ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".webp" || ext == ".bmp" || ext == ".gif" ||
		       ext == ".tga";
	}

	std::string RepoRelativeLower(fs::path repoRoot, const fs::path& filePath) {
		std::error_code ec;
		fs::path rel = fs::relative(filePath, repoRoot, ec);
		if (ec) {
			return {};
		}
		std::string s = rel.generic_string();
		AsciiLowerInPlace(s);
		return s;
	}

} // namespace

namespace Editor::ValuesProviders {

	std::vector<std::string> GetBackgroundTextures() {
		const fs::path root = Engine::ContentRoot();
		const fs::path dir = root / "resources" / "textures" / "backgrounds";
		std::error_code ec;
		if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) {
			return {};
		}
		std::vector<std::string> out;
		for (fs::recursive_directory_iterator it(dir, fs::directory_options::skip_permission_denied), end; it != end;
		    ++it) {
			if (!it->is_regular_file()) {
				continue;
			}
			if (!HasImageExtension(it->path())) {
				continue;
			}
			std::string rel = RepoRelativeLower(root, it->path());
			if (!rel.empty()) {
				out.push_back(std::move(rel));
			}
		}
		std::sort(out.begin(), out.end());
		out.erase(std::unique(out.begin(), out.end()), out.end());
		return out;
	}

} // namespace Editor::ValuesProviders
