#pragma once

#include "Kargono/Core/Timestep.h"

#include <filesystem>
#include <string>
#include <map>

//============================================================
// Forward Declarations for mono classes
//============================================================
// These are here to prevent including mono in the header
// file. Mono is pretty heavy.
// Also these are C Style forward declaration
extern "C"
{
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoDomain MonoDomain;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoClassField MonoClassField;
}

namespace Kargono
{
	// More Forward Declarations!
	namespace Events { class PhysicsCollisionEvent; }
	class ScriptClassEntityInstance;
	class Scene;
	class ScriptClass;
	class Entity;
	struct ScriptMethod;

	//============================================================
	// Enums and Other Type Declarations
	//============================================================

	// Enum represents field types inside C# Classes inside Mono Assembly
	enum class ScriptFieldType
	{
		None = 0,
		Float, Double,
		Bool, Char, Byte, Short, Int, Long,
		UByte, UShort, UInt, ULong,
		Vector2, Vector3, Vector4,
		Entity
	};

	// Utility Functions for Serializing/Deserializing ScriptFieldType
	namespace Utils
	{
		inline const char* ScriptFieldTypeToString(ScriptFieldType fieldType)
		{
			switch (fieldType)
			{
			case ScriptFieldType::None:		return "None";
			case ScriptFieldType::Float:	return "Float";
			case ScriptFieldType::Double:	return "Double";
			case ScriptFieldType::Bool:		return "Bool";
			case ScriptFieldType::Char:		return "Char";
			case ScriptFieldType::Byte:		return "Byte";
			case ScriptFieldType::Short:	return "Short";
			case ScriptFieldType::Int:		return "Int";
			case ScriptFieldType::UInt:		return "UInt";
			case ScriptFieldType::ULong:	return "ULong";
			case ScriptFieldType::Vector2:	return "Vector2";
			case ScriptFieldType::Vector3:	return "Vector3";
			case ScriptFieldType::Vector4:	return "Vector4";
			case ScriptFieldType::Entity:	return "Entity";
			}
			KG_CORE_ASSERT(false, "Unknown field type");
			return "None";
		}

		inline ScriptFieldType ScriptFieldTypeFromString(std::string_view fieldType)
		{
			if (fieldType == "None")	return ScriptFieldType::None;
			if (fieldType == "Float")	return ScriptFieldType::Float;
			if (fieldType == "Double")	return ScriptFieldType::Double;
			if (fieldType == "Bool")	return ScriptFieldType::Bool;
			if (fieldType == "Char")	return ScriptFieldType::Char;
			if (fieldType == "Byte")	return ScriptFieldType::Byte;
			if (fieldType == "Short")	return ScriptFieldType::Short;
			if (fieldType == "Int")		return ScriptFieldType::Int;
			if (fieldType == "Long")	return ScriptFieldType::Long;
			if (fieldType == "UByte")	return ScriptFieldType::UByte;
			if (fieldType == "UShort")	return ScriptFieldType::UShort;
			if (fieldType == "UInt")	return ScriptFieldType::UInt;
			if (fieldType == "ULong")	return ScriptFieldType::ULong;
			if (fieldType == "Vector2")	return ScriptFieldType::Vector2;
			if (fieldType == "Vector3")	return ScriptFieldType::Vector3;
			if (fieldType == "Vector4")	return ScriptFieldType::Vector4;
			if (fieldType == "Entity")	return ScriptFieldType::Entity;

			KG_CORE_ASSERT(false, "Unknown field type");
			return ScriptFieldType::None;
		}
	}

	// Struct representation of field types inside C# classes inside
	// Mono Assembly
	struct ScriptField
	{
		ScriptFieldType Type;
		std::string Name;
		MonoClassField* ClassField;
	};

