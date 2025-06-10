#include "kgpch.h"

#include "Windows/MainWindow/SceneEditorPanel.h"

#include "EditorApp.h"

#include "Kargono/Scenes/Scene.h"
#include "Modules/Events/SceneEvent.h"

#include "Modules/EditorUI/ExternalAPI/ImGuiBackendAPI.h"

static Kargono::EditorApp* s_EditorApp { nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };

namespace Kargono::Panels
{
	void SceneEditorPanel::InitializeSceneHierarchy()
	{
		m_MainSceneHeader.m_Label = "No Scene Name";
		m_MainSceneHeader.m_EditColorActive = false;
		m_MainSceneHeader.AddToSelectionList("Create Scene", []()
		{
			s_MainWindow->NewSceneDialog();
		});
		m_MainSceneHeader.AddToSelectionList("Open Scene", []()
		{
			s_MainWindow->OpenSceneDialog();
		});

		m_MainSceneHeader.AddToSelectionList("Duplicate Scene", []()
		{
			s_MainWindow->DuplicateEditorScene();
		});

		m_SceneHierarchyTree.m_Label = "Scene Hierarchy";
		m_SceneHierarchyTree.m_OnRefresh = [&]()
		{
			if (Scenes::SceneService::GetActiveScene())
			{
				m_SceneHierarchyTree.ClearTree();

				EditorUI::TreePath sceneEntryPath;
				sceneEntryPath.AddNode(0);
				EditorUI::TreeEntry sceneEntry{};
				sceneEntry.m_Label = Assets::AssetService::GetSceneRegistry().at
				(
					Scenes::SceneService::GetActiveSceneHandle()
				).Data.FileLocation.stem().string();
				sceneEntry.m_IconHandle = EditorUI::EditorUIService::s_IconScene;
				sceneEntry.m_Handle = Assets::EmptyHandle;
				sceneEntry.m_OnLeftClick = [&](EditorUI::TreeEntry& entry)
				{
					UNREFERENCED_PARAMETER(entry);
					// Display scene options in properties panel
					m_CurrentDisplayed = ScenePropertiesDisplay::Scene;

					// Display properties panel
					s_MainWindow->m_ShowProperties = true;
					EditorUI::EditorUIService::BringWindowToFront(s_MainWindow->m_PropertiesPanel->m_PanelName);
					s_MainWindow->m_PropertiesPanel->m_ActiveParent = m_PanelName;
				};

				sceneEntry.m_OnRightClick = [&](EditorUI::TreeEntry& entry)
				{
					m_SelectTooltip.ClearEntries();

					// Create add component entry
					EditorUI::TooltipEntry addEntityEntry{ "Add Entity", [&](EditorUI::TooltipEntry& tooltipEntry)
					{
						UNREFERENCED_PARAMETER(entry);
						EngineService::GetActiveEngine().GetThread().SubmitFunction([]()
						{
							Scenes::SceneService::GetActiveScene()->CreateEntity("Empty Entity");
						});
					} };
					// Add the new entry
					m_SelectTooltip.AddTooltipEntry(addEntityEntry);

					// Active the tooltip
					m_SelectTooltip.m_TooltipActive = true;
				};

				// Add all entities from the scene into the tree UI
				Scenes::SceneService::GetActiveScene()->m_EntityRegistry.m_EnTTRegistry.each([&](entt::entity enttID)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(enttID);
					CreateSceneEntityInTree(entity, sceneEntry);
				});

				m_SceneHierarchyTree.InsertEntry(sceneEntry);
				m_SceneHierarchyTree.ExpandNodePath(sceneEntryPath);
			}
		};

