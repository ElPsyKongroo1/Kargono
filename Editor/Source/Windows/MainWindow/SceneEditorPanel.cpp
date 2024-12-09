#include "kgpch.h"

#include "Windows/MainWindow/SceneEditorPanel.h"

#include "EditorApp.h"

#include "API/EditorUI/ImGuiBackendAPI.h"

static Kargono::EditorApp* s_EditorApp { nullptr };
static Kargono::Windows::MainWindow* s_MainWindow{ nullptr };

namespace Kargono::Panels
{
	void SceneEditorPanel::CreateSceneEntityInTree(ECS::Entity entity)
	{
		EditorUI::TreeEntry newEntry {};
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
			Rendering::EditorCamera& editorCamera = s_MainWindow->m_ViewportPanel->m_EditorCamera;
			ECS::TransformComponent& transformComponent = entity.GetComponent<ECS::TransformComponent>();
			editorCamera.SetFocalPoint(transformComponent.Translation);
			editorCamera.SetDistance(std::max({ transformComponent.Scale.x, transformComponent.Scale.y, transformComponent.Scale.z }) * 2.5f);
			editorCamera.SetMovementType(Rendering::EditorCamera::MovementType::ModelView);
		};

		newEntry.m_OnRightClickSelection.push_back({ "Add Component", [&](EditorUI::TreeEntry& entry)
		{
			m_AddComponent.m_OpenPopup = true;
			m_AddComponentEntity = (int32_t)entry.m_Handle;
		} });

