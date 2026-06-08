export module Test.Serialization;

import Test.Framework;

import Core;

export namespace Test
{
	// Test classes for complex serialization
	struct SimpleData
	{
		int intValue = 0;
		float floatValue = 0.0f;
		std::string name;
		vec3 position;

		SERIALIZE()
		{
			ar& NVP(intValue)& NVP(floatValue)& NVP(name)& NVP(position);
		}

		bool operator==(const SimpleData& other) const
		{
			return intValue == other.intValue &&
				   floatValue == other.floatValue &&
				   name == other.name &&
				   position.x == other.position.x &&
				   position.y == other.position.y &&
				   position.z == other.position.z;
		}

		friend std::ostream& operator<<(std::ostream& os, const SimpleData& data)
		{
			os << "SimpleData{int:" << data.intValue << ",float:" << data.floatValue
			   << ",name:" << data.name << "}";
			return os;
		}
	};

	struct NestedData
	{
		SimpleData simple;
		std::string description;
		quat rotation;
		mat4x4 transform;

		SERIALIZE()
		{
			ar& NVP(simple)& NVP(description)& NVP(rotation)& NVP(transform);
		}

		bool operator==(const NestedData& other) const
		{
			return simple == other.simple &&
				   description == other.description &&
				   rotation.x == other.rotation.x &&
				   rotation.y == other.rotation.y &&
				   rotation.z == other.rotation.z &&
				   rotation.w == other.rotation.w;
		}

		friend std::ostream& operator<<(std::ostream& os, const NestedData& data)
		{
			os << "NestedData{description:" << data.description << "}";
			return os;
		}
	};

	// Classes with inheritance for serialization testing
	struct BaseEntity
	{
		int id = 0;
		std::string entityType;
		vec3 position;

		virtual ~BaseEntity() = default;

		SERIALIZE()
		{
			ar& NVP(id)& NVP(entityType)& NVP(position);
		}

		virtual std::string GetTypeName() const { return "BaseEntity"; }

		bool operator==(const BaseEntity& other) const
		{
			return id == other.id &&
				   entityType == other.entityType &&
				   position.x == other.position.x &&
				   position.y == other.position.y &&
				   position.z == other.position.z;
		}

		friend std::ostream& operator<<(std::ostream& os, const BaseEntity& data)
		{
			os << "BaseEntity{id:" << data.id << ",type:" << data.entityType << "}";
			return os;
		}
	};

	struct DerivedEntity : public BaseEntity
	{
		float health = 100.0f;
		std::string name;
		quat orientation;

		SERIALIZE()
		{
			SAVE_PARENT(BaseEntity);
			ar& NVP(health)& NVP(name)& NVP(orientation);
		}

		std::string GetTypeName() const override { return "DerivedEntity"; }

		bool operator==(const DerivedEntity& other) const
		{
			return BaseEntity::operator==(other) &&
				   health == other.health &&
				   name == other.name &&
				   orientation.x == other.orientation.x &&
				   orientation.y == other.orientation.y &&
				   orientation.z == other.orientation.z &&
				   orientation.w == other.orientation.w;
		}

		friend std::ostream& operator<<(std::ostream& os, const DerivedEntity& data)
		{
			os << "DerivedEntity{id:" << data.id << ",name:" << data.name
			   << ",health:" << data.health << "}";
			return os;
		}
	};

	struct Component
	{
		std::string componentName;
		bool enabled = true;

		virtual ~Component() = default;

		SERIALIZE()
		{
			ar& NVP(componentName)& NVP(enabled);
		}

		virtual std::string GetComponentType() const { return "Component"; }

		bool operator==(const Component& other) const
		{
			return componentName == other.componentName && enabled == other.enabled;
		}

		friend std::ostream& operator<<(std::ostream& os, const Component& data)
		{
			os << "Component{" << data.componentName << "}";
			return os;
		}
	};

	struct GameObject : public BaseEntity
	{
		Component component;
		std::vector<std::string> tags;

		SERIALIZE()
		{
			SAVE_PARENT(BaseEntity);
			ar& NVP(component)& NVP(tags);
		}

		std::string GetTypeName() const override { return "GameObject"; }

