#include "kgpch.h"

#include "Kargono/Input/InputPolling.h"
#include "Kargono/Core/KeyCodes.h"
#include "Kargono/Script/ScriptGlue.h"
#include "Kargono/Script/ScriptEngine.h"
#include "Kargono/Physics/Physics2D.h"
#include "Kargono/Scene/Entity.h"
#include "Kargono/Scene/Scene.h"
#include "Kargono/Audio/AudioEngine.h"
#include "Kargono/Core/Application.h"
#include "Kargono/Input/InputMode.h"
#include "Kargono/Math/Math.h"
#include "Kargono/Events/ApplicationEvent.h"
#include "Kargono/Events/NetworkingEvent.h"
#include "Kargono/Network/Client.h"

#include "mono/jit/jit.h"
#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

namespace Kargono::Script
{
	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;

#define KG_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Kargono.InternalCalls::" #Name, Name)

	static bool Entity_HasComponent(UUID entityID, MonoReflectionType* componentType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		KG_ASSERT(scene)
		Entity entity = scene->GetEntityByUUID(entityID);
		KG_ASSERT(entity)

		MonoType* managedType = mono_reflection_type_get_type(componentType);
		
		KG_ASSERT(s_EntityHasComponentFuncs.contains(managedType))
		return s_EntityHasComponentFuncs.at(managedType)(entity);
	}

	static uint64_t Entity_FindEntityByName(MonoString* name)
	{
		char* cStr = mono_string_to_utf8(name);

		Scene* scene = ScriptEngine::GetSceneContext();
		KG_ASSERT(scene)
		Entity entity = scene->FindEntityByName(cStr);
		mono_free(cStr);

		if (!entity) { return 0; }

		return entity.GetUUID();
	}

	static MonoObject* GetScriptInstance(UUID entityID)
	{
		return ScriptEngine::GetManagedInstance(entityID);
	}

	static void TransformComponent_GetTranslation(UUID entityID, Math::vec3* outTranslation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);

