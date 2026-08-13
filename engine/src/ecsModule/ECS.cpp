//
// Created by Vladimir Glushkov on 23.07.2026.
//

#include "engine/ecsModule/ECS.h"

#pragma region Entity

pce::Entity pce::EntityManager::CreateEntity() {
	uint32 freeIndex;
	if (!m_freeList.empty()) {
		freeIndex = m_freeList.back();
		m_freeList.pop_back();
		m_entityComponentSignatures[freeIndex].reset();
	} else {
		freeIndex = m_generations.size();
		m_generations.push_back(0);
		m_entityComponentSignatures.emplace_back();
	}
	return Entity::MakeEntity(freeIndex, m_generations[freeIndex]);
}

void pce::EntityManager::DestroyEntity(const Entity& entity) {
	PCE_ASSERT(IsAlive(entity), "Entity is not alive!");
	const auto index = entity.GetIndex();
	m_freeList.push_back(index);
	++m_generations[index];
	m_entityComponentSignatures[index].reset();
}

void pce::EntityManager::SetSignature(const Entity& entity, const details::Signature& signature) {
	PCE_ASSERT(IsAlive(entity), "Entity is not alive!");
	m_entityComponentSignatures[entity.GetIndex()] = signature;
}

pce::details::Signature& pce::EntityManager::GetSignature(const Entity& entity) {
	PCE_ASSERT(IsAlive(entity), "Entity is not alive!");
	return m_entityComponentSignatures[entity.GetIndex()];
}

bool pce::EntityManager::IsAlive(const Entity& entity) const {
	const auto index = entity.GetIndex();
	const auto gen = entity.GetGeneration();

	return index < m_generations.size() && m_generations[index] == gen;
}

const pce::details::Signature& pce::EntityManager::GetSignature(const Entity& entity) const {
	PCE_ASSERT(IsAlive(entity), "Entity is not alive!");
	return m_entityComponentSignatures[entity.GetIndex()];
}

#pragma endregion
#pragma region Component

void pce::PoolManager::ClearComponents(const Entity& entity, const details::Signature& signature) const {
	for (auto i = 0; i < signature.size(); ++i) {
		if (!signature.test(i)) {
			continue;
		}
		auto pool = GetPool(i);
		PCE_ASSERT(pool, "Component pool is not registered!");
		pool->Remove(entity);
	}
}

#pragma endregion
#pragma region System

void pce::SystemManager::Update(Registry& registry, float dt) {
	try {
		for (auto i = 0; i < m_systems.size(); ++i) {
			m_systems[i]->Update(registry, m_commandBuffers[i], dt);
		}

		for (auto& buffer: m_commandBuffers) {
			if (!buffer.Empty()) {
				buffer.ProcessCommands(registry);
			}
		}
	} catch (...) {
		for (auto& buffer: m_commandBuffers) {
			buffer.Clear();
		}
		throw;
	}
}

void pce::SystemManager::EmplaceBuffer() {
	m_commandBuffers.emplace_back();
}

pce::Entity pce::Registry::CreateEntity() {
	return m_entityManager.CreateEntity();
}

bool pce::Registry::IsEntityAlive(const Entity& entity) const {
	return m_entityManager.IsAlive(entity);
}

void pce::Registry::DestroyEntity(const Entity& entity) {
	PCE_ASSERT(m_entityManager.IsAlive(entity), "Entity is not alive!");
	m_poolManager.ClearComponents(entity, m_entityManager.GetSignature(entity));
	m_entityManager.DestroyEntity(entity);
}

pce::Entity pce::CommandBuffer::CreateEntity(Registry& registry) {
	return registry.CreateEntity();
}

void pce::CommandBuffer::DestroyEntity(const Entity& entity) {
	m_entitiesToDestroy.push_back(entity);
	++m_commandsCount;
}

void pce::CommandBuffer::ProcessCommands(Registry& registry) {
	struct Guard {
		explicit Guard(CommandBuffer& buffer): buffer(buffer) {}

		~Guard() {
			buffer.Clear();
		}

		CommandBuffer& buffer;
	} guard(*this);

	for (auto& group: m_componentGroups) {
		if (group) {
			group->Execute(registry);
		}
	}
	for (const auto& entity: m_entitiesToDestroy) {
		registry.DestroyEntity(entity);
	}
}

void pce::CommandBuffer::Clear() noexcept {
	for (auto& group: m_componentGroups) {
		if (group) {
			group->Clear();
		}
	}
	m_entitiesToDestroy.clear();
	m_commandsCount = 0;
}

bool pce::CommandBuffer::Empty() const noexcept {
	return m_commandsCount == 0;
}

#pragma endregion

size_t std::hash<pce::Entity>::operator()(const pce::Entity& entity) const noexcept {
	return std::hash<pce::Entity::ValueType>{}(entity.m_value);
}
