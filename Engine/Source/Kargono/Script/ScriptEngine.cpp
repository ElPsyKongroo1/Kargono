#include "kgpch.h"

#include "Kargono/Core/Buffer.h"
#include "Kargono/Scene/Scene.h"
#include "Kargono/Script/ScriptEngine.h"
#include "Kargono/Script/ScriptGlue.h"
#include "Kargono/Scene/Entity.h"
#include "Kargono/Core/UUID.h"
#include "Kargono/Core/EngineCore.h"
#include "Kargono/Utility/FileSystem.h"
#include "Kargono/Projects/Project.h"
#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/Events/KeyEvent.h"
#include "Kargono/Input/InputMode.h"
#include "Kargono/Input/InputPolling.h"

#include "API/Scripting/MonoBackendAPI.h"
#include "API/FileWatch/FileWatchAPI.h"

namespace Kargono::Script
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
}

namespace Kargono::Utility
{
	static void PrintMonoException(MonoObject* exception)
	{
		MonoClass* exceptionClass = mono_object_get_class(exception);
		std::string className = mono_class_get_name(exceptionClass);
		std::string assemblyName = mono_class_get_namespace(exceptionClass);
		KG_ERROR("An exception was thrown in the InvokeMethod Function! \nException Type: {} \nException Message: {}", className, assemblyName);
	}

	static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath, bool loadPDB = false)
	{
		Buffer fileData = Utility::FileSystem::ReadFileBinary(assemblyPath);

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
				ScopedBuffer pdbFileData = Utility::FileSystem::ReadFileBinary(pdbPath);

				mono_debug_open_image_from_memory(image, pdbFileData.As<mono_byte>(), (uint32_t)pdbFileData.Size());
				KG_INFO("Loaded PDB {}", pdbPath);

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

			KG_INFO("{}.{}", nameSpace, name);
		}
	}

	Script::ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType)
	{
		std::string typeName = mono_type_get_name(monoType);

		auto it = Script::s_ScriptFieldTypeMap.find(typeName);

		if (it == Script::s_ScriptFieldTypeMap.end())
		{
			KG_CRITICAL("Unknown type: {}", typeName);
			return Script::ScriptFieldType::None;
		}
		return it->second;
	}

	
}

namespace Kargono::Script
{

	struct ScriptEngineData
	{
		// Mono Environment
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;
		// Mono Assemblies and their Images
		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage= nullptr;

		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;
		// Stored File paths for assemblies
		std::filesystem::path CoreAssemblyFilepath;
		std::filesystem::path AppAssemblyFilepath;
		// Base Entity Class
		ScriptClass EntityClass;
		// Maps for all entity classes, their specific instances, and the instances' fields
		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		std::unordered_map<UUID, Ref<ScriptClassEntityInstance>> EntityInstances;
		std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;

		// Class and Instance used to make runtime calls that make static changes to the engine.
		//		These functions are created by the end-user for specific needs.
		Ref<ScriptClass> CustomEngineCalls;
		ScriptClassCustomCallInstance CustomEngineCallInstance;

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
			EngineCore::GetCurrentEngineCore().SubmitToMainThread([]()
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

		KG_ASSERT(status, "Failed to load Mono Root Domain!");

		if (s_ScriptData->EnableDebugging)
		{
			mono_debug_domain_create(s_ScriptData->RootDomain);
		}

		mono_thread_set_main(mono_thread_current());


	}
	void ScriptEngine::ShutdownMono()
	{
		KG_ASSERT(s_ScriptData->RootDomain, "Mono Shutdown Initiated without valid root domain present!")
		KG_WARN("Shutting down Mono. Root Domain is {}. App Domain is {}", static_cast<bool>(s_ScriptData->RootDomain), static_cast<bool>(s_ScriptData->AppDomain));

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
		s_ScriptData->CoreAssembly = Utility::LoadMonoAssembly(filepath, s_ScriptData->EnableDebugging);
		if (s_ScriptData->CoreAssembly == nullptr) { return false; }
		s_ScriptData->CoreAssemblyImage = mono_assembly_get_image(s_ScriptData->CoreAssembly);
		Utility::PrintAssemblyTypes(s_ScriptData->CoreAssembly);
		return true;
	}

