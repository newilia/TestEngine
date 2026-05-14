#include "Engine/Serialization/SceneEntityRegistry.h"

#include <utility>
#include <vector>

namespace Engine::Serialization {
	namespace {

		std::vector<SceneEntityRegistration>& PendingSceneEntityRegistrations() {
			static std::vector<SceneEntityRegistration> pending;
			return pending;
		}

	} // namespace

	namespace SceneEntityRegistrationDetail {

		void AppendSceneEntityRegistration(SceneEntityRegistration registration) {
			PendingSceneEntityRegistrations().push_back(std::move(registration));
		}

	} // namespace SceneEntityRegistrationDetail

	const SceneEntityRegistry& SceneEntityRegistry::GetInstance() {
		static const SceneEntityRegistry instance;
		return instance;
	}

	SceneEntityRegistry::SceneEntityRegistry() {
		_registrations = PendingSceneEntityRegistrations();
	}

	const std::vector<SceneEntityRegistration>& SceneEntityRegistry::GetAll() const {
		return _registrations;
	}

	std::shared_ptr<EntityOnNode> SceneEntityRegistry::CreateByTypeId(std::string_view typeId) const {
		for (const SceneEntityRegistration& registration : _registrations) {
			if (registration.typeId == typeId && registration.createInstance) {
				return registration.createInstance();
			}
		}
		return nullptr;
	}

	std::optional<SceneEntityKind> SceneEntityRegistry::GetKindByTypeId(std::string_view typeId) const {
		for (const SceneEntityRegistration& registration : _registrations) {
			if (registration.typeId == typeId) {
				return registration.kind;
			}
		}
		return std::nullopt;
	}

	std::string SceneEntityRegistry::GetTypeIdForEntity(const std::shared_ptr<EntityOnNode>& entity) const {
		if (!entity) {
			return {};
		}
		for (const SceneEntityRegistration& registration : _registrations) {
			if (registration.isType && registration.isType(entity)) {
				return registration.typeId;
			}
		}
		return {};
	}

} // namespace Engine::Serialization
