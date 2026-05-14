#pragma once

#include <string>
#include <utility>
#include <vector>

namespace Engine::Serialization {

	enum class SerializationDiagnosticLevel
	{
		Warning,
		Error
	};

	struct SerializationDiagnostic
	{
		SerializationDiagnosticLevel level = SerializationDiagnosticLevel::Error;
		std::string path;
		std::string message;
	};

	struct SerializationResult
	{
		bool isSuccess = true;
		std::vector<SerializationDiagnostic> diagnostics;

		void AddError(std::string path, std::string message) {
			isSuccess = false;
			diagnostics.push_back(
			    SerializationDiagnostic{SerializationDiagnosticLevel::Error, std::move(path), std::move(message)});
		}

		void AddWarning(std::string path, std::string message) {
			diagnostics.push_back(
			    SerializationDiagnostic{SerializationDiagnosticLevel::Warning, std::move(path), std::move(message)});
		}

		void Merge(const SerializationResult& other) {
			if (!other.isSuccess) {
				isSuccess = false;
			}
			diagnostics.insert(diagnostics.end(), other.diagnostics.begin(), other.diagnostics.end());
		}
	};

} // namespace Engine::Serialization