		bool operator==(const GameObject& other) const
		{
			return BaseEntity::operator==(other) &&
				   component == other.component &&
				   tags == other.tags;
		}

		friend std::ostream& operator<<(std::ostream& os, const GameObject& data)
		{
			os << "GameObject{id:" << data.id << ",tags:" << data.tags.size() << "}";
			return os;
		}
	};

	// Basic types serialization tests
	TEST(Core.Serialization, SerializeInt)
	{
		int original = 42;
		std::string serialized = Serializer::serialize_simple(original);
		ASSERT_TRUE(serialized.length() > 0);
	}

	TEST(Core.Serialization, SerializeFloat)
	{
		float original = 3.14159f;
		std::string serialized = Serializer::serialize_simple(original);
		ASSERT_TRUE(serialized.length() > 0);
	}

	TEST(Core.Serialization, SerializeString)
	{
		std::string original = "Hello, Serialization!";
		std::string serialized = Serializer::serialize_simple(original);
		ASSERT_TRUE(serialized.length() > 0);
	}

	TEST(Core.Serialization, SerializeVec3)
	{
		vec3 original(1.5f, 2.5f, 3.5f);
		std::string serialized = Serializer::serialize_simple(original);
		ASSERT_TRUE(serialized.length() > 0);
	}

	// Round-trip serialization tests
	TEST(Core.Serialization, RoundTripInt)
	{
		int original = 12345;
		std::string serialized = Serializer::serialize(original);
		auto deserialized = Serializer::deserialize<int>(serialized);
		ASSERT_TRUE(deserialized != nullptr);
		ASSERT_EQ(*deserialized, original);
	}

	TEST(Core.Serialization, RoundTripFloat)
	{
		float original = 2.71828f;
		std::string serialized = Serializer::serialize(original);
		auto deserialized = Serializer::deserialize<float>(serialized);
		ASSERT_TRUE(deserialized != nullptr);
		ASSERT_TRUE(std::abs(*deserialized - original) < 0.0001f);
	}

	TEST(Core.Serialization, RoundTripString)
	{
		std::string original = "Test String with Spaces";
		std::string serialized = Serializer::serialize(original);
		auto deserialized = Serializer::deserialize<std::string>(serialized);
		ASSERT_TRUE(deserialized != nullptr);
		ASSERT_EQ(*deserialized, original);
	}

	TEST(Core.Serialization, RoundTripVec3)
	{
		vec3 original(1.0f, 2.0f, 3.0f);
		std::string serialized = Serializer::serialize(original);
		auto deserialized = Serializer::deserialize<vec3>(serialized);
		ASSERT_TRUE(deserialized != nullptr);
		ASSERT_EQ(deserialized->x, original.x);
		ASSERT_EQ(deserialized->y, original.y);
		ASSERT_EQ(deserialized->z, original.z);
	}

	TEST(Core.Serialization, RoundTripMat4x4)
	{
		mat4x4 original;
		original.identity();
		std::string serialized = Serializer::serialize(original);
		auto deserialized = Serializer::deserialize<mat4x4>(serialized);
		ASSERT_TRUE(deserialized != nullptr);
		for (int i = 0; i < 16; ++i)
		{
			ASSERT_TRUE(std::abs(original.elems[i] - deserialized->elems[i]) < 0.0001f);
		}
	}

	TEST(Core.Serialization, RoundTripQuat)
	{
		quat original(0.0f, 0.0f, 0.0f, 1.0f);
		std::string serialized = Serializer::serialize(original);
		auto deserialized = Serializer::deserialize<quat>(serialized);
		ASSERT_TRUE(deserialized != nullptr);
		ASSERT_EQ(deserialized->x, original.x);
		ASSERT_EQ(deserialized->y, original.y);
		ASSERT_EQ(deserialized->z, original.z);
		ASSERT_EQ(deserialized->w, original.w);
	}

	// Data integrity tests
	TEST(Core.Serialization, IntegritySmallValues)
	{
		int values[] = { -1, 0, 1, 100, -100 };
		for (int val : values)
		{
			std::string serialized = Serializer::serialize(val);
			auto deserialized = Serializer::deserialize<int>(serialized);
			ASSERT_TRUE(deserialized != nullptr);
			ASSERT_EQ(*deserialized, val);
		}
	}