	// Struct holds ScriptField Struct and Data Associated with field
	// inside byte array (m_Buffer)
	// Ex: Float (ScriptField) and 4 Bytes of data (four bytes of m_Buffer)
	// Script field + data storage
	struct ScriptFieldInstance
	{
		// Represent Field Type inside instance
		ScriptField Field;
		//============================
		// Constructor
		//============================
		// This constructor sets the current buffer to 0
		ScriptFieldInstance()
		{
			memset(m_Buffer, 0, sizeof(m_Buffer));
		}
		//============================
		// Getter/Setters
		//============================
		// Provides access to buffer
		// Template used to allow access using different
		// types of fields (Ex: A float would access the first
		// for bytes of m_Buffer while a long would access all
		// 8 bytes.
		template<typename T>
		T GetValue()
		{
			KG_CORE_ASSERT(sizeof(T) <= 16, "Type too large!");
			return *(T*)m_Buffer;
		}
		// Provides ability to set data inside m_Buffer based
		// on type presented
		template<typename T>
		void SetValue(T value)
		{
			KG_CORE_ASSERT(sizeof(T) <= 16, "Type too large!");
			memcpy(m_Buffer, &value, sizeof(T));
		}
	private:
		// Byte buffer that holds field value at runtime
		uint8_t m_Buffer[8];

		friend class ScriptEngine;
		friend class ScriptClassEntityInstance;
	};

	// This map serves as the C++ representation of the fields associated with
	// a particular entity. Each entity with a script has a ScriptFieldMap
	// to access Field Data in C++.
	using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;


	//============================================================
	// Script Engine Class
	//============================================================
	// Main Scripting Engine Instance implemented with Mono.
	// Functions are static since Mono should be a singleton.
	class ScriptEngine
	{
	public:
		//============================
		// LifeCycle Functions
		//============================
		// This method initializes Mono runtime and registers C++ Functions to C#.
		// (*) Note this function does not load any assemblies
		static void Init();

		// This method closes any open assemblies and shuts down
		//		Mono runtime cleanly.
		// This method also cleans static data inside ScriptEngine.cpp
		static void Shutdown();

		// These functions provide initialization and clean-up for scripts at runtime

		// This function stores the active scene pointer and associates each entity
		//		that has a script component with an EntityInstance class and adds it
		//		to the EntityInstances Map
		static void OnRuntimeStart(Scene* scene);
		// This function removes the scene pointer and clears the EntityInstances Map
		static void OnRuntimeStop();
		// This function calls the OnUpdate function for all registered entities with a ScriptComponent
		static void OnUpdate(Timestep ts);
		// This function handles collision events that are registered with individual entities
		static void OnPhysicsCollision(Events::PhysicsCollisionEvent event);

		// Private Functions that serve as supporting functionality for above LifeCycle Functions
	private:
		// Function initializes mono root domain, adds optional debugging, and makes mono current
		//		to the calling thread
		// Function is called by Init()
		static void InitMono();
		// Function closes Root and App domain
		// Function is called by Shutdown()
		static void ShutdownMono();
		// Function calls OnUpdate function in C# for an individual entity
		// Function called in OnUpdate(ts)
		static void OnUpdateEntity(Entity entity, Timestep ts);
		// Add Entity into EntityInstances Map and corresponding C# entity
		// public fields
		// Function called in OnRuntimeStart()
		static void OnCreateEntity(Entity entity);

	public:
		//============================
		// Assembly Related Functions
		//============================
		// These functions manage the core and app assemblies.
		//		These assemblies are the .dll binaries that hold
		//		the C# classes/methods/namespace/fields.
		// Ex:	Core Assembly is the binary produced by the C# project
		//		Kargono-ScriptCore inside this directory. This project
		//		holds classes/functions used by the class inside
		//		the app assembly.
		//		The assembly can be reflected on to locate
		//		classes, functions, namespaces, etc...
		//		App Assembly is the binary produced by the end-game.
		//		This includes game logic and game classes through
		//		scripts.

