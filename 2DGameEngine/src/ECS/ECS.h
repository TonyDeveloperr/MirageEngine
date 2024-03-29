#pragma once

#include <bitset>
#include <memory>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <set>
#include "../Logger/Logger.h"

const unsigned int MAX_COMPONENTS = 32;

typedef std::bitset<MAX_COMPONENTS> Signature;

struct BaseComponent {
protected:
	static int nextId;
};

// Used to assign a unique id to a component type
template <typename T>
class Component: public BaseComponent {
    public:
	// Returns the unique id of Component<T>
	static int GetId() {
		static auto id = nextId++;
		return id;
	}
};


class Entity {
private:
	int id;

public:
	Entity(int id) : id(id) {};
	Entity(const Entity& entity) = default;
	int GetId() const;

	Entity& operator =(const Entity& other) = default;

	bool operator ==(const Entity& other) const {
		return id == other.id;
	}
	bool operator !=(const Entity& other) const {
		return id != other.id;
	}

	bool operator <(const Entity& other) const {
		return id < other.id;
	}

	bool operator >(const Entity& other) const {
		return id > other.id;
	}

	template <typename TComponent, typename ...TArgs> void AddComponent(TArgs&& ...args);
	template <typename TComponent> void RemoveComponent();
	template <typename TComponent> bool HasComponent() const;
	template <typename TComponent> TComponent& GetComponent() const;

	class Registry* registry;

};

class System {
private:
	Signature componentSignature;
	std::vector<Entity> entities;

public:
	System() = default;
	~System() = default;

	void AddEntityToSystem(Entity entity);
	void RemoveEntityFromSystem(Entity entity);
	std::vector<Entity> GetSystemEntities() const;
	const Signature& GetComponentSignature() const;

	template <typename TComponent> void RequireComponent();

};



class IPool {
public:
	virtual ~IPool() {}
};

template <typename T>
class Pool: public IPool {
private:
	std::vector<T> data;

public:
	Pool(int size = 100) {
		data.resize(size);
	}

	virtual ~Pool() = default;

	bool isEmpty() const {
		return data.empty();
	}

	int GetSize() const {
		return data.size();
	}

	void Resize(int newSize) {
		data.resize(newSize);
	}

	void Clear() {
		data.clear();
	}

	void Add(T object) {
		data.push_back(object);
	}

	void Set(int index, T object) {
		data[index] = object;
	}

	T& Get(int index) {
		return static_cast<T&>(data[index]);
	}

	T& operator [](unsigned int index) {
		return data[index];
	}
};


// The registry handles the creation and destruction of entities,
// adds systems and components
class Registry {

private:
	int activeEntities = 0;

	// This is a vector of component pools, each pool contains all the data for a certain component type
	// Vector index = component type id
	// Pool index entity id
	std::vector<std::shared_ptr<IPool>> componentPools;

	// A vector of component signatures per entity saying which component is enabled for a given entity
	// Vector index = entity id
	std::vector<Signature> entityComponentSignatures;

	std::unordered_map<std::type_index, std::shared_ptr<System>> systems;

	// Set of entities that are to be added in the next registry frame Update()
	std::set<Entity> entitiesToBeAdded;
	std::set<Entity> entitiesToBeKilled;

public:
	Registry() {
		Logger::Log("Registry constructor called");
	};

	~Registry() {
		Logger::Log("Registry destructor called");
	};

	// The registry update processes the entities that are waiting to be added/destroyed
	void Update();

	// Entity management
	Entity CreateEntity();

	// Component management
	template <typename TComponent, typename ...TArgs> void AddComponent(Entity entity, TArgs&& ...args);
	template <typename TComponent> void RemoveComponent(Entity entity);
	template <typename TComponent> bool HasComponent(Entity entity) const;
	template <typename TComponent> TComponent& GetComponent(Entity entity) const;

	// System management
	template <typename TSystem, typename ...TArgs> void AddSystem(TArgs&& ...args);
	template <typename TSystem> void RemoveSystem();
	template <typename TSystem> bool HasSystem() const;
	template <typename TSystem> TSystem& GetSystem() const;

	// Checks the component signature of an entity and addds it to the systems that are interested in it
	void AddEntityToSystems(Entity entity);

};

template <typename TComponent> 
void System::RequireComponent() {
	const auto componentId = Component<TComponent>::GetId();
	componentSignature.set(componentId);
}

template <typename TSystem, typename ...TArgs> 
void Registry::AddSystem(TArgs&& ...args) {
	std::shared_ptr<TSystem> newSystem = std::make_shared<TSystem>(std::forward<TArgs>(args)...);
	systems.insert(std::make_pair(std::type_index(typeid(TSystem)), newSystem));
}

template <typename TSystem> 
void Registry::RemoveSystem() {
	auto system = systems.find(std::type_index(typeid(TSystem)));
	systems.erase(system);
}

template <typename TSystem> 
bool Registry::HasSystem() const {
	return systems.find(std::type_index(typeid(TSystem))) != systems.end();
}

template <typename TSystem> 
TSystem& Registry::GetSystem() const {
	auto system = systems.find(std::type_index(typeid(TSystem)));
	return *(std::static_pointer_cast<TSystem>(system->second));
}

template <typename TComponent, typename ...TArgs>
void Registry::AddComponent(Entity entity, TArgs&& ...args) {
	const int componentId = Component<TComponent>::GetId();
	const int entityId = entity.GetId();

	if (componentId >= componentPools.size()) {
		componentPools.resize(componentId + 1, nullptr);
	}

	if (!componentPools[componentId]) {
		std::shared_ptr<Pool<TComponent>> newComponentPool = std::make_shared<Pool<TComponent>>();
		componentPools[componentId] = newComponentPool;
	}

	std::shared_ptr<Pool<TComponent>> componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);

	if (entityId >= componentPool->GetSize()) {
		componentPool->Resize(activeEntities);
	}

	TComponent newComponent(std::forward<TArgs>(args)...);

	componentPool->Set(entityId, newComponent);

	entityComponentSignatures[entityId].set(componentId);

	Logger::Log("Component id: " + std::to_string(componentId) + " was added to entity id " + std::to_string(entityId));
}

template <typename TComponent>
void Registry::RemoveComponent(Entity entity) {
	const int componentId = Component<TComponent>::GetId();
	const int entityId = entity.GetId();

	entityComponentSignatures[entityId].set(componentId, false);

	Logger::Log("Component id: " + std::to_string(componentId) + " was removed from entity id " + std::to_string(entityId));
}

template <typename TComponent> 
bool Registry::HasComponent(Entity entity) const{
	const int entityId = entity.GetId();
	const int componentId = Component<TComponent>::GetId();

	return entityComponentSignatures[entityId].test(componentId);
}

template <typename TComponent> 
TComponent& Registry::GetComponent(Entity entity) const{
	const int entityId = entity.GetId();
	const int componentId = Component<TComponent>::GetId();
	auto componentPool = std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);
	return componentPool->Get(entityId);
}

template <typename TComponent, typename ...TArgs>
void Entity::AddComponent(TArgs&& ...args) {
	registry->AddComponent<TComponent>(*this, std::forward<TArgs>(args)...);
}

template <typename TComponent>
void Entity::RemoveComponent() {
	registry->RemoveComponent<TComponent>(*this);
}

template <typename TComponent>
bool Entity::HasComponent() const {
	registry->HasComponent<TComponent>(*this);
}

template <typename TComponent>
TComponent& Entity::GetComponent() const {
	return registry->GetComponent<TComponent>(*this);
}




