#include "kgpch.h"

#include "Kargono/Core/Buffer.h"
#include "Kargono/Scene/Scene.h"
#include "Kargono/Scripting/ScriptEngine.h"
#include "Kargono/Scripting/ScriptGlue.h"
#include "Kargono/Scene/Entity.h"
#include "Kargono/Core/UUID.h"
#include "Kargono/Core/Application.h"
#include "Kargono/Core/FileSystem.h"
#include "Kargono/Project/Project.h"
#include "Kargono/Events/ApplicationEvent.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"
#include "mono/metadata/tabledefs.h"
#include "mono/metadata/mono-debug.h"
#include "mono/metadata/threads.h"
#include "FileWatch.hpp"

namespace Kargono
{
	static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap = 
	{
		{"System.Single", ScriptFieldType::Float},
		{"System.Double", ScriptFieldType::Double},

		{"System.Boolean", ScriptFieldType::Bool},
		{"System.Char", ScriptFieldType::Char},
		{"System.Int16", ScriptFieldType::Short},
		{"System.Int32", ScriptFieldType::Int},
		{"System.Int64", ScriptFieldType::Long},

		{"System.Byte", ScriptFieldType::Byte},
		{"System.UInt16", ScriptFieldType::UShort},
		{"System.UInt32", ScriptFieldType::UInt},
		{"System.UInt64", ScriptFieldType::ULong},

		{"Kargono.Vector2", ScriptFieldType::Vector2},
		{"Kargono.Vector3", ScriptFieldType::Vector3},
		{"Kargono.Vector4", ScriptFieldType::Vector4},
		{"Kargono.Entity", ScriptFieldType::Entity},

	};

	namespace Utils
	{

		static void PrintMonoException(MonoObject* exception)
		{
			MonoClass* exceptionClass = mono_object_get_class(exception);
			std::string className = mono_class_get_name(exceptionClass);
			std::string assemblyName = mono_class_get_namespace(exceptionClass);
			KG_CORE_ERROR("An exception was thrown in the InvokeMethod Function! \nException Type: {} \nException Message: {}", className, assemblyName);
			KG_CORE_ASSERT(false);
		}

		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath, bool loadPDB = false)
		{
			Buffer fileData = FileSystem::ReadFileBinary(assemblyPath);

			// NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData.As<char>(), (uint32_t)fileData.Size, 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				const char* errorMessage = mono_image_strerror(status);
				// Log some error message using the errorMessage data
				return nullptr;
			}

			if (loadPDB)
			{
				std::filesystem::path pdbPath = assemblyPath;
				pdbPath.replace_extension(".pdb");

				if (std::filesystem::exists(pdbPath))
				{
					ScopedBuffer pdbFileData = FileSystem::ReadFileBinary(pdbPath);

					mono_debug_open_image_from_memory(image,pdbFileData.As<mono_byte>(), (uint32_t)pdbFileData.Size());
					KG_CORE_INFO("Loaded PDB {}", pdbPath);

				}
			}

			MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.string().c_str(), &status, 0);
			mono_image_close(image);

			// Don't forget to free the file data
			fileData.Release();

			return assembly;
		}

		static void PrintAssemblyTypes(MonoAssembly* assembly)
		{
			MonoImage* image = mono_assembly_get_image(assembly);
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

				KG_CORE_TRACE("{}.{}", nameSpace, name);
			}
		}

		ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType)
		{
			std::string typeName = mono_type_get_name(monoType);

			auto it = s_ScriptFieldTypeMap.find(typeName);

			if (it == s_ScriptFieldTypeMap.end())
			{
				KG_CORE_ERROR("Unknown type: {}", typeName);
				return ScriptFieldType::None;
			}
			return it->second;
		}

	}

	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage= nullptr;

		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;

		std::filesystem::path CoreAssemblyFilepath;
		std::filesystem::path AppAssemblyFilepath;

		ScriptClass EntityClass;

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, Ref<ScriptClassInstance>> EntityInstances;
		std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;

		Scope<filewatch::FileWatch<std::string>> AppAssemblyFileWatcher;
		bool AssemblyReloadPending = false;

#ifdef KG_DEBUG
		bool EnableDebugging = true;
#else
		bool EnableDebugging = false;