		// These functions initialize and reinitialize the core and app assemblies
		//		These functions serve similar purposes, except the ReloadAssembly
		//		function unloads any previously loaded assemblies/domains. Both
		//		functions load the app domain, load the core and app assemblies,
		//		reflect on all class types and fields available in assemblies,
		//		and initialize/reinitialize C++ component map in ScriptGlue
		static void InitialAssemblyLoad();
		static void ReloadAssembly();

		// Private Functions that serve as supporting functions for above assembly API
	private:
		// These functions initialize the root domain and app domain respectively
		//		and store the pointer in ScriptEngine.cpp static variables
		static bool InitializeRootDomain();
		static bool InitializeAppDomain();
		// Functions load Core and App Assemblies as bytes arrays and register
		// them with the Mono runtime.
		static bool LoadCoreAssembly(const std::filesystem::path& filepath);
		static bool LoadAppAssembly(const std::filesystem::path& filepath);
		// Function queries app assembly for classes and loads EntityClasses Map
		// with references to class types
		static void LoadAssemblyClasses();

	public:
		//============================
		// Getter/Setter/Query Functions
		//============================
		static MonoImage* GetCoreAssemblyImage();
		static bool AppDomainExists();
		static MonoDomain* GetAppDomain();
		static MonoObject* GetManagedInstance(UUID uuid);
		static Ref<ScriptClass> GetEntityClass(const std::string& name);
		static std::unordered_map<std::string, Ref<ScriptClass>> GetEntityClasses();
		static std::unordered_map<std::string, ScriptMethod>& GetCustomCallMap();
		static bool EntityClassExists(const std::string& fullClassName);
		static Scene* GetSceneContext();
		static ScriptFieldMap& GetScriptFieldMap(Entity entity);
		static Ref<ScriptClassEntityInstance> GetEntityScriptInstance(UUID entityID);

	private:
		// Friend declarations
		friend class ScriptClass;
		friend class ScriptGlue;
	};

	//============================================================
	// ScriptClass Class
	//============================================================
	// This class represents an individual C# Class that has been identified
	//		in the Mono Core or App Assembly Currently, instances of the
	//		ScriptClass are instantiated when a new assembly is loaded.
	//		In the context of C#, this object simply represents a class, not
	//		a C# instance of a class. The ScriptClassEntityInstance object represents
	//		an actual C# object.
	class ScriptClass
	{
	public:
		//============================
		// Constructors
		//============================
		ScriptClass() = default;
		// This is the main method for creating a ScriptClass. This constructor simply saves parameters
		//		and instantiates the underlying MonoClass* (m_MonoClass).
		ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore = false);

		//============================
		// Getter/Setter
		//============================

		// This function returns a particular method associated with the ScriptClass
		MonoMethod* GetMethod(const std::string& name, int parameterCount);
		// These functions serve allow access to public fields (type not instance) associated with the C# Class
		const std::map<std::string, ScriptField>& GetFields() const { return m_Fields; }
		bool AddField(const char* fieldName, const ScriptFieldType& fieldType, MonoClassField* field);
		//============================
		// Functions for ScriptClassInstances's use
		//============================