	bool ScriptEngine::LoadAppAssembly(const std::filesystem::path& filepath)
	{
		s_ScriptData->AppAssemblyFilepath = filepath;
		s_ScriptData->AppAssembly = Utility::LoadMonoAssembly(filepath, s_ScriptData->EnableDebugging);
		if (s_ScriptData->AppAssembly == nullptr){return false;}
		s_ScriptData->AppAssemblyImage = mono_assembly_get_image(s_ScriptData->AppAssembly);
		Utility::PrintAssemblyTypes(s_ScriptData->AppAssembly);

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
			KG_ERROR("[ScriptEngine] Could not initialize App Domain.");
			return;
		}

		status = LoadCoreAssembly(s_ScriptData->CoreAssemblyFilepath);
		if (!status)
		{
			KG_ERROR("[ScriptEngine] Could not load Kargono-ScriptCore assembly.");
			return;
		}
		const auto scriptModulePath = Projects::Project::GetScriptModulePath(true);
		status = LoadAppAssembly(scriptModulePath);
		if (!status)
		{
			KG_ERROR("[ScriptEngine] Could not load app assembly.");
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
			KG_ERROR("[ScriptEngine] Could not initialize App Domain.");
			return;
		}

		status = LoadCoreAssembly("Resources/Scripts/ScriptEngine.dll");
		if (!status)
		{
			KG_ERROR("[ScriptEngine] Could not load ScriptEngine assembly.");
			return;
		}

		auto scriptModulePath = Projects::Project::GetScriptModulePath(true);
		status = LoadAppAssembly(scriptModulePath);
		if (!status)
		{
			KG_ERROR("[ScriptEngine] Could not load app assembly.");
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
			Ref<ScriptClassEntityInstance> instance = CreateRef<ScriptClassEntityInstance>(s_ScriptData->EntityClasses[sc.ClassName], entity);
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

		KG_ASSERT(s_ScriptData->EntityInstances.contains(entityUUID), "Could not find ScriptInstance for entity");
		Ref<ScriptClassEntityInstance> instance = s_ScriptData->EntityInstances[entityUUID];
		instance->InvokeOnUpdate(ts);
	}

	Ref<ScriptClassEntityInstance> ScriptEngine::GetEntityScriptInstance(UUID entityID)
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
		if (InputMode::s_InputMode)
		{
			auto& globalBindings = InputMode::GetCustomCallsOnUpdate();
			for (auto& binding : globalBindings)
			{
				InputMode::KeyboardActionBinding* keyboardBinding = (InputMode::KeyboardActionBinding*)binding.get();
				if (!InputPolling::IsKeyPressed(keyboardBinding->GetKeyBinding())) { continue; }
				void* param = &ts;
				s_ScriptData->CustomEngineCallInstance.InvokeCustomMethod(binding->GetFunctionBinding(), &param);
			}

			for (auto& [className, bindingVector] : InputMode::GetScriptClassOnUpdate())
			{
				if (!Scene::GetScriptClassToEntityList().contains(className)) { continue; }

				for (auto& entity : Scene::GetScriptClassToEntityList().at(className))
				{
					for (auto& binding : bindingVector)
					{
						InputMode::KeyboardActionBinding* keyboardBinding = (InputMode::KeyboardActionBinding*)binding.get();
						if (!InputPolling::IsKeyPressed(keyboardBinding->GetKeyBinding())) { continue; }

						Ref<ScriptClassEntityInstance> instance = s_ScriptData->EntityInstances[entity];
						void* param = &ts;
						instance->m_ScriptClass->InvokeCustomMethod(instance->GetInstance(), binding->GetFunctionBinding(), &param);
					}
				}

			}
		}

		auto view = s_ScriptData->SceneContext->GetAllEntitiesWith<ScriptComponent>();
		// Run On-Update function for each ScriptObject
		for (auto e : view)
		{
			Entity entity = { e, s_ScriptData->SceneContext };
			if (entity.GetComponent<ScriptComponent>().ClassName == "None") { continue; }
			ScriptEngine::OnUpdateEntity(entity, ts);
		}
	}