#endif


		// Runtime
		Scene* SceneContext = nullptr;
	};

	static ScriptEngineData* s_ScriptData = nullptr;

	static void OnAppAssemblyFileSystemEvent(const std::string& path, const filewatch::Event change_type)
	{
		if (!s_ScriptData->AssemblyReloadPending && change_type == filewatch::Event::modified)
		{
			s_ScriptData->AssemblyReloadPending = true;
			//using namespace std::chrono_literals;
			//std::this_thread::sleep_for(500ms);
			// reload assembly
			// add reload to main thread queue
			Application::GetCurrentApp().SubmitToMainThread([]()
				{
					s_ScriptData->AppAssemblyFileWatcher.reset();
					ScriptEngine::ReloadAssembly();
				});
		}
	}


	void ScriptEngine::Init()
	{
		s_ScriptData = new ScriptEngineData();
		InitMono();
		ScriptGlue::RegisterFunctions();

	}
	void ScriptEngine::Shutdown()
	{
		ShutdownMono();
		delete s_ScriptData;
		s_ScriptData = nullptr;
	}

	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");

		// This code allows debugging to occur inside Script Assembly at runtime
		// Ex: Pong Game can use normal debugging tools!
		if (s_ScriptData->EnableDebugging)
		{
			const char* argv[2] =
			{
				"--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n,loglevel=3,logfile=MonoDebugger.log",
				"--soft-breakpoints"
			};

			mono_jit_parse_options(2, (char**)argv);
			mono_debug_init(MONO_DEBUG_FORMAT_MONO);
		}

		bool status = InitializeRootDomain();

		KG_CORE_ASSERT(status, "Failed to load Mono Root Domain!");

		if (s_ScriptData->EnableDebugging)
		{
			mono_debug_domain_create(s_ScriptData->RootDomain);
		}

		mono_thread_set_main(mono_thread_current());


	}
	void ScriptEngine::ShutdownMono()
	{
		KG_CORE_ASSERT(s_ScriptData->RootDomain, "Mono Shutdown Initiated without valid root domain present!")
		KG_CORE_WARN("Shutting down Mono. Root Domain is {}. App Domain is {}", static_cast<bool>(s_ScriptData->RootDomain), static_cast<bool>(s_ScriptData->AppDomain));

		mono_domain_set(mono_get_root_domain(), false);
		if (s_ScriptData->AppDomain)
		{
			mono_domain_unload(s_ScriptData->AppDomain);
		}
		s_ScriptData->AppDomain = nullptr;
		if (s_ScriptData->RootDomain)
		{
			mono_jit_cleanup(s_ScriptData->RootDomain);
		}
		
		s_ScriptData->RootDomain = nullptr;
	}

	bool ScriptEngine::InitializeRootDomain()
	{
		MonoDomain* rootDomain = mono_jit_init("KargonoJITRuntime");
		// Store the root domain pointer
		s_ScriptData->RootDomain = rootDomain;
		if (!s_ScriptData->RootDomain) { return false; }
		return true;
	}

	bool ScriptEngine::InitializeAppDomain()
	{
		// Create an App Domain
		s_ScriptData->AppDomain = mono_domain_create_appdomain((char*)"KargonoScriptRuntime", nullptr);
		if (!s_ScriptData->AppDomain) { return false; }
		mono_domain_set(s_ScriptData->AppDomain, true);
		return true;
	}

	bool ScriptEngine::LoadCoreAssembly(const std::filesystem::path& filepath)
	{
		s_ScriptData->CoreAssemblyFilepath = filepath;
		s_ScriptData->CoreAssembly = Utils::LoadMonoAssembly(filepath, s_ScriptData->EnableDebugging);
		if (s_ScriptData->CoreAssembly == nullptr) { return false; }
		s_ScriptData->CoreAssemblyImage = mono_assembly_get_image(s_ScriptData->CoreAssembly);
		Utils::PrintAssemblyTypes(s_ScriptData->CoreAssembly);
		return true;
	}

	bool ScriptEngine::LoadAppAssembly(const std::filesystem::path& filepath)
	{
		s_ScriptData->AppAssemblyFilepath = filepath;
		s_ScriptData->AppAssembly = Utils::LoadMonoAssembly(filepath, s_ScriptData->EnableDebugging);
		if (s_ScriptData->AppAssembly == nullptr){return false;}
		s_ScriptData->AppAssemblyImage = mono_assembly_get_image(s_ScriptData->AppAssembly);
		Utils::PrintAssemblyTypes(s_ScriptData->AppAssembly);

		s_ScriptData->AppAssemblyFileWatcher = CreateScope<filewatch::FileWatch<std::string>>(
			filepath.string(), OnAppAssemblyFileSystemEvent);
		s_ScriptData->AssemblyReloadPending = false;
		return true;
	}

	void ScriptEngine::ReloadAssembly()
	{
		mono_domain_set(mono_get_root_domain(), false);
		mono_domain_unload(s_ScriptData->AppDomain);

		bool status = InitializeAppDomain();
		if (!status)
		{
			KG_CORE_ERROR("[ScriptEngine] Could not initialize App Domain.");
			return;
		}

		status = LoadCoreAssembly(s_ScriptData->CoreAssemblyFilepath);
		if (!status)
		{
			KG_CORE_ERROR("[ScriptEngine] Could not load Kargono-ScriptCore assembly.");
			return;
		}
		const auto scriptModulePath = Project::GetAssetDirectory() / Project::GetActive()->GetConfig().ScriptModulePath;
		status = LoadAppAssembly(scriptModulePath);
		if (!status)
		{
			KG_CORE_ERROR("[ScriptEngine] Could not load app assembly.");
			return;
		}

		// Retrieve and instantiate class
		LoadAssemblyClasses();
		ScriptGlue::RegisterComponents();

		s_ScriptData->EntityClass = ScriptClass("Kargono", "Entity", true);

	}

	void ScriptEngine::InitialAssemblyLoad()
	{
		bool status = InitializeAppDomain();
		if (!status)
		{
			KG_CORE_ERROR("[ScriptEngine] Could not initialize App Domain.");
			return;
		}

		status = LoadCoreAssembly("Resources/Scripts/Kargono-ScriptCore.dll");
		if (!status)
		{
			KG_CORE_ERROR("[ScriptEngine] Could not load Kargono-ScriptCore assembly.");
			return;
		}

		auto scriptModulePath = Project::GetAssetDirectory() / Project::GetActive()->GetConfig().ScriptModulePath;
		status = LoadAppAssembly(scriptModulePath);
		if (!status)
		{
			KG_CORE_ERROR("[ScriptEngine] Could not load app assembly.");
			return;
		}
		LoadAssemblyClasses();
		ScriptGlue::RegisterComponents();

		s_ScriptData->EntityClass = ScriptClass("Kargono", "Entity", true);
	}

	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{
		s_ScriptData->SceneContext = scene;

		// Instantiate all script entities
		auto view = scene->GetAllEntitiesWith<ScriptComponent>();
		for (auto e : view)
		{
			Entity entity = { e, scene };

			OnCreateEntity(entity);
		}
	}

	bool ScriptEngine::EntityClassExists(const std::string& fullClassName)
	{
		return s_ScriptData->EntityClasses.find(fullClassName) != s_ScriptData->EntityClasses.end();
	}

	bool ScriptEngine::AppDomainExists()
	{
		return static_cast<bool>(s_ScriptData->AppDomain);
	}

	MonoDomain* ScriptEngine::GetAppDomain()
	{
		return s_ScriptData->AppDomain;
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		const auto& sc = entity.GetComponent<ScriptComponent>();
		if (ScriptEngine::EntityClassExists(sc.ClassName))
		{
			// Create a ScriptInstance specific to the entity and its ScriptClass
			// Store ScriptInstance inside EntityInstances map with UUID
			UUID entityID = entity.GetUUID();
			Ref<ScriptClassInstance> instance = CreateRef<ScriptClassInstance>(s_ScriptData->EntityClasses[sc.ClassName], entity);
			s_ScriptData->EntityInstances[entity.GetUUID()] = instance;

			// Copy field values into C# Script
			if (s_ScriptData->EntityScriptFields.contains(entityID))
			{
				const ScriptFieldMap& fieldMap = s_ScriptData->EntityScriptFields.at(entityID);
				for (const auto& [name, fieldInstance] : fieldMap)
				{
					instance->SetFieldValueInternal(name, fieldInstance.m_Buffer);
				}
			}

			instance->InvokeOnCreate();
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
	{
		UUID entityUUID = entity.GetUUID();

		KG_CORE_ASSERT(s_ScriptData->EntityInstances.contains(entityUUID), "Could not find ScriptInstance for entity");
		Ref<ScriptClassInstance> instance = s_ScriptData->EntityInstances[entityUUID];
		instance->InvokeOnUpdate(ts);
	}

	Ref<ScriptClassInstance> ScriptEngine::GetEntityScriptInstance(UUID entityID)
	{
		auto it = s_ScriptData->EntityInstances.find(entityID);
		if (it == s_ScriptData->EntityInstances.end()) { return nullptr; }

		return it->second;
	}


	void ScriptEngine::OnRuntimeStop()
	{
		s_ScriptData->SceneContext = nullptr;

		s_ScriptData->EntityInstances.clear();
	}

	void ScriptEngine::OnUpdate(Timestep ts)
	{
		auto view = s_ScriptData->SceneContext->GetAllEntitiesWith<ScriptComponent>();
		// Run On-Update function for each ScriptObject
		for (auto e : view)
		{
			Entity entity = { e, s_ScriptData->SceneContext };
			if (entity.GetComponent<ScriptComponent>().ClassName == "None") { continue; }
			ScriptEngine::OnUpdateEntity(entity, ts);
		}
	}

	void ScriptEngine::OnPhysicsCollision(PhysicsCollisionEvent event)
	{
		UUID entityOne = event.GetEntityOne();
		UUID entityTwo = event.GetEntityTwo();
		bool collisionHandled = false;

		if (s_ScriptData->EntityInstances.contains(entityOne))
		{
			Ref<ScriptClassInstance> monoEntityInstance = s_ScriptData->EntityInstances[entityOne];
			collisionHandled = monoEntityInstance->InvokeOnPhysicsCollision(entityTwo);
		}

		if (!collisionHandled && s_ScriptData->EntityInstances.contains(entityTwo))
		{
			Ref<ScriptClassInstance> monoEntityInstance = s_ScriptData->EntityInstances[entityTwo];
			monoEntityInstance->InvokeOnPhysicsCollision(entityOne);
		}
	}

	void ScriptEngine::LoadAssemblyClasses()
	{
		s_ScriptData->EntityClasses.clear();

		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_ScriptData->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		MonoClass* entityClass = mono_class_from_name(s_ScriptData->CoreAssemblyImage, "Kargono", "Entity");

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(s_ScriptData->AppAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* className = mono_metadata_string_heap(s_ScriptData->AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);
			std::string fullName;
			if (strlen(nameSpace) != 0) { fullName = fmt::format("{}.{}", nameSpace, className); }
			else { fullName = className; }

			MonoClass* monoClass = mono_class_from_name(s_ScriptData->AppAssemblyImage, nameSpace, className);

			if (monoClass == entityClass) { continue; }

			bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);

			if (!isEntity) { continue; }
			void* iter = nullptr;
			MonoMethod* method;
			KG_CORE_WARN("{} class methods:", className);
			while (method = mono_class_get_methods(monoClass, &iter))
			{
				KG_CORE_WARN("	{}", mono_method_get_name(method));
			}

			Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, className);
			s_ScriptData->EntityClasses[fullName] = scriptClass;

			int fieldCount = mono_class_num_fields(monoClass);
			KG_CORE_WARN("{} has {} fields:", className, fieldCount);
			void* iterator = nullptr;
			while (MonoClassField* field = mono_class_get_fields(monoClass, &iterator))
			{
				const char* fieldName = mono_field_get_name(field);
				uint32_t flags = mono_field_get_flags(field);
				if (flags & FIELD_ATTRIBUTE_PUBLIC)
				{
					MonoType* type = mono_field_get_type(field);
					ScriptFieldType fieldType = Utils::MonoTypeToScriptFieldType(type);
					KG_CORE_WARN("  {} ({})", fieldName, Utils::ScriptFieldTypeToString(fieldType));
					scriptClass->AddField(fieldName, fieldType, field);
				}
			}
		}
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return s_ScriptData->CoreAssemblyImage;
	}

	MonoObject* ScriptEngine::GetManagedInstance(UUID uuid)
	{
		KG_CORE_ASSERT(s_ScriptData->EntityInstances.contains(uuid))
		return s_ScriptData->EntityInstances.at(uuid)->GetInstance();
	}

	Scene* ScriptEngine::GetSceneContext()
	{
		return s_ScriptData->SceneContext;
	}

	Ref<ScriptClass> ScriptEngine::GetEntityClass(const std::string& name)
	{
		if (s_ScriptData->EntityClasses.find(name) == s_ScriptData->EntityClasses.end()) { return nullptr; }

		return s_ScriptData->EntityClasses.at(name);
	}

	std::unordered_map<std::string, Ref<ScriptClass>> ScriptEngine::GetEntityClasses()
	{
		return s_ScriptData->EntityClasses;
	}

	ScriptFieldMap& ScriptEngine::GetScriptFieldMap(Entity entity)
	{
		KG_CORE_ASSERT(entity)
		UUID entityID = entity.GetUUID();
		//KG_CORE_ASSERT(s_ScriptData->EntityScriptFields.find(entity.GetUUID()) != s_ScriptData->EntityScriptFields.end())

		return s_ScriptData->EntityScriptFields[entityID];
	}

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore)
		: m_ClassNamespace(classNamespace), m_ClassName(className)
	{
		m_MonoClass = mono_class_from_name(isCore ? s_ScriptData->CoreAssemblyImage : s_ScriptData->AppAssemblyImage,
		                                   classNamespace.c_str(), className.c_str());

	}

	MonoObject* ScriptClass::Instantiate()
	{
		MonoObject* instance = mono_object_new(s_ScriptData->AppDomain, m_MonoClass);
		mono_runtime_object_init(instance);
		return instance;
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, int parameterCount)
	{
		MonoMethod* returnMethod = mono_class_get_method_from_name(m_MonoClass, name.c_str(), parameterCount);
		return returnMethod;
	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
	{
		MonoObject* exception = nullptr;
		MonoObject* returnValue = mono_runtime_invoke(method, instance, params, &exception);

		if (exception) { Utils::PrintMonoException(exception); return nullptr; }

		return returnValue;

	}

	bool ScriptClass::AddField(const char* fieldName, const ScriptFieldType& fieldType, MonoClassField* field)
	{
		auto [newElement, insertionSuccessful] = m_Fields.insert({fieldName, {fieldType, fieldName, field}});
		return insertionSuccessful;
	}

	ScriptClassInstance::ScriptClassInstance(Ref<ScriptClass> scriptClass, Entity entity)
		: m_ScriptClass(scriptClass)
	{
		m_Instance = scriptClass->Instantiate();

		m_Constructor = s_ScriptData->EntityClass.GetMethod(".ctor", 1);

		m_OnCreateMethod = scriptClass->GetMethod("OnCreate", 0);
		m_OnUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);
		m_OnCollisionMethod = scriptClass->GetMethod("OnPhysicsCollision", 1);


		// Call Entity Constructor
		{
			UUID entityID = entity.GetUUID();
			void* param = &entityID;
			m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);
		}
	}

	void ScriptClassInstance::InvokeOnCreate()
	{
		if (m_OnCreateMethod) { m_ScriptClass->InvokeMethod(m_Instance, m_OnCreateMethod);}
	}

	void ScriptClassInstance::InvokeOnUpdate(float ts)
	{
		KG_CORE_ASSERT(m_Instance, "Empty Script Instance!")
		if (m_OnUpdateMethod)
		{
			void* param = &ts;
			m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdateMethod, &param);
		}
	}

	bool ScriptClassInstance::InvokeOnPhysicsCollision(UUID otherEntity)
	{
		KG_CORE_ASSERT(m_Instance, "Empty Script Instance!")
			if (m_OnCollisionMethod)
			{
				void* param = &otherEntity;
				return *(bool*)m_ScriptClass->InvokeMethod(m_Instance, m_OnCollisionMethod, &param);
			}
		return false;
	}

	bool ScriptClassInstance::GetFieldValueInternal(const std::string& name, void* buffer)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end()) { return false; }

		const ScriptField& field = it->second;
		mono_field_get_value(m_Instance, field.ClassField, buffer);
		return true;

	}

	bool ScriptClassInstance::SetFieldValueInternal(const std::string& name, const void* value)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end()) { return false; }

		const ScriptField& field = it->second;
		mono_field_set_value(m_Instance, field.ClassField, (void*)value);
		return true;

	}

}