		// Instantiate produces a MonoObject* to a new instance object of the C# class.
		//		This function is used to create the underlying mono object associated with
		//		a ScriptClassEntityInstance
		MonoObject* Instantiate();
		// This function is used by a ScriptClassEntityInstance to invoke a class method from
		//		the C# class
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* method, void** params = nullptr);
		MonoClass* GetMonoClass() { return m_MonoClass; }
	private:
		// Namespace and ClassName Describing the class. Mostly for debug purposes
		std::string m_ClassNamespace;
		std::string m_ClassName;
		// Map Contains all ScriptField Types associated with C# Class
		std::map<std::string, ScriptField> m_Fields;
		// Underlying Mono Class Pointer
		MonoClass* m_MonoClass = nullptr;

		friend class ScriptEngine;
	};

	struct ScriptMethod
	{
	public:
		std::string m_Name{};
		uint32_t m_NumParameters{};
		MonoMethod* m_MonoMethod{ nullptr };
	};

	class ScriptClassCustomCallInstance
	{
	public:
		ScriptClassCustomCallInstance() = default;
		ScriptClassCustomCallInstance(Ref<ScriptClass> scriptClass);
	public:
		void InvokeMethod(const std::string& methodName);
		void AddMethod(ScriptMethod method) { m_Methods.insert_or_assign(method.m_Name, method); }
		std::unordered_map<std::string, ScriptMethod> m_Methods{};
	private:
		// Underlying MonoObject*
		MonoObject* m_Instance = nullptr;
		// Class
		Ref<ScriptClass> m_ScriptClass;
		// All Available Methods

		friend class ScriptEngine;
	};

	//============================================================
	// ScriptClassEntityInstance Class
	//============================================================
	// This class represents an object instance of a C# class in an
	//		assembly file.
	// Ex: A C# Player Class inside the App Assembly is represented by
	//		the C++ ScriptClass. A new instance of this C# Player Class is
	//		represented by the C++ ScriptClassEntityInstance Class.
	class ScriptClassEntityInstance
	{
	public:
		//============================
		// Constructor(s)
		//============================
		// This constructor does a few tasks:
		//		1. Uses the ScriptClass method Instantiate() to create
		//		a MonoObject* and store it in m_Instance
		//		2. Get references to class methods (currently this includes
		//		the constructor, OnCreate, OnUpdate, and OnPhysicsCollision) from scriptClass
		//		3. Calls constructor for MonoObject*
		ScriptClassEntityInstance(Ref<ScriptClass> scriptClass, Entity entity);
		//============================
		// Interface for Method Calls
		//============================
		// Calls C# OnCreate Function
		void InvokeOnCreate();
		// Calls C# OnUpdate Function
		void InvokeOnUpdate(float ts);
		// Calls C# OnUpdate Function
		bool InvokeOnPhysicsCollision(UUID otherEntity);

		//============================
		// Getter/Setter
		//============================
		// Get internal reference to ScriptClass
		Ref<ScriptClass> GetScriptClass() { return m_ScriptClass; }
		// Returns pointer to object!
		MonoObject* GetInstance() { return m_Instance; }

		// Public API for getting and setting field values inside C# object.
		//		These methods are templated to allow for different types
		//		of fields to be Get/Set. These functions call the internal
		//		functions Get/SetFieldValueInternal to actually interact
		//		with Mono
		template<typename T>
		T GetFieldValue(const std::string& name)
		{
			KG_CORE_ASSERT(sizeof(T) <= 16, "Type too large!");

			bool success = GetFieldValueInternal(name, s_FieldValueBuffer);
			if (!success) { return T(); }
			return *(T*)s_FieldValueBuffer;
		}
		template<typename T>
		void SetFieldValue(const std::string& name, T value)
		{
			KG_CORE_ASSERT(sizeof(T) <= 16, "Type too large!");
			SetFieldValueInternal(name, &value);
		}

	private:
		// These functions get and set field values inside the C# instance
		//		during runtime.
		bool GetFieldValueInternal(const std::string& name, void* buffer);
		bool SetFieldValueInternal(const std::string& name, const void* value);
	private:
		// MonoClass that this object is an instance of
		Ref<ScriptClass> m_ScriptClass;
		// Underlying MonoObject*
		MonoObject* m_Instance = nullptr;
		// Method Pointers
		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreateMethod = nullptr;
		MonoMethod* m_OnUpdateMethod = nullptr;
		MonoMethod* m_OnCollisionMethod = nullptr;
		// Buffer used to transfer data when performing field update operations
		//		(Get/SetFieldValueInternal)
		inline static char s_FieldValueBuffer[8];

		friend class ScriptEngine;
		friend struct ScriptFieldInstance;
	};
}