		newEntry.m_OnRightClickSelection.push_back({ "Delete Entity", [](EditorUI::TreeEntry& entry)
		{
			static ECS::Entity entityToDelete;
			entityToDelete = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.m_Handle));

			EngineService::SubmitToMainThread([&]()
			{
				if (!entityToDelete)
				{
					KG_WARN("Attempt to delete entity that does not exist");
					return;
				}
				Scenes::SceneService::GetActiveScene()->DestroyEntity(entityToDelete);
			});

		} });

		EditorUI::TreeEntry componentEntry {};
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
			
		m_SceneHierarchyTree.InsertEntry(newEntry);
	}

	void SceneEditorPanel::InitializeSceneHierarchy()
	{
		m_MainSceneHeader.m_Label = "No Scene Name";
		m_MainSceneHeader.m_EditColorActive = false;
		m_MainSceneHeader.AddToSelectionList("Create New Scene", []()
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

		m_MainSceneHeader.AddToSelectionList("Create Entity", []()
		{
			Scenes::SceneService::GetActiveScene()->CreateEntity("Empty Entity");
		});
		

		m_SceneHierarchyTree.m_Label = "Scene Hierarchy";
		m_SceneHierarchyTree.m_OnRefresh = [&]()
		{
			if (Scenes::SceneService::GetActiveScene())
			{
				m_SceneHierarchyTree.ClearTree();
				Scenes::SceneService::GetActiveScene()->m_EntityRegistry.m_EnTTRegistry.each([&](auto enttID)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(enttID);
					CreateSceneEntityInTree(entity);
					
				});
			}
		};

		m_AddComponent.m_Label = "Add Component";
		m_AddComponent.m_Flags = EditorUI::SelectOption_PopupOnly;
		m_AddComponent.m_PopupAction = [&]()
		{
			ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity(m_AddComponentEntity));
			if (!entity)
			{
				KG_WARN("Attempt to add component to empty entity");
				return;
			}
			m_AddComponent.ClearOptions();
			m_AddComponent.AddToOptions("Clear", "None", Assets::EmptyHandle);
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				m_AddComponent.AddToOptions("Engine Component", "Camera", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<ECS::ShapeComponent>())
			{
				m_AddComponent.AddToOptions("Engine Component", "Shape", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<ECS::Rigidbody2DComponent>())
			{
				m_AddComponent.AddToOptions("Engine Component", "Rigidbody 2D", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				m_AddComponent.AddToOptions("Engine Component", "Box Collider 2D", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				m_AddComponent.AddToOptions("Engine Component", "Circle Collider 2D", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<ECS::OnCreateComponent>())
			{
				m_AddComponent.AddToOptions("Engine Component", "On Create", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<ECS::OnUpdateComponent>())
			{
				m_AddComponent.AddToOptions("Engine Component", "On Update", Assets::EmptyHandle);
			}

			if (!entity.HasComponent<ECS::AIStateComponent>())
			{
				m_AddComponent.AddToOptions("Engine Component", "AI State", Assets::EmptyHandle);
			}

			for (auto& [handle, asset] : Assets::AssetService::GetProjectComponentRegistry())
			{
				Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(handle);
				KG_ASSERT(projectComponentRef);
				if (!entity.HasProjectComponentData(handle) && projectComponentRef->m_BufferSize != 0)
				{
					m_AddComponent.AddToOptions("Project Component", asset.Data.GetSpecificMetaData<Assets::ProjectComponentMetaData>()->Name, handle);
				}
			}

			m_AddComponent.m_CurrentOption = { "None", Assets::EmptyHandle };
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
		 	EditorUI::TreeEntry* currentEntry = m_SceneHierarchyTree.SearchFirstLayer((uint64_t)entity);
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

	void SceneEditorPanel::InitializeTagComponent()
	{
		m_TagHeader.m_Label = "Tag";
		m_TagHeader.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_TagHeader.m_Expanded = true;

		m_TagEdit.m_Label = "Tag Label";
		m_TagEdit.m_Flags |= EditorUI::EditText_Indented;
		m_TagEdit.m_ConfirmAction = [&](EditorUI::EditTextSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (entity && entity.HasComponent<ECS::TagComponent>())
			{
				auto& component = entity.GetComponent<ECS::TagComponent>();
				component.Tag = m_TagEdit.m_CurrentOption;

				m_SceneHierarchyTree.EditDepth([](EditorUI::TreeEntry& entry) 
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID((entt::entity)(int32_t)entry.m_Handle);
					KG_ASSERT(entity);
					if (entity.GetUUID() == Scenes::SceneService::GetActiveScene()->GetSelectedEntity()->GetUUID())
					{
						entry.m_Label = entity.GetComponent<ECS::TagComponent>().Tag;
					}
				}, 0);
			}
		};

		m_TagGroupEdit.m_Label = "Tag Group";
		m_TagGroupEdit.m_Flags |= EditorUI::EditText_Indented;
		m_TagGroupEdit.m_ConfirmAction = [&](EditorUI::EditTextSpec& spec)
		{
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
		m_TransformEditTranslation.m_ConfirmAction = [&](EditorUI::EditVec3Spec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::TransformComponent>())
			{
				KG_ERROR("Attempt to edit entity transform component when none exists!");
				return;
			}
			auto& transformComp = entity.GetComponent<ECS::TransformComponent>();
			transformComp.Translation = m_TransformEditTranslation.m_CurrentVec3;
		};
		
		m_TransformEditScale.m_Label = "Scale";
		m_TransformEditScale.m_Flags = EditorUI::EditVec3_Indented;
		m_TransformEditScale.m_ConfirmAction = [&](EditorUI::EditVec3Spec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::TransformComponent>())
			{
				KG_ERROR("Attempt to edit entity transform component when none exists!");
				return;
			}
			auto& transformComp = entity.GetComponent<ECS::TransformComponent>();
			transformComp.Scale = m_TransformEditScale.m_CurrentVec3;
		};
		m_TransformEditRotation.m_Label = "Rotation";
		m_TransformEditRotation.m_Flags = EditorUI::EditVec3_Indented;
		m_TransformEditRotation.m_ConfirmAction = [&](EditorUI::EditVec3Spec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::TransformComponent>())
			{
				KG_ERROR("Attempt to edit entity transform component when none exists!");
				return;
			}
			auto& transformComp = entity.GetComponent<ECS::TransformComponent>();
			transformComp.Rotation = m_TransformEditRotation.m_CurrentVec3;
		};
		
	}

	void SceneEditorPanel::InitializeRigidbody2DComponent()
	{
		m_Rigidbody2DHeader.m_Label = "Rigid Body 2D";
		m_Rigidbody2DHeader.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_Rigidbody2DHeader.m_Expanded = true;
		m_Rigidbody2DHeader.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderSpec& spec)
		{
			EngineService::SubmitToMainThread([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<ECS::Rigidbody2DComponent>())
				{
					for (auto& entry : m_SceneHierarchyTree.GetTreeEntries())
					{
						if ((uint32_t)entry.m_Handle == (uint32_t)entity)
						{
							EditorUI::TreePath newPath {};
							for (auto& subEntry : entry.m_SubEntries)
							{
								SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.m_ProvidedData.get();
								if (entryData.m_ComponentType == ECS::ComponentType::Rigidbody2D)
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

							m_SceneHierarchyTree.RemoveEntry(newPath);

							break;
						}
					}

					entity.RemoveComponent<ECS::Rigidbody2DComponent>();
				}
			});
		});

		m_Rigidbody2DType.m_Label = "Interaction Type";
		m_Rigidbody2DType.m_FirstOptionLabel = "Static";
		m_Rigidbody2DType.m_SecondOptionLabel = "Dynamic";
		m_Rigidbody2DType.m_Flags |= EditorUI::RadioSelector_Indented;
		m_Rigidbody2DType.m_SelectAction = [&]()
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::Rigidbody2DComponent>())
			{
				KG_ERROR("Attempt to edit entity rigid body 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::Rigidbody2DComponent>();

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
		m_RigidBody2DFixedRotation.m_ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::Rigidbody2DComponent>())
			{
				KG_ERROR("Attempt to edit entity rigid body 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::Rigidbody2DComponent>();
			component.FixedRotation = spec.m_CurrentBoolean;
		};

		m_SelectRigidBody2DCollisionStartScript.m_Label = "On Collision Start";
		m_SelectRigidBody2DCollisionStartScript.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectRigidBody2DCollisionStartScript.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectRigidBody2DCollisionStartScript.m_PopupAction = [&]()
		{
			m_SelectRigidBody2DCollisionStartScript.ClearOptions();
			m_SelectRigidBody2DCollisionStartScript.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				KG_ASSERT(script);

				if (script->m_FuncType != WrappedFuncType::Bool_EntityEntity)
				{
					continue;
				}
				m_SelectRigidBody2DCollisionStartScript.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
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

		m_SelectRigidBody2DCollisionStartScript.m_OnEdit = [&]()
			{
				// Initialize tooltip with options
				m_SelectScriptTooltip.ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					m_SelectRigidBody2DCollisionStartScript.m_OpenPopup = true;
				} };
				m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
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
								m_SelectRigidBody2DCollisionStartScript.m_CurrentOption = { script->m_ScriptName, scriptHandle };
							}, {"activeEntity", "collidedEntity"});

						} };
				m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_SelectScriptTooltip.m_TooltipActive = true;

			};

		m_SelectRigidBody2DCollisionEndScript.m_Label = "On Collision End";
		m_SelectRigidBody2DCollisionEndScript.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectRigidBody2DCollisionEndScript.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectRigidBody2DCollisionEndScript.m_PopupAction = [&]()
		{
			m_SelectRigidBody2DCollisionEndScript.ClearOptions();
			m_SelectRigidBody2DCollisionEndScript.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				KG_ASSERT(script);

				if (script->m_FuncType != WrappedFuncType::Bool_EntityEntity)
				{
					continue;
				}
				m_SelectRigidBody2DCollisionEndScript.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
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

		m_SelectRigidBody2DCollisionEndScript.m_OnEdit = [&]()
			{
				// Initialize tooltip with options
				m_SelectScriptTooltip.ClearEntries();
				EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
				{
					m_SelectRigidBody2DCollisionEndScript.m_OpenPopup = true;
				} };
				m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

				EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
				{
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
						m_SelectRigidBody2DCollisionEndScript.m_CurrentOption = { script->m_ScriptName, scriptHandle };
					}, {"activeEntity", "collidedEntity"});

			}		};
				m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

				// Open tooltip
				m_SelectScriptTooltip.m_TooltipActive = true;

			};

	}

	void SceneEditorPanel::InitializeBoxCollider2DComponent()
	{
		m_BoxCollider2DHeader.m_Label = "Box Collider 2D";
		m_BoxCollider2DHeader.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_BoxCollider2DHeader.m_Expanded = true;
		m_BoxCollider2DHeader.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderSpec& spec)
		{
			EngineService::SubmitToMainThread([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<ECS::BoxCollider2DComponent>())
				{
					for (auto& entry : m_SceneHierarchyTree.GetTreeEntries())
					{
						if ((uint32_t)entry.m_Handle == (uint32_t)entity)
						{
							EditorUI::TreePath newPath {};
							for (auto& subEntry : entry.m_SubEntries)
							{
								SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.m_ProvidedData.get();
								if (entryData.m_ComponentType == ECS::ComponentType::BoxCollider2D)
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

							m_SceneHierarchyTree.RemoveEntry(newPath);

							break;
						}
					}

					entity.RemoveComponent<ECS::BoxCollider2DComponent>();
				}
			});
		});

		m_BoxColliderOffset.m_Label = "Offset";
		m_BoxColliderOffset.m_Flags |= EditorUI::EditVec2_Indented;
		m_BoxColliderOffset.m_ConfirmAction = [&](EditorUI::EditVec2Spec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
			component.Offset = m_BoxColliderOffset.m_CurrentVec2;
		};

		m_BoxColliderSize.m_Label = "Size";
		m_BoxColliderSize.m_Flags |= EditorUI::EditVec2_Indented;
		m_BoxColliderSize.m_ConfirmAction = [&](EditorUI::EditVec2Spec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
			component.Size = m_BoxColliderSize.m_CurrentVec2;
		};

		m_BoxColliderDensity.m_Label = "Density";
		m_BoxColliderDensity.m_Flags |= EditorUI::EditFloat_Indented;
		m_BoxColliderDensity.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
			component.Density = m_BoxColliderDensity.m_CurrentFloat;
		};

		m_BoxColliderFriction.m_Label = "Friction";
		m_BoxColliderFriction.m_Flags |= EditorUI::EditFloat_Indented;
		m_BoxColliderFriction.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
			component.Friction = m_BoxColliderFriction.m_CurrentFloat;
		};

		m_BoxColliderRestitution.m_Label = "Restitution";
		m_BoxColliderRestitution.m_Flags |= EditorUI::EditFloat_Indented;
		m_BoxColliderRestitution.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
			component.Restitution = m_BoxColliderRestitution.m_CurrentFloat;
		};

		m_BoxColliderRestitutionThreshold.m_Label = "Restitution Threshold";
		m_BoxColliderRestitutionThreshold.m_Flags |= EditorUI::EditFloat_Indented;
		m_BoxColliderRestitutionThreshold.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
			component.RestitutionThreshold = m_BoxColliderRestitutionThreshold.m_CurrentFloat;
		};

		// Set whether box collider is treated as a sensor
		m_BoxColliderIsSensor.m_Label = "Is Sensor";
		m_BoxColliderIsSensor.m_Flags |= EditorUI::Checkbox_Indented;
		m_BoxColliderIsSensor.m_ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
			component.IsSensor = spec.m_CurrentBoolean;
		};
	}
	void SceneEditorPanel::InitializeCircleCollider2DComponent()
	{
		m_CircleCollider2DHeader.m_Label = "Circle Collider 2D";
		m_CircleCollider2DHeader.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_CircleCollider2DHeader.m_Expanded = true;
		m_CircleCollider2DHeader.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderSpec& spec)
		{
			EngineService::SubmitToMainThread([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<ECS::CircleCollider2DComponent>())
				{
					for (auto& entry : m_SceneHierarchyTree.GetTreeEntries())
					{
						if ((uint32_t)entry.m_Handle == (uint32_t)entity)
						{
							EditorUI::TreePath newPath {};
							for (auto& subEntry : entry.m_SubEntries)
							{
								SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.m_ProvidedData.get();
								if (entryData.m_ComponentType == ECS::ComponentType::CircleCollider2D)
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

							m_SceneHierarchyTree.RemoveEntry(newPath);

							break;
						}
					}

					entity.RemoveComponent<ECS::CircleCollider2DComponent>();
				}
			});
		});

		m_CircleColliderOffset.m_Label = "Offset";
		m_CircleColliderOffset.m_Flags |= EditorUI::EditVec2_Indented;
		m_CircleColliderOffset.m_ConfirmAction = [&](EditorUI::EditVec2Spec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
			component.Offset = m_CircleColliderOffset.m_CurrentVec2;
		};

		m_CircleColliderRadius.m_Label = "Radius";
		m_CircleColliderRadius.m_Flags |= EditorUI::EditFloat_Indented;
		m_CircleColliderRadius.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
			component.Radius = m_CircleColliderRadius.m_CurrentFloat;
		};

		m_CircleColliderDensity.m_Label = "Density";
		m_CircleColliderDensity.m_Flags |= EditorUI::EditFloat_Indented;
		m_CircleColliderDensity.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
			component.Density = m_CircleColliderDensity.m_CurrentFloat;
		};

		m_CircleColliderFriction.m_Label = "Friction";
		m_CircleColliderFriction.m_Flags |= EditorUI::EditFloat_Indented;
		m_CircleColliderFriction.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
			component.Friction = m_CircleColliderFriction.m_CurrentFloat;
		};

		m_CircleColliderRestitution.m_Label = "Restitution";
		m_CircleColliderRestitution.m_Flags |= EditorUI::EditFloat_Indented;
		m_CircleColliderRestitution.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
			component.Restitution = m_CircleColliderRestitution.m_CurrentFloat;
		};

		m_CircleColliderRestitutionThreshold.m_Label = "Restitution Threshold";
		m_CircleColliderRestitutionThreshold.m_Flags |= EditorUI::EditFloat_Indented;
		m_CircleColliderRestitutionThreshold.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
			component.RestitutionThreshold = m_CircleColliderRestitutionThreshold.m_CurrentFloat;
		};

		// Set whether circle collider is treated as a sensor
		m_CircleColliderIsSensor.m_Label = "Is Sensor";
		m_CircleColliderIsSensor.m_Flags |= EditorUI::Checkbox_Indented;
		m_CircleColliderIsSensor.m_ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
			component.IsSensor = spec.m_CurrentBoolean;
		};
	}

	void SceneEditorPanel::InitializeCameraComponent()
	{
		m_CameraHeader.m_Label = "Camera";
		m_CameraHeader.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_CameraHeader.m_Expanded = true;
		m_CameraHeader.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderSpec& spec)
		{
			EngineService::SubmitToMainThread([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<ECS::CameraComponent>())
				{
					for (auto& entry : m_SceneHierarchyTree.GetTreeEntries())
					{
						if ((uint32_t)entry.m_Handle == (uint32_t)entity)
						{
							EditorUI::TreePath newPath {};
							for (auto& subEntry : entry.m_SubEntries)
							{
								SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.m_ProvidedData.get();
								if (entryData.m_ComponentType == ECS::ComponentType::Camera)
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

							m_SceneHierarchyTree.RemoveEntry(newPath);

							break;
						}
					}

					entity.RemoveComponent<ECS::CameraComponent>();
				}
			});
		});

		// Set Primary Camera Checkbox
		m_CameraPrimary.m_Label = "Primary Camera";
		m_CameraPrimary.m_Flags |= EditorUI::Checkbox_Indented;
		m_CameraPrimary.m_ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CameraComponent>();
			component.Primary = spec.m_CurrentBoolean;
		};

		m_CameraProjection.m_Label = "Projection Type";
		m_CameraProjection.m_Flags |= EditorUI::RadioSelector_Indented;
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
			auto& component = entity.GetComponent<ECS::CameraComponent>();

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
		m_CameraOrthographicSize.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CameraComponent>();
			component.Camera.SetOrthographicSize(m_CameraOrthographicSize.m_CurrentFloat);
		};

		m_CameraOrthographicNearPlane.m_Label = "Near Plane";
		m_CameraOrthographicNearPlane.m_Flags |= EditorUI::EditFloat_Indented;
		m_CameraOrthographicNearPlane.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CameraComponent>();
			component.Camera.SetOrthographicNearClip(m_CameraOrthographicNearPlane.m_CurrentFloat);
		};

		m_CameraOrthographicFarPlane.m_Label = "Far Plane";
		m_CameraOrthographicFarPlane.m_Flags |= EditorUI::EditFloat_Indented;
		m_CameraOrthographicFarPlane.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CameraComponent>();
			component.Camera.SetOrthographicFarClip(m_CameraOrthographicFarPlane.m_CurrentFloat);
		};

		m_CameraPerspectiveFOV.m_Label = "Vertical FOV";
		m_CameraPerspectiveFOV.m_Flags |= EditorUI::EditFloat_Indented;
		m_CameraPerspectiveFOV.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CameraComponent>();
			component.Camera.SetPerspectiveVerticalFOV(m_CameraPerspectiveFOV.m_CurrentFloat);
		};

		m_CameraPerspectiveNearPlane.m_Label = "Near Plane";
		m_CameraPerspectiveNearPlane.m_Flags |= EditorUI::EditFloat_Indented;
		m_CameraPerspectiveNearPlane.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CameraComponent>();
			component.Camera.SetPerspectiveNearClip(m_CameraPerspectiveNearPlane.m_CurrentFloat);
		};

		m_CameraPerspectiveFarPlane.m_Label = "Far Plane";
		m_CameraPerspectiveFarPlane.m_Flags |= EditorUI::EditFloat_Indented;
		m_CameraPerspectiveFarPlane.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CameraComponent>();
			component.Camera.SetPerspectiveFarClip(m_CameraPerspectiveFarPlane.m_CurrentFloat);
		};
	}

	void SceneEditorPanel::InitializeOnUpdateComponent()
	{
		m_OnUpdateHeader.m_Label = "On Update";
		m_OnUpdateHeader.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_OnUpdateHeader.m_Expanded = true;
		m_OnUpdateHeader.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderSpec& spec)
		{
			EngineService::SubmitToMainThread([&]()
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
					}, 0);

					KG_ASSERT(pathToDelete);
					m_SceneHierarchyTree.RemoveEntry(pathToDelete);
					entity.RemoveComponent<ECS::OnUpdateComponent>();
				}
			});
		});

		m_SelectOnUpdateScript.m_Label = "On Update Script";
		m_SelectOnUpdateScript.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectOnUpdateScript.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectOnUpdateScript.m_PopupAction = [&]()
		{
			m_SelectOnUpdateScript.ClearOptions();
			m_SelectOnUpdateScript.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				KG_ASSERT(script);

				if (script->m_FuncType != WrappedFuncType::Void_EntityFloat)
				{
					continue;
				}
				m_SelectOnUpdateScript.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
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

		m_SelectOnUpdateScript.m_OnEdit = [&]()
		{
			// Initialize tooltip with options
			m_SelectScriptTooltip.ClearEntries();
			EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
			{
				m_SelectOnUpdateScript.m_OpenPopup = true;
			}};
			m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

			EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
			{
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
							m_SelectOnUpdateScript.m_CurrentOption = { script->m_ScriptName, scriptHandle };
						}, {"activeEntity", "deltaTime"});

					} };
			m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

			// Open tooltip
			m_SelectScriptTooltip.m_TooltipActive = true;

		};
	}

	void SceneEditorPanel::InitializeOnCreateComponent()
	{
		m_OnCreateHeader.m_Label = "On Create";
		m_OnCreateHeader.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_OnCreateHeader.m_Expanded = true;
		m_OnCreateHeader.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderSpec& spec)
		{
			EngineService::SubmitToMainThread([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				EditorUI::TreePath pathToDelete;
				if (entity.HasComponent<ECS::OnCreateComponent>())
				{
					m_SceneHierarchyTree.EditDepth([&](EditorUI::TreeEntry& entry)
					{
						if ((uint32_t)entry.m_Handle == (uint32_t)entity)
						{
							for (auto& subEntry : entry.m_SubEntries)
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
					}, 0);

					KG_ASSERT(pathToDelete);
					m_SceneHierarchyTree.RemoveEntry(pathToDelete);
					entity.RemoveComponent<ECS::OnCreateComponent>();
				}
			});
		});

		m_SelectOnCreateScript.m_Label = "On Create Script";
		m_SelectOnCreateScript.m_Flags |= EditorUI::SelectOption_Indented | EditorUI::SelectOption_HandleEditButtonExternally;
		m_SelectOnCreateScript.m_CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectOnCreateScript.m_PopupAction = [&]()
		{
			m_SelectOnCreateScript.ClearOptions();
			m_SelectOnCreateScript.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				KG_ASSERT(script);

				if (script->m_FuncType != WrappedFuncType::Void_Entity)
				{
					continue;
				}
				m_SelectOnCreateScript.AddToOptions(Utility::ScriptToEditorUIGroup(script), script->m_ScriptName, handle);
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

		m_SelectOnCreateScript.m_OnEdit = [&]()
		{
			// Initialize tooltip with options
			m_SelectScriptTooltip.ClearEntries();
			EditorUI::TooltipEntry openScriptOptions{ "Open Script", [&](EditorUI::TooltipEntry& entry)
			{
				m_SelectOnCreateScript.m_OpenPopup = true;
			} };
			m_SelectScriptTooltip.AddTooltipEntry(openScriptOptions);

			EditorUI::TooltipEntry createScriptOptions{ "Create Script", [&](EditorUI::TooltipEntry& entry)
			{
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
							m_SelectOnCreateScript.m_CurrentOption = { script->m_ScriptName, scriptHandle };
						}, {"activeEntity"});

					}};
			m_SelectScriptTooltip.AddTooltipEntry(createScriptOptions);

			// Open tooltip
			m_SelectScriptTooltip.m_TooltipActive = true;

		};
	}

	void SceneEditorPanel::InitializeAIComponent()
	{
		// Set up AI state header
		m_AIStateHeader.m_Label = "AI State";
		m_AIStateHeader.m_Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_AIStateHeader.m_Expanded = true;
		m_AIStateHeader.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderSpec& spec)
			{
				EngineService::SubmitToMainThread([&]()
					{
						ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
						EditorUI::TreePath pathToDelete;
						if (entity.HasComponent<ECS::AIStateComponent>())
						{
							m_SceneHierarchyTree.EditDepth([&](EditorUI::TreeEntry& entry)
								{
									if ((uint32_t)entry.m_Handle == (uint32_t)entity)
									{
										for (auto& subEntry : entry.m_SubEntries)
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
								}, 0);

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
		m_SelectGlobalState.m_PopupAction = [&]()
		{
			m_SelectGlobalState.ClearOptions();
			m_SelectGlobalState.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetAIStateRegistry())
			{
				Ref<AI::AIState> aiStateRef = Assets::AssetService::GetAIState(handle);
				KG_ASSERT(aiStateRef);

				m_SelectGlobalState.AddToOptions("All States", asset.Data.FileLocation.string(), handle);
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
		m_SelectCurrentState.m_PopupAction = [&]()
		{
			m_SelectCurrentState.ClearOptions();
			m_SelectCurrentState.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetAIStateRegistry())
			{
				Ref<AI::AIState> aiStateRef = Assets::AssetService::GetAIState(handle);
				KG_ASSERT(aiStateRef);

				m_SelectCurrentState.AddToOptions("All States", asset.Data.FileLocation.string(), handle);
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
		m_SelectPreviousState.m_PopupAction = [&]()
		{
			m_SelectPreviousState.ClearOptions();
			m_SelectPreviousState.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetAIStateRegistry())
			{
				Ref<AI::AIState> aiStateRef = Assets::AssetService::GetAIState(handle);
				KG_ASSERT(aiStateRef);

				m_SelectPreviousState.AddToOptions("All States", asset.Data.FileLocation.string(), handle);
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
		m_ShapeHeader.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderSpec& spec)
		{
			EngineService::SubmitToMainThread([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<ECS::ShapeComponent>())
				{
					for (auto& entry : m_SceneHierarchyTree.GetTreeEntries())
					{
						if ((uint32_t)entry.m_Handle == (uint32_t)entity)
						{
							EditorUI::TreePath newPath {};
							for (auto& subEntry : entry.m_SubEntries)
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

							m_SceneHierarchyTree.RemoveEntry(newPath);

							break;
						}
					}

					entity.RemoveComponent<ECS::ShapeComponent>();
				}
			});
		});

		m_ShapeSelect.m_Label = "Mesh";
		m_ShapeSelect.m_Flags |= EditorUI::SelectOption_Indented;
		m_ShapeSelect.m_PopupAction = [&]()
		{
			m_ShapeSelect.ClearOptions();
			m_ShapeSelect.AddToOptions("Clear", "None", Assets::EmptyHandle);
			m_ShapeSelect.AddToOptions("All Shapes", "Quad", Assets::EmptyHandle);
			m_ShapeSelect.AddToOptions("All Shapes", "Pyramid", Assets::EmptyHandle);
			m_ShapeSelect.AddToOptions("All Shapes", "Cube", Assets::EmptyHandle);
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
		m_ShapeColorType.m_PopupAction = [&]()
		{
			m_ShapeColorType.ClearOptions();
			m_ShapeColorType.AddToOptions("Clear", "None", Assets::EmptyHandle);
			m_ShapeColorType.AddToOptions("All Types", "FlatColor", Assets::EmptyHandle);
			m_ShapeColorType.AddToOptions("All Types", "VertexColor", Assets::EmptyHandle);
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
				Rendering::Shader::SetDataAtInputLocation<Math::vec4>({ 1.0f, 1.0f, 1.0f, 1.0f }, "a_Color", component.ShaderData, component.Shader);
			}
			if (entry.m_Label == "VertexColor")
			{
				Math::vec4 transferColor {1.0f, 1.0f, 1.0f, 1.0f};
				if (component.ShaderSpecification.ColorInput == Rendering::ColorInputType::FlatColor)
				{
					transferColor = *Rendering::Shader::GetInputLocation<Math::vec4>("a_Color", component.ShaderData, component.Shader);
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
		m_ShapeColor.m_ConfirmAction = [&](EditorUI::EditVec4Spec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::ShapeComponent>())
			{
				KG_ERROR("Attempt to edit entity shape 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::ShapeComponent>();
			Math::vec4* color = Rendering::Shader::GetInputLocation<Math::vec4>("a_Color", component.ShaderData, component.Shader);
			*color = m_ShapeColor.m_CurrentVec4;
		};

		// Set Shape Add Texture Checkbox
		m_ShapeAddTexture.m_Label = "Use Texture";
		m_ShapeAddTexture.m_Flags |= EditorUI::Checkbox_Indented;
		m_ShapeAddTexture.m_ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
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
				Rendering::Shader::SetDataAtInputLocation<float>(1.0f, "a_TilingFactor", component.ShaderData, component.Shader);
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
		m_ShapeSetTexture.m_PopupAction = [&]()
		{
			m_ShapeSetTexture.ClearOptions();
			for (auto& [handle, asset] : Assets::AssetService::GetTexture2DRegistry())
			{
				m_ShapeSetTexture.AddToOptions("All Textures", asset.Data.FileLocation.string(), handle);
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
		m_ShapeTilingFactor.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::ShapeComponent>())
			{
				KG_ERROR("Attempt to edit entity shape 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::ShapeComponent>();
			float* tilingFactor = Rendering::Shader::GetInputLocation<float>("a_TilingFactor", component.ShaderData, component.Shader);
			*tilingFactor = m_ShapeTilingFactor.m_CurrentFloat;
		};

		// Set Shape Circle Option
		m_ShapeAddCircle.m_Label = "Use Circle Shape";
		m_ShapeAddCircle.m_Flags |= EditorUI::Checkbox_Indented;
		m_ShapeAddCircle.m_ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			component.ShaderSpecification.AddCircleShape = spec.m_CurrentBoolean;
			UpdateShapeComponent();
			if (spec.m_CurrentBoolean)
			{
				Rendering::Shader::SetDataAtInputLocation<float>(1.0f, "a_Thickness", component.ShaderData, component.Shader);
				Rendering::Shader::SetDataAtInputLocation<float>(0.005f, "a_Fade", component.ShaderData, component.Shader);
			}
		};

		m_ShapeCircleThickness.m_Label = "Circle Thickness";
		m_ShapeCircleThickness.m_Flags |= EditorUI::EditFloat_Indented;
		m_ShapeCircleThickness.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::ShapeComponent>())
			{
				KG_ERROR("Attempt to edit entity shape 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::ShapeComponent>();
			float* thickness = Rendering::Shader::GetInputLocation<float>("a_Thickness", component.ShaderData, component.Shader);
			*thickness = m_ShapeCircleThickness.m_CurrentFloat;
		};

		m_ShapeCircleFade.m_Label = "Circle Fade";
		m_ShapeCircleFade.m_Flags |= EditorUI::EditFloat_Indented;
		m_ShapeCircleFade.m_ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::ShapeComponent>())
			{
				KG_ERROR("Attempt to edit entity shape 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::ShapeComponent>();
			float* fade = Rendering::Shader::GetInputLocation<float>("a_Fade", component.ShaderData, component.Shader);
			*fade = m_ShapeCircleFade.m_CurrentFloat;
		};

		// Set Shape Add Projection Option
		m_ShapeAddProjection.m_Label = "Use Projection Matrix";
		m_ShapeAddProjection.m_Flags |= EditorUI::Checkbox_Indented;
		m_ShapeAddProjection.m_ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			component.ShaderSpecification.AddProjectionMatrix = spec.m_CurrentBoolean;
			UpdateShapeComponent();
		};

		// Set Shape Add Entity ID Option
		m_ShapeAddEntityID.m_Label = "Use Entity ID";
		m_ShapeAddEntityID.m_Flags |= EditorUI::Checkbox_Indented;
		m_ShapeAddEntityID.m_ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
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
		newWidgetData.m_Header.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderSpec& spec)
		{
			EngineService::SubmitToMainThread([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				Assets::AssetHandle projectComponentHandle = *(Assets::AssetHandle*)spec.m_ProvidedData.get();
				if (entity.HasProjectComponentData(projectComponentHandle))
				{
					for (auto& entry : m_SceneHierarchyTree.GetTreeEntries())
					{
						if ((uint32_t)entry.m_Handle == (uint32_t)entity)
						{
							EditorUI::TreePath newPath {};
							for (auto& subEntry : entry.m_SubEntries)
							{
								SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.m_ProvidedData.get();
								if (entryData.m_ComponentType == ECS::ComponentType::ProjectComponent &&
									entryData.m_ProjectComponentHandle == projectComponentHandle)
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

							m_SceneHierarchyTree.RemoveEntry(newPath);

							break;
						}
					}
					entity.RemoveProjectComponentData(projectComponentHandle);
				}
			});
		});

		// Initialize project component fields into scene editor UI
		for (size_t iteration{ 0 }; iteration < component->m_DataNames.size(); iteration++)
		{
			static EditorUI::EditFloatSpec newFloatSpec;
			static EditorUI::EditVec3Spec newVector3Spec;
			static EditorUI::EditTextSpec newStringSpec;
			static EditorUI::CheckboxSpec newBoolSpec;
			static EditorUI::EditIntegerSpec newIntegerSpec;

			WrappedVarType currentType = component->m_DataTypes.at(iteration);
			const std::string& currentName = component->m_DataNames.at(iteration);

			switch (currentType)
			{
			case WrappedVarType::Float:
				newFloatSpec = {};
				newFloatSpec.m_Label = currentName;
				newFloatSpec.m_Flags |= EditorUI::EditFloat_Indented;
				newFloatSpec.m_ProvidedData = CreateRef<ProjectComponentFieldInfo>(projectComponentHandle, iteration);
				newFloatSpec.m_ConfirmAction = [](EditorUI::EditFloatSpec& spec)
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
				newBoolSpec.m_ConfirmAction = [](EditorUI::CheckboxSpec& spec)
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
				newVector3Spec.m_ConfirmAction = [](EditorUI::EditVec3Spec& spec)
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
				newIntegerSpec.m_ConfirmAction = [](EditorUI::EditIntegerSpec& spec)
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
				newIntegerSpec.m_ConfirmAction = [](EditorUI::EditIntegerSpec& spec)
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
				newIntegerSpec.m_ConfirmAction = [](EditorUI::EditIntegerSpec& spec)
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
				newIntegerSpec.m_ConfirmAction = [](EditorUI::EditIntegerSpec& spec)
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
		InitializeTagComponent();
		InitializeTransformComponent();
		InitializeRigidbody2DComponent();
		InitializeOnUpdateComponent();
		InitializeOnCreateComponent();
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
			
			EditorUI::EditorUIService::PanelHeader(m_MainSceneHeader);

			EditorUI::EditorUIService::Tree(m_SceneHierarchyTree);

			EditorUI::EditorUIService::SelectOption(m_AddComponent);

			EditorUI::EditorUIService::Tooltip(m_SelectScriptTooltip);
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
				uint32_t iteration{ 0 };
				// Remove entry from Hierarchy Tree
				for (auto& entry : m_SceneHierarchyTree.GetTreeEntries())
				{
					if ((uint32_t)entry.m_Handle == (uint32_t)entityToDelete)
					{
						EditorUI::TreePath path = m_SceneHierarchyTree.GetPathFromEntryReference(&entry);
						if (!path)
						{
							KG_WARN("Could not locate path from entry reference!");
							return false;
						}
						m_SceneHierarchyTree.RemoveEntry(path);
						break;
					}
					iteration++;
				}
			}

			if (manageEntity->GetAction() == Events::ManageEntityAction::Create)
			{
				ECS::Entity entityToCreate = Scenes::SceneService::GetActiveScene()->GetEntityByUUID(manageEntity->GetEntityID());
				if (!entityToCreate)
				{
					KG_WARN("Could not locate entity by UUID");
					return false;
				}

				CreateSceneEntityInTree(entityToCreate);
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
			manageAsset->GetAction() == Events::ManageAssetAction::Delete)
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
			manageAsset->GetAction() == Events::ManageAssetAction::Delete)
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
				1);

				// Remove all of those tree entries
				for (auto& entryPath : entriesToRemove)
				{
					m_SceneHierarchyTree.RemoveEntry(entryPath);
				}
			}
			m_SceneHierarchyTree.SelectFirstEntry();
			SetSelectedEntity({});
		}

		if (manageAsset->GetAssetType() == Assets::AssetType::ProjectComponent &&
			manageAsset->GetAction() == Events::ManageAssetAction::Delete)
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
			1);

			// Remove all of those tree entries
			for (auto& entryPath : entriesToRemove)
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
		}
		if (entity)
		{
			EditorUI::TreePath path;
			for (auto& entry : m_SceneHierarchyTree.GetTreeEntries())
			{
				if ((uint32_t)entry.m_Handle == (uint32_t)entity)
				{
					path = m_SceneHierarchyTree.GetPathFromEntryReference(&entry);
				}
			}

			if (path)
			{
				m_SceneHierarchyTree.m_SelectedEntry = path;
			}
			else
			{
				KG_WARN("Failed to locate entity inside tree");
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
			auto& transformComp = entity.GetComponent<ECS::TransformComponent>();
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
		EditorUI::EditorUIService::CollapsingHeader(m_TagHeader);
		if (m_TagHeader.m_Expanded)
		{
			m_TagEdit.m_CurrentOption = component.Tag;
			EditorUI::EditorUIService::EditText(m_TagEdit);
			m_TagGroupEdit.m_CurrentOption = component.Group;
			EditorUI::EditorUIService::EditText(m_TagGroupEdit);
		}
	}
	void SceneEditorPanel::DrawTransformComponent(ECS::Entity entity)
	{
		if (!entity.HasComponent<ECS::TransformComponent>())
		{
			return;
		}
		ECS::TransformComponent& component = entity.GetComponent<ECS::TransformComponent>();
		EditorUI::EditorUIService::CollapsingHeader(m_TransformHeader);
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
		EditorUI::EditorUIService::CollapsingHeader(m_Rigidbody2DHeader);
		if (m_Rigidbody2DHeader.m_Expanded)
		{
			m_Rigidbody2DType.m_SelectedOption = component.Type == ECS::Rigidbody2DComponent::BodyType::Static ?
				0 : 1;
			EditorUI::EditorUIService::RadioSelector(m_Rigidbody2DType);
			m_RigidBody2DFixedRotation.m_CurrentBoolean = component.FixedRotation;
			EditorUI::EditorUIService::Checkbox(m_RigidBody2DFixedRotation);

			// Display collision script functions
			Ref<Scripting::Script> collisionStartScript = Assets::AssetService::GetScript(component.OnCollisionStartScriptHandle);
			m_SelectRigidBody2DCollisionStartScript.m_CurrentOption = component.OnCollisionStartScriptHandle == Assets::EmptyHandle ?
				EditorUI::OptionEntry("None", Assets::EmptyHandle) :
				EditorUI::OptionEntry(Utility::ScriptToString(collisionStartScript), component.OnCollisionStartScriptHandle);
			EditorUI::EditorUIService::SelectOption(m_SelectRigidBody2DCollisionStartScript);

			Ref<Scripting::Script> collisionEndScript = Assets::AssetService::GetScript(component.OnCollisionEndScriptHandle);
			m_SelectRigidBody2DCollisionEndScript.m_CurrentOption = component.OnCollisionEndScriptHandle == Assets::EmptyHandle ?
				EditorUI::OptionEntry("None", Assets::EmptyHandle) :
				EditorUI::OptionEntry(Utility::ScriptToString(collisionEndScript), component.OnCollisionEndScriptHandle);
			EditorUI::EditorUIService::SelectOption(m_SelectRigidBody2DCollisionEndScript);
		}
	}
	void SceneEditorPanel::DrawBoxCollider2DComponent(ECS::Entity entity)
	{
		if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
		{
			return;
		}
		ECS::BoxCollider2DComponent& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
		EditorUI::EditorUIService::CollapsingHeader(m_BoxCollider2DHeader);
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
			EditorUI::EditorUIService::Checkbox(m_BoxColliderIsSensor);

		}
		
	}
	void SceneEditorPanel::DrawCircleCollider2DComponent(ECS::Entity entity)
	{
		if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
		{
			return;
		}
		ECS::CircleCollider2DComponent& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
		EditorUI::EditorUIService::CollapsingHeader(m_CircleCollider2DHeader);
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
			EditorUI::EditorUIService::Checkbox(m_CircleColliderIsSensor);
		}
		
	}
	void SceneEditorPanel::DrawCameraComponent(ECS::Entity entity)
	{
		if (!entity.HasComponent<ECS::CameraComponent>())
		{
			return;
		}
		ECS::CameraComponent& component = entity.GetComponent<ECS::CameraComponent>();
		EditorUI::EditorUIService::CollapsingHeader(m_CameraHeader);
		if (m_CameraHeader.m_Expanded)
		{
			m_CameraPrimary.m_CurrentBoolean = component.Primary;
			EditorUI::EditorUIService::Checkbox(m_CameraPrimary);
			m_CameraProjection.m_SelectedOption = component.Camera.GetProjectionType() ==
				Scenes::SceneCamera::ProjectionType::Perspective ? 0 : 1;
			EditorUI::EditorUIService::RadioSelector(m_CameraProjection);

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
	void SceneEditorPanel::DrawOnUpdateComponent(ECS::Entity entity)
	{
		if (!entity.HasComponent<ECS::OnUpdateComponent>())
		{
			return;
		}
		ECS::OnUpdateComponent& component = entity.GetComponent<ECS::OnUpdateComponent>();
		EditorUI::EditorUIService::CollapsingHeader(m_OnUpdateHeader);
		if (m_OnUpdateHeader.m_Expanded)
		{
			Ref<Scripting::Script> script = Assets::AssetService::GetScript(component.OnUpdateScriptHandle);
			m_SelectOnUpdateScript.m_CurrentOption = component.OnUpdateScriptHandle == Assets::EmptyHandle ? 
				EditorUI::OptionEntry( "None", Assets::EmptyHandle ) :
				EditorUI::OptionEntry(Utility::ScriptToString(script), component.OnUpdateScriptHandle);
			EditorUI::EditorUIService::SelectOption(m_SelectOnUpdateScript);
		}
	}
	void SceneEditorPanel::DrawAIStateComponent(ECS::Entity entity)
	{
		if (!entity.HasComponent<ECS::AIStateComponent>())
		{
			return;
		}
		ECS::AIStateComponent& component = entity.GetComponent<ECS::AIStateComponent>();
		EditorUI::EditorUIService::CollapsingHeader(m_AIStateHeader);
		if (m_AIStateHeader.m_Expanded)
		{
			// Select global state
			bool optionValid = component.GlobalStateHandle != Assets::EmptyHandle;
			if (optionValid)
			{
				Assets::AssetInfo& globalAsset = Assets::AssetService::GetAIStateRegistry().at(component.GlobalStateHandle);
				m_SelectGlobalState.m_CurrentOption = { globalAsset.Data.FileLocation.string(), component.GlobalStateHandle };
			}
			else
			{
				m_SelectGlobalState.m_CurrentOption = { "None", Assets::EmptyHandle };
			}
			EditorUI::EditorUIService::SelectOption(m_SelectGlobalState);

			// Select current state
			optionValid = component.CurrentStateHandle != Assets::EmptyHandle;
			if (optionValid)
			{
				Assets::AssetInfo& currentAsset = Assets::AssetService::GetAIStateRegistry().at(component.CurrentStateHandle);
				m_SelectCurrentState.m_CurrentOption = { currentAsset.Data.FileLocation.string(), component.CurrentStateHandle };
			}
			else
			{
				m_SelectCurrentState.m_CurrentOption = { "None", Assets::EmptyHandle };
			}
			EditorUI::EditorUIService::SelectOption(m_SelectCurrentState);

			// Select previous state
			optionValid = component.PreviousStateHandle != Assets::EmptyHandle;
			if (optionValid)
			{
				Assets::AssetInfo& previousAsset = Assets::AssetService::GetAIStateRegistry().at(component.PreviousStateHandle);
				m_SelectPreviousState.m_CurrentOption = { previousAsset.Data.FileLocation.string(), component.PreviousStateHandle };
			}
			else
			{
				m_SelectPreviousState.m_CurrentOption = { "None", Assets::EmptyHandle };
			}
			EditorUI::EditorUIService::SelectOption(m_SelectPreviousState);
		}
	}
	void SceneEditorPanel::DrawOnCreateComponent(ECS::Entity entity)
	{
		if (!entity.HasComponent<ECS::OnCreateComponent>())
		{
			return;
		}
		ECS::OnCreateComponent& component = entity.GetComponent<ECS::OnCreateComponent>();
		EditorUI::EditorUIService::CollapsingHeader(m_OnCreateHeader);
		if (m_OnCreateHeader.m_Expanded)
		{
			Ref<Scripting::Script> script = Assets::AssetService::GetScript(component.OnCreateScriptHandle);
			m_SelectOnCreateScript.m_CurrentOption = component.OnCreateScriptHandle == Assets::EmptyHandle ?
				EditorUI::OptionEntry("None", Assets::EmptyHandle) :
				EditorUI::OptionEntry(Utility::ScriptToString(script), component.OnCreateScriptHandle);
			EditorUI::EditorUIService::SelectOption(m_SelectOnCreateScript);
		}
	}
	void SceneEditorPanel::DrawShapeComponent(ECS::Entity entity)
	{
		if (!entity.HasComponent<ECS::ShapeComponent>())
		{
			return;
		}
		ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
		EditorUI::EditorUIService::CollapsingHeader(m_ShapeHeader);
		if (m_ShapeHeader.m_Expanded)
		{
			m_ShapeSelect.m_CurrentOption = { Utility::ShapeTypeToString(component.CurrentShape), Assets::EmptyHandle };
			EditorUI::EditorUIService::SelectOption(m_ShapeSelect);

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
		void operator()(EditorUI::CheckboxSpec& spec)
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
			EditorUI::EditorUIService::Checkbox(spec);
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
			EditorUI::EditorUIService::EditText(spec);
		}
		void operator()(EditorUI::EditIntegerSpec& spec)
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
		void operator()(EditorUI::EditFloatSpec& spec)
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
		void operator()(EditorUI::EditVec2Spec& spec)
		{
			// TODO: Add support
			EditorUI::EditorUIService::EditVec2(spec);
		}
		void operator()(EditorUI::EditVec3Spec& spec)
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
		void operator()(EditorUI::EditVec4Spec& spec)
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
		EditorUI::EditorUIService::CollapsingHeader(activeWidgetData.m_Header);

		// Display all component fields
		if (activeWidgetData.m_Header.m_Expanded)
		{
			for (size_t iteration{ 0 }; iteration < activeWidgetData.m_Fields.size(); iteration++)
			{
				std::visit(DrawProjectComponentFieldsVisitor{}, activeWidgetData.m_Fields.at(iteration));
			}
		}
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
			if (newShader->GetInputLayout().FindElementByName(element.Name))
			{
				// Get Location of Old Data Pointer
				std::size_t oldLocation = element.Offset;
				uint8_t* oldLocationPointer = oldBuffer.As<uint8_t>(oldLocation);

				// Get Location of New Data Pointer
				uint8_t* newLocationPointer = Rendering::Shader::GetInputLocation<uint8_t>(element.Name, newBuffer, newShader);

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
		EditorUI::EditorUIService::SelectOption(m_ShapeColorType);

		if (component.ShaderSpecification.ColorInput == Rendering::ColorInputType::None) { return; }

		if (component.ShaderSpecification.ColorInput == Rendering::ColorInputType::FlatColor)
		{
			m_ShapeColor.m_CurrentVec4 = *(Rendering::Shader::GetInputLocation<Math::vec4>("a_Color", component.ShaderData, component.Shader));
			EditorUI::EditorUIService::EditVec4(m_ShapeColor);
		}
		if (component.ShaderSpecification.ColorInput == Rendering::ColorInputType::VertexColor)
		{
			uint32_t iterator{ 1 };
			for (auto& color : *component.VertexColors)
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
		EditorUI::EditorUIService::Checkbox(m_ShapeAddTexture);
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
					Assets::AssetService::GetTexture2DRegistry().at(component.TextureHandle).Data.FileLocation.string(),
					component.TextureHandle
				};
			}
			EditorUI::EditorUIService::SelectOption(m_ShapeSetTexture);

			m_ShapeTilingFactor.m_CurrentFloat = *Rendering::Shader::GetInputLocation<float>("a_TilingFactor", component.ShaderData, component.Shader);
			EditorUI::EditorUIService::EditFloat(m_ShapeTilingFactor);
		}
	}
	void SceneEditorPanel::DrawShapeComponentCircle()
	{
		ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
		m_ShapeAddCircle.m_CurrentBoolean = component.ShaderSpecification.AddCircleShape;
		EditorUI::EditorUIService::Checkbox(m_ShapeAddCircle);
		if (component.ShaderSpecification.AddCircleShape)
		{
			m_ShapeCircleThickness.m_CurrentFloat = *Rendering::Shader::GetInputLocation<float>("a_Thickness", component.ShaderData, component.Shader);
			EditorUI::EditorUIService::EditFloat(m_ShapeCircleThickness);

			m_ShapeCircleFade.m_CurrentFloat = *Rendering::Shader::GetInputLocation<float>("a_Fade", component.ShaderData, component.Shader);
			EditorUI::EditorUIService::EditFloat(m_ShapeCircleFade);
		}
	}
	void SceneEditorPanel::DrawShapeComponentProjection()
	{
		ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
		m_ShapeAddProjection.m_CurrentBoolean = component.ShaderSpecification.AddProjectionMatrix;
		EditorUI::EditorUIService::Checkbox(m_ShapeAddProjection);
	}
	void SceneEditorPanel::DrawShapeComponentEntityID()
	{
		ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
		m_ShapeAddEntityID.m_CurrentBoolean = component.ShaderSpecification.AddEntityID;
		EditorUI::EditorUIService::Checkbox(m_ShapeAddEntityID);
	}
}