		*outTranslation = entity.GetComponent<TransformComponent>().Translation;

	}
	
	static void TransformComponent_SetTranslation(UUID entityID, Math::vec3* translation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);

		entity.GetComponent<TransformComponent>().Translation = *translation;

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
			b2Body* body = (b2Body*)rb2d.RuntimeBody;
			body->SetTransform({ translation->x, translation->y }, body->GetAngle());
		}
		
	}

	static void Rigidbody2DComponent_ApplyLinearImpulse(UUID entityID, Math::vec2* impulse, Math::vec2* point, bool wake)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		KG_ASSERT(scene)
		Entity entity = scene->GetEntityByUUID(entityID);
		KG_ASSERT(entity)

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulse(b2Vec2(impulse->x, impulse->y), b2Vec2(point->x, point->y), wake);
	}

	static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(UUID entityID, Math::vec2* impulse, bool wake)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		KG_ASSERT(scene)
			Entity entity = scene->GetEntityByUUID(entityID);
		KG_ASSERT(entity)

			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->ApplyLinearImpulseToCenter(b2Vec2(impulse->x, impulse->y), wake);
	}

	static void Rigidbody2DComponent_GetLinearVelocity(UUID entityID, Math::vec2* outLinearVelocity)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		KG_ASSERT(scene)
			Entity entity = scene->GetEntityByUUID(entityID);
		KG_ASSERT(entity)

			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		const b2Vec2& linearVelocity = body->GetLinearVelocity();
		*outLinearVelocity = Math::vec2(linearVelocity.x, linearVelocity.y);
	}

	static void Rigidbody2DComponent_SetLinearVelocity(UUID entityID, Math::vec2* linearVelocity)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		KG_ASSERT(scene)
		Entity entity = scene->GetEntityByUUID(entityID);
		KG_ASSERT(entity)

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->SetLinearVelocity(b2Vec2(linearVelocity->x, linearVelocity->y));
	}

	static void Rigidbody2DComponent_SetType(UUID entityID, Rigidbody2DComponent::BodyType bodyType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		KG_ASSERT(scene)
			Entity entity = scene->GetEntityByUUID(entityID);
		KG_ASSERT(entity)

			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		body->SetType(Utility::Rigidbody2DTypeToBox2DBody(bodyType));
	}

	static Rigidbody2DComponent::BodyType Rigidbody2DComponent_GetType(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		KG_ASSERT(scene)
			Entity entity = scene->GetEntityByUUID(entityID);
		KG_ASSERT(entity)

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		b2Body* body = (b2Body*)rb2d.RuntimeBody;
		return Utility::Rigidbody2DTypeFromBox2DBody(body->GetType());
	}

	static MonoString* TagComponent_GetTag(UUID entityID)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		KG_ASSERT(scene)
		Entity entity = scene->GetEntityByUUID(entityID);
		KG_ASSERT(entity)

		auto& tagComponent = entity.GetComponent<TagComponent>();
		return mono_string_new(ScriptEngine::GetAppDomain(), tagComponent.Tag.c_str());
	}

	static void* GameState_GetField(MonoString* fieldName)
	{
		const std::string name = std::string(mono_string_to_utf8(fieldName));
		if (!GameState::s_GameState)
		{
			KG_ERROR("Unable to get active Game State when getting field in C# marshalling method");
			return nullptr;
		}

		Ref<WrappedVariable> field = GameState::s_GameState->GetField(name);
		if (!field)
		{
			KG_ERROR("Unable to get field from active game state in C# marshalling method");
			return nullptr;
		}

		if (field->Type() == WrappedVarType::UInteger16)
		{
			return (void*)&(field->GetWrappedValue<uint16_t>());
		}
		return nullptr;
	}

	static void GameState_SetField(MonoString* fieldName, void* value)
	{
		const std::string name = std::string(mono_string_to_utf8(fieldName));
		if (!GameState::s_GameState)
		{
			KG_ERROR("Unable to get active Game State when getting field in C# marshalling method");
			return;
		}

		Ref<WrappedVariable> field = GameState::s_GameState->GetField(name);
		if (!field)
		{
			KG_ERROR("Unable to get field from active game state in C# marshalling method");
			return;
		}

		if (field->Type() == WrappedVarType::UInteger16)
		{
			((WrappedUInteger16*)field.get())->m_Value = *(uint16_t*)value;
			return;
		}
		KG_ERROR("Unable to find type in Set Field Method");
		return;
	}

	static void Audio_PlayAudio(MonoString* audioFileLocation)
	{
		const std::string audio = std::string(mono_string_to_utf8(audioFileLocation));
		auto [handle, audioBuffer] = Assets::AssetManager::GetAudio(audio);
		if (audioBuffer)
		{
			Audio::AudioEngine::PlaySound(audioBuffer);
		}
	}

	static void Audio_PlayStereoAudio(MonoString* audioFileLocation)
	{
		const std::string audio = std::string(mono_string_to_utf8(audioFileLocation));
		auto [handle, audioBuffer] = Assets::AssetManager::GetAudio(audio);
		if (audioBuffer)
		{
			Audio::AudioEngine::PlayStereoSound(audioBuffer);
		}
	}

	static void Scene_TransitionScene(MonoString* sceneFileLocation)
	{
		const std::string sceneLocation = std::string(mono_string_to_utf8(sceneFileLocation));
		auto [handle, sceneReference] = Assets::AssetManager::GetScene(sceneLocation);
		if (sceneReference)
		{
			Scene::TransitionScene(sceneReference);
		}
	}

	static void UserInterface_LoadUserInterface(MonoString* uiFileLocation)
	{
		const std::string uiLocation = std::string(mono_string_to_utf8(uiFileLocation));
		auto [handle, uiReference] = Assets::AssetManager::GetUIObject(uiLocation);
		if (uiReference)
		{
			RuntimeUI::Runtime::LoadUIObject(uiReference, handle);
		}
	}

	static void UserInterface_MoveRight()
	{
		RuntimeUI::Runtime::MoveRight();
	}

	static void UserInterface_MoveLeft()
	{
		RuntimeUI::Runtime::MoveLeft();
	}

	static void UserInterface_MoveUp()
	{
		RuntimeUI::Runtime::MoveUp();
	}

	static void UserInterface_MoveDown()
	{
		RuntimeUI::Runtime::MoveDown();
	}

	static void UserInterface_OnPress()
	{
		RuntimeUI::Runtime::OnPress();
	}

	static void UserInterface_SetWidgetText(MonoString* windowTag, MonoString* widgetTag, MonoString* newText )
	{
		const std::string window = std::string(mono_string_to_utf8(windowTag));
		const std::string widget = std::string(mono_string_to_utf8(widgetTag));
		const std::string text = std::string(mono_string_to_utf8(newText));
		RuntimeUI::Runtime::SetWidgetText(window, widget, text);
	}

	static void UserInterface_SetSelectedWidget(MonoString* windowTag, MonoString* widgetTag)
	{
		const std::string window = std::string(mono_string_to_utf8(windowTag));
		const std::string widget = std::string(mono_string_to_utf8(widgetTag));
		RuntimeUI::Runtime::SetSelectedWidget(window, widget);
	}
	

	static void UserInterface_SetWidgetTextColor(MonoString* windowTag, MonoString* widgetTag, Math::vec4* color)
	{
		const std::string window = std::string(mono_string_to_utf8(windowTag));
		const std::string widget = std::string(mono_string_to_utf8(widgetTag));
		RuntimeUI::Runtime::SetWidgetTextColor(window, widget, *color);
	}

	static void UserInterface_SetWidgetBackgroundColor(MonoString* windowTag, MonoString* widgetTag, Math::vec4* color)
	{
		const std::string window = std::string(mono_string_to_utf8(windowTag));
		const std::string widget = std::string(mono_string_to_utf8(widgetTag));
		RuntimeUI::Runtime::SetWidgetBackgroundColor(window, widget, *color);
	}

	static void UserInterface_SetWidgetSelectable(MonoString* windowTag, MonoString* widgetTag, bool selectable)
	{
		const std::string window = std::string(mono_string_to_utf8(windowTag));
		const std::string widget = std::string(mono_string_to_utf8(widgetTag));
		RuntimeUI::Runtime::SetWidgetSelectable(window, widget, selectable);
	}

	static void UserInterface_SetDisplayWindow(MonoString* windowTag, bool display)
	{
		const std::string window = std::string(mono_string_to_utf8(windowTag));
		
		RuntimeUI::Runtime::SetDisplayWindow(window, display);
	}

	static void InputMode_LoadInputMode(MonoString* inputModeLocation)
	{
		static Ref<InputMode> s_InputRef {nullptr};
		static Assets::AssetHandle s_InputHandle {0};

		const std::string inputLocation = std::string(mono_string_to_utf8(inputModeLocation));
		auto [handle, inputReference] = Assets::AssetManager::GetInputMode(inputLocation);
		s_InputRef = inputReference;
		s_InputHandle = handle;
		if (inputReference)
		{
			Application::GetCurrentApp().SubmitToMainThread([&]()
			{
				InputMode::LoadInputMode(s_InputRef, s_InputHandle);
			});
			
		}
	}

	static void Core_CloseApplication()
	{
		Application::GetCurrentApp().SubmitToMainThread([&]()
			{
				Events::ApplicationCloseEvent event {};
				Events::EventCallbackFn eventCallback = Application::GetCurrentApp().GetWindow().GetEventCallback();
				eventCallback(event);
			});
	}

	static void Network_RequestJoinSession()
	{
		Network::Client::GetActiveClient()->SubmitToEventQueue(CreateRef<Events::RequestJoinSession>());
	}

	static void Network_RequestUserCount()
	{
		if (Network::Client::GetActiveClient())
		{
			Network::Client::GetActiveClient()->SubmitToEventQueue(CreateRef<Events::RequestUserCount>());
		}
	}

	static uint16_t Network_GetSessionSlot()
	{
		if (Network::Client::GetActiveClient())
		{
			return Network::Client::GetActiveClient()->GetSessionSlot();
		}
		// Client is unavailable so send error response
		return std::numeric_limits<uint16_t>::max();
	}

	static void Network_LeaveCurrentSession()
	{
		if (Network::Client::GetActiveClient())
		{
			Network::Client::GetActiveClient()->SubmitToEventQueue(CreateRef<Events::LeaveCurrentSession>());
		}
	}

	static void Network_EnableReadyCheck()
	{
		if (Network::Client::GetActiveClient())
		{
			Network::Client::GetActiveClient()->SubmitToEventQueue(CreateRef<Events::EnableReadyCheck>());
		}
	}

	static void Network_SessionReadyCheck()
	{
		if (Network::Client::GetActiveClient())
		{
			Network::Client::GetActiveClient()->SubmitToEventQueue(CreateRef<Events::SessionReadyCheck>());
		}
	}

	static void Network_SendAllEntityLocation(uint64_t id, Math::vec3* translation)
	{
		if (Network::Client::GetActiveClient())
		{
			Network::Client::GetActiveClient()->SubmitToEventQueue(CreateRef<Events::SendAllEntityLocation>(id, *translation));
		}
	}

	static void Network_SendAllEntityPhysics(uint64_t id, Math::vec3* translation, Math::vec2* linearVelocity)
	{
		if (Network::Client::GetActiveClient())
		{
			Network::Client::GetActiveClient()->SubmitToEventQueue(CreateRef<Events::SendAllEntityPhysics>(id, *translation, *linearVelocity));
		}
	}

	static void Network_SignalAll(uint16_t signal)
	{
		if (Network::Client::GetActiveClient())
		{
			Network::Client::GetActiveClient()->SubmitToEventQueue(CreateRef<Events::SignalAll>(signal));
		}
	}

	static void AudioComponent_PlayAudio(UUID entityID)
	{

		Scene* scene = ScriptEngine::GetSceneContext();
		KG_ASSERT(scene);
			Entity entity = scene->GetEntityByUUID(entityID);
		KG_ASSERT(entity);

		if (!entity.HasComponent<AudioComponent>()) { return; }
		auto& audioComponent = entity.GetComponent<AudioComponent>();
		Audio::AudioEngine::PlaySound(audioComponent.Audio);
	}

	static void AudioComponent_PlayAudioByName(UUID entityID, MonoString* audioTag)
	{
		std::string aTag = std::string(mono_string_to_utf8(audioTag));
		Scene* scene = ScriptEngine::GetSceneContext();
		KG_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		KG_ASSERT(entity);

		if (entity.HasComponent<MultiAudioComponent>())
		{
			if (entity.GetComponent<MultiAudioComponent>().AudioComponents.contains(aTag))
			{
				Audio::AudioEngine::PlaySound(entity.GetComponent<MultiAudioComponent>().AudioComponents.at(aTag).Audio);
				return;
			}
		}

		auto& audioComponent = entity.GetComponent<AudioComponent>();
		if (audioComponent.Name == aTag)
		{
			Audio::AudioEngine::PlaySound(audioComponent.Audio);
		}
		
	}

	static bool Input_IsKeyDown(KeyCode keycode)
	{
		return InputPolling::IsKeyPressed(keycode);
	}

	static bool InputMode_IsKeySlotDown(uint16_t keySlot)
	{
		return InputMode::IsKeyboardSlotPressed(keySlot);
	}

	

	template<typename ... Component>
	static void RegisterComponent()
	{
		([]() 
			{
		std::string_view typeName =  typeid(Component).name();
		size_t pos = typeName.find_last_of(':');
		std::string_view structName = typeName.substr(pos + 1);
		std::string managedTypename = fmt::format("Kargono.{}", structName);

		MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), ScriptEngine::GetCoreAssemblyImage());
			if (!managedType)
			{
				//KG_ERROR("Could not find component type {}", managedTypename);
				return;
			}
		s_EntityHasComponentFuncs[managedType] = [](Entity entity) { return entity.HasComponent<Component>(); };
			}(), ...);
	}

	template<typename ... Component>
	static void RegisterComponent(ComponentGroup<Component ...>)
	{
		RegisterComponent<Component ...>();
	}

	void ScriptGlue::RegisterComponents()
	{
		s_EntityHasComponentFuncs.clear();
		RegisterComponent (AllComponents{});
	}

	void ScriptGlue::RegisterFunctions()
    {
		KG_ADD_INTERNAL_CALL(Entity_HasComponent);
		KG_ADD_INTERNAL_CALL(Entity_FindEntityByName);
		KG_ADD_INTERNAL_CALL(GetScriptInstance);

		KG_ADD_INTERNAL_CALL(GameState_GetField);
		KG_ADD_INTERNAL_CALL(GameState_SetField);

		KG_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
		KG_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);

		KG_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulse);
		KG_ADD_INTERNAL_CALL(Rigidbody2DComponent_ApplyLinearImpulseToCenter);
		KG_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetLinearVelocity);
		KG_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetLinearVelocity);
		KG_ADD_INTERNAL_CALL(Rigidbody2DComponent_SetType);
		KG_ADD_INTERNAL_CALL(Rigidbody2DComponent_GetType);

		KG_ADD_INTERNAL_CALL(TagComponent_GetTag);

		KG_ADD_INTERNAL_CALL(Audio_PlayAudio);
		KG_ADD_INTERNAL_CALL(Audio_PlayStereoAudio);
		KG_ADD_INTERNAL_CALL(Scene_TransitionScene);
		KG_ADD_INTERNAL_CALL(AudioComponent_PlayAudio);
		KG_ADD_INTERNAL_CALL(AudioComponent_PlayAudioByName);

		KG_ADD_INTERNAL_CALL(Input_IsKeyDown);
		KG_ADD_INTERNAL_CALL(InputMode_IsKeySlotDown);
		KG_ADD_INTERNAL_CALL(UserInterface_LoadUserInterface);
		KG_ADD_INTERNAL_CALL(UserInterface_MoveRight);
		KG_ADD_INTERNAL_CALL(UserInterface_MoveLeft);
		KG_ADD_INTERNAL_CALL(UserInterface_MoveUp);
		KG_ADD_INTERNAL_CALL(UserInterface_MoveDown);
		KG_ADD_INTERNAL_CALL(UserInterface_OnPress);
		KG_ADD_INTERNAL_CALL(UserInterface_SetWidgetText);
		KG_ADD_INTERNAL_CALL(UserInterface_SetWidgetTextColor);
		KG_ADD_INTERNAL_CALL(UserInterface_SetWidgetBackgroundColor);
		KG_ADD_INTERNAL_CALL(UserInterface_SetWidgetSelectable);
		KG_ADD_INTERNAL_CALL(UserInterface_SetSelectedWidget);

		KG_ADD_INTERNAL_CALL(UserInterface_SetDisplayWindow);

		KG_ADD_INTERNAL_CALL(InputMode_LoadInputMode);
		KG_ADD_INTERNAL_CALL(Core_CloseApplication);
		KG_ADD_INTERNAL_CALL(Network_RequestJoinSession);
		KG_ADD_INTERNAL_CALL(Network_RequestUserCount);
		KG_ADD_INTERNAL_CALL(Network_LeaveCurrentSession);
		KG_ADD_INTERNAL_CALL(Network_GetSessionSlot);
		KG_ADD_INTERNAL_CALL(Network_EnableReadyCheck);
		KG_ADD_INTERNAL_CALL(Network_SessionReadyCheck);
		KG_ADD_INTERNAL_CALL(Network_SendAllEntityLocation);
		KG_ADD_INTERNAL_CALL(Network_SendAllEntityPhysics);
		KG_ADD_INTERNAL_CALL(Network_SignalAll);
		
		

		
		
    }
}