	void ScriptEngine::OnKeyPressed(Events::KeyPressedEvent event)
	{
		KG_PROFILE_FUNCTION()

		if (event.IsRepeat()) { return; }
		if (InputMode::s_InputMode)
		{
			auto& globalBindings = InputMode::GetCustomCallsOnKeyPressed();
			for (auto& binding : globalBindings)
			{
				InputMode::KeyboardActionBinding* keyboardBinding = (InputMode::KeyboardActionBinding*)binding.get();
				if (!InputPolling::IsKeyPressed(keyboardBinding->GetKeyBinding())) { continue; }

				s_ScriptData->CustomEngineCallInstance.InvokeCustomMethod(binding->GetFunctionBinding());
			}
			auto& scriptClassBindings = InputMode::GetScriptClassOnKeyPressed();
			for (auto& [className, bindingVector] : scriptClassBindings)
			{
				if (!Scene::GetScriptClassToEntityList().contains(className)) { continue; }

				for (auto& entity : Scene::GetScriptClassToEntityList().at(className))
				{
					for (auto& binding : bindingVector)
					{
						InputMode::KeyboardActionBinding* keyboardBinding = (InputMode::KeyboardActionBinding*)binding.get();
						if (!InputPolling::IsKeyPressed(keyboardBinding->GetKeyBinding())) { continue; }

						Ref<ScriptClassEntityInstance> instance = s_ScriptData->EntityInstances[entity];
						instance->m_ScriptClass->InvokeCustomMethod(instance->GetInstance(), binding->GetFunctionBinding());
					}
				}

			}
		}
		
	}

	void ScriptEngine::OnPhysicsCollision(Events::PhysicsCollisionEvent event)
	{
		UUID entityOne = event.GetEntityOne();
		UUID entityTwo = event.GetEntityTwo();
		bool collisionHandled = false;

		if (s_ScriptData->EntityInstances.contains(entityOne))
		{
			Ref<ScriptClassEntityInstance> monoEntityInstance = s_ScriptData->EntityInstances[entityOne];
			collisionHandled = monoEntityInstance->InvokeOnPhysicsCollision(entityTwo);
		}

		if (!collisionHandled && s_ScriptData->EntityInstances.contains(entityTwo))
		{
			Ref<ScriptClassEntityInstance> monoEntityInstance = s_ScriptData->EntityInstances[entityTwo];
			monoEntityInstance->InvokeOnPhysicsCollision(entityOne);
		}
	}

	void ScriptEngine::OnPhysicsCollisionEnd(Events::PhysicsCollisionEnd event)
	{
		UUID entityOne = event.GetEntityOne();
		UUID entityTwo = event.GetEntityTwo();
		bool collisionHandled = false;

		if (s_ScriptData->EntityInstances.contains(entityOne))
		{
			Ref<ScriptClassEntityInstance> monoEntityInstance = s_ScriptData->EntityInstances[entityOne];
			collisionHandled = monoEntityInstance->InvokeOnPhysicsCollisionEnd(entityTwo);
		}

		if (!collisionHandled && s_ScriptData->EntityInstances.contains(entityTwo))
		{
			Ref<ScriptClassEntityInstance> monoEntityInstance = s_ScriptData->EntityInstances[entityTwo];
			monoEntityInstance->InvokeOnPhysicsCollisionEnd(entityOne);
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

			if (std::string(className) == "CustomCalls")
			{
				s_ScriptData->CustomEngineCalls = CreateRef<ScriptClass>(nameSpace, className, false);
				s_ScriptData->CustomEngineCallInstance = ScriptClassCustomCallInstance(s_ScriptData->CustomEngineCalls);
				void* iter = nullptr;
				MonoMethod* method;
				KG_WARN("{} class methods:", className);
				while (method = mono_class_get_methods(monoClass, &iter))
				{
					KG_WARN("	{}", mono_method_get_name(method));
					if (std::string(mono_method_get_name(method)) == ".ctor") { continue; }
					MonoMethodSignature* methodSignature = mono_method_get_signature(method, s_ScriptData->AppAssemblyImage, 0);
					ScriptMethod newMethod = ScriptMethod(mono_method_get_name(method), mono_signature_get_param_count(methodSignature), method);
					s_ScriptData->CustomEngineCalls->AddCustomMethod(newMethod);
				}
			}

			bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);

			if (!isEntity) { continue; }

			Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, className);
			scriptClass->SetConstructor(entityClass);
			s_ScriptData->EntityClasses[fullName] = scriptClass;