	TEST(Core.Serialization, IntegrityLargeValues)
	{
		int values[] = { 2147483647, -2147483647 };
		for (int val : values)
		{
			std::string serialized = Serializer::serialize(val);
			auto deserialized = Serializer::deserialize<int>(serialized);
			ASSERT_TRUE(deserialized != nullptr);
			ASSERT_EQ(*deserialized, val);
		}
	}

	TEST(Core.Serialization, IntegrityVec3Extreme)
	{
		vec3 values[] = {
			vec3(0.0f, 0.0f, 0.0f),
			vec3(-1.0f, -1.0f, -1.0f),
			vec3(1000.0f, 1000.0f, 1000.0f),
			vec3(-1000.0f, -1000.0f, -1000.0f)
		};
		for (const auto& original : values)
		{
			std::string serialized = Serializer::serialize(original);
			auto deserialized = Serializer::deserialize<vec3>(serialized);
			ASSERT_TRUE(deserialized != nullptr);
			ASSERT_TRUE(std::abs(deserialized->x - original.x) < 0.0001f);
			ASSERT_TRUE(std::abs(deserialized->y - original.y) < 0.0001f);
			ASSERT_TRUE(std::abs(deserialized->z - original.z) < 0.0001f);
		}
	}

	TEST(Core.Serialization, IntegrityEmptyString)
	{
		std::string original = "";
		std::string serialized = Serializer::serialize(original);
		auto deserialized = Serializer::deserialize<std::string>(serialized);
		ASSERT_TRUE(deserialized != nullptr);
		ASSERT_EQ(*deserialized, original);
	}

	TEST(Core.Serialization, IntegrityLongString)
	{
		std::string original = "This is a much longer string with more characters to test serialization with longer data";
		std::string serialized = Serializer::serialize(original);
		auto deserialized = Serializer::deserialize<std::string>(serialized);
		ASSERT_TRUE(deserialized != nullptr);
		ASSERT_EQ(*deserialized, original);
	}

	// Complex object serialization tests
	TEST(Core.Serialization, SerializeSimpleObject)
	{
		SimpleData original;
		original.intValue = 42;
		original.floatValue = 3.14f;
		original.name = "TestObject";
		original.position = vec3(1.0f, 2.0f, 3.0f);

		std::string serialized = Serializer::serialize(original);
		ASSERT_TRUE(serialized.length() > 0);
	}

	TEST(Core.Serialization, RoundTripSimpleObject)
	{
		SimpleData original;
		original.intValue = 42;
		original.floatValue = 3.14f;
		original.name = "TestObject";
		original.position = vec3(1.0f, 2.0f, 3.0f);

		std::string serialized = Serializer::serialize(original);
		auto deserialized = Serializer::deserialize<SimpleData>(serialized);

		ASSERT_TRUE(deserialized != nullptr);
		ASSERT_EQ(deserialized->intValue, original.intValue);
		ASSERT_TRUE(std::abs(deserialized->floatValue - original.floatValue) < 0.0001f);
		ASSERT_EQ(deserialized->name, original.name);
		ASSERT_EQ(deserialized->position.x, original.position.x);
		ASSERT_EQ(deserialized->position.y, original.position.y);
		ASSERT_EQ(deserialized->position.z, original.position.z);
	}

	TEST(Core.Serialization, SerializeNestedObject)
	{
		NestedData original;
		original.simple.intValue = 100;
		original.simple.floatValue = 2.71f;
		original.simple.name = "NestedSimple";
		original.simple.position = vec3(5.0f, 6.0f, 7.0f);
		original.description = "This is a nested object";
		original.rotation = quat(0.0f, 0.0f, 0.0f, 1.0f);
		original.transform.identity();

		std::string serialized = Serializer::serialize(original);
		ASSERT_TRUE(serialized.length() > 0);
	}

