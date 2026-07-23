//
// Created by Vladimir Glushkov on 23.07.2026.
//

#include "engine/ecsModule/ECS.h"

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

size_t std::hash<pce::Entity>::operator()(const pce::Entity& entity) const noexcept {
	return std::hash<pce::Entity::ValueType>{}(entity.m_value);
}