			void* iter = nullptr;
			MonoMethod* method;
			KG_WARN("{} class methods:", className);
			while (method = mono_class_get_methods(monoClass, &iter))
			{
				std::string methodName = mono_method_get_name(method);
				KG_WARN("	{}", methodName);
				if (methodName == ".ctor" || methodName == "OnUpdate" || methodName == "OnPhysicsCollision" || methodName == "OnCreate") { continue; }
				MonoMethodSignature* methodSignature = mono_method_get_signature(method, s_ScriptData->AppAssemblyImage, 0);
				ScriptMethod newMethod = ScriptMethod(mono_method_get_name(method), mono_signature_get_param_count(methodSignature), method);
				scriptClass->AddCustomMethod(newMethod);
			}


			int fieldCount = mono_class_num_fields(monoClass);
			KG_WARN("{} has {} fields:", className, fieldCount);
			void* iterator = nullptr;
			while (MonoClassField* field = mono_class_get_fields(monoClass, &iterator))
			{
				const char* fieldName = mono_field_get_name(field);
				uint32_t flags = mono_field_get_flags(field);
				if (flags & FIELD_ATTRIBUTE_PUBLIC)
				{
					MonoType* type = mono_field_get_type(field);
					ScriptFieldType fieldType = Utility::MonoTypeToScriptFieldType(type);
					KG_WARN("  {} ({})", fieldName, Utility::ScriptFieldTypeToString(fieldType));
					scriptClass->AddField(fieldName, fieldType, field);
				}
			}
		}
	}

	void ScriptEngine::RunCustomCallsFunction(const std::string& functionName, void** parameters)
	{
		s_ScriptData->CustomEngineCallInstance.InvokeCustomMethod(functionName, parameters);
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return s_ScriptData->CoreAssemblyImage;
	}

	MonoObject* ScriptEngine::GetManagedInstance(UUID uuid)
	{
		KG_ASSERT(s_ScriptData->EntityInstances.contains(uuid))
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

	std::unordered_map<std::string, ScriptMethod>& ScriptEngine::GetCustomCallMap()
	{
		return s_ScriptData->CustomEngineCalls->m_Methods;
	}

	ScriptFieldMap& ScriptEngine::GetScriptFieldMap(Entity entity)
	{
		KG_ASSERT(entity)
		UUID entityID = entity.GetUUID();
		//KG_CORE_ASSERT(s_ScriptData->EntityScriptFields.find(entity.GetUUID()) != s_ScriptData->EntityScriptFields.end())

		return s_ScriptData->EntityScriptFields[entityID];
	}

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore)
		: m_ClassNamespace(classNamespace), m_ClassName(className)
	{
		m_MonoClass = mono_class_from_name(isCore ? s_ScriptData->CoreAssemblyImage : s_ScriptData->AppAssemblyImage,
		                                   classNamespace.c_str(), className.c_str());
		
		m_OnCreateMethod = GetMethod("OnCreate", 0);
		m_OnUpdateMethod = GetMethod("OnUpdate", 1);
		m_OnCollisionMethod = GetMethod("OnPhysicsCollision", 1);
		m_OnCollisionEndMethod = GetMethod("OnPhysicsCollisionEnd", 1);
	}

	void ScriptClass::SetConstructor(MonoClass* entityClass)
	{
		m_Constructor = mono_class_get_method_from_name(entityClass, ".ctor", 1);
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
		KG_PROFILE_FUNCTION()

		MonoObject* exception = nullptr;
		MonoObject* returnValue = mono_runtime_invoke(method, instance, params, &exception);

		if (exception) { Utility::PrintMonoException(exception); return nullptr; }

		return returnValue;

	}

	bool ScriptClass::AddField(const char* fieldName, const ScriptFieldType& fieldType, MonoClassField* field)
	{
		auto [newElement, insertionSuccessful] = m_Fields.insert({fieldName, {fieldType, fieldName, field}});
		return insertionSuccessful;
	}

	ScriptClassEntityInstance::ScriptClassEntityInstance(Ref<ScriptClass> scriptClass, Entity entity)
		: m_ScriptClass(scriptClass)
	{
		m_Instance = scriptClass->Instantiate();


		// Call C# Entity Constructor
		{
			UUID entityID = entity.GetUUID();
			void* param = &entityID;
			m_ScriptClass->InvokeConstructor(m_Instance, &param);
		}
	}

	void ScriptClass::InvokeConstructor(MonoObject* instance, void** params)
	{
		if (m_Constructor) { InvokeMethod(instance, m_Constructor, params); }
	}

	void ScriptClass::InvokeOnCreate(MonoObject* instance)
	{
		if (m_OnCreateMethod) { InvokeMethod(instance, m_OnCreateMethod);}
	}

	void ScriptClass::InvokeOnUpdate(MonoObject* instance , float ts)
	{
		KG_ASSERT(instance, "Empty Script Instance!")
		if (m_OnUpdateMethod)
		{
			void* param = &ts;
			InvokeMethod(instance, m_OnUpdateMethod, &param);
		}
	}

	bool ScriptClass::InvokeOnPhysicsCollision(MonoObject* instance , UUID otherEntity)
	{
		KG_ASSERT(instance, "Empty Script Instance!")
			if (m_OnCollisionMethod)
			{
				void* param = &otherEntity;
				return *(bool*)InvokeMethod(instance, m_OnCollisionMethod, &param);
			}
		return false;
	}

	bool ScriptClass::InvokeOnPhysicsCollisionEnd(MonoObject* instance, UUID otherEntity)
	{
		KG_ASSERT(instance, "Empty Script Instance!")
			if (m_OnCollisionEndMethod)
			{
				void* param = &otherEntity;
				return *(bool*)InvokeMethod(instance, m_OnCollisionEndMethod, &param);
			}
		return false;
	}

	void ScriptClass::InvokeCustomMethod(MonoObject* instance , const std::string& methodName, void** params)
	{
		KG_ASSERT(instance, "Empty Script Instance!");
		if (m_Methods.contains(methodName))
		{
			ScriptMethod& method = m_Methods.at(methodName);
			if (method.MonoMethod)
			{
				if (method.NumParameters == 0) { params = nullptr; }
				InvokeMethod(instance, method.MonoMethod, params);
			}
		}
	}

	bool ScriptClassEntityInstance::GetFieldValueInternal(const std::string& name, void* buffer)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end()) { return false; }

		const ScriptField& field = it->second;
		mono_field_get_value(m_Instance, field.ClassField, buffer);
		return true;

	}

	bool ScriptClassEntityInstance::SetFieldValueInternal(const std::string& name, const void* value)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end()) { return false; }

		const ScriptField& field = it->second;
		mono_field_set_value(m_Instance, field.ClassField, (void*)value);
		return true;

	}

	// Calls C# OnCreate Function
	void ScriptClassEntityInstance::InvokeOnCreate()
	{
		m_ScriptClass->InvokeOnCreate(m_Instance);
	}
	// Calls C# OnUpdate Function
	void ScriptClassEntityInstance::InvokeOnUpdate(float ts)
	{
		m_ScriptClass->InvokeOnUpdate(m_Instance, ts);
	}
	// Calls C# OnUpdate Function
	bool ScriptClassEntityInstance::InvokeOnPhysicsCollision(UUID otherEntity)
	{
		return m_ScriptClass->InvokeOnPhysicsCollision(m_Instance, otherEntity);
	}

	bool ScriptClassEntityInstance::InvokeOnPhysicsCollisionEnd(UUID otherEntity)
	{
		return m_ScriptClass->InvokeOnPhysicsCollisionEnd(m_Instance, otherEntity);
	}

	ScriptClassCustomCallInstance::ScriptClassCustomCallInstance(Ref<ScriptClass> scriptClass)
		: m_ScriptClass(scriptClass)
	{
		m_Instance = scriptClass->Instantiate();
	}

	void ScriptClassCustomCallInstance::InvokeCustomMethod(const std::string& methodName, void** params)
	{
		KG_PROFILE_FUNCTION()

		KG_ASSERT(m_Instance, "Empty Script Instance!");
		m_ScriptClass->InvokeCustomMethod(m_Instance, methodName, params);
	}

}