	TEST(Core.Serialization, RoundTripNestedObject)
	{
		NestedData original;
		original.simple.intValue = 100;
		original.simple.floatValue = 2.71f;
		original.simple.name = "NestedSimple";
		original.simple.position = vec3(5.0f, 6.0f, 7.0f);
		original.description = "This is a nested object";
		original.rotation = quat(0.0f, 0.0f, 0.0f, 1.0f);
		original.transform.identity();

		std::string serialized = Serializer::serialize(original);
		auto deserialized = Serializer::deserialize<NestedData>(serialized);

		ASSERT_TRUE(deserialized != nullptr);

		// Check nested simple object
		ASSERT_EQ(deserialized->simple.intValue, original.simple.intValue);
		ASSERT_TRUE(std::abs(deserialized->simple.floatValue - original.simple.floatValue) < 0.0001f);
		ASSERT_EQ(deserialized->simple.name, original.simple.name);
		ASSERT_EQ(deserialized->simple.position.x, original.simple.position.x);
		ASSERT_EQ(deserialized->simple.position.y, original.simple.position.y);
		ASSERT_EQ(deserialized->simple.position.z, original.simple.position.z);

		// Check nested description
		ASSERT_EQ(deserialized->description, original.description);

		// Check nested rotation
		ASSERT_EQ(deserialized->rotation.x, original.rotation.x);
		ASSERT_EQ(deserialized->rotation.y, original.rotation.y);
		ASSERT_EQ(deserialized->rotation.z, original.rotation.z);
		ASSERT_EQ(deserialized->rotation.w, original.rotation.w);

		// Check nested transform
		for (int i = 0; i < 16; ++i)
		{
			ASSERT_TRUE(std::abs(deserialized->transform.elems[i] - original.transform.elems[i]) < 0.0001f);
		}
	}

	TEST(Core.Serialization, ComplexObjectIntegrity)
	{
		SimpleData values[] = {
			{1, 1.5f, "First", vec3(1.0f, 0.0f, 0.0f)},
			{-50, -3.14f, "Second", vec3(0.0f, 1.0f, 0.0f)},
			{999, 100.0f, "Third", vec3(0.0f, 0.0f, 1.0f)}
		};

		for (const auto& original : values)
		{
			std::string serialized = Serializer::serialize(original);
			auto deserialized = Serializer::deserialize<SimpleData>(serialized);
			ASSERT_TRUE(deserialized != nullptr);
			ASSERT_EQ(*deserialized, original);
		}
	}

	// Inheritance and polymorphism serialization tests
	TEST(Core.Serialization, SerializeBaseEntity)
	{
		BaseEntity entity;
		entity.id = 1;
		entity.entityType = "Enemy";
		entity.position = vec3(10.0f, 5.0f, 0.0f);

		std::string serialized = Serializer::serialize(entity);
		ASSERT_TRUE(serialized.length() > 0);
		ASSERT_EQ(entity.GetTypeName(), std::string("BaseEntity"));
	}

	TEST(Core.Serialization, RoundTripBaseEntity)
	{
		BaseEntity original;
		original.id = 42;
		original.entityType = "NPC";
		original.position = vec3(15.0f, 20.0f, 25.0f);

		std::string serialized = Serializer::serialize(original);
		auto deserialized = Serializer::deserialize<BaseEntity>(serialized);

		ASSERT_TRUE(deserialized != nullptr);
		ASSERT_EQ(deserialized->id, original.id);
		ASSERT_EQ(deserialized->entityType, original.entityType);
		ASSERT_EQ(deserialized->position.x, original.position.x);
		ASSERT_EQ(deserialized->position.y, original.position.y);
		ASSERT_EQ(deserialized->position.z, original.position.z);
	}

	TEST(Core.Serialization, SerializeDerivedEntity)
	{
		DerivedEntity entity;
		entity.id = 100;
		entity.entityType = "Player";
		entity.position = vec3(0.0f, 1.0f, 0.0f);
		entity.health = 85.5f;
		entity.name = "Hero";
		entity.orientation = quat(0.0f, 0.0f, 0.7071f, 0.7071f);

		std::string serialized = Serializer::serialize(entity);
		ASSERT_TRUE(serialized.length() > 0);
		ASSERT_EQ(entity.GetTypeName(), std::string("DerivedEntity"));
	}