		m_AddComponent.m_Label = "Add Component";
		m_AddComponent.m_Flags = EditorUI::SelectOption_PopupOnly;
		m_AddComponent.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity(m_AddComponentEntity));
			if (!entity)
			{
				KG_WARN("Attempt to add component to empty entity");
				return;
			}
			spec.ClearOptions();
			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				spec.AddToOptions("Engine Component", "Camera", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<ECS::ParticleEmitterComponent>())
			{
				spec.AddToOptions("Engine Component", "Particle Emitter", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<ECS::ShapeComponent>())
			{
				spec.AddToOptions("Engine Component", "Shape", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<ECS::Rigidbody2DComponent>())
			{
				spec.AddToOptions("Engine Component", "Rigidbody 2D", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				spec.AddToOptions("Engine Component", "Box Collider 2D", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				spec.AddToOptions("Engine Component", "Circle Collider 2D", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<ECS::OnCreateComponent>())
			{
				spec.AddToOptions("Engine Component", "On Create", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<ECS::OnUpdateComponent>())
			{
				spec.AddToOptions("Engine Component", "On Update", Assets::EmptyHandle);
			}

			if (!entity.HasComponent<ECS::AIStateComponent>())
			{
				spec.AddToOptions("Engine Component", "AI State", Assets::EmptyHandle);
			}

			for (auto& [handle, asset] : Assets::AssetService::GetProjectComponentRegistry())
			{
				Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(handle);
				KG_ASSERT(projectComponentRef);
				if (!entity.HasProjectComponentData(handle) && projectComponentRef->m_BufferSize != 0)
				{
					spec.AddToOptions("Project Component", asset.Data.GetSpecificMetaData<Assets::ProjectComponentMetaData>()->Name, handle);
				}
			}

			spec.m_CurrentOption = { "None", Assets::EmptyHandle };
		};

		m_AddComponent.m_ConfirmAction = [&](const EditorUI::OptionEntry& option)
		{
			// Get active entity and ensure it is valid
			ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity(m_AddComponentEntity));
			if (!entity)
			{
				KG_WARN("Attempt to add component to empty entity");
				return;
			}

			// Get active tree entry and ensure it is valid
		 	EditorUI::TreeEntry* currentEntry = m_SceneHierarchyTree.SearchDepth((uint64_t)entity, 1);
			if (!currentEntry)
			{
				KG_WARN("Could not locate entity inside m_SceneHierarchyTree");
				return;
			}
			EditorUI::TreeEntry componentEntry {};
			componentEntry.m_Handle = (uint64_t)entity;
			
			// Check for a project component
			if (option.m_Handle != Assets::EmptyHandle)
			{
				// Add component to entity & update tree
				Ref<ECS::ProjectComponent> component = Assets::AssetService::GetProjectComponent(option.m_Handle);
				KG_ASSERT(component);
				entity.AddProjectComponentData(option.m_Handle);
				componentEntry.m_Label = component->m_Name;
				componentEntry.m_ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::ProjectComponent, option.m_Handle);
				componentEntry.m_IconHandle = EditorUI::EditorUIService::s_IconEntity;
				componentEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
					s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::ProjectComponent);
					SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)entry.m_ProvidedData.get();
					s_MainWindow->m_SceneEditorPanel->SetDisplayedProjectComponent(entryData.m_ProjectComponentHandle);
				};
				currentEntry->m_SubEntries.push_back(componentEntry);
				return;
			}

			// Exit if no option is selected
			if (option.m_Label == "None")
			{
				return;
			}


			// Check for engine components 
			if (option.m_Label == "Camera")
			{
				entity.AddComponent<ECS::CameraComponent>();
				componentEntry.m_Label = "Camera";
				componentEntry.m_ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::Camera, Assets::EmptyHandle);
				componentEntry.m_IconHandle = EditorUI::EditorUIService::s_IconCamera;
				componentEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
					s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::Camera);
				};
				currentEntry->m_SubEntries.push_back(componentEntry);
				return;
			}
			if (option.m_Label == "Particle Emitter")
			{
				entity.AddComponent<ECS::ParticleEmitterComponent>();
				componentEntry.m_Label = "Particle Emitter";
				componentEntry.m_ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::ParticleEmitter, Assets::EmptyHandle);
				componentEntry.m_IconHandle = EditorUI::EditorUIService::s_IconParticles;
				componentEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
					s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::ParticleEmitter);
				};
				currentEntry->m_SubEntries.push_back(componentEntry);
				return;
			}
			if (option.m_Label == "Shape")
			{
				entity.AddComponent<ECS::ShapeComponent>();
				componentEntry.m_Label = "Shape";
				componentEntry.m_ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::Shape, Assets::EmptyHandle);
				componentEntry.m_IconHandle = EditorUI::EditorUIService::s_IconEntity;
				componentEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
					s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::Shape);
				};
				currentEntry->m_SubEntries.push_back(componentEntry);
				return;
			}
			if (option.m_Label == "Rigidbody 2D")
			{
				entity.AddComponent<ECS::Rigidbody2DComponent>();
				componentEntry.m_Label = "Rigidbody 2D";
				componentEntry.m_ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::Rigidbody2D, Assets::EmptyHandle);
				componentEntry.m_IconHandle = EditorUI::EditorUIService::s_IconRigidBody;
				componentEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
					s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::Rigidbody2D);
				};
				currentEntry->m_SubEntries.push_back(componentEntry);
				return;
			}
			if (option.m_Label == "Box Collider 2D")
			{
				entity.AddComponent<ECS::BoxCollider2DComponent>();
				componentEntry.m_Label = "Box Collider 2D";
				componentEntry.m_ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::BoxCollider2D, Assets::EmptyHandle);
				componentEntry.m_IconHandle = EditorUI::EditorUIService::s_IconBoxCollider;
				componentEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
					s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::BoxCollider2D);
				};
				currentEntry->m_SubEntries.push_back(componentEntry);
				return;
			}
			if (option.m_Label == "Circle Collider 2D")
			{
				entity.AddComponent<ECS::CircleCollider2DComponent>();
				componentEntry.m_Label = "Circle Collider 2D";
				componentEntry.m_ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::CircleCollider2D, Assets::EmptyHandle);
				componentEntry.m_IconHandle = EditorUI::EditorUIService::s_IconCircleCollider;
				componentEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
					s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::CircleCollider2D);
				};
				currentEntry->m_SubEntries.push_back(componentEntry);
				return;
			}

			if (option.m_Label == "On Update")
			{
				entity.AddComponent<ECS::OnUpdateComponent>();
				componentEntry.m_Label = "On Update";
				componentEntry.m_ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::OnUpdate, Assets::EmptyHandle);
				componentEntry.m_IconHandle = EditorUI::EditorUIService::s_IconFunction;
				componentEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
					s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::OnUpdate);
				};
				currentEntry->m_SubEntries.push_back(componentEntry);
				return;
			}

			if (option.m_Label == "AI State")
			{
				entity.AddComponent<ECS::AIStateComponent>();
				componentEntry.m_Label = "AI State";
				componentEntry.m_ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::AIState, Assets::EmptyHandle);
				componentEntry.m_IconHandle = EditorUI::EditorUIService::s_IconAI;
				componentEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
					s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::AIState);
				};
				currentEntry->m_SubEntries.push_back(componentEntry);
				return;
			}

			if (option.m_Label == "On Create")
			{
				entity.AddComponent<ECS::OnCreateComponent>();
				componentEntry.m_Label = "On Create";
				componentEntry.m_ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::OnCreate, Assets::EmptyHandle);
				componentEntry.m_IconHandle = EditorUI::EditorUIService::s_IconFunction;
				componentEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
					s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::OnCreate);
				};
				currentEntry->m_SubEntries.push_back(componentEntry);
				return;
			}

			KG_ERROR("Invalid option selected to add as component!");

		};
	}

	void SceneEditorPanel::InitializeSceneOptions()
	{
		// Set up widget to modify the the scene's background color
		m_BackgroundColorSpec.m_Label = "Background Color";
		m_BackgroundColorSpec.m_Flags |= EditorUI::EditVec4_RGBA;
		m_BackgroundColorSpec.m_Bounds = { 0.0f, 1.0f };
		m_BackgroundColorSpec.m_ConfirmAction = [&](EditorUI::EditVec4Widget& spec) 
		{
			Ref<Scenes::Scene> editorScene{ s_MainWindow->m_EditorScene };
			KG_ASSERT(editorScene);

			// Update the scene's background color
			editorScene->m_BackgroundColor = spec.m_CurrentVec4;
		};

		// Set up widget to modify the the scene's background color
		m_Gravity2DSpec.m_Label = "2D Gravity";
		m_Gravity2DSpec.m_Bounds = { -10'000.0f, 10'000.0f };
		m_Gravity2DSpec.m_ConfirmAction = [&](EditorUI::EditVec2Widget& spec)
		{
			Ref<Scenes::Scene> editorScene{ s_MainWindow->m_EditorScene };
			KG_ASSERT(editorScene);

			// Update the scene's gravity value
			editorScene->GetPhysicsSpecification().Gravity = spec.m_CurrentVec2;

			// If the simulation is running, modify the gravity value inside the simulation
			if (Physics::Physics2DService::IsContextActive())
			{
				Scenes::SceneService::GetActiveScene()->GetPhysicsSpecification().Gravity = s_MainWindow->m_EditorScene->GetPhysicsSpecification().Gravity;
				Physics::Physics2DService::GetActiveContext().SetActiveGravity(s_MainWindow->m_EditorScene->GetPhysicsSpecification().Gravity);
			}
		};
	}

	void SceneEditorPanel::InitializeTagComponent()
	{
		m_TagHeader.m_Label = "Tag";
		m_TagHeader.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_TagHeader.m_Expanded = true;

		m_TagEdit.m_Label = "Tag Label";
		m_TagEdit.m_Flags |= EditorUI::EditText_Indented;
		m_TagEdit.m_ConfirmAction = [&](EditorUI::EditTextSpec& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (entity && entity.HasComponent<ECS::TagComponent>())
			{
				ECS::TagComponent& component = entity.GetComponent<ECS::TagComponent>();
				component.Tag = m_TagEdit.m_CurrentOption;

				m_SceneHierarchyTree.EditDepth([](EditorUI::TreeEntry& entry) 
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID((entt::entity)(int32_t)entry.m_Handle);
					KG_ASSERT(entity);
					if (entity.GetUUID() == Scenes::SceneService::GetActiveScene()->GetSelectedEntity()->GetUUID())
					{
						entry.m_Label = entity.GetComponent<ECS::TagComponent>().Tag;
					}
				}, 1);
			}
		};

		m_TagGroupEdit.m_Label = "Tag Group";
		m_TagGroupEdit.m_Flags |= EditorUI::EditText_Indented;
		m_TagGroupEdit.m_ConfirmAction = [&](EditorUI::EditTextSpec& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (entity && entity.HasComponent<ECS::TagComponent>())
			{
				ECS::TagComponent& component = entity.GetComponent<ECS::TagComponent>();
				component.Group = m_TagGroupEdit.m_CurrentOption;
			}
		};
	}

	void SceneEditorPanel::InitializeTransformComponent()
	{
		m_TransformHeader.m_Label = "Transform";
		m_TransformHeader.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_TransformHeader.m_Expanded = true;

		m_TransformEditTranslation.m_Label = "Translation";
		m_TransformEditTranslation.m_Flags = EditorUI::EditVec3_Indented;
		m_TransformEditTranslation.m_ConfirmAction = [&](EditorUI::EditVec3Widget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::TransformComponent>())
			{
				KG_ERROR("Attempt to edit entity transform component when none exists!");
				return;
			}
			ECS::TransformComponent& transformComp = entity.GetComponent<ECS::TransformComponent>();
			transformComp.Translation = m_TransformEditTranslation.m_CurrentVec3;
		};
		
		m_TransformEditScale.m_Label = "Scale";
		m_TransformEditScale.m_Flags = EditorUI::EditVec3_Indented;
		m_TransformEditScale.m_ConfirmAction = [&](EditorUI::EditVec3Widget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::TransformComponent>())
			{
				KG_ERROR("Attempt to edit entity transform component when none exists!");
				return;
			}
			ECS::TransformComponent& transformComp = entity.GetComponent<ECS::TransformComponent>();
			transformComp.Scale = m_TransformEditScale.m_CurrentVec3;
		};
		m_TransformEditRotation.m_Label = "Rotation";
		m_TransformEditRotation.m_Flags = EditorUI::EditVec3_Indented;
		m_TransformEditRotation.m_ConfirmAction = [&](EditorUI::EditVec3Widget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::TransformComponent>())
			{
				KG_ERROR("Attempt to edit entity transform component when none exists!");
				return;
			}
			ECS::TransformComponent& transformComp = entity.GetComponent<ECS::TransformComponent>();
			transformComp.Rotation = m_TransformEditRotation.m_CurrentVec3;
		};
		
	}

	void SceneEditorPanel::InitializeRigidbody2DComponent()
	{
		m_Rigidbody2DHeader.m_Label = "Rigid Body 2D";
		m_Rigidbody2DHeader.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_Rigidbody2DHeader.m_Expanded = true;
		m_Rigidbody2DHeader.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			EngineService::GetActiveEngine().GetThread().SubmitFunction([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<ECS::Rigidbody2DComponent>())
				{
					// Search for indicated entity
					EditorUI::TreeEntry* entityEntry{ m_SceneHierarchyTree.SearchDepth((uint64_t)entity, 1) };
					KG_ASSERT(entityEntry);

					// Search for the rigid body component
					EditorUI::TreePath newPath {};
					for (EditorUI::TreeEntry& subEntry : entityEntry->m_SubEntries)
					{
						SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.m_ProvidedData.get();
						if (entryData.m_ComponentType == ECS::ComponentType::Rigidbody2D)
						{
							newPath = m_SceneHierarchyTree.GetPathFromEntryReference(&subEntry);
							break;
						}
					}
					KG_ASSERT(newPath);
					
					// Remove the rigid body component
					m_SceneHierarchyTree.RemoveEntry(newPath);
					entity.RemoveComponent<ECS::Rigidbody2DComponent>();
				}
			});
		});

		m_Rigidbody2DType.m_Label = "Interaction Type";
		m_Rigidbody2DType.m_FirstOptionLabel = "Static";
		m_Rigidbody2DType.m_SecondOptionLabel = "Dynamic";
		m_Rigidbody2DType.m_Flags |= EditorUI::RadioSelect_Indented;
		m_Rigidbody2DType.m_SelectAction = [&]()
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::Rigidbody2DComponent>())
			{
				KG_ERROR("Attempt to edit entity rigid body 2D component when none exists!");
				return;
			}
			ECS::Rigidbody2DComponent& component = entity.GetComponent<ECS::Rigidbody2DComponent>();

			if (m_Rigidbody2DType.m_SelectedOption == 0)
			{
				component.Type = ECS::Rigidbody2DComponent::BodyType::Static;
				return;
			}
			if (m_Rigidbody2DType.m_SelectedOption == 1)
			{
				component.Type = ECS::Rigidbody2DComponent::BodyType::Dynamic;
				return;
			}

			KG_ERROR("Invalid SelectedOption");
		};


		// Set Shape Add Fixed Rotation Option
		m_RigidBody2DFixedRotation.m_Label = "Use Fixed Rotation";
		m_RigidBody2DFixedRotation.m_Flags |= EditorUI::Checkbox_Indented;
		m_RigidBody2DFixedRotation.m_ConfirmAction = [&](EditorUI::CheckboxWidget& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::Rigidbody2DComponent>())
			{
				KG_ERROR("Attempt to edit entity rigid body 2D component when none exists!");
				return;
			}
			ECS::Rigidbody2DComponent& component = entity.GetComponent<ECS::Rigidbody2DComponent>();
			component.FixedRotation = spec.m_CurrentBoolean;
		};

		m_SelectRigidBody2DCollisionStartScript.m_Label = "On Collision Start";
		m_SelectRigidBody2DCollisionStartScript.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectRigidBody2DCollisionStartScript.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectRigidBody2DCollisionStartScript.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();
			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				KG_ASSERT(script);

				if (script->m_FuncType != WrappedFuncType::Bool_EntityEntity)
				{
					continue;
				}
				spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}
		};

		m_SelectRigidBody2DCollisionStartScript.m_ConfirmAction = [](const EditorUI::OptionEntry& entry)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::Rigidbody2DComponent>())
			{
				KG_ERROR("Attempt to edit entity CollisionStart component when none exists!");
				return;
			}
			ECS::Rigidbody2DComponent& component = entity.GetComponent<ECS::Rigidbody2DComponent>();

			// Check for empty entry
			if (entry.m_Handle == Assets::EmptyHandle)
			{
				component.OnCollisionStartScriptHandle = Assets::EmptyHandle;
				component.OnCollisionStartScript = nullptr;
			}
			// Check for a valid entry, and Update if applicable
			component.OnCollisionStartScriptHandle = entry.m_Handle;
			component.OnCollisionStartScript = Assets::AssetService::GetScript(entry.m_Handle);
		};

		m_SelectRigidBody2DCollisionStartScript.m_OnEdit = [&](EditorUI::SelectOptionWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			// Initialize tooltip with options
			m_SelectTooltip.ClearEntries();
			EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
			{
				UNREFERENCED_PARAMETER(entry);
				m_SelectRigidBody2DCollisionStartScript.m_OpenPopup = true;
			} };
			m_SelectTooltip.AddTooltipEntry(openScriptOptions);

			EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
			{
				UNREFERENCED_PARAMETER(entry);
				// Open create script dialog in script editor
				s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Bool_EntityEntity, [&](Assets::AssetHandle scriptHandle)
				{
						// Ensure handle provides a script in the registry
						if (!Assets::AssetService::HasScript(scriptHandle))
						{
							KG_WARN("Could not find script");
							return;
						}

						// Ensure function type matches definition
						Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
						if (script->m_FuncType != WrappedFuncType::Bool_EntityEntity)
						{
							KG_WARN("Incorrect function type returned when linking script to usage point");
							return;
						}

						ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
						if (!entity.HasComponent<ECS::Rigidbody2DComponent>())
						{
							KG_ERROR("Attempt to edit entity CollisionStart component when none exists!");
							return;
						}
						ECS::Rigidbody2DComponent& component = entity.GetComponent<ECS::Rigidbody2DComponent>();

						// Check for a valid entry, and Update if applicable
						component.OnCollisionStartScriptHandle = scriptHandle;
						component.OnCollisionStartScript = script;
						m_SelectRigidBody2DCollisionStartScript.m_CurrentOption = { script->m_ScriptName.c_str(), scriptHandle };
					}, {"activeEntity", "collidedEntity"});

				} };
			m_SelectTooltip.AddTooltipEntry(createScriptOptions);

			// Open tooltip
			m_SelectTooltip.m_TooltipActive = true;

		};

		m_SelectRigidBody2DCollisionEndScript.m_Label = "On Collision End";
		m_SelectRigidBody2DCollisionEndScript.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectRigidBody2DCollisionEndScript.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectRigidBody2DCollisionEndScript.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();
			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				KG_ASSERT(script);

				if (script->m_FuncType != WrappedFuncType::Bool_EntityEntity)
				{
					continue;
				}
				spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}
		};

		m_SelectRigidBody2DCollisionEndScript.m_ConfirmAction = [](const EditorUI::OptionEntry& entry)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::Rigidbody2DComponent>())
			{
				KG_ERROR("Attempt to edit entity CollisionEnd component when none exists!");
				return;
			}
			ECS::Rigidbody2DComponent& component = entity.GetComponent<ECS::Rigidbody2DComponent>();

			// Check for empty entry
			if (entry.m_Handle == Assets::EmptyHandle)
			{
				component.OnCollisionEndScriptHandle = Assets::EmptyHandle;
				component.OnCollisionEndScript = nullptr;
			}
			// Check for a valid entry, and Update if applicable
			component.OnCollisionEndScriptHandle = entry.m_Handle;
			component.OnCollisionEndScript = Assets::AssetService::GetScript(entry.m_Handle);
		};

		m_SelectRigidBody2DCollisionEndScript.m_OnEdit = [&](EditorUI::SelectOptionWidget& spec)
			{
				UNREFERENCED_PARAMETER(spec);
				// Initialize tooltip with options
				m_SelectTooltip.ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					UNREFERENCED_PARAMETER(entry);
					m_SelectRigidBody2DCollisionEndScript.m_OpenPopup = true;
				} };
				m_SelectTooltip.AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
					UNREFERENCED_PARAMETER(entry);
					// Open create script dialog in script editor
					s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Bool_EntityEntity, [&](Assets::AssetHandle scriptHandle)
					{
						// Ensure handle provides a script in the registry
						if (!Assets::AssetService::HasScript(scriptHandle))
						{
							KG_WARN("Could not find script");
							return;
						}

						// Ensure function type matches definition
						Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
						if (script->m_FuncType != WrappedFuncType::Bool_EntityEntity)
						{
							KG_WARN("Incorrect function type returned when linking script to usage point");
							return;
						}

						ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
						if (!entity.HasComponent<ECS::Rigidbody2DComponent>())
						{
							KG_ERROR("Attempt to edit entity CollisionEnd component when none exists!");
							return;
						}
						ECS::Rigidbody2DComponent& component = entity.GetComponent<ECS::Rigidbody2DComponent>();

						// Check for a valid entry, and Update if applicable
						component.OnCollisionEndScriptHandle = scriptHandle;
						component.OnCollisionEndScript = script;
						m_SelectRigidBody2DCollisionEndScript.m_CurrentOption = { script->m_ScriptName.c_str(), scriptHandle };
					}, {"activeEntity", "collidedEntity"});

			}		};
				m_SelectTooltip.AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_SelectTooltip.m_TooltipActive = true;

			};

	}

	void SceneEditorPanel::InitializeBoxCollider2DComponent()
	{
		m_BoxCollider2DHeader.m_Label = "Box Collider 2D";
		m_BoxCollider2DHeader.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_BoxCollider2DHeader.m_Expanded = true;
		m_BoxCollider2DHeader.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			EngineService::GetActiveEngine().GetThread().SubmitFunction([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<ECS::BoxCollider2DComponent>())
				{
					// Search for indicated entity
					EditorUI::TreeEntry* entityEntry{ m_SceneHierarchyTree.SearchDepth((uint64_t)entity, 1) };
					KG_ASSERT(entityEntry);

					// Find box collider component in tree
					EditorUI::TreePath newPath {};
					for (EditorUI::TreeEntry& subEntry : entityEntry->m_SubEntries)
					{
						SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.m_ProvidedData.get();
						if (entryData.m_ComponentType == ECS::ComponentType::BoxCollider2D)
						{
							newPath = m_SceneHierarchyTree.GetPathFromEntryReference(&subEntry);
							break;
						}
					}
					KG_ASSERT(newPath);

					// Remove box collider component
					m_SceneHierarchyTree.RemoveEntry(newPath);
					entity.RemoveComponent<ECS::BoxCollider2DComponent>();
				}
			});
		});

		m_BoxColliderOffset.m_Label = "Offset";
		m_BoxColliderOffset.m_Flags |= EditorUI::EditVec2_Indented;
		m_BoxColliderOffset.m_ConfirmAction = [&](EditorUI::EditVec2Widget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			ECS::BoxCollider2DComponent& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
			component.Offset = m_BoxColliderOffset.m_CurrentVec2;
		};

		m_BoxColliderSize.m_Label = "Size";
		m_BoxColliderSize.m_Flags |= EditorUI::EditVec2_Indented;
		m_BoxColliderSize.m_ConfirmAction = [&](EditorUI::EditVec2Widget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			ECS::BoxCollider2DComponent& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
			component.Size = m_BoxColliderSize.m_CurrentVec2;
		};

		m_BoxColliderDensity.m_Label = "Density";
		m_BoxColliderDensity.m_Flags |= EditorUI::EditFloat_Indented;
		m_BoxColliderDensity.m_ConfirmAction = [&](EditorUI::EditFloatWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			ECS::BoxCollider2DComponent& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
			component.Density = m_BoxColliderDensity.m_CurrentFloat;
		};

		m_BoxColliderFriction.m_Label = "Friction";
		m_BoxColliderFriction.m_Flags |= EditorUI::EditFloat_Indented;
		m_BoxColliderFriction.m_ConfirmAction = [&](EditorUI::EditFloatWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			ECS::BoxCollider2DComponent& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
			component.Friction = m_BoxColliderFriction.m_CurrentFloat;
		};

		m_BoxColliderRestitution.m_Label = "Restitution";
		m_BoxColliderRestitution.m_Flags |= EditorUI::EditFloat_Indented;
		m_BoxColliderRestitution.m_ConfirmAction = [&](EditorUI::EditFloatWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			ECS::BoxCollider2DComponent& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
			component.Restitution = m_BoxColliderRestitution.m_CurrentFloat;
		};

		m_BoxColliderRestitutionThreshold.m_Label = "Restitution Threshold";
		m_BoxColliderRestitutionThreshold.m_Flags |= EditorUI::EditFloat_Indented;
		m_BoxColliderRestitutionThreshold.m_ConfirmAction = [&](EditorUI::EditFloatWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			ECS::BoxCollider2DComponent& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
			component.RestitutionThreshold = m_BoxColliderRestitutionThreshold.m_CurrentFloat;
		};

		// Set whether box collider is treated as a sensor
		m_BoxColliderIsSensor.m_Label = "Is Sensor";
		m_BoxColliderIsSensor.m_Flags |= EditorUI::Checkbox_Indented;
		m_BoxColliderIsSensor.m_ConfirmAction = [&](EditorUI::CheckboxWidget& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			ECS::BoxCollider2DComponent& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
			component.IsSensor = spec.m_CurrentBoolean;
		};
	}
	void SceneEditorPanel::InitializeCircleCollider2DComponent()
	{
		m_CircleCollider2DHeader.m_Label = "Circle Collider 2D";
		m_CircleCollider2DHeader.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_CircleCollider2DHeader.m_Expanded = true;
		m_CircleCollider2DHeader.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			EngineService::GetActiveEngine().GetThread().SubmitFunction([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<ECS::CircleCollider2DComponent>())
				{
					// Search for indicated entity
					EditorUI::TreeEntry* entityEntry{ m_SceneHierarchyTree.SearchDepth((uint64_t)entity, 1) };
					KG_ASSERT(entityEntry);

					// Find circle collider component in tree
					EditorUI::TreePath newPath {};
					for (EditorUI::TreeEntry& subEntry : entityEntry->m_SubEntries)
					{
						SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.m_ProvidedData.get();
						if (entryData.m_ComponentType == ECS::ComponentType::CircleCollider2D)
						{
							newPath = m_SceneHierarchyTree.GetPathFromEntryReference(&subEntry);
							break;
						}
					}
					KG_ASSERT(newPath);

					// Circle collider from tree
					m_SceneHierarchyTree.RemoveEntry(newPath);
					entity.RemoveComponent<ECS::CircleCollider2DComponent>();
				}
			});
		});

		m_CircleColliderOffset.m_Label = "Offset";
		m_CircleColliderOffset.m_Flags |= EditorUI::EditVec2_Indented;
		m_CircleColliderOffset.m_ConfirmAction = [&](EditorUI::EditVec2Widget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			ECS::CircleCollider2DComponent& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
			component.Offset = m_CircleColliderOffset.m_CurrentVec2;
		};

		m_CircleColliderRadius.m_Label = "Radius";
		m_CircleColliderRadius.m_Flags |= EditorUI::EditFloat_Indented;
		m_CircleColliderRadius.m_ConfirmAction = [&](EditorUI::EditFloatWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			ECS::CircleCollider2DComponent& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
			component.Radius = m_CircleColliderRadius.m_CurrentFloat;
		};

		m_CircleColliderDensity.m_Label = "Density";
		m_CircleColliderDensity.m_Flags |= EditorUI::EditFloat_Indented;
		m_CircleColliderDensity.m_ConfirmAction = [&](EditorUI::EditFloatWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			ECS::CircleCollider2DComponent& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
			component.Density = m_CircleColliderDensity.m_CurrentFloat;
		};

		m_CircleColliderFriction.m_Label = "Friction";
		m_CircleColliderFriction.m_Flags |= EditorUI::EditFloat_Indented;
		m_CircleColliderFriction.m_ConfirmAction = [&](EditorUI::EditFloatWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			ECS::CircleCollider2DComponent& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
			component.Friction = m_CircleColliderFriction.m_CurrentFloat;
		};

		m_CircleColliderRestitution.m_Label = "Restitution";
		m_CircleColliderRestitution.m_Flags |= EditorUI::EditFloat_Indented;
		m_CircleColliderRestitution.m_ConfirmAction = [&](EditorUI::EditFloatWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			ECS::CircleCollider2DComponent& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
			component.Restitution = m_CircleColliderRestitution.m_CurrentFloat;
		};

		m_CircleColliderRestitutionThreshold.m_Label = "Restitution Threshold";
		m_CircleColliderRestitutionThreshold.m_Flags |= EditorUI::EditFloat_Indented;
		m_CircleColliderRestitutionThreshold.m_ConfirmAction = [&](EditorUI::EditFloatWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			ECS::CircleCollider2DComponent& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
			component.RestitutionThreshold = m_CircleColliderRestitutionThreshold.m_CurrentFloat;
		};

		// Set whether circle collider is treated as a sensor
		m_CircleColliderIsSensor.m_Label = "Is Sensor";
		m_CircleColliderIsSensor.m_Flags |= EditorUI::Checkbox_Indented;
		m_CircleColliderIsSensor.m_ConfirmAction = [&](EditorUI::CheckboxWidget& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			ECS::CircleCollider2DComponent& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
			component.IsSensor = spec.m_CurrentBoolean;
		};
	}

	void SceneEditorPanel::InitializeCameraComponent()
	{
		m_CameraHeader.m_Label = "Camera";
		m_CameraHeader.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_CameraHeader.m_Expanded = true;
		m_CameraHeader.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			EngineService::GetActiveEngine().GetThread().SubmitFunction([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<ECS::CameraComponent>())
				{
					// Search for indicated entity
					EditorUI::TreeEntry* entityEntry{ m_SceneHierarchyTree.SearchDepth((uint64_t)entity, 1) };
					KG_ASSERT(entityEntry);

					// Search for camera component in tree
					EditorUI::TreePath newPath {};
					for (EditorUI::TreeEntry& subEntry : entityEntry->m_SubEntries)
					{
						SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.m_ProvidedData.get();
						if (entryData.m_ComponentType == ECS::ComponentType::Camera)
						{
							newPath = m_SceneHierarchyTree.GetPathFromEntryReference(&subEntry);
							break;
						}
					}

					KG_ASSERT(newPath);

					m_SceneHierarchyTree.RemoveEntry(newPath);
					entity.RemoveComponent<ECS::CameraComponent>();
				}
			});
		});

		// Set Primary Camera Checkbox
		m_CameraPrimary.m_Label = "Primary Camera";
		m_CameraPrimary.m_Flags |= EditorUI::Checkbox_Indented;
		m_CameraPrimary.m_ConfirmAction = [&](EditorUI::CheckboxWidget& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			ECS::CameraComponent& component = entity.GetComponent<ECS::CameraComponent>();
			component.Primary = spec.m_CurrentBoolean;
		};

		m_CameraProjection.m_Label = "Projection Type";
		m_CameraProjection.m_Flags |= EditorUI::RadioSelect_Indented;
		m_CameraProjection.m_FirstOptionLabel = "Perspective";
		m_CameraProjection.m_SecondOptionLabel = "Orthographic";
		m_CameraProjection.m_SelectAction = [&]()
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			ECS::CameraComponent& component = entity.GetComponent<ECS::CameraComponent>();

			if (m_CameraProjection.m_SelectedOption == 0)
			{
				component.Camera.SetProjectionType(Scenes::SceneCamera::ProjectionType::Perspective);
				return;
			}
			if (m_CameraProjection.m_SelectedOption == 1)
			{
				component.Camera.SetProjectionType(Scenes::SceneCamera::ProjectionType::Orthographic);
				return;
			}
			KG_ERROR("Invalid selection option provided");
		};

		m_CameraOrthographicSize.m_Label = "Size";
		m_CameraOrthographicSize.m_Flags |= EditorUI::EditFloat_Indented;
		m_CameraOrthographicSize.m_ConfirmAction = [&](EditorUI::EditFloatWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			ECS::CameraComponent& component = entity.GetComponent<ECS::CameraComponent>();
			component.Camera.SetOrthographicSize(m_CameraOrthographicSize.m_CurrentFloat);
		};

		m_CameraOrthographicNearPlane.m_Label = "Near Plane";
		m_CameraOrthographicNearPlane.m_Flags |= EditorUI::EditFloat_Indented;
		m_CameraOrthographicNearPlane.m_ConfirmAction = [&](EditorUI::EditFloatWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			ECS::CameraComponent& component = entity.GetComponent<ECS::CameraComponent>();
			component.Camera.SetOrthographicNearClip(m_CameraOrthographicNearPlane.m_CurrentFloat);
		};

		m_CameraOrthographicFarPlane.m_Label = "Far Plane";
		m_CameraOrthographicFarPlane.m_Flags |= EditorUI::EditFloat_Indented;
		m_CameraOrthographicFarPlane.m_ConfirmAction = [&](EditorUI::EditFloatWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			ECS::CameraComponent& component = entity.GetComponent<ECS::CameraComponent>();
			component.Camera.SetOrthographicFarClip(m_CameraOrthographicFarPlane.m_CurrentFloat);
		};

		m_CameraPerspectiveFOV.m_Label = "Vertical FOV";
		m_CameraPerspectiveFOV.m_Flags |= EditorUI::EditFloat_Indented;
		m_CameraPerspectiveFOV.m_ConfirmAction = [&](EditorUI::EditFloatWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			ECS::CameraComponent& component = entity.GetComponent<ECS::CameraComponent>();
			component.Camera.SetPerspectiveVerticalFOV(m_CameraPerspectiveFOV.m_CurrentFloat);
		};

		m_CameraPerspectiveNearPlane.m_Label = "Near Plane";
		m_CameraPerspectiveNearPlane.m_Flags |= EditorUI::EditFloat_Indented;
		m_CameraPerspectiveNearPlane.m_ConfirmAction = [&](EditorUI::EditFloatWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			ECS::CameraComponent& component = entity.GetComponent<ECS::CameraComponent>();
			component.Camera.SetPerspectiveNearClip(m_CameraPerspectiveNearPlane.m_CurrentFloat);
		};

		m_CameraPerspectiveFarPlane.m_Label = "Far Plane";
		m_CameraPerspectiveFarPlane.m_Flags |= EditorUI::EditFloat_Indented;
		m_CameraPerspectiveFarPlane.m_ConfirmAction = [&](EditorUI::EditFloatWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			ECS::CameraComponent& component = entity.GetComponent<ECS::CameraComponent>();
			component.Camera.SetPerspectiveFarClip(m_CameraPerspectiveFarPlane.m_CurrentFloat);
		};
	}

	void SceneEditorPanel::InitializeParticleEmitterComponent()
	{
		// Set up particle emitter header
		m_ParticleEmitterHeader.m_Label = "Particle Emitter";
		m_ParticleEmitterHeader.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_ParticleEmitterHeader.m_Expanded = true;
		m_ParticleEmitterHeader.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			EngineService::GetActiveEngine().GetThread().SubmitFunction([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				EditorUI::TreePath pathToDelete;
				if (entity.HasComponent<ECS::ParticleEmitterComponent>())
				{
					m_SceneHierarchyTree.EditDepth([&](EditorUI::TreeEntry& entry)
					{
						if ((uint32_t)entry.m_Handle == (uint32_t)entity)
						{
							for (EditorUI::TreeEntry& subEntry : entry.m_SubEntries)
							{
								SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.m_ProvidedData.get();
								if (entryData.m_ComponentType == ECS::ComponentType::ParticleEmitter)
								{
									pathToDelete = m_SceneHierarchyTree.GetPathFromEntryReference(&subEntry);
									break;
								}
							}
							if (!pathToDelete)
							{
								KG_WARN("Could not locate component inside of specified entry in tree");
								return;
							}

						}
					}, 1);

					KG_ASSERT(pathToDelete);
					m_SceneHierarchyTree.RemoveEntry(pathToDelete);
					entity.RemoveComponent<ECS::ParticleEmitterComponent>();
					s_MainWindow->LoadSceneParticleEmitters();
				}
			});
		});

		// Set up particle component config select options widget
		m_SelectParticleEmitter.m_Label = "Particle Emitter";
		m_SelectParticleEmitter.m_Flags |= EditorUI::SelectOption_Indented;
		m_SelectParticleEmitter.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectParticleEmitter.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();
			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetEmitterConfigRegistry())
			{
				Ref<Particles::EmitterConfig> emitterConfigRef = Assets::AssetService::GetEmitterConfig(handle);
				KG_ASSERT(emitterConfigRef);

				spec.AddToOptions("All Emitters", asset.Data.FileLocation.filename().string(), handle);
			}
		};

		m_SelectParticleEmitter.m_ConfirmAction = [](const EditorUI::OptionEntry& entry)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::ParticleEmitterComponent>())
			{
				KG_ERROR("Attempt to edit entity particle emitter component when none exists!");
				return;
			}
			ECS::ParticleEmitterComponent& component = entity.GetComponent<ECS::ParticleEmitterComponent>();

			// Check for empty entry
			if (entry.m_Handle == Assets::EmptyHandle)
			{
				component.m_EmitterConfigHandle = Assets::EmptyHandle;
				component.m_EmitterConfigRef = nullptr;
			}
			// Check for a valid entry, and Update if applicable
			component.m_EmitterConfigHandle = entry.m_Handle;
			component.m_EmitterConfigRef = Assets::AssetService::GetEmitterConfig(entry.m_Handle);

			s_MainWindow->LoadSceneParticleEmitters();
		};


	}

	void SceneEditorPanel::InitializeOnUpdateComponent()
	{
		m_OnUpdateHeader.m_Label = "On Update";
		m_OnUpdateHeader.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_OnUpdateHeader.m_Expanded = true;
		m_OnUpdateHeader.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			EngineService::GetActiveEngine().GetThread().SubmitFunction([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				EditorUI::TreePath pathToDelete;
				if (entity.HasComponent<ECS::OnUpdateComponent>())
				{
					m_SceneHierarchyTree.EditDepth([&](EditorUI::TreeEntry& entry) 
					{
						if ((uint32_t)entry.m_Handle == (uint32_t)entity)
						{
							for (auto& subEntry : entry.m_SubEntries)
							{
								SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.m_ProvidedData.get();
								if (entryData.m_ComponentType == ECS::ComponentType::OnUpdate)
								{
									pathToDelete = m_SceneHierarchyTree.GetPathFromEntryReference(&subEntry);
									break;
								}
							}
							if (!pathToDelete)
							{
								KG_WARN("Could not locate component inside of specified entry in tree");
								return;
							}

						}
					}, 1);

					KG_ASSERT(pathToDelete);
					m_SceneHierarchyTree.RemoveEntry(pathToDelete);
					entity.RemoveComponent<ECS::OnUpdateComponent>();
				}
			});
		});

		m_SelectOnUpdateScript.m_Label = "On Update Script";
		m_SelectOnUpdateScript.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectOnUpdateScript.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectOnUpdateScript.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();
			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				KG_ASSERT(script);

				if (script->m_FuncType != WrappedFuncType::Void_EntityFloat)
				{
					continue;
				}
				spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}
		};

		m_SelectOnUpdateScript.m_ConfirmAction = [](const EditorUI::OptionEntry& entry)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::OnUpdateComponent>())
			{
				KG_ERROR("Attempt to edit entity OnUpdate component when none exists!");
				return;
			}
			ECS::OnUpdateComponent& component = entity.GetComponent<ECS::OnUpdateComponent>();

			// Check for empty entry
			if (entry.m_Handle == Assets::EmptyHandle)
			{
				component.OnUpdateScriptHandle = Assets::EmptyHandle;
				component.OnUpdateScript = nullptr;
			}
			// Check for a valid entry, and Update if applicable
			component.OnUpdateScriptHandle = entry.m_Handle;
			component.OnUpdateScript = Assets::AssetService::GetScript(entry.m_Handle);
		};

		m_SelectOnUpdateScript.m_OnEdit = [&](EditorUI::SelectOptionWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			// Initialize tooltip with options
			m_SelectTooltip.ClearEntries();
			EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
			{
				UNREFERENCED_PARAMETER(entry);
				m_SelectOnUpdateScript.m_OpenPopup = true;
			}};
			m_SelectTooltip.AddTooltipEntry(openScriptOptions);

			EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
			{
				UNREFERENCED_PARAMETER(entry);
				// Open create script dialog in script editor
				s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_EntityFloat, [&](Assets::AssetHandle scriptHandle)
				{
					// Ensure handle provides a script in the registry
					if (!Assets::AssetService::HasScript(scriptHandle))
					{
						KG_WARN("Could not find runtime start function in Project Panel");
						return;
					}

					// Ensure function type matches definition
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
					if (script->m_FuncType != WrappedFuncType::Void_EntityFloat)
					{
						KG_WARN("Incorrect function type returned when linking script to usage point");
						return;
					}

					ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
					if (!entity.HasComponent<ECS::OnUpdateComponent>())
					{
						KG_ERROR("Attempt to edit entity OnUpdate component when none exists!");
						return;
					}
					ECS::OnUpdateComponent& component = entity.GetComponent<ECS::OnUpdateComponent>();

					// Update component's data
					component.OnUpdateScriptHandle = scriptHandle;
					component.OnUpdateScript = script;
					m_SelectOnUpdateScript.m_CurrentOption = { script->m_ScriptName.c_str(), scriptHandle };
				}, {"activeEntity", "deltaTime"});

				} };
			m_SelectTooltip.AddTooltipEntry(createScriptOptions);

			// Open tooltip
			m_SelectTooltip.m_TooltipActive = true;

		};
	}

	void SceneEditorPanel::InitializeOnCreateComponent()
	{
		m_OnCreateHeader.m_Label = "On Create";
		m_OnCreateHeader.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_OnCreateHeader.m_Expanded = true;
		m_OnCreateHeader.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			EngineService::GetActiveEngine().GetThread().SubmitFunction([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				EditorUI::TreePath pathToDelete;
				if (entity.HasComponent<ECS::OnCreateComponent>())
				{
					m_SceneHierarchyTree.EditDepth([&](EditorUI::TreeEntry& entry)
					{
						if ((uint32_t)entry.m_Handle == (uint32_t)entity)
						{
							for (EditorUI::TreeEntry& subEntry : entry.m_SubEntries)
							{
								SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.m_ProvidedData.get();
								if (entryData.m_ComponentType == ECS::ComponentType::OnCreate)
								{
									pathToDelete = m_SceneHierarchyTree.GetPathFromEntryReference(&subEntry);
									break;
								}
							}
							if (!pathToDelete)
							{
								KG_WARN("Could not locate component inside of specified entry in tree");
								return;
							}

						}
					}, 1);

					KG_ASSERT(pathToDelete);
					m_SceneHierarchyTree.RemoveEntry(pathToDelete);
					entity.RemoveComponent<ECS::OnCreateComponent>();
				}
			});
		});

		m_SelectOnCreateScript.m_Label = "On Create Script";
		m_SelectOnCreateScript.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectOnCreateScript.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectOnCreateScript.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();
			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				KG_ASSERT(script);

				if (script->m_FuncType != WrappedFuncType::Void_Entity)
				{
					continue;
				}
				spec.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
			}
		};

		m_SelectOnCreateScript.m_ConfirmAction = [](const EditorUI::OptionEntry& entry) 
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::OnCreateComponent>())
			{
				KG_ERROR("Attempt to edit entity OnCreate component when none exists!");
				return;
			}
			ECS::OnCreateComponent& component = entity.GetComponent<ECS::OnCreateComponent>();

			// Check for empty entry
			if (entry.m_Handle == Assets::EmptyHandle)
			{
				component.OnCreateScriptHandle = Assets::EmptyHandle;
				component.OnCreateScript = nullptr;
			}
			// Check for a valid entry, and Create if applicable
			component.OnCreateScriptHandle = entry.m_Handle;
			component.OnCreateScript = Assets::AssetService::GetScript(entry.m_Handle);
		};

		m_SelectOnCreateScript.m_OnEdit = [&](EditorUI::SelectOptionWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			// Initialize tooltip with options
			m_SelectTooltip.ClearEntries();
			EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
			{
				UNREFERENCED_PARAMETER(entry);
				m_SelectOnCreateScript.m_OpenPopup = true;
			} };
			m_SelectTooltip.AddTooltipEntry(openScriptOptions);

			EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
			{
				UNREFERENCED_PARAMETER(entry);
				// Open create script dialog in script editor
				s_MainWindow->m_ScriptEditorPanel->OpenCreateScriptDialogFromUsagePoint(WrappedFuncType::Void_Entity, [&](Assets::AssetHandle scriptHandle)
				{
					// Ensure handle provides a script in the registry
					if (!Assets::AssetService::HasScript(scriptHandle))
					{
						KG_WARN("Could not find script");
						return;
					}

					// Ensure function type matches definition
					Ref<Scripting::Script> script = Assets::AssetService::GetScript(scriptHandle);
					if (script->m_FuncType != WrappedFuncType::Void_Entity)
					{
						KG_WARN("Incorrect function type returned when linking script to usage point");
						return;
					}

					ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
					if (!entity.HasComponent<ECS::OnCreateComponent>())
					{
						KG_ERROR("Attempt to edit entity OnCreate component when none exists!");
						return;
					}
					ECS::OnCreateComponent& component = entity.GetComponent<ECS::OnCreateComponent>();

					// Update component's data
					component.OnCreateScriptHandle = scriptHandle;
					component.OnCreateScript = script;
					m_SelectOnCreateScript.m_CurrentOption = { script->m_ScriptName.c_str(), scriptHandle };
				}, {"activeEntity"});

				}};
			m_SelectTooltip.AddTooltipEntry(createScriptOptions);

			// Open tooltip
			m_SelectTooltip.m_TooltipActive = true;

		};
	}

	void SceneEditorPanel::InitializeAIComponent()
	{
		// Set up AI state header
		m_AIStateHeader.m_Label = "AI State";
		m_AIStateHeader.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_AIStateHeader.m_Expanded = true;
		m_AIStateHeader.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			EngineService::GetActiveEngine().GetThread().SubmitFunction([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				EditorUI::TreePath pathToDelete;
				if (entity.HasComponent<ECS::AIStateComponent>())
				{
					m_SceneHierarchyTree.EditDepth([&](EditorUI::TreeEntry& entry)
					{
						if ((uint32_t)entry.m_Handle == (uint32_t)entity)
						{
							for (EditorUI::TreeEntry& subEntry : entry.m_SubEntries)
							{
								SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.m_ProvidedData.get();
								if (entryData.m_ComponentType == ECS::ComponentType::AIState)
								{
									pathToDelete = m_SceneHierarchyTree.GetPathFromEntryReference(&subEntry);
									break;
								}
							}
							if (!pathToDelete)
							{
								KG_WARN("Could not locate component inside of specified entry in tree");
								return;
							}

						}
					}, 1);

					KG_ASSERT(pathToDelete);
					m_SceneHierarchyTree.RemoveEntry(pathToDelete);
					entity.RemoveComponent<ECS::AIStateComponent>();
				}
			});
		});

		// Set up global state select options widget
		m_SelectGlobalState.m_Label = "Global State";
		m_SelectGlobalState.m_Flags |= EditorUI::SelectOption_Indented;
		m_SelectGlobalState.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectGlobalState.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();
			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetAIStateRegistry())
			{
				Ref<AI::AIState> aiStateRef = Assets::AssetService::GetAIState(handle);
				KG_ASSERT(aiStateRef);

				spec.AddToOptions("All States", asset.Data.FileLocation.filename().string(), handle);
			}
		};

		m_SelectGlobalState.m_ConfirmAction = [](const EditorUI::OptionEntry& entry)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::AIStateComponent>())
			{
				KG_ERROR("Attempt to edit entity AIState component when none exists!");
				return;
			}
			ECS::AIStateComponent& component = entity.GetComponent<ECS::AIStateComponent>();

			// Check for empty entry
			if (entry.m_Handle == Assets::EmptyHandle)
			{
				component.GlobalStateHandle = Assets::EmptyHandle;
				component.GlobalStateReference = nullptr;
			}
			// Check for a valid entry, and Update if applicable
			component.GlobalStateHandle = entry.m_Handle;
			component.GlobalStateReference = Assets::AssetService::GetAIState(entry.m_Handle);
		};

		// Set up current state select options widget
		m_SelectCurrentState.m_Label = "Current State";
		m_SelectCurrentState.m_Flags |= EditorUI::SelectOption_Indented;
		m_SelectCurrentState.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectCurrentState.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();
			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetAIStateRegistry())
			{
				Ref<AI::AIState> aiStateRef = Assets::AssetService::GetAIState(handle);
				KG_ASSERT(aiStateRef);

				spec.AddToOptions("All States", asset.Data.FileLocation.filename().string(), handle);
			}
		};

		m_SelectCurrentState.m_ConfirmAction = [](const EditorUI::OptionEntry& entry)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::AIStateComponent>())
			{
				KG_ERROR("Attempt to edit entity AIState component when none exists!");
				return;
			}
			ECS::AIStateComponent& component = entity.GetComponent<ECS::AIStateComponent>();

			// Check for empty entry
			if (entry.m_Handle == Assets::EmptyHandle)
			{
				component.CurrentStateHandle = Assets::EmptyHandle;
				component.CurrentStateReference = nullptr;
			}
			// Check for a valid entry, and Update if applicable
			component.CurrentStateHandle = entry.m_Handle;
			component.CurrentStateReference = Assets::AssetService::GetAIState(entry.m_Handle);
		};

		// Set up previous state select options widget
		m_SelectPreviousState.m_Label = "Previous State";
		m_SelectPreviousState.m_Flags |= EditorUI::SelectOption_Indented;
		m_SelectPreviousState.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectPreviousState.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();
			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetAIStateRegistry())
			{
				Ref<AI::AIState> aiStateRef = Assets::AssetService::GetAIState(handle);
				KG_ASSERT(aiStateRef);

				spec.AddToOptions("All States", asset.Data.FileLocation.filename().string(), handle);
			}
		};

		m_SelectPreviousState.m_ConfirmAction = [](const EditorUI::OptionEntry& entry)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::AIStateComponent>())
			{
				KG_ERROR("Attempt to edit entity AIState component when none exists!");
				return;
			}
			ECS::AIStateComponent& component = entity.GetComponent<ECS::AIStateComponent>();

			// Check for empty entry
			if (entry.m_Handle == Assets::EmptyHandle)
			{
				component.PreviousStateHandle = Assets::EmptyHandle;
				component.PreviousStateReference = nullptr;
			}
			// Check for a valid entry, and Update if applicable
			component.PreviousStateHandle = entry.m_Handle;
			component.PreviousStateReference = Assets::AssetService::GetAIState(entry.m_Handle);
		};

	}

	void SceneEditorPanel::InitializeShapeComponent()
	{
		m_ShapeHeader.m_Label = "Shape";
		m_ShapeHeader.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_ShapeHeader.m_Expanded = true;
		m_ShapeHeader.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			EngineService::GetActiveEngine().GetThread().SubmitFunction([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<ECS::ShapeComponent>())
				{
					// Search for indicated entity
					EditorUI::TreeEntry* entityEntry{ m_SceneHierarchyTree.SearchDepth((uint64_t)entity, 1) };
					KG_ASSERT(entityEntry);

					// Search for shape component in tree
					EditorUI::TreePath newPath {};
					for (EditorUI::TreeEntry& subEntry : entityEntry->m_SubEntries)
					{
						SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.m_ProvidedData.get();
						if (entryData.m_ComponentType == ECS::ComponentType::Shape)
						{
							newPath = m_SceneHierarchyTree.GetPathFromEntryReference(&subEntry);
							break;
						}
					}
					if (!newPath)
					{
						KG_WARN("Could not locate component inside Tree");
						return;
					}

					// Remove shape component from tree and ECS
					m_SceneHierarchyTree.RemoveEntry(newPath);
					entity.RemoveComponent<ECS::ShapeComponent>();
				}
			});
		});

		m_ShapeSelect.m_Label = "Mesh";
		m_ShapeSelect.m_Flags |= EditorUI::SelectOption_Indented;
		m_ShapeSelect.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();
			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			spec.AddToOptions("All Shapes", "Quad", Assets::EmptyHandle);
			spec.AddToOptions("All Shapes", "Pyramid", Assets::EmptyHandle);
			spec.AddToOptions("All Shapes", "Cube", Assets::EmptyHandle);
		};
		m_ShapeSelect.m_ConfirmAction = [&](const EditorUI::OptionEntry& entry)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			Rendering::Shape* shape {nullptr};

			if (entry.m_Label == "None")
			{
				shape = &Rendering::Shape::s_None;
			}
			if (entry.m_Label == "Quad")
			{
				shape = &Rendering::Shape::s_Quad;
			}
			if (entry.m_Label == "Cube")
			{
				shape = &Rendering::Shape::s_Cube;
			}
			if (entry.m_Label == "Pyramid")
			{
				shape = &Rendering::Shape::s_Pyramid;
			}

			if (!shape)
			{
				KG_ERROR("Invalid shape component provided");
			}

			component.CurrentShape = shape->GetShapeType();
			component.ShaderSpecification.RenderType = shape->GetRenderingType();
			if (shape->GetRenderingType() == Rendering::RenderingType::DrawIndex)
			{
				component.Vertices = CreateRef<std::vector<Math::vec3>>(shape->GetIndexVertices());
				component.Indices = CreateRef<std::vector<uint32_t>>(shape->GetIndices());
				component.TextureCoordinates = CreateRef<std::vector<Math::vec2>>(shape->GetIndexTextureCoordinates());
			}
			if (shape->GetRenderingType() == Rendering::RenderingType::DrawTriangle)
			{
				component.Vertices = CreateRef<std::vector<Math::vec3>>(shape->GetTriangleVertices());
				component.TextureCoordinates = CreateRef<std::vector<Math::vec2>>(shape->GetTriangleTextureCoordinates());
			}
			if (component.CurrentShape == Rendering::ShapeTypes::Cube || component.CurrentShape == Rendering::ShapeTypes::Pyramid)
			{
				//component.ShaderSpecification.AddTexture = false;
				component.ShaderSpecification.AddCircleShape = false;
			}
			UpdateShapeComponent();
			if (component.VertexColors)
			{
				component.VertexColors->resize(component.Vertices->size(), { 1.0f, 1.0f, 1.0f, 1.0f });
			}
		};

		m_ShapeColorType.m_Label = "Color Type";
		m_ShapeColorType.m_Flags |= EditorUI::SelectOption_Indented;
		m_ShapeColorType.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();
			spec.AddToOptions("Clear", "None", Assets::EmptyHandle);
			spec.AddToOptions("All Types", "FlatColor", Assets::EmptyHandle);
			spec.AddToOptions("All Types", "VertexColor", Assets::EmptyHandle);
		};
		m_ShapeColorType.m_ConfirmAction = [&](const EditorUI::OptionEntry& entry)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			if (entry.m_Label == "None")
			{
				component.ShaderSpecification.ColorInput = Rendering::ColorInputType::None;
				UpdateShapeComponent();
			}
			if (entry.m_Label == "FlatColor")
			{
				component.ShaderSpecification.ColorInput = Rendering::ColorInputType::FlatColor;
				UpdateShapeComponent();
				Rendering::Shader::SetDataAtInputLocation<Math::vec4>({ 1.0f, 1.0f, 1.0f, 1.0f }, 
					Utility::FileSystem::CRCFromString("a_Color"),
					component.ShaderData, component.Shader);
			}
			if (entry.m_Label == "VertexColor")
			{
				Math::vec4 transferColor {1.0f, 1.0f, 1.0f, 1.0f};
				if (component.ShaderSpecification.ColorInput == Rendering::ColorInputType::FlatColor)
				{
					transferColor = *Rendering::Shader::GetInputLocation<Math::vec4>(
						Utility::FileSystem::CRCFromString("a_Color"),
						component.ShaderData, component.Shader);
				}
				component.ShaderSpecification.ColorInput = Rendering::ColorInputType::VertexColor;
				UpdateShapeComponent();
				if (component.VertexColors) { component.VertexColors->clear(); }
				component.VertexColors = CreateRef<std::vector<Math::vec4>>();
				for (uint32_t iterator{ 0 }; iterator < component.Vertices->size(); iterator++)
				{
					component.VertexColors->push_back(transferColor);
				}
			}
		};

		m_ShapeColor.m_Label = "Flat Color";
		m_ShapeColor.m_Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_ShapeColor.m_ConfirmAction = [&](EditorUI::EditVec4Widget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::ShapeComponent>())
			{
				KG_ERROR("Attempt to edit entity shape 2D component when none exists!");
				return;
			}
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			Math::vec4* color = Rendering::Shader::GetInputLocation<Math::vec4>(
				Utility::FileSystem::CRCFromString("a_Color"), 
				component.ShaderData, component.Shader);
			*color = m_ShapeColor.m_CurrentVec4;
		};

		// Set Shape Add Texture Checkbox
		m_ShapeAddTexture.m_Label = "Use Texture";
		m_ShapeAddTexture.m_Flags |= EditorUI::Checkbox_Indented;
		m_ShapeAddTexture.m_ConfirmAction = [&](EditorUI::CheckboxWidget& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			spec.m_CurrentBoolean ? component.ShaderSpecification.TextureInput = Rendering::TextureInputType::ColorTexture :
				component.ShaderSpecification.TextureInput = Rendering::TextureInputType::None;
			UpdateShapeComponent();
			// Checkbox is switched on
			if (spec.m_CurrentBoolean)
			{
				if (component.CurrentShape == Rendering::ShapeTypes::Cube || component.CurrentShape == Rendering::ShapeTypes::Pyramid)
				{
					component.ShaderSpecification.RenderType = Rendering::RenderingType::DrawTriangle;
					UpdateShapeComponent();
					component.Vertices = CreateRef<std::vector<Math::vec3>>(Utility::ShapeTypeToShape(component.CurrentShape).GetTriangleVertices());
					component.TextureCoordinates = CreateRef<std::vector<Math::vec2>>(Utility::ShapeTypeToShape(component.CurrentShape).GetTriangleTextureCoordinates());
					if (component.VertexColors) { component.VertexColors->resize(component.Vertices->size(), { 1.0f, 1.0f, 1.0f, 1.0f }); }
				}
				Rendering::Shader::SetDataAtInputLocation<float>(1.0f, 
					Utility::FileSystem::CRCFromString("a_TilingFactor"),
					component.ShaderData, component.Shader);
			}
			// Checkbox is switched off
			if (!spec.m_CurrentBoolean)
			{
				if (component.CurrentShape == Rendering::ShapeTypes::Cube || component.CurrentShape == Rendering::ShapeTypes::Pyramid)
				{
					component.ShaderSpecification.RenderType = Rendering::RenderingType::DrawIndex;
					UpdateShapeComponent();
					component.Vertices = CreateRef<std::vector<Math::vec3>>(Utility::ShapeTypeToShape(component.CurrentShape).GetIndexVertices());
					component.Indices = CreateRef<std::vector<uint32_t>>(Utility::ShapeTypeToShape(component.CurrentShape).GetIndices());
					component.TextureCoordinates = CreateRef<std::vector<Math::vec2>>(Utility::ShapeTypeToShape(component.CurrentShape).GetIndexTextureCoordinates());
					if (component.VertexColors) { component.VertexColors->resize(component.Vertices->size(), { 1.0f, 1.0f, 1.0f, 1.0f }); }
				}
			}
		};

		m_ShapeSetTexture.m_Label = "Select Texture";
		m_ShapeSetTexture.m_Flags |= EditorUI::SelectOption_Indented;
		m_ShapeSetTexture.m_PopupAction = [&](EditorUI::SelectOptionWidget& spec)
		{
			spec.ClearOptions();
			for (auto& [handle, asset] : Assets::AssetService::GetTexture2DRegistry())
			{
				spec.AddToOptions("All Textures", asset.Data.FileLocation.filename().string(), handle);
			}
		};
		m_ShapeSetTexture.m_ConfirmAction = [&](const EditorUI::OptionEntry& entry)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			if (entry.m_Handle == Assets::EmptyHandle)
			{
				Buffer textureBuffer{ 4 };
				textureBuffer.SetDataToByte(0xff);
				component.TextureHandle = Assets::AssetService::ImportNewTextureFromData(textureBuffer, 1, 1, 4);
				component.Texture = Assets::AssetService::GetTexture2D(component.TextureHandle);
				textureBuffer.Release();
			}

			if (!Assets::AssetService::GetTexture2DRegistry().contains(entry.m_Handle))
			{
				KG_WARN("Could not locate texture in asset registry!");
				return;
			}

			component.TextureHandle = entry.m_Handle;
			component.Texture = Assets::AssetService::GetTexture2D(entry.m_Handle);
		};

		m_ShapeTilingFactor.m_Label = "Tiling Factor";
		m_ShapeTilingFactor.m_Flags |= EditorUI::EditFloat_Indented;
		m_ShapeTilingFactor.m_ConfirmAction = [&](EditorUI::EditFloatWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::ShapeComponent>())
			{
				KG_ERROR("Attempt to edit entity shape 2D component when none exists!");
				return;
			}
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			float* tilingFactor = Rendering::Shader::GetInputLocation<float>(
				Utility::FileSystem::CRCFromString("a_TilingFactor"), 
				component.ShaderData, component.Shader);
			*tilingFactor = m_ShapeTilingFactor.m_CurrentFloat;
		};

		// Set Shape Circle Option
		m_ShapeAddCircle.m_Label = "Use Circle Shape";
		m_ShapeAddCircle.m_Flags |= EditorUI::Checkbox_Indented;
		m_ShapeAddCircle.m_ConfirmAction = [&](EditorUI::CheckboxWidget& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			component.ShaderSpecification.AddCircleShape = spec.m_CurrentBoolean;
			UpdateShapeComponent();
			if (spec.m_CurrentBoolean)
			{
				Rendering::Shader::SetDataAtInputLocation<float>(1.0f, 
					Utility::FileSystem::CRCFromString("a_Thickness"),
					component.ShaderData, component.Shader);
				Rendering::Shader::SetDataAtInputLocation<float>(0.005f, 
					Utility::FileSystem::CRCFromString("a_Fade"),
					component.ShaderData, component.Shader);
			}
		};

		m_ShapeCircleThickness.m_Label = "Circle Thickness";
		m_ShapeCircleThickness.m_Flags |= EditorUI::EditFloat_Indented;
		m_ShapeCircleThickness.m_ConfirmAction = [&](EditorUI::EditFloatWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::ShapeComponent>())
			{
				KG_ERROR("Attempt to edit entity shape 2D component when none exists!");
				return;
			}
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			float* thickness = Rendering::Shader::GetInputLocation<float>(
				Utility::FileSystem::CRCFromString("a_Thickness"), 
				component.ShaderData, component.Shader);
			*thickness = m_ShapeCircleThickness.m_CurrentFloat;
		};

		m_ShapeCircleFade.m_Label = "Circle Fade";
		m_ShapeCircleFade.m_Flags |= EditorUI::EditFloat_Indented;
		m_ShapeCircleFade.m_ConfirmAction = [&](EditorUI::EditFloatWidget& spec)
		{
			UNREFERENCED_PARAMETER(spec);
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::ShapeComponent>())
			{
				KG_ERROR("Attempt to edit entity shape 2D component when none exists!");
				return;
			}
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			float* fade = Rendering::Shader::GetInputLocation<float>(
				Utility::FileSystem::CRCFromString("a_Fade"), 
				component.ShaderData, component.Shader);
			*fade = m_ShapeCircleFade.m_CurrentFloat;
		};

		// Set Shape Add Projection Option
		m_ShapeAddProjection.m_Label = "Use Projection Matrix";
		m_ShapeAddProjection.m_Flags |= EditorUI::Checkbox_Indented;
		m_ShapeAddProjection.m_ConfirmAction = [&](EditorUI::CheckboxWidget& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			component.ShaderSpecification.AddProjectionMatrix = spec.m_CurrentBoolean;
			UpdateShapeComponent();
		};

		// Set Shape Add Entity ID Option
		m_ShapeAddEntityID.m_Label = "Use Entity ID";
		m_ShapeAddEntityID.m_Flags |= EditorUI::Checkbox_Indented;
		m_ShapeAddEntityID.m_ConfirmAction = [&](EditorUI::CheckboxWidget& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			component.ShaderSpecification.AddEntityID = spec.m_CurrentBoolean;
			UpdateShapeComponent();
		};
	}

	struct ProjectComponentFieldInfo
	{
		Assets::AssetHandle m_ProjectComponentHandle {Assets::EmptyHandle};
		size_t m_FieldSlot{ std::numeric_limits<size_t>().max() };
	};

	void SceneEditorPanel::InitializeProjectComponents()
	{
		for (auto& [handle, asset] : Assets::AssetService::GetProjectComponentRegistry())
		{
			InitializeProjectComponent(handle);
		}
		
	}

	void SceneEditorPanel::InitializeProjectComponent(Assets::AssetHandle projectComponentHandle)
	{
		Ref<ECS::ProjectComponent> component = Assets::AssetService::GetProjectComponent(projectComponentHandle);
		KG_ASSERT(component, "Invalid component provided when initializing SceneEditorPanel");

		// Initialize Collapsing Header
		ProjectComponentWidgetData newWidgetData;
		newWidgetData.m_Header.m_Label = component->m_Name;
		newWidgetData.m_Header.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		newWidgetData.m_Header.m_Expanded = true;
		newWidgetData.m_Header.m_ProvidedData = CreateRef<Assets::AssetHandle>(projectComponentHandle);
		newWidgetData.m_Header.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderWidget& spec)
		{
			EngineService::GetActiveEngine().GetThread().SubmitFunction([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				Assets::AssetHandle projectComponentHandle = *(Assets::AssetHandle*)spec.m_ProvidedData.get();
				if (entity.HasProjectComponentData(projectComponentHandle))
				{
					// Search for indicated entity
					EditorUI::TreeEntry* entityEntry{ m_SceneHierarchyTree.SearchDepth((uint64_t)entity, 1) };
					KG_ASSERT(entityEntry);

					// Search for project component in tree
					EditorUI::TreePath newPath {};
					for (EditorUI::TreeEntry& subEntry : entityEntry->m_SubEntries)
					{
						SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.m_ProvidedData.get();
						if (entryData.m_ComponentType == ECS::ComponentType::ProjectComponent &&
							entryData.m_ProjectComponentHandle == projectComponentHandle)
						{
							newPath = m_SceneHierarchyTree.GetPathFromEntryReference(&subEntry);
							break;
						}
					}
					KG_ASSERT(newPath);
					
					// Remove project component from tree and ECS
					m_SceneHierarchyTree.RemoveEntry(newPath);
					entity.RemoveProjectComponentData(projectComponentHandle);
				}
			});
		});

		// Initialize project component fields into scene editor UI
		for (size_t iteration{ 0 }; iteration < component->m_DataNames.size(); iteration++)
		{
			static EditorUI::EditFloatWidget newFloatSpec;
			static EditorUI::EditVec3Widget newVector3Spec;
			static EditorUI::EditTextSpec newStringSpec;
			static EditorUI::CheckboxWidget newBoolSpec;
			static EditorUI::EditIntegerWidget newIntegerSpec;

			WrappedVarType currentType = component->m_DataTypes.at(iteration);
			std::string_view currentName = component->m_DataNames.at(iteration);

			switch (currentType)
			{
			case WrappedVarType::Float:
				newFloatSpec = {};
				newFloatSpec.m_Label = currentName;
				newFloatSpec.m_Flags |= EditorUI::EditFloat_Indented;
				newFloatSpec.m_ProvidedData = CreateRef<ProjectComponentFieldInfo>(projectComponentHandle, iteration);
				newFloatSpec.m_ConfirmAction = [](EditorUI::EditFloatWidget& spec)
				{
					// Get component data pointer
					ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.m_ProvidedData.get();
					Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
					ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
					uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

					// Get field data pointer
					uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

					// Set the data
					*(float*)fieldDataRef = spec.m_CurrentFloat;
				};
				newWidgetData.m_Fields.push_back(newFloatSpec);
				break;
			case WrappedVarType::String:
				newStringSpec = {};
				newStringSpec.m_Label = currentName;
				newStringSpec.m_Flags |= EditorUI::EditText_Indented;
				newStringSpec.m_ProvidedData = CreateRef<ProjectComponentFieldInfo>(projectComponentHandle, iteration);
				newStringSpec.m_ConfirmAction = [](EditorUI::EditTextSpec& spec)
				{
					// Get component data pointer
					ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.m_ProvidedData.get();
					Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
					ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
					uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

					// Get field data pointer
					uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

					// Set the data
					*(std::string*)fieldDataRef = spec.m_CurrentOption;
				};
				newWidgetData.m_Fields.push_back(newStringSpec);
				break;
			case WrappedVarType::Bool:
				newBoolSpec = {};
				newBoolSpec.m_Label = currentName;
				newBoolSpec.m_Flags |= EditorUI::Checkbox_Indented;
				newBoolSpec.m_ProvidedData = CreateRef<ProjectComponentFieldInfo>(projectComponentHandle, iteration);
				newBoolSpec.m_ConfirmAction = [](EditorUI::CheckboxWidget& spec)
				{
					// Get component data pointer
					ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.m_ProvidedData.get();
					Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
					ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
					uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

					// Get field data pointer
					uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

					// Set the data
					*(bool*)fieldDataRef = spec.m_CurrentBoolean;
				};
				newWidgetData.m_Fields.push_back(newBoolSpec);
				break;
			case WrappedVarType::Vector3:
				newVector3Spec = {};
				newVector3Spec.m_Label = currentName;
				newVector3Spec.m_Flags |= EditorUI::EditVec3_Indented;
				newVector3Spec.m_ProvidedData = CreateRef<ProjectComponentFieldInfo>(projectComponentHandle, iteration);
				newVector3Spec.m_ConfirmAction = [](EditorUI::EditVec3Widget& spec)
				{
					// Get component data pointer
					ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.m_ProvidedData.get();
					Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
					ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
					uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

					// Get field data pointer
					uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

					// Set the data
					*(Math::vec3*)fieldDataRef = spec.m_CurrentVec3;
				};
				newWidgetData.m_Fields.push_back(newVector3Spec);
				break;
			case WrappedVarType::Integer32:
				newIntegerSpec = {};
				newIntegerSpec.m_Label = currentName;
				newIntegerSpec.m_Flags |= EditorUI::EditInteger_Indented;
				newIntegerSpec.m_ProvidedData = CreateRef<ProjectComponentFieldInfo>(projectComponentHandle, iteration);
				newIntegerSpec.m_ConfirmAction = [](EditorUI::EditIntegerWidget& spec)
				{
					// Get component data pointer
					ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.m_ProvidedData.get();
					Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
					ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
					uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

					// Get field data pointer
					uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

					// Set the data
					*(int32_t*)fieldDataRef = (int32_t)spec.m_CurrentInteger;
				};
				newWidgetData.m_Fields.push_back(newIntegerSpec);
				break;
			case WrappedVarType::UInteger16:
				newIntegerSpec = {};
				newIntegerSpec.m_Label = currentName;
				newIntegerSpec.m_Flags |= EditorUI::EditInteger_Indented;
				newIntegerSpec.m_ProvidedData = CreateRef<ProjectComponentFieldInfo>(projectComponentHandle, iteration);
				newIntegerSpec.m_ConfirmAction = [](EditorUI::EditIntegerWidget& spec)
				{
					// Get component data pointer
					ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.m_ProvidedData.get();
					Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
					ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
					uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

					// Get field data pointer
					uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

					// Set the data
					*(uint16_t*)fieldDataRef = (uint16_t)spec.m_CurrentInteger;
				};
				newWidgetData.m_Fields.push_back(newIntegerSpec);
				break;
			case WrappedVarType::UInteger32:
				newIntegerSpec = {};
				newIntegerSpec.m_Label = currentName;
				newIntegerSpec.m_Flags |= EditorUI::EditInteger_Indented;
				newIntegerSpec.m_ProvidedData = CreateRef<ProjectComponentFieldInfo>(projectComponentHandle, iteration);
				newIntegerSpec.m_ConfirmAction = [](EditorUI::EditIntegerWidget& spec)
				{
					// Get component data pointer
					ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.m_ProvidedData.get();
					Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
					ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
					uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

					// Get field data pointer
					uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

					// Set the data
					*(uint32_t*)fieldDataRef = (uint32_t)spec.m_CurrentInteger;
				};
				newWidgetData.m_Fields.push_back(newIntegerSpec);
				break;
			case WrappedVarType::UInteger64:
				newIntegerSpec = {};
				newIntegerSpec.m_Label = currentName;
				newIntegerSpec.m_Flags |= EditorUI::EditInteger_Indented;
				newIntegerSpec.m_ProvidedData = CreateRef<ProjectComponentFieldInfo>(projectComponentHandle, iteration);
				newIntegerSpec.m_ConfirmAction = [](EditorUI::EditIntegerWidget& spec)
				{
					// Get component data pointer
					ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.m_ProvidedData.get();
					Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
					ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
					uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

					// Get field data pointer
					uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

					// Set the data
					*(uint64_t*)fieldDataRef = (uint64_t)spec.m_CurrentInteger;
				};
				newWidgetData.m_Fields.push_back(newIntegerSpec);
				break;
			case WrappedVarType::Void:
			case WrappedVarType::None:
				KG_ERROR("Invalid wrapped variable type provided when initializing project component editor ui");
				break;
			}
		}

		m_AllProjectComponents.insert_or_assign(projectComponentHandle, newWidgetData);
	}

	SceneEditorPanel::SceneEditorPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_MainWindow = s_EditorApp->m_MainWindow.get();
		s_MainWindow->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
			KG_BIND_CLASS_FN(SceneEditorPanel::OnKeyPressedEditor));
		InitializeSceneHierarchy();
		InitializeSceneOptions();
		InitializeTagComponent();
		InitializeTransformComponent();
		InitializeRigidbody2DComponent();
		InitializeOnUpdateComponent();
		InitializeOnCreateComponent();
		InitializeParticleEmitterComponent();
		InitializeBoxCollider2DComponent();
		InitializeCircleCollider2DComponent();
		InitializeAIComponent();
		InitializeCameraComponent();
		InitializeShapeComponent();
		InitializeProjectComponents();
	}
	void SceneEditorPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_MainWindow->m_ShowSceneHierarchy);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		if (Scenes::SceneService::GetActiveScene())
		{
			//TODO: Why, fix this plzzz
			m_MainSceneHeader.m_Label = Assets::AssetService::GetSceneRegistry().at(
				Scenes::SceneService::GetActiveSceneHandle()).Data.FileLocation.filename().string();
			
			m_MainSceneHeader.RenderHeader();

			EditorUI::EditorUIService::Tree(m_SceneHierarchyTree);

			m_AddComponent.RenderOptions();

			m_SelectTooltip.RenderTooltip();
		}
		EditorUI::EditorUIService::EndWindow();
	}
	bool SceneEditorPanel::OnSceneEvent(Events::Event* event)
	{

		if (event->GetEventType() == Events::EventType::ManageScene)
		{
			Events::ManageScene* manageEvent = (Events::ManageScene*)event;
			if (manageEvent->GetAction() == Events::ManageSceneAction::Open)
			{
				m_SceneHierarchyTree.m_OnRefresh();
			}
		}

		if (event->GetEventType() == Events::EventType::ManageEntity)
		{
			Events::ManageEntity* manageEntity = (Events::ManageEntity*)event;
			if (Scenes::SceneService::GetActiveScene().get() != manageEntity->GetSceneReference())
			{
				return false;
			}
			if (manageEntity->GetAction() == Events::ManageEntityAction::Delete)
			{
				ECS::Entity entityToDelete = Scenes::SceneService::GetActiveScene()->GetEntityByUUID(manageEntity->GetEntityID());
				if (!entityToDelete)
				{
					KG_WARN("Could not locate entity by UUID");
					return false;
				}

				if (*Scenes::SceneService::GetActiveScene()->GetSelectedEntity() == entityToDelete)
				{
					*Scenes::SceneService::GetActiveScene()->GetSelectedEntity() = {};
					s_MainWindow->m_SceneEditorPanel->SetSelectedEntity({});
				}

				// Search for indicated entity
				EditorUI::TreeEntry* entityEntry{ m_SceneHierarchyTree.SearchDepth((uint64_t)entityToDelete, 1) };
				KG_ASSERT(entityEntry);
				EditorUI::TreePath path = m_SceneHierarchyTree.GetPathFromEntryReference(entityEntry);
				if (!path)
				{
					KG_WARN("Could not locate path from entry reference!");
					return false;
				}

				// Remove the indicated entity
				m_SceneHierarchyTree.RemoveEntry(path);

			}

			if (manageEntity->GetAction() == Events::ManageEntityAction::Create)
			{
				ECS::Entity entityToCreate = Scenes::SceneService::GetActiveScene()->GetEntityByUUID(manageEntity->GetEntityID());
				if (!entityToCreate)
				{
					KG_WARN("Could not locate entity by UUID");
					return false;
				}

				EditorUI::TreePath sceneEntryPath;
				sceneEntryPath.AddNode(0);
				EditorUI::TreeEntry* sceneEntryRef{ m_SceneHierarchyTree.GetEntryFromPath(sceneEntryPath) };
				KG_ASSERT(sceneEntryRef);

				// Add new entity to the scene
				CreateSceneEntityInTree(entityToCreate, *sceneEntryRef);
			}
			
		}
		return false;
	}
	bool SceneEditorPanel::OnAssetEvent(Events::Event* event)
	{
		
		Events::ManageAsset* manageAsset = (Events::ManageAsset*)event;
		if (manageAsset->GetAssetType() == Assets::AssetType::ProjectComponent && 
			manageAsset->GetAction() == Events::ManageAssetAction::Create)
		{
			InitializeProjectComponent(manageAsset->GetAssetID());
			SetSelectedEntity({});
		}

		if (manageAsset->GetAssetType() == Assets::AssetType::Script &&
			manageAsset->GetAction() == Events::ManageAssetAction::PreDelete)
		{
			if (m_SelectOnUpdateScript.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_SelectOnUpdateScript.m_CurrentOption = { "None", Assets::EmptyHandle };
			}

			if (m_SelectOnCreateScript.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_SelectOnCreateScript.m_CurrentOption = { "None", Assets::EmptyHandle };
			}

			if (m_SelectRigidBody2DCollisionStartScript.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_SelectRigidBody2DCollisionStartScript.m_CurrentOption = { "None", Assets::EmptyHandle };
			}

			if (m_SelectRigidBody2DCollisionEndScript.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_SelectRigidBody2DCollisionEndScript.m_CurrentOption = { "None", Assets::EmptyHandle };
			}
		}

		if (manageAsset->GetAssetType() == Assets::AssetType::AIState &&
			manageAsset->GetAction() == Events::ManageAssetAction::PreDelete)
		{
			if (m_SelectCurrentState.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_SelectCurrentState.m_CurrentOption = { "None", Assets::EmptyHandle };
			}

			if (m_SelectGlobalState.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_SelectGlobalState.m_CurrentOption = { "None", Assets::EmptyHandle };
			}

			if (m_SelectPreviousState.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_SelectPreviousState.m_CurrentOption = { "None", Assets::EmptyHandle };
			}
		}

		// Remove emitter config from editor UI
		if (manageAsset->GetAssetType() == Assets::AssetType::EmitterConfig &&
			manageAsset->GetAction() == Events::ManageAssetAction::PreDelete)
		{
			if (m_SelectParticleEmitter.m_CurrentOption.m_Handle == manageAsset->GetAssetID())
			{
				m_SelectParticleEmitter.m_CurrentOption = { "None", Assets::EmptyHandle };
			}

		}

		if (manageAsset->GetAssetType() == Assets::AssetType::ProjectComponent &&
			manageAsset->GetAction() == Events::ManageAssetAction::UpdateAsset)
		{
			Ref<ECS::ProjectComponent> currentComponent = Assets::AssetService::GetProjectComponent(manageAsset->GetAssetID());

			if (currentComponent->m_DataLocations.size() > 0)
			{
				m_AllProjectComponents.erase(manageAsset->GetAssetID());
				InitializeProjectComponent(manageAsset->GetAssetID());
			}
			else
			{
				m_AllProjectComponents.erase(manageAsset->GetAssetID());

				// Get all tree nodes that contain the provided project component
				std::vector<EditorUI::TreePath> entriesToRemove = m_SceneHierarchyTree.SearchDepth([&](EditorUI::TreeEntry& entry)
				{
					SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)entry.m_ProvidedData.get();
					if (entryData.m_ComponentType == ECS::ComponentType::ProjectComponent &&
						entryData.m_ProjectComponentHandle == manageAsset->GetAssetID())
					{
						return true;
					}
					return false;
				},
				2);

				// Remove all of those tree entries
				for (EditorUI::TreePath& entryPath : entriesToRemove)
				{
					m_SceneHierarchyTree.RemoveEntry(entryPath);
				}
			}
			m_SceneHierarchyTree.SelectFirstEntry();
			SetSelectedEntity({});
		}

		if (manageAsset->GetAssetType() == Assets::AssetType::ProjectComponent &&
			manageAsset->GetAction() == Events::ManageAssetAction::PreDelete)
		{
			m_AllProjectComponents.erase(manageAsset->GetAssetID());

			// Get all tree nodes that contain the provided project component
			std::vector<EditorUI::TreePath> entriesToRemove = m_SceneHierarchyTree.SearchDepth([&](EditorUI::TreeEntry& entry) 
			{
				SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)entry.m_ProvidedData.get();
				if (entryData.m_ComponentType == ECS::ComponentType::ProjectComponent &&
					entryData.m_ProjectComponentHandle == manageAsset->GetAssetID())
				{
					return true;
				}
				return false;
			}, 
			2);

			// Remove all of those tree entries
			for (EditorUI::TreePath& entryPath : entriesToRemove)
			{
				m_SceneHierarchyTree.RemoveEntry(entryPath);
			}
			m_SceneHierarchyTree.SelectFirstEntry();
			SetSelectedEntity({});
		}
		
		return false;
	}
	bool SceneEditorPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		switch (event.GetKeyCode())
		{
			case Key::Escape:
			{
				s_MainWindow->m_SceneEditorPanel->SetSelectedEntity({});
				return true;
			}
			default:
			{
				return false;
			}
		}
	}
	void SceneEditorPanel::SetSelectedEntity(ECS::Entity entity)
	{
		*Scenes::SceneService::GetActiveScene()->GetSelectedEntity() = entity;
		if (!entity)
		{
			m_SceneHierarchyTree.m_SelectedEntry = {};
			m_CurrentDisplayed = ScenePropertiesDisplay::None;
		}
		if (entity)
		{
			// Search for indicated entity
			EditorUI::TreeEntry* entityEntry{ m_SceneHierarchyTree.SearchDepth((uint64_t)entity, 1) };
			KG_ASSERT(entityEntry);
			EditorUI::TreePath entityPath{ m_SceneHierarchyTree.GetPathFromEntryReference(entityEntry) };

			if (entityPath)
			{
				m_SceneHierarchyTree.m_SelectedEntry = entityPath;
				m_CurrentDisplayed = ScenePropertiesDisplay::Entity;
			}
			else
			{
				KG_WARN("Failed to locate entity inside tree");
				m_CurrentDisplayed = ScenePropertiesDisplay::None;
			}
		}

		RefreshTransformComponent();
		s_MainWindow->m_ShowProperties = true;
		EditorUI::EditorUIService::BringWindowToFront(s_MainWindow->m_PropertiesPanel->m_PanelName);
		s_MainWindow->m_PropertiesPanel->m_ActiveParent = m_PanelName;
		
	}
	void SceneEditorPanel::RefreshTransformComponent()
	{
		ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		if (!entity)
		{
			return;
		}
		if (entity.HasComponent<ECS::TransformComponent>())
		{
			ECS::TransformComponent& transformComp = entity.GetComponent<ECS::TransformComponent>();
			m_TransformEditTranslation.m_CurrentVec3 = transformComp.Translation;
			m_TransformEditRotation.m_CurrentVec3 = transformComp.Rotation;
			m_TransformEditScale.m_CurrentVec3 = transformComp.Scale;
		}
	}

	void SceneEditorPanel::DrawAllComponents(ECS::Entity entity)
	{
		DrawTagComponent(entity);
		DrawTransformComponent(entity);
		DrawRigidbody2DComponent(entity);
		DrawBoxCollider2DComponent(entity);
		DrawOnUpdateComponent(entity);
		DrawOnCreateComponent(entity);
		DrawAIStateComponent(entity);
		DrawCircleCollider2DComponent(entity);
		DrawCameraComponent(entity);
		DrawParticleEmitterComponent(entity);
		DrawShapeComponent(entity);
		for (auto& [handle, asset] : Assets::AssetService::GetProjectComponentRegistry())
		{
			DrawProjectComponent(entity, handle);
		}
	}
	void SceneEditorPanel::DrawSingleComponent(ECS::Entity entity)
	{
		switch (m_DisplayedComponent)
		{
		case ECS::ComponentType::Tag:
			DrawTagComponent(entity);
			return;
		case ECS::ComponentType::Transform:
			DrawTransformComponent(entity);
			return;
		case ECS::ComponentType::Rigidbody2D:
			DrawRigidbody2DComponent(entity);
			return;
		case ECS::ComponentType::BoxCollider2D:
			DrawBoxCollider2DComponent(entity);
			return;
		case ECS::ComponentType::CircleCollider2D:
			DrawCircleCollider2DComponent(entity);
			return;
		case ECS::ComponentType::Camera:
			DrawCameraComponent(entity);
			return;
		case ECS::ComponentType::ParticleEmitter:
			DrawParticleEmitterComponent(entity);
			return;
		case ECS::ComponentType::OnUpdate:
			DrawOnUpdateComponent(entity);
			return;
		case ECS::ComponentType::OnCreate:
			DrawOnCreateComponent(entity);
			return;
		case ECS::ComponentType::AIState:
			DrawAIStateComponent(entity);
			return;
		case ECS::ComponentType::Shape:
			DrawShapeComponent(entity);
			return;
		case ECS::ComponentType::ProjectComponent:
			DrawProjectComponent(entity, m_DisplayedProjectComponentHandle);
			return;
		

		case ECS::ComponentType::None:
		case ECS::ComponentType::Network:
		default:
			KG_ERROR("Undefined component type provided")
		}
	}
	void SceneEditorPanel::DrawTagComponent(ECS::Entity entity)
	{
		if (!entity.HasComponent<ECS::TagComponent>())
		{
			return;
		}
		ECS::TagComponent& component = entity.GetComponent<ECS::TagComponent>();
		m_TagHeader.RenderHeader();
		if (m_TagHeader.m_Expanded)
		{
			m_TagEdit.m_CurrentOption = component.Tag;
			m_TagEdit.RenderText();
			m_TagGroupEdit.m_CurrentOption = component.Group;
			m_TagGroupEdit.RenderText();
		}
	}
	void SceneEditorPanel::DrawTransformComponent(ECS::Entity entity)
	{
		if (!entity.HasComponent<ECS::TransformComponent>())
		{
			return;
		}
		ECS::TransformComponent& component = entity.GetComponent<ECS::TransformComponent>();
		m_TransformHeader.RenderHeader();
		if (m_TransformHeader.m_Expanded)
		{
			m_TransformEditTranslation.m_CurrentVec3 = component.Translation;
			EditorUI::EditorUIService::EditVec3(m_TransformEditTranslation);
			m_TransformEditScale.m_CurrentVec3 = component.Scale;
			EditorUI::EditorUIService::EditVec3(m_TransformEditScale);
			m_TransformEditRotation.m_CurrentVec3 = component.Rotation;
			EditorUI::EditorUIService::EditVec3(m_TransformEditRotation);
		}
	}
	void SceneEditorPanel::DrawRigidbody2DComponent(ECS::Entity entity)
	{
		if (!entity.HasComponent<ECS::Rigidbody2DComponent>())
		{
			return;
		}
		ECS::Rigidbody2DComponent& component = entity.GetComponent<ECS::Rigidbody2DComponent>();
		m_Rigidbody2DHeader.RenderHeader();
		if (m_Rigidbody2DHeader.m_Expanded)
		{
			m_Rigidbody2DType.m_SelectedOption = component.Type == ECS::Rigidbody2DComponent::BodyType::Static ?
				0 : 1;
			m_Rigidbody2DType.RenderRadio();
			m_RigidBody2DFixedRotation.m_CurrentBoolean = component.FixedRotation;
			m_RigidBody2DFixedRotation.RenderCheckbox();

			// Display collision script functions
			Ref<Scripting::Script> collisionStartScript = Assets::AssetService::GetScript(component.OnCollisionStartScriptHandle);
			m_SelectRigidBody2DCollisionStartScript.m_CurrentOption = component.OnCollisionStartScriptHandle == Assets::EmptyHandle ?
				EditorUI::OptionEntry("None", Assets::EmptyHandle) :
				EditorUI::OptionEntry(Utility::ScriptToString(collisionStartScript).c_str(), component.OnCollisionStartScriptHandle);
			m_SelectRigidBody2DCollisionStartScript.RenderOptions();

			Ref<Scripting::Script> collisionEndScript = Assets::AssetService::GetScript(component.OnCollisionEndScriptHandle);
			m_SelectRigidBody2DCollisionEndScript.m_CurrentOption = component.OnCollisionEndScriptHandle == Assets::EmptyHandle ?
				EditorUI::OptionEntry("None", Assets::EmptyHandle) :
				EditorUI::OptionEntry(Utility::ScriptToString(collisionEndScript).c_str(), component.OnCollisionEndScriptHandle);
			m_SelectRigidBody2DCollisionEndScript.RenderOptions();
		}
	}
	void SceneEditorPanel::DrawBoxCollider2DComponent(ECS::Entity entity)
	{
		if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
		{
			return;
		}
		ECS::BoxCollider2DComponent& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
		m_BoxCollider2DHeader.RenderHeader();
		if (m_BoxCollider2DHeader.m_Expanded)
		{
			m_BoxColliderOffset.m_CurrentVec2 = component.Offset;
			EditorUI::EditorUIService::EditVec2(m_BoxColliderOffset);
			m_BoxColliderSize.m_CurrentVec2 = component.Size;
			EditorUI::EditorUIService::EditVec2(m_BoxColliderSize);
			m_BoxColliderDensity.m_CurrentFloat = component.Density;
			EditorUI::EditorUIService::EditFloat(m_BoxColliderDensity);
			m_BoxColliderFriction.m_CurrentFloat = component.Friction;
			EditorUI::EditorUIService::EditFloat(m_BoxColliderFriction);
			m_BoxColliderRestitution.m_CurrentFloat = component.Restitution;
			EditorUI::EditorUIService::EditFloat(m_BoxColliderRestitution);
			m_BoxColliderRestitutionThreshold.m_CurrentFloat = component.RestitutionThreshold;
			EditorUI::EditorUIService::EditFloat(m_BoxColliderRestitutionThreshold);
			m_BoxColliderIsSensor.m_CurrentBoolean = component.IsSensor;
			m_BoxColliderIsSensor.RenderCheckbox();

		}
		
	}
	void SceneEditorPanel::DrawCircleCollider2DComponent(ECS::Entity entity)
	{
		if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
		{
			return;
		}
		ECS::CircleCollider2DComponent& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
		m_CircleCollider2DHeader.RenderHeader();
		if (m_CircleCollider2DHeader.m_Expanded)
		{
			m_CircleColliderOffset.m_CurrentVec2 = component.Offset;
			EditorUI::EditorUIService::EditVec2(m_CircleColliderOffset);
			m_CircleColliderRadius.m_CurrentFloat = component.Radius;
			EditorUI::EditorUIService::EditFloat(m_CircleColliderRadius);
			m_CircleColliderDensity.m_CurrentFloat = component.Density;
			EditorUI::EditorUIService::EditFloat(m_CircleColliderDensity);
			m_CircleColliderFriction.m_CurrentFloat = component.Friction;
			EditorUI::EditorUIService::EditFloat(m_CircleColliderFriction);
			m_CircleColliderRestitution.m_CurrentFloat = component.Restitution;
			EditorUI::EditorUIService::EditFloat(m_CircleColliderRestitution);
			m_CircleColliderRestitutionThreshold.m_CurrentFloat = component.RestitutionThreshold;
			EditorUI::EditorUIService::EditFloat(m_CircleColliderRestitutionThreshold);
			m_CircleColliderIsSensor.m_CurrentBoolean = component.IsSensor;
			m_CircleColliderIsSensor.RenderCheckbox();
		}
		
	}
	void SceneEditorPanel::DrawCameraComponent(ECS::Entity entity)
	{
		if (!entity.HasComponent<ECS::CameraComponent>())
		{
			return;
		}
		ECS::CameraComponent& component = entity.GetComponent<ECS::CameraComponent>();
		m_CameraHeader.RenderHeader();
		if (m_CameraHeader.m_Expanded)
		{
			m_CameraPrimary.m_CurrentBoolean = component.Primary;
			m_CameraPrimary.RenderCheckbox();
			m_CameraProjection.m_SelectedOption = component.Camera.GetProjectionType() ==
				Scenes::SceneCamera::ProjectionType::Perspective ? 0 : 1;
			m_CameraProjection.RenderRadio();

			if (component.Camera.GetProjectionType() == Scenes::SceneCamera::ProjectionType::Perspective)
			{
				m_CameraPerspectiveFOV.m_CurrentFloat = component.Camera.GetPerspectiveVerticalFOV();
				EditorUI::EditorUIService::EditFloat(m_CameraPerspectiveFOV);
				m_CameraPerspectiveNearPlane.m_CurrentFloat = component.Camera.GetPerspectiveNearClip();
				EditorUI::EditorUIService::EditFloat(m_CameraPerspectiveNearPlane);
				m_CameraPerspectiveFarPlane.m_CurrentFloat = component.Camera.GetPerspectiveFarClip();
				EditorUI::EditorUIService::EditFloat(m_CameraPerspectiveFarPlane);
			}
			else
			{
				m_CameraOrthographicSize.m_CurrentFloat = component.Camera.GetOrthographicSize();
				EditorUI::EditorUIService::EditFloat(m_CameraOrthographicSize);
				m_CameraOrthographicNearPlane.m_CurrentFloat = component.Camera.GetOrthographicNearClip();
				EditorUI::EditorUIService::EditFloat(m_CameraOrthographicNearPlane);
				m_CameraOrthographicFarPlane.m_CurrentFloat = component.Camera.GetOrthographicFarClip();
				EditorUI::EditorUIService::EditFloat(m_CameraOrthographicFarPlane);
			}
		}
		
	}
	void SceneEditorPanel::DrawParticleEmitterComponent(ECS::Entity entity)
	{
		if (!entity.HasComponent<ECS::ParticleEmitterComponent>())
		{
			return;
		}
		ECS::ParticleEmitterComponent& component = entity.GetComponent<ECS::ParticleEmitterComponent>();
		m_ParticleEmitterHeader.RenderHeader();
		if (m_ParticleEmitterHeader.m_Expanded)
		{
			Ref<Particles::EmitterConfig> emitterConfig = Assets::AssetService::GetEmitterConfig(component.m_EmitterConfigHandle);
			Assets::AssetInfo emitterInfo = Assets::AssetService::GetEmitterConfigInfo(component.m_EmitterConfigHandle);
			m_SelectParticleEmitter.m_CurrentOption = component.m_EmitterConfigHandle == Assets::EmptyHandle ?
				EditorUI::OptionEntry("None", Assets::EmptyHandle) :
				EditorUI::OptionEntry(emitterInfo.Data.FileLocation.filename().string().c_str(), component.m_EmitterConfigHandle);
			m_SelectParticleEmitter.RenderOptions();
		}
	}
	void SceneEditorPanel::DrawOnUpdateComponent(ECS::Entity entity)
	{
		if (!entity.HasComponent<ECS::OnUpdateComponent>())
		{
			return;
		}
		ECS::OnUpdateComponent& component = entity.GetComponent<ECS::OnUpdateComponent>();
		m_OnUpdateHeader.RenderHeader();
		if (m_OnUpdateHeader.m_Expanded)
		{
			Ref<Scripting::Script> script = Assets::AssetService::GetScript(component.OnUpdateScriptHandle);
			m_SelectOnUpdateScript.m_CurrentOption = component.OnUpdateScriptHandle == Assets::EmptyHandle ? 
				EditorUI::OptionEntry( "None", Assets::EmptyHandle ) :
				EditorUI::OptionEntry(Utility::ScriptToString(script).c_str(), component.OnUpdateScriptHandle);
			m_SelectOnUpdateScript.RenderOptions();
		}
	}
	void SceneEditorPanel::DrawAIStateComponent(ECS::Entity entity)
	{
		if (!entity.HasComponent<ECS::AIStateComponent>())
		{
			return;
		}
		ECS::AIStateComponent& component = entity.GetComponent<ECS::AIStateComponent>();
		m_AIStateHeader.RenderHeader();
		if (m_AIStateHeader.m_Expanded)
		{
			// Select global state
			bool optionValid = component.GlobalStateHandle != Assets::EmptyHandle;
			if (optionValid)
			{
				Assets::AssetInfo& globalAsset = Assets::AssetService::GetAIStateRegistry().at(component.GlobalStateHandle);
				m_SelectGlobalState.m_CurrentOption = { globalAsset.Data.FileLocation.filename().string().c_str(), component.GlobalStateHandle };
			}
			else
			{
				m_SelectGlobalState.m_CurrentOption = { "None", Assets::EmptyHandle };
			}
			m_SelectGlobalState.RenderOptions();

			// Select current state
			optionValid = component.CurrentStateHandle != Assets::EmptyHandle;
			if (optionValid)
			{
				Assets::AssetInfo& currentAsset = Assets::AssetService::GetAIStateRegistry().at(component.CurrentStateHandle);
				m_SelectCurrentState.m_CurrentOption = { currentAsset.Data.FileLocation.filename().string().c_str(), component.CurrentStateHandle };
			}
			else
			{
				m_SelectCurrentState.m_CurrentOption = { "None", Assets::EmptyHandle };
			}
			m_SelectCurrentState.RenderOptions();

			// Select previous state
			optionValid = component.PreviousStateHandle != Assets::EmptyHandle;
			if (optionValid)
			{
				Assets::AssetInfo& previousAsset = Assets::AssetService::GetAIStateRegistry().at(component.PreviousStateHandle);
				m_SelectPreviousState.m_CurrentOption = { previousAsset.Data.FileLocation.filename().string().c_str(), component.PreviousStateHandle };
			}
			else
			{
				m_SelectPreviousState.m_CurrentOption = { "None", Assets::EmptyHandle };
			}
			m_SelectPreviousState.RenderOptions();
		}
	}
	void SceneEditorPanel::DrawOnCreateComponent(ECS::Entity entity)
	{
		if (!entity.HasComponent<ECS::OnCreateComponent>())
		{
			return;
		}
		ECS::OnCreateComponent& component = entity.GetComponent<ECS::OnCreateComponent>();
		m_OnCreateHeader.RenderHeader();
		if (m_OnCreateHeader.m_Expanded)
		{
			Ref<Scripting::Script> script = Assets::AssetService::GetScript(component.OnCreateScriptHandle);
			m_SelectOnCreateScript.m_CurrentOption = component.OnCreateScriptHandle == Assets::EmptyHandle ?
				EditorUI::OptionEntry("None", Assets::EmptyHandle) :
				EditorUI::OptionEntry(Utility::ScriptToString(script).c_str(), component.OnCreateScriptHandle);
			m_SelectOnCreateScript.RenderOptions();
		}
	}
	void SceneEditorPanel::DrawShapeComponent(ECS::Entity entity)
	{
		if (!entity.HasComponent<ECS::ShapeComponent>())
		{
			return;
		}
		ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
		m_ShapeHeader.RenderHeader();
		if (m_ShapeHeader.m_Expanded)
		{
			m_ShapeSelect.m_CurrentOption = { Utility::ShapeTypeToString(component.CurrentShape), Assets::EmptyHandle };
			m_ShapeSelect.RenderOptions();

			// This section displays the shader specification options available for the chosen object
			if (component.CurrentShape == Rendering::ShapeTypes::None)
			{
				return;
			}
			if (component.CurrentShape == Rendering::ShapeTypes::Quad)
			{
				DrawShapeComponentColor();
				DrawShapeComponentTexture();
				DrawShapeComponentCircle();
				DrawShapeComponentProjection();
				DrawShapeComponentEntityID();

			}
			if (component.CurrentShape == Rendering::ShapeTypes::Cube || component.CurrentShape == Rendering::ShapeTypes::Pyramid)
			{
				DrawShapeComponentColor();
				DrawShapeComponentTexture();
				DrawShapeComponentProjection();
				DrawShapeComponentEntityID();
			}
		}
	}

	struct DrawProjectComponentFieldsVisitor
	{
		void operator()(EditorUI::CheckboxWidget& spec)
		{
			// Get component data pointer
			ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.m_ProvidedData.get();
			Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
			ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

			// Get field data pointer
			uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

			// Set the data
			spec.m_CurrentBoolean = *(bool*)fieldDataRef;
			
			// Display Checkbox
			spec.RenderCheckbox();
		}
		void operator()(EditorUI::EditTextSpec& spec)
		{
			// Get component data pointer
			ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.m_ProvidedData.get();
			Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
			ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

			// Get field data pointer
			uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

			// Set the data
			spec.m_CurrentOption = *(std::string*)fieldDataRef;

			// Display Text
			spec.RenderText();
		}
		void operator()(EditorUI::EditIntegerWidget& spec)
		{
			// Get component data pointer
			ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.m_ProvidedData.get();
			Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
			ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

			// Get field data pointer
			uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

			// Get field type
			WrappedVarType fieldType = projectComponentRef->m_DataTypes.at(projectCompFieldInfo.m_FieldSlot);

			// Set the data
			switch (fieldType)
			{
			case WrappedVarType::Integer32:
				spec.m_CurrentInteger = *(int32_t*)fieldDataRef;
				break;
			case WrappedVarType::UInteger16:
				spec.m_CurrentInteger = (int32_t)*(uint16_t*)fieldDataRef;
				break;
			case WrappedVarType::UInteger32:
				spec.m_CurrentInteger = (int32_t)*(uint32_t*)fieldDataRef;
				break;
			case WrappedVarType::UInteger64:
				spec.m_CurrentInteger = (int32_t)*(uint64_t*)fieldDataRef;
				break;
			default:
				KG_ERROR("Invalid type provide when processing integers in editor ui");
				break;
			}
		
			EditorUI::EditorUIService::EditInteger(spec);
		}
		void operator()(EditorUI::EditFloatWidget& spec)
		{
			// Get component data pointer
			ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.m_ProvidedData.get();
			Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
			ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

			// Get field data pointer
			uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

			// Set the data
			spec.m_CurrentFloat = *(float*)fieldDataRef;

			// Dispaly float
			EditorUI::EditorUIService::EditFloat(spec);
		}
		void operator()(EditorUI::EditVec2Widget& spec)
		{
			// TODO: Add support
			EditorUI::EditorUIService::EditVec2(spec);
		}
		void operator()(EditorUI::EditVec3Widget& spec)
		{
			// Get component data pointer
			ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.m_ProvidedData.get();
			Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
			ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

			// Get field data pointer
			uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

			// Set the data
			spec.m_CurrentVec3 = *(Math::vec3*)fieldDataRef;

			// Display vector 3
			EditorUI::EditorUIService::EditVec3(spec);
		}
		void operator()(EditorUI::EditVec4Widget& spec)
		{
			// TODO: Add support
			EditorUI::EditorUIService::EditVec4(spec);
		}
	};

	void SceneEditorPanel::DrawProjectComponent(ECS::Entity entity, Assets::AssetHandle componentHandle)
	{
		if (!entity.HasProjectComponentData(componentHandle))
		{
			return;
		}

		ProjectComponentWidgetData& activeWidgetData = m_AllProjectComponents.at(componentHandle);
		activeWidgetData.m_Header.RenderHeader();

		// Display all component fields
		if (activeWidgetData.m_Header.m_Expanded)
		{
			for (size_t iteration{ 0 }; iteration < activeWidgetData.m_Fields.size(); iteration++)
			{
				std::visit(DrawProjectComponentFieldsVisitor{}, activeWidgetData.m_Fields.at(iteration));
			}
		}
	}
	void SceneEditorPanel::DrawSceneOptions()
	{
		Ref<Scenes::Scene> editorScene{ s_MainWindow->m_EditorScene };
		KG_ASSERT(editorScene);
		// Draw background color option
		m_BackgroundColorSpec.m_CurrentVec4 = editorScene->m_BackgroundColor;
		EditorUI::EditorUIService::EditVec4(m_BackgroundColorSpec);
		// Draw gravity option
		m_Gravity2DSpec.m_CurrentVec2 = editorScene->GetPhysicsSpecification().Gravity;
		EditorUI::EditorUIService::EditVec2(m_Gravity2DSpec);
	}
	void SceneEditorPanel::UpdateShapeComponent()
	{
		ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
		// Get Previous Buffer and Previous Shader
		Buffer oldBuffer = component.ShaderData;
		Ref<Rendering::Shader> oldShader = component.Shader;
		// Get New Shader
		auto [newShaderAssetHandle, newShader] = Assets::AssetService::GetShader(component.ShaderSpecification);
		// Assign New Shader to Component
		component.ShaderHandle = newShaderAssetHandle;
		component.Shader = newShader;
		// Create New Buffer with New ShaderLayout Size and Set Entire Buffer to Zero
		Buffer newBuffer(newShader->GetInputLayout().GetStride() * sizeof(uint8_t));
		newBuffer.SetDataToByte(0);

		// Transfer Data from Old Buffer to New Buffer if applicable
		// This for loop checks if each element in the old shader exists in the new shader.
		// If an element does exist in both shaders, the data from the old buffer is
		// transferred to the new buffer!
		for (const auto& element : oldShader->GetInputLayout().GetElements())
		{
			if (newShader->GetInputLayout().FindElementByName(Utility::FileSystem::CRCFromString(element.Name.c_str())))
			{
				// Get Location of Old Data Pointer
				std::size_t oldLocation = element.Offset;
				uint8_t* oldLocationPointer = oldBuffer.As<uint8_t>(oldLocation);

				// Get Location of New Data Pointer
				uint8_t* newLocationPointer = Rendering::Shader::GetInputLocation<uint8_t>(
					Utility::FileSystem::CRCFromString(element.Name.c_str()),
					newBuffer, newShader);

				// Get Size of Data to Transfer
				std::size_t size = element.Size;

				// Final Memory Copy
				memcpy(newLocationPointer, oldLocationPointer, size);
			}
		}

		// Assign and Zero Out New Buffer
		component.ShaderData = newBuffer;

		// Clear old buffer
		if (oldBuffer)
		{
			oldBuffer.Release();
		}
	}
	void SceneEditorPanel::DrawShapeComponentColor()
	{
		ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
		m_ShapeColorType.m_CurrentOption = { Utility::ColorInputTypeToString(component.ShaderSpecification.ColorInput), Assets::EmptyHandle };
		m_ShapeColorType.RenderOptions();

		if (component.ShaderSpecification.ColorInput == Rendering::ColorInputType::None) { return; }

		if (component.ShaderSpecification.ColorInput == Rendering::ColorInputType::FlatColor)
		{
			m_ShapeColor.m_CurrentVec4 = *(Rendering::Shader::GetInputLocation<Math::vec4>(
				Utility::FileSystem::CRCFromString("a_Color"), 
				component.ShaderData, component.Shader));
			EditorUI::EditorUIService::EditVec4(m_ShapeColor);
		}
		if (component.ShaderSpecification.ColorInput == Rendering::ColorInputType::VertexColor)
		{
			uint32_t iterator{ 1 };
			for (Math::vec4& color : *component.VertexColors)
			{
				ImGui::SetCursorPosX(30.0f);
				ImGui::ColorEdit4(("Vertex " + std::to_string(iterator)).c_str(), glm::value_ptr(color));
				iterator++;
			}
		}
	}
	void SceneEditorPanel::DrawShapeComponentTexture()
	{
		ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
		m_ShapeAddTexture.m_CurrentBoolean = component.ShaderSpecification.TextureInput == Rendering::TextureInputType::ColorTexture ? true : false;
		m_ShapeAddTexture.RenderCheckbox();
		if (m_ShapeAddTexture.m_CurrentBoolean)
		{
			if (component.TextureHandle == Assets::EmptyHandle)
			{
				m_ShapeSetTexture.m_CurrentOption = { "None", Assets::EmptyHandle };
			}
			else
			{
				m_ShapeSetTexture.m_CurrentOption =
				{
					Assets::AssetService::GetTexture2DRegistry().at(component.TextureHandle).Data.FileLocation.filename().string().c_str(),
					component.TextureHandle
				};
			}
			m_ShapeSetTexture.RenderOptions();

			m_ShapeTilingFactor.m_CurrentFloat = *Rendering::Shader::GetInputLocation<float>(
				Utility::FileSystem::CRCFromString("a_TilingFactor"), 
				component.ShaderData, component.Shader);
			EditorUI::EditorUIService::EditFloat(m_ShapeTilingFactor);
		}
	}
	void SceneEditorPanel::DrawShapeComponentCircle()
	{
		ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
		m_ShapeAddCircle.m_CurrentBoolean = component.ShaderSpecification.AddCircleShape;
		m_ShapeAddCircle.RenderCheckbox();
		if (component.ShaderSpecification.AddCircleShape)
		{
			m_ShapeCircleThickness.m_CurrentFloat = *Rendering::Shader::GetInputLocation<float>(
				Utility::FileSystem::CRCFromString("a_Thickness"), 
				component.ShaderData, component.Shader);
			EditorUI::EditorUIService::EditFloat(m_ShapeCircleThickness);

			m_ShapeCircleFade.m_CurrentFloat = *Rendering::Shader::GetInputLocation<float>(\
				Utility::FileSystem::CRCFromString("a_Fade"), 
				component.ShaderData, component.Shader);
			EditorUI::EditorUIService::EditFloat(m_ShapeCircleFade);
		}
	}
	void SceneEditorPanel::DrawShapeComponentProjection()
	{
		ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
		m_ShapeAddProjection.m_CurrentBoolean = component.ShaderSpecification.AddProjectionMatrix;
		m_ShapeAddProjection.RenderCheckbox();
	}
	void SceneEditorPanel::DrawShapeComponentEntityID()
	{
		ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
		m_ShapeAddEntityID.m_CurrentBoolean = component.ShaderSpecification.AddEntityID;
		m_ShapeAddEntityID.RenderCheckbox();
	}
	void SceneEditorPanel::CreateSceneEntityInTree(ECS::Entity entity, EditorUI::TreeEntry& sceneEntry)
	{
		EditorUI::TreeEntry newEntry{};
		newEntry.m_Label = entity.GetComponent<ECS::TagComponent>().Tag;
		newEntry.m_IconHandle = EditorUI::EditorUIService::s_IconEntity;
		newEntry.m_Handle = (uint64_t)entity;
		newEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
			{
				ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
				s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
				s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::None);
			};
		newEntry.m_OnDoubleLeftClick = [](EditorUI::TreeEntry& entry)
			{
				ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
				Rendering::EditorPerspectiveCamera& editorCamera = s_MainWindow->m_ViewportPanel->m_EditorCamera;
				ECS::TransformComponent& transformComponent = entity.GetComponent<ECS::TransformComponent>();
				editorCamera.SetFocalPoint(transformComponent.Translation);
				editorCamera.SetDistance(std::max({ transformComponent.Scale.x, transformComponent.Scale.y, transformComponent.Scale.z }) * 2.5f);
				editorCamera.SetMovementType(Rendering::EditorPerspectiveCamera::MovementType::ModelView);
			};

		newEntry.m_OnRightClick = [&](EditorUI::TreeEntry& entry) 
		{
			m_SelectTooltip.ClearEntries();

			// Create add component entry
			EditorUI::TooltipEntry addComponentEntry{ "Add Component", [&](EditorUI::TooltipEntry& tooltipEntry) 
			{
				m_AddComponent.m_OpenPopup = true;
				m_AddComponentEntity = (int32_t)tooltipEntry.m_UserHandle;
			}};
			addComponentEntry.m_UserHandle = entry.m_Handle;

			// Add the new entry
			m_SelectTooltip.AddTooltipEntry(addComponentEntry);

			// Create delete entity entry
			EditorUI::TooltipEntry deleteEntityEntry{ "Delete Entity", [&](EditorUI::TooltipEntry& tooltipEntry)
			{
				static ECS::Entity entityToDelete;
				entityToDelete = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));

				EngineService::GetActiveEngine().GetThread().SubmitFunction([&]()
				{
					if (!entityToDelete)
					{
						KG_WARN("Attempt to delete entity that does not exist");
						return;
					}
					Scenes::SceneService::GetActiveScene()->DestroyEntity(entityToDelete);
					s_MainWindow->LoadSceneParticleEmitters();
				});
			}};
			deleteEntityEntry.m_UserHandle = entry.m_Handle;

			// Add the new entry
			m_SelectTooltip.AddTooltipEntry(deleteEntityEntry);

			// Active the tooltip
			m_SelectTooltip.m_TooltipActive = true;
		};

		EditorUI::TreeEntry componentEntry{};
		componentEntry.m_Handle = (uint64_t)entity;
		if (entity.HasComponent<ECS::TagComponent>())
		{
			componentEntry.m_Label = "Tag";
			componentEntry.m_ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::Tag, Assets::EmptyHandle);
			componentEntry.m_IconHandle = EditorUI::EditorUIService::s_IconTag;
			componentEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
					s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::Tag);
				};
			newEntry.m_SubEntries.push_back(componentEntry);
		}
		if (entity.HasComponent<ECS::TransformComponent>())
		{
			componentEntry.m_Label = "Transform";
			componentEntry.m_ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::Transform, Assets::EmptyHandle);
			componentEntry.m_IconHandle = EditorUI::EditorUIService::s_IconTransform;
			componentEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
					s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::Transform);
				};
			newEntry.m_SubEntries.push_back(componentEntry);
		}

		if (entity.HasComponent<ECS::Rigidbody2DComponent>())
		{
			componentEntry.m_Label = "Rigid Body 2D";
			componentEntry.m_ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::Rigidbody2D, Assets::EmptyHandle);
			componentEntry.m_IconHandle = EditorUI::EditorUIService::s_IconRigidBody;
			componentEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
					s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::Rigidbody2D);
				};
			newEntry.m_SubEntries.push_back(componentEntry);
		}

		if (entity.HasComponent<ECS::BoxCollider2DComponent>())
		{
			componentEntry.m_Label = "Box Collider 2D";
			componentEntry.m_ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::BoxCollider2D, Assets::EmptyHandle);
			componentEntry.m_IconHandle = EditorUI::EditorUIService::s_IconBoxCollider;
			componentEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
					s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::BoxCollider2D);
				};
			newEntry.m_SubEntries.push_back(componentEntry);
		}

		if (entity.HasComponent<ECS::CircleCollider2DComponent>())
		{
			componentEntry.m_Label = "Circle Collider 2D";
			componentEntry.m_ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::CircleCollider2D, Assets::EmptyHandle);
			componentEntry.m_IconHandle = EditorUI::EditorUIService::s_IconCircleCollider;
			componentEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
					s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::CircleCollider2D);
				};
			newEntry.m_SubEntries.push_back(componentEntry);
		}

		if (entity.HasComponent<ECS::CameraComponent>())
		{
			componentEntry.m_Label = "Camera";
			componentEntry.m_ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::Camera, Assets::EmptyHandle);
			componentEntry.m_IconHandle = EditorUI::EditorUIService::s_IconCamera;
			componentEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
					s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::Camera);
				};
			newEntry.m_SubEntries.push_back(componentEntry);
		}

		if (entity.HasComponent<ECS::ParticleEmitterComponent>())
		{
			componentEntry.m_Label = "Particle Emitter";
			componentEntry.m_ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::ParticleEmitter, Assets::EmptyHandle);
			componentEntry.m_IconHandle = EditorUI::EditorUIService::s_IconParticles;
			componentEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
					s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::ParticleEmitter);
				};
			newEntry.m_SubEntries.push_back(componentEntry);
		}

		if (entity.HasComponent<ECS::ShapeComponent>())
		{
			componentEntry.m_Label = "Shape";
			componentEntry.m_ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::Shape, Assets::EmptyHandle);
			componentEntry.m_IconHandle = EditorUI::EditorUIService::s_IconEntity;
			componentEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
					s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::Shape);
				};
			newEntry.m_SubEntries.push_back(componentEntry);
		}

		if (entity.HasComponent<ECS::OnCreateComponent>())
		{
			componentEntry.m_Label = "On Create";
			componentEntry.m_ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::OnCreate, Assets::EmptyHandle);
			componentEntry.m_IconHandle = EditorUI::EditorUIService::s_IconFunction;
			componentEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
					s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::OnCreate);
				};
			newEntry.m_SubEntries.push_back(componentEntry);
		}

		if (entity.HasComponent<ECS::OnUpdateComponent>())
		{
			componentEntry.m_Label = "On Update";
			componentEntry.m_ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::OnUpdate, Assets::EmptyHandle);
			componentEntry.m_IconHandle = EditorUI::EditorUIService::s_IconFunction;
			componentEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
					s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::OnUpdate);
				};
			newEntry.m_SubEntries.push_back(componentEntry);
		}

		if (entity.HasComponent<ECS::AIStateComponent>())
		{
			componentEntry.m_Label = "AI State";
			componentEntry.m_ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::AIState, Assets::EmptyHandle);
			componentEntry.m_IconHandle = EditorUI::EditorUIService::s_IconAI;
			componentEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
					s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::AIState);
				};
			newEntry.m_SubEntries.push_back(componentEntry);
		}

		// Handle adding project components
		for (auto& [handle, asset] : Assets::AssetService::GetProjectComponentRegistry())
		{
			if (!entity.HasProjectComponentData(handle))
			{
				continue;
			}
			// Add component to entity & update tree
			Ref<ECS::ProjectComponent> component = Assets::AssetService::GetProjectComponent(handle);
			KG_ASSERT(component);
			componentEntry.m_Label = component->m_Name;
			componentEntry.m_ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::ProjectComponent, handle);
			componentEntry.m_IconHandle = EditorUI::EditorUIService::s_IconEntity;
			componentEntry.m_OnLeftClick = [](EditorUI::TreeEntry& entry)
			{
				ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));
				s_MainWindow->m_SceneEditorPanel->SetSelectedEntity(entity);
				s_MainWindow->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::ProjectComponent);
				SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)entry.m_ProvidedData.get();
				s_MainWindow->m_SceneEditorPanel->SetDisplayedProjectComponent(entryData.m_ProjectComponentHandle);
			};
			newEntry.m_SubEntries.push_back(componentEntry);
		}
		sceneEntry.m_SubEntries.push_back(newEntry);
	}
}
