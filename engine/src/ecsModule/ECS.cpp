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
	const auto index = entity.GetIndex();
	m_freeList.push_back(index);
	++m_generations[index];
	m_entityComponentSignatures[index].reset();
}

void pce::EntityManager::SetSignature(const Entity& entity, const details::Signature& signature) {
	m_entityComponentSignatures[entity.GetIndex()] = signature;
}

pce::details::Signature& pce::EntityManager::GetSignature(const Entity& entity) {
	return m_entityComponentSignatures[entity.GetIndex()];
}

bool pce::EntityManager::IsAlive(const Entity& entity) const {
	const auto index = entity.GetIndex();
	const auto gen = entity.GetGeneration();

	return index < m_generations.size() && m_generations[index] == gen;
}

const pce::details::Signature& pce::EntityManager::GetSignature(const Entity& entity) const {
	return m_entityComponentSignatures[entity.GetIndex()];
}

#pragma endregion
#pragma region Component

void pce::PoolManager::ClearComponents(const Entity& entity, const details::Signature& signature) const {
	for (auto i = 0; i < signature.size(); ++i) {
		if (!signature.test(i)) {
			continue;
		}
		if (i < m_componentPools.size() && m_componentPools[i]) {
			m_componentPools[i]->Remove(entity);
		}
	}
}

#pragma endregion
#pragma region System

void pce::SystemManager::Update(Registry& registry, float dt) {
	for (auto i = 0; i < m_systems.size(); ++i) {
		m_systems[i]->Update(registry, m_commandBuffers[i], dt);
	}

	for (auto& buffer: m_commandBuffers) {
		if (!buffer.Empty()) {
			buffer.ProcessCommands(registry);
		}
	}
}

void pce::SystemManager::EmplaceBuffer() {
	m_commandBuffers.emplace_back();
}

pce::Entity pce::Registry::CreateEntity() {
	return m_entityManager.CreateEntity();
}

void pce::Registry::DestroyEntity(const Entity& entity) {
	m_poolManager.ClearComponents(entity, m_entityManager.GetSignature(entity));
	m_entityManager.DestroyEntity(entity);
}

void pce::CommandBuffer::CreateEntity(std::function<void(Entity, Registry&)>&& callback) {
	m_commands.emplace_back([setup = std::move(callback)](Registry& registry) {
		const Entity entity = registry.CreateEntity();
		if (setup) {
			setup(entity, registry);
		}
	});
}

void pce::CommandBuffer::DestroyEntity(const Entity& entity) {
	m_commands.emplace_back([entity](Registry& registry) {
		registry.DestroyEntity(entity);
	});
}

void pce::CommandBuffer::ProcessCommands(Registry& registry) {
	for (auto& command: m_commands) {
		command(registry);
	}
	m_commands.clear();
}

bool pce::CommandBuffer::Empty() const noexcept {
	return m_commands.empty();
}

#pragma endregion

size_t std::hash<pce::Entity>::operator()(const pce::Entity& entity) const noexcept {
	return std::hash<pce::Entity::ValueType>{}(entity.m_value);
}