	TEST(Core.Serialization, RoundTripDerivedEntity)
	{
		DerivedEntity original;
		original.id = 200;
		original.entityType = "Boss";
		original.position = vec3(50.0f, 100.0f, 75.0f);
		original.health = 250.0f;
		original.name = "GreatDragon";
		original.orientation = quat(0.0f, 0.0f, 0.0f, 1.0f);

		std::string serialized = Serializer::serialize(original);
		auto deserialized = Serializer::deserialize<DerivedEntity>(serialized);

		ASSERT_TRUE(deserialized != nullptr);
		// Check base class members
		ASSERT_EQ(deserialized->id, original.id);
		ASSERT_EQ(deserialized->entityType, original.entityType);
		ASSERT_EQ(deserialized->position.x, original.position.x);
		// Check derived class members
		ASSERT_TRUE(std::abs(deserialized->health - original.health) < 0.0001f);
		ASSERT_EQ(deserialized->name, original.name);
		ASSERT_EQ(deserialized->orientation.x, original.orientation.x);
		ASSERT_EQ(deserialized->orientation.w, original.orientation.w);
	}

	TEST(Core.Serialization, SerializeGameObject)
	{
		GameObject gameObj;
		gameObj.id = 300;
		gameObj.entityType = "Item";
		gameObj.position = vec3(5.0f, 5.0f, 5.0f);
		gameObj.component.componentName = "PhysicsComponent";
		gameObj.component.enabled = true;
		gameObj.tags.push_back("Collectible");
		gameObj.tags.push_back("Item");

		std::string serialized = Serializer::serialize(gameObj);
		ASSERT_TRUE(serialized.length() > 0);
		ASSERT_EQ(gameObj.GetTypeName(), std::string("GameObject"));
	}

	TEST(Core.Serialization, RoundTripGameObject)
	{
		GameObject original;
		original.id = 400;
		original.entityType = "Chest";
		original.position = vec3(30.0f, 10.0f, 20.0f);
		original.component.componentName = "TriggerComponent";
		original.component.enabled = false;
		original.tags.push_back("Interactive");
		original.tags.push_back("Loot");
		original.tags.push_back("Trap");

		std::string serialized = Serializer::serialize(original);
		auto deserialized = Serializer::deserialize<GameObject>(serialized);

		ASSERT_TRUE(deserialized != nullptr);
		// Check base entity members
		ASSERT_EQ(deserialized->id, original.id);
		ASSERT_EQ(deserialized->entityType, original.entityType);
		ASSERT_EQ(deserialized->position.x, original.position.x);
		// Check component members
		ASSERT_EQ(deserialized->component.componentName, original.component.componentName);
		ASSERT_EQ(deserialized->component.enabled, original.component.enabled);
		// Check tags
		ASSERT_EQ(deserialized->tags.size(), original.tags.size());
		for (size_t i = 0; i < original.tags.size(); ++i)
		{
			ASSERT_EQ(deserialized->tags[i], original.tags[i]);
		}
	}

	TEST(Core.Serialization, InheritanceIntegrity)
	{
		DerivedEntity entity1;
		entity1.id = 100;
		entity1.entityType = "Player";
		entity1.position = vec3(0.0f, 0.0f, 0.0f);
		entity1.health = 100.0f;
		entity1.name = "Warrior";
		entity1.orientation = quat(0.0f, 0.0f, 0.0f, 1.0f);

		DerivedEntity entity2;
		entity2.id = 101;
		entity2.entityType = "Player";
		entity2.position = vec3(10.0f, 10.0f, 10.0f);
		entity2.health = 75.5f;
		entity2.name = "Mage";
		entity2.orientation = quat(0.0f, 0.0f, 0.0f, 1.0f);

		DerivedEntity entity3;
		entity3.id = 102;
		entity3.entityType = "NPC";
		entity3.position = vec3(-5.0f, -5.0f, -5.0f);
		entity3.health = 50.0f;
		entity3.name = "Merchant";
		entity3.orientation = quat(0.0f, 0.0f, 0.0f, 1.0f);

		DerivedEntity entities[] = {entity1, entity2, entity3};

		for (const auto& original : entities)
		{
			std::string serialized = Serializer::serialize(original);
			auto deserialized = Serializer::deserialize<DerivedEntity>(serialized);
			ASSERT_TRUE(deserialized != nullptr);
			ASSERT_EQ(*deserialized, original);
		}
	}
}
