#include "kgpch.h"

#include "Panels/SceneEditorPanel.h"

#include "EditorApp.h"

#include "API/EditorUI/ImGuiBackendAPI.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{
	void SceneEditorPanel::CreateSceneEntityInTree(ECS::Entity entity)
	{
		EditorUI::TreeEntry newEntry {};
		newEntry.Label = entity.GetComponent<ECS::TagComponent>().Tag;
		newEntry.IconHandle = EditorUI::EditorUIService::s_IconEntity;
		newEntry.Handle = (uint64_t)entity;
		newEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
		{
			ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
			s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
			s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::None);
		};
		newEntry.OnDoubleLeftClick = [](EditorUI::TreeEntry& entry)
		{
			ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
			auto& editorCamera = EditorApp::GetCurrentApp()->m_ViewportPanel->m_EditorCamera;
			auto& transformComponent = entity.GetComponent<ECS::TransformComponent>();
			editorCamera.SetFocalPoint(transformComponent.Translation);
			editorCamera.SetDistance(std::max({ transformComponent.Scale.x, transformComponent.Scale.y, transformComponent.Scale.z }) * 2.5f);
			editorCamera.SetMovementType(Rendering::EditorCamera::MovementType::ModelView);
		};

		newEntry.OnRightClickSelection.push_back({ "Add Component", [&](EditorUI::TreeEntry& entry)
		{
			m_AddComponent.PopupActive = true;
			m_AddComponentEntity = (int32_t)entry.Handle;
		} });

		newEntry.OnRightClickSelection.push_back({ "Delete Entity", [](EditorUI::TreeEntry& entry)
		{
			static ECS::Entity entityToDelete;
			entityToDelete = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));

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
		componentEntry.Handle = (uint64_t)entity;
		if (entity.HasComponent<ECS::TagComponent>())
		{
			componentEntry.Label = "Tag";
			componentEntry.ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::Tag, Assets::EmptyHandle);
			componentEntry.IconHandle = EditorUI::EditorUIService::s_IconTag;
			componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
			{
				ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
				s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
				s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::Tag);
			};
			newEntry.SubEntries.push_back(componentEntry);
		}
		if (entity.HasComponent<ECS::TransformComponent>())
		{
			componentEntry.Label = "Transform";
			componentEntry.ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::Transform, Assets::EmptyHandle);
			componentEntry.IconHandle = EditorUI::EditorUIService::s_IconTransform;
			componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
			{
				ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
				s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
				s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::Transform);
			};
			newEntry.SubEntries.push_back(componentEntry);
		}

		if (entity.HasComponent<ECS::Rigidbody2DComponent>())
		{
			componentEntry.Label = "Rigid Body 2D";
			componentEntry.ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::Rigidbody2D, Assets::EmptyHandle);
			componentEntry.IconHandle = EditorUI::EditorUIService::s_IconRigidBody;
			componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
			{
				ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
				s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
				s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::Rigidbody2D);
			};
			newEntry.SubEntries.push_back(componentEntry);
		}

		if (entity.HasComponent<ECS::BoxCollider2DComponent>())
		{
			componentEntry.Label = "Box Collider 2D";
			componentEntry.ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::BoxCollider2D, Assets::EmptyHandle);
			componentEntry.IconHandle = EditorUI::EditorUIService::s_IconBoxCollider;
			componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
			{
				ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
				s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
				s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::BoxCollider2D);
			};
			newEntry.SubEntries.push_back(componentEntry);
		}

		if (entity.HasComponent<ECS::CircleCollider2DComponent>())
		{
			componentEntry.Label = "Circle Collider 2D";
			componentEntry.ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::CircleCollider2D, Assets::EmptyHandle);
			componentEntry.IconHandle = EditorUI::EditorUIService::s_IconCircleCollider;
			componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
			{
				ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
				s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
				s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::CircleCollider2D);
			};
			newEntry.SubEntries.push_back(componentEntry);
		}

		if (entity.HasComponent<ECS::CameraComponent>())
		{
			componentEntry.Label = "Camera";
			componentEntry.ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::Camera, Assets::EmptyHandle);
			componentEntry.IconHandle = EditorUI::EditorUIService::s_IconCamera;
			componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
			{
				ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
				s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
				s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::Camera);
			};
			newEntry.SubEntries.push_back(componentEntry);
		}

		if (entity.HasComponent<ECS::ShapeComponent>())
		{
			componentEntry.Label = "Shape";
			componentEntry.ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::Shape, Assets::EmptyHandle);
			componentEntry.IconHandle = EditorUI::EditorUIService::s_IconEntity;
			componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
			{
				ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
				s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
				s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::Shape);
			};
			newEntry.SubEntries.push_back(componentEntry);
		}

		if (entity.HasComponent<ECS::OnCreateComponent>())
		{
			componentEntry.Label = "On Create";
			componentEntry.ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::OnCreate, Assets::EmptyHandle);
			componentEntry.IconHandle = EditorUI::EditorUIService::s_IconFunction;
			componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
			{
				ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
				s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
				s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::OnCreate);
			};
			newEntry.SubEntries.push_back(componentEntry);
		}

		if (entity.HasComponent<ECS::OnUpdateComponent>())
		{
			componentEntry.Label = "On Update";
			componentEntry.ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::OnUpdate, Assets::EmptyHandle);
			componentEntry.IconHandle = EditorUI::EditorUIService::s_IconFunction;
			componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
			{
				ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
				s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
				s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::OnUpdate);
			};
			newEntry.SubEntries.push_back(componentEntry);
		}

		if (entity.HasComponent<ECS::AIStateComponent>())
		{
			componentEntry.Label = "AI State";
			componentEntry.ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::AIState, Assets::EmptyHandle);
			componentEntry.IconHandle = EditorUI::EditorUIService::s_IconAI;
			componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
			{
				ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
				s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
				s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::AIState);
			};
			newEntry.SubEntries.push_back(componentEntry);
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
			componentEntry.Label = component->m_Name;
			componentEntry.ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::ProjectComponent, handle);
			componentEntry.IconHandle = EditorUI::EditorUIService::s_IconEntity;
			componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
			{
				ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
				s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
				s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::ProjectComponent);
				SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)entry.ProvidedData.get();
				s_EditorApp->m_SceneEditorPanel->SetDisplayedProjectComponent(entryData.m_ProjectComponentHandle);
			};
			newEntry.SubEntries.push_back(componentEntry);
		}
			
		m_SceneHierarchyTree.InsertEntry(newEntry);
	}

	void SceneEditorPanel::InitializeSceneHierarchy()
	{
		m_MainSceneHeader.Label = "No Scene Name";
		m_MainSceneHeader.EditColorActive = false;
		m_MainSceneHeader.AddToSelectionList("Create New Scene", []()
		{
			s_EditorApp->NewSceneDialog();
		});
		m_MainSceneHeader.AddToSelectionList("Open Scene", []()
		{
			s_EditorApp->OpenSceneDialog();
		});
		m_MainSceneHeader.AddToSelectionList("Create Entity", []()
		{
			Scenes::SceneService::GetActiveScene()->CreateEntity("Empty Entity");
		});
		

		m_SceneHierarchyTree.Label = "Scene Hierarchy";
		m_SceneHierarchyTree.OnRefresh = [&]()
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

		m_AddComponent.Label = "Add Component";
		m_AddComponent.Flags = EditorUI::SelectOption_PopupOnly;
		m_AddComponent.PopupAction = [&]()
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

			m_AddComponent.CurrentOption = { "None", Assets::EmptyHandle };
		};

		m_AddComponent.ConfirmAction = [&](const EditorUI::OptionEntry& option)
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
			componentEntry.Handle = (uint64_t)entity;
			
			// Check for a project component
			if (option.Handle != Assets::EmptyHandle)
			{
				// Add component to entity & update tree
				Ref<ECS::ProjectComponent> component = Assets::AssetService::GetProjectComponent(option.Handle);
				KG_ASSERT(component);
				entity.AddProjectComponentData(option.Handle);
				componentEntry.Label = component->m_Name;
				componentEntry.ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::ProjectComponent, option.Handle);
				componentEntry.IconHandle = EditorUI::EditorUIService::s_IconEntity;
				componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
					s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::ProjectComponent);
					SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)entry.ProvidedData.get();
					s_EditorApp->m_SceneEditorPanel->SetDisplayedProjectComponent(entryData.m_ProjectComponentHandle);
				};
				currentEntry->SubEntries.push_back(componentEntry);
				return;
			}

			// Exit if no option is selected
			if (option.Label == "None")
			{
				return;
			}


			// Check for engine components 
			if (option.Label == "Camera")
			{
				entity.AddComponent<ECS::CameraComponent>();
				componentEntry.Label = "Camera";
				componentEntry.ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::Camera, Assets::EmptyHandle);
				componentEntry.IconHandle = EditorUI::EditorUIService::s_IconCamera;
				componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
					s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::Camera);
				};
				currentEntry->SubEntries.push_back(componentEntry);
				return;
			}
			if (option.Label == "Shape")
			{
				entity.AddComponent<ECS::ShapeComponent>();
				componentEntry.Label = "Shape";
				componentEntry.ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::Shape, Assets::EmptyHandle);
				componentEntry.IconHandle = EditorUI::EditorUIService::s_IconEntity;
				componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
					s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::Shape);
				};
				currentEntry->SubEntries.push_back(componentEntry);
				return;
			}
			if (option.Label == "Rigidbody 2D")
			{
				entity.AddComponent<ECS::Rigidbody2DComponent>();
				componentEntry.Label = "Rigidbody 2D";
				componentEntry.ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::Rigidbody2D, Assets::EmptyHandle);
				componentEntry.IconHandle = EditorUI::EditorUIService::s_IconRigidBody;
				componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
					s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::Rigidbody2D);
				};
				currentEntry->SubEntries.push_back(componentEntry);
				return;
			}
			if (option.Label == "Box Collider 2D")
			{
				entity.AddComponent<ECS::BoxCollider2DComponent>();
				componentEntry.Label = "Box Collider 2D";
				componentEntry.ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::BoxCollider2D, Assets::EmptyHandle);
				componentEntry.IconHandle = EditorUI::EditorUIService::s_IconBoxCollider;
				componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
					s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::BoxCollider2D);
				};
				currentEntry->SubEntries.push_back(componentEntry);
				return;
			}
			if (option.Label == "Circle Collider 2D")
			{
				entity.AddComponent<ECS::CircleCollider2DComponent>();
				componentEntry.Label = "Circle Collider 2D";
				componentEntry.ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::CircleCollider2D, Assets::EmptyHandle);
				componentEntry.IconHandle = EditorUI::EditorUIService::s_IconCircleCollider;
				componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
					s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::CircleCollider2D);
				};
				currentEntry->SubEntries.push_back(componentEntry);
				return;
			}

			if (option.Label == "On Update")
			{
				entity.AddComponent<ECS::OnUpdateComponent>();
				componentEntry.Label = "On Update";
				componentEntry.ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::OnUpdate, Assets::EmptyHandle);
				componentEntry.IconHandle = EditorUI::EditorUIService::s_IconFunction;
				componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
					s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::OnUpdate);
				};
				currentEntry->SubEntries.push_back(componentEntry);
				return;
			}

			if (option.Label == "AI State")
			{
				entity.AddComponent<ECS::AIStateComponent>();
				componentEntry.Label = "AI State";
				componentEntry.ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::AIState, Assets::EmptyHandle);
				componentEntry.IconHandle = EditorUI::EditorUIService::s_IconAI;
				componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
					s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::AIState);
				};
				currentEntry->SubEntries.push_back(componentEntry);
				return;
			}

			if (option.Label == "On Create")
			{
				entity.AddComponent<ECS::OnCreateComponent>();
				componentEntry.Label = "On Create";
				componentEntry.ProvidedData = CreateRef<SceneEditorTreeEntryData>(ECS::ComponentType::OnCreate, Assets::EmptyHandle);
				componentEntry.IconHandle = EditorUI::EditorUIService::s_IconFunction;
				componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
					s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::OnCreate);
				};
				currentEntry->SubEntries.push_back(componentEntry);
				return;
			}

			KG_ERROR("Invalid option selected to add as component!");

		};
	}

	void SceneEditorPanel::InitializeTagComponent()
	{
		m_TagHeader.Label = "Tag";
		m_TagHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_TagHeader.Expanded = true;

		m_TagEdit.Label = "Tag Label";
		m_TagEdit.Flags |= EditorUI::EditText_Indented;
		m_TagEdit.ConfirmAction = [&](EditorUI::EditTextSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (entity && entity.HasComponent<ECS::TagComponent>())
			{
				auto& component = entity.GetComponent<ECS::TagComponent>();
				component.Tag = m_TagEdit.CurrentOption;

				m_SceneHierarchyTree.EditDepth([](EditorUI::TreeEntry& entry) 
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID((entt::entity)(int32_t)entry.Handle);
					KG_ASSERT(entity);
					if (entity.GetUUID() == Scenes::SceneService::GetActiveScene()->GetSelectedEntity()->GetUUID())
					{
						entry.Label = entity.GetComponent<ECS::TagComponent>().Tag;
					}
				}, 0);
			}
		};

		m_TagGroupEdit.Label = "Tag Group";
		m_TagGroupEdit.Flags |= EditorUI::EditText_Indented;
		m_TagGroupEdit.ConfirmAction = [&](EditorUI::EditTextSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (entity && entity.HasComponent<ECS::TagComponent>())
			{
				ECS::TagComponent& component = entity.GetComponent<ECS::TagComponent>();
				component.Group = m_TagGroupEdit.CurrentOption;
			}
		};
	}

	void SceneEditorPanel::InitializeTransformComponent()
	{
		m_TransformHeader.Label = "Transform";
		m_TransformHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_TransformHeader.Expanded = true;

		m_TransformEditTranslation.Label = "Translation";
		m_TransformEditTranslation.Flags = EditorUI::EditVec3_Indented;
		m_TransformEditTranslation.ConfirmAction = [&](EditorUI::EditVec3Spec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::TransformComponent>())
			{
				KG_ERROR("Attempt to edit entity transform component when none exists!");
				return;
			}
			auto& transformComp = entity.GetComponent<ECS::TransformComponent>();
			transformComp.Translation = m_TransformEditTranslation.CurrentVec3;
		};
		
		m_TransformEditScale.Label = "Scale";
		m_TransformEditScale.Flags = EditorUI::EditVec3_Indented;
		m_TransformEditScale.ConfirmAction = [&](EditorUI::EditVec3Spec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::TransformComponent>())
			{
				KG_ERROR("Attempt to edit entity transform component when none exists!");
				return;
			}
			auto& transformComp = entity.GetComponent<ECS::TransformComponent>();
			transformComp.Scale = m_TransformEditScale.CurrentVec3;
		};
		m_TransformEditRotation.Label = "Rotation";
		m_TransformEditRotation.Flags = EditorUI::EditVec3_Indented;
		m_TransformEditRotation.ConfirmAction = [&](EditorUI::EditVec3Spec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::TransformComponent>())
			{
				KG_ERROR("Attempt to edit entity transform component when none exists!");
				return;
			}
			auto& transformComp = entity.GetComponent<ECS::TransformComponent>();
			transformComp.Rotation = m_TransformEditRotation.CurrentVec3;
		};
		
	}

	void SceneEditorPanel::InitializeRigidbody2DComponent()
	{
		m_Rigidbody2DHeader.Label = "Rigid Body 2D";
		m_Rigidbody2DHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_Rigidbody2DHeader.Expanded = true;
		m_Rigidbody2DHeader.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderSpec& spec)
		{
			EngineService::SubmitToMainThread([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<ECS::Rigidbody2DComponent>())
				{
					for (auto& entry : m_SceneHierarchyTree.GetTreeEntries())
					{
						if ((uint32_t)entry.Handle == (uint32_t)entity)
						{
							EditorUI::TreePath newPath {};
							for (auto& subEntry : entry.SubEntries)
							{
								SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.ProvidedData.get();
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

		m_Rigidbody2DType.Label = "Interaction Type";
		m_Rigidbody2DType.FirstOptionLabel = "Static";
		m_Rigidbody2DType.SecondOptionLabel = "Dynamic";
		m_Rigidbody2DType.Flags |= EditorUI::RadioSelector_Indented;
		m_Rigidbody2DType.SelectAction = [&]()
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::Rigidbody2DComponent>())
			{
				KG_ERROR("Attempt to edit entity rigid body 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::Rigidbody2DComponent>();

			if (m_Rigidbody2DType.SelectedOption == 0)
			{
				component.Type = ECS::Rigidbody2DComponent::BodyType::Static;
				return;
			}
			if (m_Rigidbody2DType.SelectedOption == 1)
			{
				component.Type = ECS::Rigidbody2DComponent::BodyType::Dynamic;
				return;
			}

			KG_ERROR("Invalid SelectedOption");
		};


		// Set Shape Add Fixed Rotation Option
		m_RigidBody2DFixedRotation.Label = "Use Fixed Rotation";
		m_RigidBody2DFixedRotation.Flags |= EditorUI::Checkbox_Indented;
		m_RigidBody2DFixedRotation.ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::Rigidbody2DComponent>())
			{
				KG_ERROR("Attempt to edit entity rigid body 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::Rigidbody2DComponent>();
			component.FixedRotation = spec.CurrentBoolean;
		};

		m_SelectRigidBody2DCollisionStartScript.Label = "On Physics Collision Start";
		m_SelectRigidBody2DCollisionStartScript.Flags |= EditorUI::SelectOption_Indented;
		m_SelectRigidBody2DCollisionStartScript.CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectRigidBody2DCollisionStartScript.PopupAction = [&]()
		{
			m_SelectRigidBody2DCollisionStartScript.ClearOptions();
			m_SelectRigidBody2DCollisionStartScript.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				KG_ASSERT(script);

				if (script->m_FuncType != WrappedFuncType::Bool_UInt64UInt64)
				{
					continue;
				}
				m_SelectRigidBody2DCollisionStartScript.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType) +
					"::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}
		};

		m_SelectRigidBody2DCollisionStartScript.ConfirmAction = [](const EditorUI::OptionEntry& entry)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::Rigidbody2DComponent>())
			{
				KG_ERROR("Attempt to edit entity CollisionStart component when none exists!");
				return;
			}
			ECS::Rigidbody2DComponent& component = entity.GetComponent<ECS::Rigidbody2DComponent>();

			// Check for empty entry
			if (entry.Handle == Assets::EmptyHandle)
			{
				component.OnCollisionStartScriptHandle = Assets::EmptyHandle;
				component.OnCollisionStartScript = nullptr;
			}
			// Check for a valid entry, and Update if applicable
			component.OnCollisionStartScriptHandle = entry.Handle;
			component.OnCollisionStartScript = Assets::AssetService::GetScript(entry.Handle);
		};

		m_SelectRigidBody2DCollisionEndScript.Label = "On Physics Collision End";
		m_SelectRigidBody2DCollisionEndScript.Flags |= EditorUI::SelectOption_Indented;
		m_SelectRigidBody2DCollisionEndScript.CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectRigidBody2DCollisionEndScript.PopupAction = [&]()
		{
			m_SelectRigidBody2DCollisionEndScript.ClearOptions();
			m_SelectRigidBody2DCollisionEndScript.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				KG_ASSERT(script);

				if (script->m_FuncType != WrappedFuncType::Bool_UInt64UInt64)
				{
					continue;
				}
				m_SelectRigidBody2DCollisionEndScript.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType) +
					"::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}
		};

		m_SelectRigidBody2DCollisionEndScript.ConfirmAction = [](const EditorUI::OptionEntry& entry)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::Rigidbody2DComponent>())
			{
				KG_ERROR("Attempt to edit entity CollisionEnd component when none exists!");
				return;
			}
			ECS::Rigidbody2DComponent& component = entity.GetComponent<ECS::Rigidbody2DComponent>();

			// Check for empty entry
			if (entry.Handle == Assets::EmptyHandle)
			{
				component.OnCollisionEndScriptHandle = Assets::EmptyHandle;
				component.OnCollisionEndScript = nullptr;
			}
			// Check for a valid entry, and Update if applicable
			component.OnCollisionEndScriptHandle = entry.Handle;
			component.OnCollisionEndScript = Assets::AssetService::GetScript(entry.Handle);
		};

	}

	void SceneEditorPanel::InitializeBoxCollider2DComponent()
	{
		m_BoxCollider2DHeader.Label = "Box Collider 2D";
		m_BoxCollider2DHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_BoxCollider2DHeader.Expanded = true;
		m_BoxCollider2DHeader.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderSpec& spec)
		{
			EngineService::SubmitToMainThread([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<ECS::BoxCollider2DComponent>())
				{
					for (auto& entry : m_SceneHierarchyTree.GetTreeEntries())
					{
						if ((uint32_t)entry.Handle == (uint32_t)entity)
						{
							EditorUI::TreePath newPath {};
							for (auto& subEntry : entry.SubEntries)
							{
								SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.ProvidedData.get();
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

		m_BoxColliderOffset.Label = "Offset";
		m_BoxColliderOffset.Flags |= EditorUI::EditVec2_Indented;
		m_BoxColliderOffset.ConfirmAction = [&](EditorUI::EditVec2Spec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
			component.Offset = m_BoxColliderOffset.CurrentVec2;
		};

		m_BoxColliderSize.Label = "Size";
		m_BoxColliderSize.Flags |= EditorUI::EditVec2_Indented;
		m_BoxColliderSize.ConfirmAction = [&](EditorUI::EditVec2Spec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
			component.Size = m_BoxColliderSize.CurrentVec2;
		};

		m_BoxColliderDensity.Label = "Density";
		m_BoxColliderDensity.Flags |= EditorUI::EditFloat_Indented;
		m_BoxColliderDensity.ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
			component.Density = m_BoxColliderDensity.CurrentFloat;
		};

		m_BoxColliderFriction.Label = "Friction";
		m_BoxColliderFriction.Flags |= EditorUI::EditFloat_Indented;
		m_BoxColliderFriction.ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
			component.Friction = m_BoxColliderFriction.CurrentFloat;
		};

		m_BoxColliderRestitution.Label = "Restitution";
		m_BoxColliderRestitution.Flags |= EditorUI::EditFloat_Indented;
		m_BoxColliderRestitution.ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
			component.Restitution = m_BoxColliderRestitution.CurrentFloat;
		};

		m_BoxColliderRestitutionThreshold.Label = "Restitution Threshold";
		m_BoxColliderRestitutionThreshold.Flags |= EditorUI::EditFloat_Indented;
		m_BoxColliderRestitutionThreshold.ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
			component.RestitutionThreshold = m_BoxColliderRestitutionThreshold.CurrentFloat;
		};

		// Set whether box collider is treated as a sensor
		m_BoxColliderIsSensor.Label = "Is Sensor";
		m_BoxColliderIsSensor.Flags |= EditorUI::Checkbox_Indented;
		m_BoxColliderIsSensor.ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::BoxCollider2DComponent>();
			component.IsSensor = spec.CurrentBoolean;
		};
	}
	void SceneEditorPanel::InitializeCircleCollider2DComponent()
	{
		m_CircleCollider2DHeader.Label = "Circle Collider 2D";
		m_CircleCollider2DHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_CircleCollider2DHeader.Expanded = true;
		m_CircleCollider2DHeader.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderSpec& spec)
		{
			EngineService::SubmitToMainThread([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<ECS::CircleCollider2DComponent>())
				{
					for (auto& entry : m_SceneHierarchyTree.GetTreeEntries())
					{
						if ((uint32_t)entry.Handle == (uint32_t)entity)
						{
							EditorUI::TreePath newPath {};
							for (auto& subEntry : entry.SubEntries)
							{
								SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.ProvidedData.get();
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

		m_CircleColliderOffset.Label = "Offset";
		m_CircleColliderOffset.Flags |= EditorUI::EditVec2_Indented;
		m_CircleColliderOffset.ConfirmAction = [&](EditorUI::EditVec2Spec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
			component.Offset = m_CircleColliderOffset.CurrentVec2;
		};

		m_CircleColliderRadius.Label = "Radius";
		m_CircleColliderRadius.Flags |= EditorUI::EditFloat_Indented;
		m_CircleColliderRadius.ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
			component.Radius = m_CircleColliderRadius.CurrentFloat;
		};

		m_CircleColliderDensity.Label = "Density";
		m_CircleColliderDensity.Flags |= EditorUI::EditFloat_Indented;
		m_CircleColliderDensity.ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
			component.Density = m_CircleColliderDensity.CurrentFloat;
		};

		m_CircleColliderFriction.Label = "Friction";
		m_CircleColliderFriction.Flags |= EditorUI::EditFloat_Indented;
		m_CircleColliderFriction.ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
			component.Friction = m_CircleColliderFriction.CurrentFloat;
		};

		m_CircleColliderRestitution.Label = "Restitution";
		m_CircleColliderRestitution.Flags |= EditorUI::EditFloat_Indented;
		m_CircleColliderRestitution.ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
			component.Restitution = m_CircleColliderRestitution.CurrentFloat;
		};

		m_CircleColliderRestitutionThreshold.Label = "Restitution Threshold";
		m_CircleColliderRestitutionThreshold.Flags |= EditorUI::EditFloat_Indented;
		m_CircleColliderRestitutionThreshold.ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
			component.RestitutionThreshold = m_CircleColliderRestitutionThreshold.CurrentFloat;
		};

		// Set whether circle collider is treated as a sensor
		m_CircleColliderIsSensor.Label = "Is Sensor";
		m_CircleColliderIsSensor.Flags |= EditorUI::Checkbox_Indented;
		m_CircleColliderIsSensor.ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CircleCollider2DComponent>();
			component.IsSensor = spec.CurrentBoolean;
		};
	}

	void SceneEditorPanel::InitializeCameraComponent()
	{
		m_CameraHeader.Label = "Camera";
		m_CameraHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_CameraHeader.Expanded = true;
		m_CameraHeader.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderSpec& spec)
		{
			EngineService::SubmitToMainThread([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<ECS::CameraComponent>())
				{
					for (auto& entry : m_SceneHierarchyTree.GetTreeEntries())
					{
						if ((uint32_t)entry.Handle == (uint32_t)entity)
						{
							EditorUI::TreePath newPath {};
							for (auto& subEntry : entry.SubEntries)
							{
								SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.ProvidedData.get();
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
		m_CameraPrimary.Label = "Primary Camera";
		m_CameraPrimary.Flags |= EditorUI::Checkbox_Indented;
		m_CameraPrimary.ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CameraComponent>();
			component.Primary = spec.CurrentBoolean;
		};

		m_CameraProjection.Label = "Projection Type";
		m_CameraProjection.Flags |= EditorUI::RadioSelector_Indented;
		m_CameraProjection.FirstOptionLabel = "Perspective";
		m_CameraProjection.SecondOptionLabel = "Orthographic";
		m_CameraProjection.SelectAction = [&]()
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CameraComponent>();

			if (m_CameraProjection.SelectedOption == 0)
			{
				component.Camera.SetProjectionType(Scenes::SceneCamera::ProjectionType::Perspective);
				return;
			}
			if (m_CameraProjection.SelectedOption == 1)
			{
				component.Camera.SetProjectionType(Scenes::SceneCamera::ProjectionType::Orthographic);
				return;
			}
			KG_ERROR("Invalid selection option provided");
		};

		m_CameraOrthographicSize.Label = "Size";
		m_CameraOrthographicSize.Flags |= EditorUI::EditFloat_Indented;
		m_CameraOrthographicSize.ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CameraComponent>();
			component.Camera.SetOrthographicSize(m_CameraOrthographicSize.CurrentFloat);
		};

		m_CameraOrthographicNearPlane.Label = "Near Plane";
		m_CameraOrthographicNearPlane.Flags |= EditorUI::EditFloat_Indented;
		m_CameraOrthographicNearPlane.ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CameraComponent>();
			component.Camera.SetOrthographicNearClip(m_CameraOrthographicNearPlane.CurrentFloat);
		};

		m_CameraOrthographicFarPlane.Label = "Far Plane";
		m_CameraOrthographicFarPlane.Flags |= EditorUI::EditFloat_Indented;
		m_CameraOrthographicFarPlane.ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CameraComponent>();
			component.Camera.SetOrthographicFarClip(m_CameraOrthographicFarPlane.CurrentFloat);
		};

		m_CameraPerspectiveFOV.Label = "Vertical FOV";
		m_CameraPerspectiveFOV.Flags |= EditorUI::EditFloat_Indented;
		m_CameraPerspectiveFOV.ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CameraComponent>();
			component.Camera.SetPerspectiveVerticalFOV(m_CameraPerspectiveFOV.CurrentFloat);
		};

		m_CameraPerspectiveNearPlane.Label = "Near Plane";
		m_CameraPerspectiveNearPlane.Flags |= EditorUI::EditFloat_Indented;
		m_CameraPerspectiveNearPlane.ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CameraComponent>();
			component.Camera.SetPerspectiveNearClip(m_CameraPerspectiveNearPlane.CurrentFloat);
		};

		m_CameraPerspectiveFarPlane.Label = "Far Plane";
		m_CameraPerspectiveFarPlane.Flags |= EditorUI::EditFloat_Indented;
		m_CameraPerspectiveFarPlane.ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CameraComponent>();
			component.Camera.SetPerspectiveFarClip(m_CameraPerspectiveFarPlane.CurrentFloat);
		};
	}

	void SceneEditorPanel::InitializeOnUpdateComponent()
	{
		m_OnUpdateHeader.Label = "On Update";
		m_OnUpdateHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_OnUpdateHeader.Expanded = true;
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
						if ((uint32_t)entry.Handle == (uint32_t)entity)
						{
							for (auto& subEntry : entry.SubEntries)
							{
								SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.ProvidedData.get();
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

		m_SelectOnUpdateScript.Label = "On Update Script";
		m_SelectOnUpdateScript.Flags |= EditorUI::SelectOption_Indented;
		m_SelectOnUpdateScript.CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectOnUpdateScript.PopupAction = [&]()
		{
			m_SelectOnUpdateScript.ClearOptions();
			m_SelectOnUpdateScript.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				KG_ASSERT(script);

				if (script->m_FuncType != WrappedFuncType::Void_UInt64Float)
				{
					continue;
				}
				m_SelectOnUpdateScript.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType) +
					"::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}
		};

		m_SelectOnUpdateScript.ConfirmAction = [](const EditorUI::OptionEntry& entry)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::OnUpdateComponent>())
			{
				KG_ERROR("Attempt to edit entity OnUpdate component when none exists!");
				return;
			}
			ECS::OnUpdateComponent& component = entity.GetComponent<ECS::OnUpdateComponent>();

			// Check for empty entry
			if (entry.Handle == Assets::EmptyHandle)
			{
				component.OnUpdateScriptHandle = Assets::EmptyHandle;
				component.OnUpdateScript = nullptr;
			}
			// Check for a valid entry, and Update if applicable
			component.OnUpdateScriptHandle = entry.Handle;
			component.OnUpdateScript = Assets::AssetService::GetScript(entry.Handle);
		};
	}

	void SceneEditorPanel::InitializeOnCreateComponent()
	{
		m_OnCreateHeader.Label = "On Create";
		m_OnCreateHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_OnCreateHeader.Expanded = true;
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
						if ((uint32_t)entry.Handle == (uint32_t)entity)
						{
							for (auto& subEntry : entry.SubEntries)
							{
								SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.ProvidedData.get();
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

		m_SelectOnCreateScript.Label = "On Create Script";
		m_SelectOnCreateScript.Flags |= EditorUI::SelectOption_Indented;
		m_SelectOnCreateScript.CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectOnCreateScript.PopupAction = [&]()
		{
			m_SelectOnCreateScript.ClearOptions();
			m_SelectOnCreateScript.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetScriptRegistry())
			{
				Ref<Scripting::Script> script = Assets::AssetService::GetScript(handle);
				KG_ASSERT(script);

				if (script->m_FuncType != WrappedFuncType::Void_UInt64)
				{
					continue;
				}
				m_SelectOnCreateScript.AddToOptions(Utility::ScriptTypeToString(script->m_ScriptType) +
					"::" + script->m_SectionLabel, script->m_ScriptName, handle);
			}
		};

		m_SelectOnCreateScript.ConfirmAction = [](const EditorUI::OptionEntry& entry) 
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::OnCreateComponent>())
			{
				KG_ERROR("Attempt to edit entity OnCreate component when none exists!");
				return;
			}
			ECS::OnCreateComponent& component = entity.GetComponent<ECS::OnCreateComponent>();

			// Check for empty entry
			if (entry.Handle == Assets::EmptyHandle)
			{
				component.OnCreateScriptHandle = Assets::EmptyHandle;
				component.OnCreateScript = nullptr;
			}
			// Check for a valid entry, and Create if applicable
			component.OnCreateScriptHandle = entry.Handle;
			component.OnCreateScript = Assets::AssetService::GetScript(entry.Handle);
		};
	}

	void SceneEditorPanel::InitializeAIComponent()
	{
		// Set up AI state header
		m_AIStateHeader.Label = "AI State";
		m_AIStateHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_AIStateHeader.Expanded = true;
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
									if ((uint32_t)entry.Handle == (uint32_t)entity)
									{
										for (auto& subEntry : entry.SubEntries)
										{
											SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.ProvidedData.get();
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
		m_SelectGlobalState.Label = "Global State";
		m_SelectGlobalState.Flags |= EditorUI::SelectOption_Indented;
		m_SelectGlobalState.CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectGlobalState.PopupAction = [&]()
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

		m_SelectGlobalState.ConfirmAction = [](const EditorUI::OptionEntry& entry)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::AIStateComponent>())
			{
				KG_ERROR("Attempt to edit entity AIState component when none exists!");
				return;
			}
			ECS::AIStateComponent& component = entity.GetComponent<ECS::AIStateComponent>();

			// Check for empty entry
			if (entry.Handle == Assets::EmptyHandle)
			{
				component.GlobalStateHandle = Assets::EmptyHandle;
				component.GlobalStateReference = nullptr;
			}
			// Check for a valid entry, and Update if applicable
			component.GlobalStateHandle = entry.Handle;
			component.GlobalStateReference = Assets::AssetService::GetAIState(entry.Handle);
		};

		// Set up current state select options widget
		m_SelectCurrentState.Label = "Current State";
		m_SelectCurrentState.Flags |= EditorUI::SelectOption_Indented;
		m_SelectCurrentState.CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectCurrentState.PopupAction = [&]()
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

		m_SelectCurrentState.ConfirmAction = [](const EditorUI::OptionEntry& entry)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::AIStateComponent>())
			{
				KG_ERROR("Attempt to edit entity AIState component when none exists!");
				return;
			}
			ECS::AIStateComponent& component = entity.GetComponent<ECS::AIStateComponent>();

			// Check for empty entry
			if (entry.Handle == Assets::EmptyHandle)
			{
				component.CurrentStateHandle = Assets::EmptyHandle;
				component.CurrentStateReference = nullptr;
			}
			// Check for a valid entry, and Update if applicable
			component.CurrentStateHandle = entry.Handle;
			component.CurrentStateReference = Assets::AssetService::GetAIState(entry.Handle);
		};

		// Set up previous state select options widget
		m_SelectPreviousState.Label = "Previous State";
		m_SelectPreviousState.Flags |= EditorUI::SelectOption_Indented;
		m_SelectPreviousState.CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectPreviousState.PopupAction = [&]()
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

		m_SelectPreviousState.ConfirmAction = [](const EditorUI::OptionEntry& entry)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::AIStateComponent>())
			{
				KG_ERROR("Attempt to edit entity AIState component when none exists!");
				return;
			}
			ECS::AIStateComponent& component = entity.GetComponent<ECS::AIStateComponent>();

			// Check for empty entry
			if (entry.Handle == Assets::EmptyHandle)
			{
				component.PreviousStateHandle = Assets::EmptyHandle;
				component.PreviousStateReference = nullptr;
			}
			// Check for a valid entry, and Update if applicable
			component.PreviousStateHandle = entry.Handle;
			component.PreviousStateReference = Assets::AssetService::GetAIState(entry.Handle);
		};

	}

	void SceneEditorPanel::InitializeShapeComponent()
	{
		m_ShapeHeader.Label = "Shape";
		m_ShapeHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_ShapeHeader.Expanded = true;
		m_ShapeHeader.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderSpec& spec)
		{
			EngineService::SubmitToMainThread([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<ECS::ShapeComponent>())
				{
					for (auto& entry : m_SceneHierarchyTree.GetTreeEntries())
					{
						if ((uint32_t)entry.Handle == (uint32_t)entity)
						{
							EditorUI::TreePath newPath {};
							for (auto& subEntry : entry.SubEntries)
							{
								SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.ProvidedData.get();
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

		m_ShapeSelect.Label = "Mesh";
		m_ShapeSelect.Flags |= EditorUI::SelectOption_Indented;
		m_ShapeSelect.PopupAction = [&]()
		{
			m_ShapeSelect.ClearOptions();
			m_ShapeSelect.AddToOptions("Clear", "None", Assets::EmptyHandle);
			m_ShapeSelect.AddToOptions("All Shapes", "Quad", Assets::EmptyHandle);
			m_ShapeSelect.AddToOptions("All Shapes", "Pyramid", Assets::EmptyHandle);
			m_ShapeSelect.AddToOptions("All Shapes", "Cube", Assets::EmptyHandle);
		};
		m_ShapeSelect.ConfirmAction = [&](const EditorUI::OptionEntry& entry)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			Rendering::Shape* shape {nullptr};

			if (entry.Label == "None")
			{
				shape = &Rendering::Shape::s_None;
			}
			if (entry.Label == "Quad")
			{
				shape = &Rendering::Shape::s_Quad;
			}
			if (entry.Label == "Cube")
			{
				shape = &Rendering::Shape::s_Cube;
			}
			if (entry.Label == "Pyramid")
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

		m_ShapeColorType.Label = "Color Type";
		m_ShapeColorType.Flags |= EditorUI::SelectOption_Indented;
		m_ShapeColorType.PopupAction = [&]()
		{
			m_ShapeColorType.ClearOptions();
			m_ShapeColorType.AddToOptions("Clear", "None", Assets::EmptyHandle);
			m_ShapeColorType.AddToOptions("All Types", "FlatColor", Assets::EmptyHandle);
			m_ShapeColorType.AddToOptions("All Types", "VertexColor", Assets::EmptyHandle);
		};
		m_ShapeColorType.ConfirmAction = [&](const EditorUI::OptionEntry& entry)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			if (entry.Label == "None")
			{
				component.ShaderSpecification.ColorInput = Rendering::ColorInputType::None;
				UpdateShapeComponent();
			}
			if (entry.Label == "FlatColor")
			{
				component.ShaderSpecification.ColorInput = Rendering::ColorInputType::FlatColor;
				UpdateShapeComponent();
				Rendering::Shader::SetDataAtInputLocation<Math::vec4>({ 1.0f, 1.0f, 1.0f, 1.0f }, "a_Color", component.ShaderData, component.Shader);
			}
			if (entry.Label == "VertexColor")
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

		m_ShapeColor.Label = "Flat Color";
		m_ShapeColor.Flags |= EditorUI::EditVec4_Indented | EditorUI::EditVec4_RGBA;
		m_ShapeColor.ConfirmAction = [&](EditorUI::EditVec4Spec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::ShapeComponent>())
			{
				KG_ERROR("Attempt to edit entity shape 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::ShapeComponent>();
			Math::vec4* color = Rendering::Shader::GetInputLocation<Math::vec4>("a_Color", component.ShaderData, component.Shader);
			*color = m_ShapeColor.CurrentVec4;
		};

		// Set Shape Add Texture Checkbox
		m_ShapeAddTexture.Label = "Use Texture";
		m_ShapeAddTexture.Flags |= EditorUI::Checkbox_Indented;
		m_ShapeAddTexture.ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			spec.CurrentBoolean ? component.ShaderSpecification.TextureInput = Rendering::TextureInputType::ColorTexture :
				component.ShaderSpecification.TextureInput = Rendering::TextureInputType::None;
			UpdateShapeComponent();
			// Checkbox is switched on
			if (spec.CurrentBoolean)
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
			if (!spec.CurrentBoolean)
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

		m_ShapeSetTexture.Label = "Select Texture";
		m_ShapeSetTexture.Flags |= EditorUI::SelectOption_Indented;
		m_ShapeSetTexture.PopupAction = [&]()
		{
			m_ShapeSetTexture.ClearOptions();
			for (auto& [handle, asset] : Assets::AssetService::GetTexture2DRegistry())
			{
				m_ShapeSetTexture.AddToOptions("All Textures", asset.Data.FileLocation.string(), handle);
			}
		};
		m_ShapeSetTexture.ConfirmAction = [&](const EditorUI::OptionEntry& entry)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			if (entry.Handle == Assets::EmptyHandle)
			{
				Buffer textureBuffer{ 4 };
				textureBuffer.SetDataToByte(0xff);
				component.TextureHandle = Assets::AssetService::ImportNewTextureFromData(textureBuffer, 1, 1, 4);
				component.Texture = Assets::AssetService::GetTexture2D(component.TextureHandle);
				textureBuffer.Release();
			}

			if (!Assets::AssetService::GetTexture2DRegistry().contains(entry.Handle))
			{
				KG_WARN("Could not locate texture in asset registry!");
				return;
			}

			component.TextureHandle = entry.Handle;
			component.Texture = Assets::AssetService::GetTexture2D(entry.Handle);
		};

		m_ShapeTilingFactor.Label = "Tiling Factor";
		m_ShapeTilingFactor.Flags |= EditorUI::EditFloat_Indented;
		m_ShapeTilingFactor.ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::ShapeComponent>())
			{
				KG_ERROR("Attempt to edit entity shape 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::ShapeComponent>();
			float* tilingFactor = Rendering::Shader::GetInputLocation<float>("a_TilingFactor", component.ShaderData, component.Shader);
			*tilingFactor = m_ShapeTilingFactor.CurrentFloat;
		};

		// Set Shape Circle Option
		m_ShapeAddCircle.Label = "Use Circle Shape";
		m_ShapeAddCircle.Flags |= EditorUI::Checkbox_Indented;
		m_ShapeAddCircle.ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			component.ShaderSpecification.AddCircleShape = spec.CurrentBoolean;
			UpdateShapeComponent();
			if (spec.CurrentBoolean)
			{
				Rendering::Shader::SetDataAtInputLocation<float>(1.0f, "a_Thickness", component.ShaderData, component.Shader);
				Rendering::Shader::SetDataAtInputLocation<float>(0.005f, "a_Fade", component.ShaderData, component.Shader);
			}
		};

		m_ShapeCircleThickness.Label = "Circle Thickness";
		m_ShapeCircleThickness.Flags |= EditorUI::EditFloat_Indented;
		m_ShapeCircleThickness.ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::ShapeComponent>())
			{
				KG_ERROR("Attempt to edit entity shape 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::ShapeComponent>();
			float* thickness = Rendering::Shader::GetInputLocation<float>("a_Thickness", component.ShaderData, component.Shader);
			*thickness = m_ShapeCircleThickness.CurrentFloat;
		};

		m_ShapeCircleFade.Label = "Circle Fade";
		m_ShapeCircleFade.Flags |= EditorUI::EditFloat_Indented;
		m_ShapeCircleFade.ConfirmAction = [&](EditorUI::EditFloatSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::ShapeComponent>())
			{
				KG_ERROR("Attempt to edit entity shape 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::ShapeComponent>();
			float* fade = Rendering::Shader::GetInputLocation<float>("a_Fade", component.ShaderData, component.Shader);
			*fade = m_ShapeCircleFade.CurrentFloat;
		};

		// Set Shape Add Projection Option
		m_ShapeAddProjection.Label = "Use Projection Matrix";
		m_ShapeAddProjection.Flags |= EditorUI::Checkbox_Indented;
		m_ShapeAddProjection.ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			component.ShaderSpecification.AddProjectionMatrix = spec.CurrentBoolean;
			UpdateShapeComponent();
		};

		// Set Shape Add Entity ID Option
		m_ShapeAddEntityID.Label = "Use Entity ID";
		m_ShapeAddEntityID.Flags |= EditorUI::Checkbox_Indented;
		m_ShapeAddEntityID.ConfirmAction = [&](EditorUI::CheckboxSpec& spec)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			component.ShaderSpecification.AddEntityID = spec.CurrentBoolean;
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
		newWidgetData.m_Header.Label = component->m_Name;
		newWidgetData.m_Header.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		newWidgetData.m_Header.Expanded = true;
		newWidgetData.m_Header.ProvidedData = CreateRef<Assets::AssetHandle>(projectComponentHandle);
		newWidgetData.m_Header.AddToSelectionList("Remove Component", [&](EditorUI::CollapsingHeaderSpec& spec)
		{
			EngineService::SubmitToMainThread([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				Assets::AssetHandle projectComponentHandle = *(Assets::AssetHandle*)spec.ProvidedData.get();
				if (entity.HasProjectComponentData(projectComponentHandle))
				{
					for (auto& entry : m_SceneHierarchyTree.GetTreeEntries())
					{
						if ((uint32_t)entry.Handle == (uint32_t)entity)
						{
							EditorUI::TreePath newPath {};
							for (auto& subEntry : entry.SubEntries)
							{
								SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)subEntry.ProvidedData.get();
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
				newFloatSpec.Label = currentName;
				newFloatSpec.Flags |= EditorUI::EditFloat_Indented;
				newFloatSpec.ProvidedData = CreateRef<ProjectComponentFieldInfo>(projectComponentHandle, iteration);
				newFloatSpec.ConfirmAction = [](EditorUI::EditFloatSpec& spec)
				{
					// Get component data pointer
					ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.ProvidedData.get();
					Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
					ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
					uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

					// Get field data pointer
					uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

					// Set the data
					*(float*)fieldDataRef = spec.CurrentFloat;
				};
				newWidgetData.m_Fields.push_back(newFloatSpec);
				break;
			case WrappedVarType::String:
				newStringSpec = {};
				newStringSpec.Label = currentName;
				newStringSpec.Flags |= EditorUI::EditText_Indented;
				newStringSpec.ProvidedData = CreateRef<ProjectComponentFieldInfo>(projectComponentHandle, iteration);
				newStringSpec.ConfirmAction = [](EditorUI::EditTextSpec& spec)
				{
					// Get component data pointer
					ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.ProvidedData.get();
					Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
					ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
					uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

					// Get field data pointer
					uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

					// Set the data
					*(std::string*)fieldDataRef = spec.CurrentOption;
				};
				newWidgetData.m_Fields.push_back(newStringSpec);
				break;
			case WrappedVarType::Bool:
				newBoolSpec = {};
				newBoolSpec.Label = currentName;
				newBoolSpec.Flags |= EditorUI::Checkbox_Indented;
				newBoolSpec.ProvidedData = CreateRef<ProjectComponentFieldInfo>(projectComponentHandle, iteration);
				newBoolSpec.ConfirmAction = [](EditorUI::CheckboxSpec& spec)
				{
					// Get component data pointer
					ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.ProvidedData.get();
					Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
					ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
					uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

					// Get field data pointer
					uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

					// Set the data
					*(bool*)fieldDataRef = spec.CurrentBoolean;
				};
				newWidgetData.m_Fields.push_back(newBoolSpec);
				break;
			case WrappedVarType::Vector3:
				newVector3Spec = {};
				newVector3Spec.Label = currentName;
				newVector3Spec.Flags |= EditorUI::EditVec3_Indented;
				newVector3Spec.ProvidedData = CreateRef<ProjectComponentFieldInfo>(projectComponentHandle, iteration);
				newVector3Spec.ConfirmAction = [](EditorUI::EditVec3Spec& spec)
				{
					// Get component data pointer
					ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.ProvidedData.get();
					Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
					ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
					uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

					// Get field data pointer
					uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

					// Set the data
					*(Math::vec3*)fieldDataRef = spec.CurrentVec3;
				};
				newWidgetData.m_Fields.push_back(newVector3Spec);
				break;
			case WrappedVarType::Integer32:
				newIntegerSpec = {};
				newIntegerSpec.Label = currentName;
				newIntegerSpec.Flags |= EditorUI::EditInteger_Indented;
				newIntegerSpec.ProvidedData = CreateRef<ProjectComponentFieldInfo>(projectComponentHandle, iteration);
				newIntegerSpec.ConfirmAction = [](EditorUI::EditIntegerSpec& spec)
				{
					// Get component data pointer
					ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.ProvidedData.get();
					Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
					ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
					uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

					// Get field data pointer
					uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

					// Set the data
					*(int32_t*)fieldDataRef = (int32_t)spec.CurrentInteger;
				};
				newWidgetData.m_Fields.push_back(newIntegerSpec);
				break;
			case WrappedVarType::UInteger16:
				newIntegerSpec = {};
				newIntegerSpec.Label = currentName;
				newIntegerSpec.Flags |= EditorUI::EditInteger_Indented;
				newIntegerSpec.ProvidedData = CreateRef<ProjectComponentFieldInfo>(projectComponentHandle, iteration);
				newIntegerSpec.ConfirmAction = [](EditorUI::EditIntegerSpec& spec)
				{
					// Get component data pointer
					ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.ProvidedData.get();
					Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
					ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
					uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

					// Get field data pointer
					uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

					// Set the data
					*(uint16_t*)fieldDataRef = (uint16_t)spec.CurrentInteger;
				};
				newWidgetData.m_Fields.push_back(newIntegerSpec);
				break;
			case WrappedVarType::UInteger32:
				newIntegerSpec = {};
				newIntegerSpec.Label = currentName;
				newIntegerSpec.Flags |= EditorUI::EditInteger_Indented;
				newIntegerSpec.ProvidedData = CreateRef<ProjectComponentFieldInfo>(projectComponentHandle, iteration);
				newIntegerSpec.ConfirmAction = [](EditorUI::EditIntegerSpec& spec)
				{
					// Get component data pointer
					ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.ProvidedData.get();
					Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
					ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
					uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

					// Get field data pointer
					uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

					// Set the data
					*(uint32_t*)fieldDataRef = (uint32_t)spec.CurrentInteger;
				};
				newWidgetData.m_Fields.push_back(newIntegerSpec);
				break;
			case WrappedVarType::UInteger64:
				newIntegerSpec = {};
				newIntegerSpec.Label = currentName;
				newIntegerSpec.Flags |= EditorUI::EditInteger_Indented;
				newIntegerSpec.ProvidedData = CreateRef<ProjectComponentFieldInfo>(projectComponentHandle, iteration);
				newIntegerSpec.ConfirmAction = [](EditorUI::EditIntegerSpec& spec)
				{
					// Get component data pointer
					ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.ProvidedData.get();
					Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
					ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
					uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

					// Get field data pointer
					uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

					// Set the data
					*(uint64_t*)fieldDataRef = (uint64_t)spec.CurrentInteger;
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
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName.CString(),
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
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EditorApp->m_ShowSceneHierarchy);

		if (!EditorUI::EditorUIService::IsCurrentWindowVisible())
		{
			EditorUI::EditorUIService::EndWindow();
			return;
		}

		if (Scenes::SceneService::GetActiveScene())
		{
			m_MainSceneHeader.Label = Assets::AssetService::GetSceneRegistry().at(
				Scenes::SceneService::GetActiveSceneHandle()).Data.FileLocation.string();
			
			EditorUI::EditorUIService::PanelHeader(m_MainSceneHeader);

			EditorUI::EditorUIService::Tree(m_SceneHierarchyTree);

			EditorUI::EditorUIService::SelectOption(m_AddComponent);
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
				m_SceneHierarchyTree.OnRefresh();
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
					s_EditorApp->m_SceneEditorPanel->SetSelectedEntity({});
				}
				uint32_t iteration{ 0 };
				// Remove entry from Hierarchy Tree
				for (auto& entry : m_SceneHierarchyTree.GetTreeEntries())
				{
					if ((uint32_t)entry.Handle == (uint32_t)entityToDelete)
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
					SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)entry.ProvidedData.get();
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
				SceneEditorTreeEntryData& entryData = *(SceneEditorTreeEntryData*)entry.ProvidedData.get();
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
				s_EditorApp->m_SceneEditorPanel->SetSelectedEntity({});
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
			m_SceneHierarchyTree.SelectedEntry = {};
		}
		if (entity)
		{
			EditorUI::TreePath path;
			for (auto& entry : m_SceneHierarchyTree.GetTreeEntries())
			{
				if ((uint32_t)entry.Handle == (uint32_t)entity)
				{
					path = m_SceneHierarchyTree.GetPathFromEntryReference(&entry);
				}
			}

			if (path)
			{
				m_SceneHierarchyTree.SelectedEntry = path;
			}
			else
			{
				KG_WARN("Failed to locate entity inside tree");
			}
		}


		RefreshTransformComponent();
		s_EditorApp->m_ShowProperties = true;
		EditorUI::EditorUIService::BringWindowToFront(s_EditorApp->m_PropertiesPanel->m_PanelName);
		s_EditorApp->m_PropertiesPanel->m_ActiveParent = m_PanelName;
		
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
			m_TransformEditTranslation.CurrentVec3 = transformComp.Translation;
			m_TransformEditRotation.CurrentVec3 = transformComp.Rotation;
			m_TransformEditScale.CurrentVec3 = transformComp.Scale;
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
		if (m_TagHeader.Expanded)
		{
			m_TagEdit.CurrentOption = component.Tag;
			EditorUI::EditorUIService::EditText(m_TagEdit);
			m_TagGroupEdit.CurrentOption = component.Group;
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
		if (m_TransformHeader.Expanded)
		{
			m_TransformEditTranslation.CurrentVec3 = component.Translation;
			EditorUI::EditorUIService::EditVec3(m_TransformEditTranslation);
			m_TransformEditScale.CurrentVec3 = component.Scale;
			EditorUI::EditorUIService::EditVec3(m_TransformEditScale);
			m_TransformEditRotation.CurrentVec3 = component.Rotation;
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
		if (m_Rigidbody2DHeader.Expanded)
		{
			m_Rigidbody2DType.SelectedOption = component.Type == ECS::Rigidbody2DComponent::BodyType::Static ?
				0 : 1;
			EditorUI::EditorUIService::RadioSelector(m_Rigidbody2DType);
			m_RigidBody2DFixedRotation.CurrentBoolean = component.FixedRotation;
			EditorUI::EditorUIService::Checkbox(m_RigidBody2DFixedRotation);

			// Display collision script functions
			Ref<Scripting::Script> collisionStartScript = Assets::AssetService::GetScript(component.OnCollisionStartScriptHandle);
			m_SelectRigidBody2DCollisionStartScript.CurrentOption = component.OnCollisionStartScriptHandle == Assets::EmptyHandle ?
				EditorUI::OptionEntry("None", Assets::EmptyHandle) :
				EditorUI::OptionEntry(Utility::ScriptToString(collisionStartScript), component.OnCollisionStartScriptHandle);
			EditorUI::EditorUIService::SelectOption(m_SelectRigidBody2DCollisionStartScript);

			Ref<Scripting::Script> collisionEndScript = Assets::AssetService::GetScript(component.OnCollisionEndScriptHandle);
			m_SelectRigidBody2DCollisionEndScript.CurrentOption = component.OnCollisionEndScriptHandle == Assets::EmptyHandle ?
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
		if (m_BoxCollider2DHeader.Expanded)
		{
			m_BoxColliderOffset.CurrentVec2 = component.Offset;
			EditorUI::EditorUIService::EditVec2(m_BoxColliderOffset);
			m_BoxColliderSize.CurrentVec2 = component.Size;
			EditorUI::EditorUIService::EditVec2(m_BoxColliderSize);
			m_BoxColliderDensity.CurrentFloat = component.Density;
			EditorUI::EditorUIService::EditFloat(m_BoxColliderDensity);
			m_BoxColliderFriction.CurrentFloat = component.Friction;
			EditorUI::EditorUIService::EditFloat(m_BoxColliderFriction);
			m_BoxColliderRestitution.CurrentFloat = component.Restitution;
			EditorUI::EditorUIService::EditFloat(m_BoxColliderRestitution);
			m_BoxColliderRestitutionThreshold.CurrentFloat = component.RestitutionThreshold;
			EditorUI::EditorUIService::EditFloat(m_BoxColliderRestitutionThreshold);
			m_BoxColliderIsSensor.CurrentBoolean = component.IsSensor;
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
		if (m_CircleCollider2DHeader.Expanded)
		{
			m_CircleColliderOffset.CurrentVec2 = component.Offset;
			EditorUI::EditorUIService::EditVec2(m_CircleColliderOffset);
			m_CircleColliderRadius.CurrentFloat = component.Radius;
			EditorUI::EditorUIService::EditFloat(m_CircleColliderRadius);
			m_CircleColliderDensity.CurrentFloat = component.Density;
			EditorUI::EditorUIService::EditFloat(m_CircleColliderDensity);
			m_CircleColliderFriction.CurrentFloat = component.Friction;
			EditorUI::EditorUIService::EditFloat(m_CircleColliderFriction);
			m_CircleColliderRestitution.CurrentFloat = component.Restitution;
			EditorUI::EditorUIService::EditFloat(m_CircleColliderRestitution);
			m_CircleColliderRestitutionThreshold.CurrentFloat = component.RestitutionThreshold;
			EditorUI::EditorUIService::EditFloat(m_CircleColliderRestitutionThreshold);
			m_CircleColliderIsSensor.CurrentBoolean = component.IsSensor;
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
		if (m_CameraHeader.Expanded)
		{
			m_CameraPrimary.CurrentBoolean = component.Primary;
			EditorUI::EditorUIService::Checkbox(m_CameraPrimary);
			m_CameraProjection.SelectedOption = component.Camera.GetProjectionType() ==
				Scenes::SceneCamera::ProjectionType::Perspective ? 0 : 1;
			EditorUI::EditorUIService::RadioSelector(m_CameraProjection);

			if (component.Camera.GetProjectionType() == Scenes::SceneCamera::ProjectionType::Perspective)
			{
				m_CameraPerspectiveFOV.CurrentFloat = component.Camera.GetPerspectiveVerticalFOV();
				EditorUI::EditorUIService::EditFloat(m_CameraPerspectiveFOV);
				m_CameraPerspectiveNearPlane.CurrentFloat = component.Camera.GetPerspectiveNearClip();
				EditorUI::EditorUIService::EditFloat(m_CameraPerspectiveNearPlane);
				m_CameraPerspectiveFarPlane.CurrentFloat = component.Camera.GetPerspectiveFarClip();
				EditorUI::EditorUIService::EditFloat(m_CameraPerspectiveFarPlane);
			}
			else
			{
				m_CameraOrthographicSize.CurrentFloat = component.Camera.GetOrthographicSize();
				EditorUI::EditorUIService::EditFloat(m_CameraOrthographicSize);
				m_CameraOrthographicNearPlane.CurrentFloat = component.Camera.GetOrthographicNearClip();
				EditorUI::EditorUIService::EditFloat(m_CameraOrthographicNearPlane);
				m_CameraOrthographicFarPlane.CurrentFloat = component.Camera.GetOrthographicFarClip();
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
		if (m_OnUpdateHeader.Expanded)
		{
			Ref<Scripting::Script> script = Assets::AssetService::GetScript(component.OnUpdateScriptHandle);
			m_SelectOnUpdateScript.CurrentOption = component.OnUpdateScriptHandle == Assets::EmptyHandle ? 
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
		if (m_AIStateHeader.Expanded)
		{
			// Select global state
			bool optionValid = component.GlobalStateHandle != Assets::EmptyHandle;
			if (optionValid)
			{
				Assets::Asset& globalAsset = Assets::AssetService::GetAIStateRegistry().at(component.GlobalStateHandle);
				m_SelectGlobalState.CurrentOption = { globalAsset.Data.FileLocation.string(), component.GlobalStateHandle };
			}
			else
			{
				m_SelectGlobalState.CurrentOption = { "None", Assets::EmptyHandle };
			}
			EditorUI::EditorUIService::SelectOption(m_SelectGlobalState);

			// Select current state
			optionValid = component.CurrentStateHandle != Assets::EmptyHandle;
			if (optionValid)
			{
				Assets::Asset& currentAsset = Assets::AssetService::GetAIStateRegistry().at(component.CurrentStateHandle);
				m_SelectCurrentState.CurrentOption = { currentAsset.Data.FileLocation.string(), component.CurrentStateHandle };
			}
			else
			{
				m_SelectCurrentState.CurrentOption = { "None", Assets::EmptyHandle };
			}
			EditorUI::EditorUIService::SelectOption(m_SelectCurrentState);

			// Select previous state
			optionValid = component.PreviousStateHandle != Assets::EmptyHandle;
			if (optionValid)
			{
				Assets::Asset& previousAsset = Assets::AssetService::GetAIStateRegistry().at(component.PreviousStateHandle);
				m_SelectPreviousState.CurrentOption = { previousAsset.Data.FileLocation.string(), component.PreviousStateHandle };
			}
			else
			{
				m_SelectPreviousState.CurrentOption = { "None", Assets::EmptyHandle };
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
		if (m_OnCreateHeader.Expanded)
		{
			Ref<Scripting::Script> script = Assets::AssetService::GetScript(component.OnCreateScriptHandle);
			m_SelectOnCreateScript.CurrentOption = component.OnCreateScriptHandle == Assets::EmptyHandle ?
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
		if (m_ShapeHeader.Expanded)
		{
			m_ShapeSelect.CurrentOption = { Utility::ShapeTypeToString(component.CurrentShape), Assets::EmptyHandle };
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
			ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.ProvidedData.get();
			Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
			ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

			// Get field data pointer
			uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

			// Set the data
			spec.CurrentBoolean = *(bool*)fieldDataRef;
			
			// Display Checkbox
			EditorUI::EditorUIService::Checkbox(spec);
		}
		void operator()(EditorUI::EditTextSpec& spec)
		{
			// Get component data pointer
			ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.ProvidedData.get();
			Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
			ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

			// Get field data pointer
			uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

			// Set the data
			spec.CurrentOption = *(std::string*)fieldDataRef;

			// Display Text
			EditorUI::EditorUIService::EditText(spec);
		}
		void operator()(EditorUI::EditIntegerSpec& spec)
		{
			// Get component data pointer
			ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.ProvidedData.get();
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
				spec.CurrentInteger = *(int32_t*)fieldDataRef;
				break;
			case WrappedVarType::UInteger16:
				spec.CurrentInteger = (int32_t)*(uint16_t*)fieldDataRef;
				break;
			case WrappedVarType::UInteger32:
				spec.CurrentInteger = (int32_t)*(uint32_t*)fieldDataRef;
				break;
			case WrappedVarType::UInteger64:
				spec.CurrentInteger = (int32_t)*(uint64_t*)fieldDataRef;
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
			ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.ProvidedData.get();
			Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
			ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

			// Get field data pointer
			uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

			// Set the data
			spec.CurrentFloat = *(float*)fieldDataRef;

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
			ProjectComponentFieldInfo& projectCompFieldInfo = *(ProjectComponentFieldInfo*)spec.ProvidedData.get();
			Ref<ECS::ProjectComponent> projectComponentRef = Assets::AssetService::GetProjectComponent(projectCompFieldInfo.m_ProjectComponentHandle);
			ECS::Entity selectedEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			uint8_t* componentDataRef = (uint8_t*)selectedEntity.GetProjectComponentData(projectCompFieldInfo.m_ProjectComponentHandle);

			// Get field data pointer
			uint8_t* fieldDataRef = componentDataRef + projectComponentRef->m_DataLocations.at(projectCompFieldInfo.m_FieldSlot);

			// Set the data
			spec.CurrentVec3 = *(Math::vec3*)fieldDataRef;

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
		if (activeWidgetData.m_Header.Expanded)
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
				memcpy_s(newLocationPointer, size, oldLocationPointer, size);
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
		m_ShapeColorType.CurrentOption = { Utility::ColorInputTypeToString(component.ShaderSpecification.ColorInput), Assets::EmptyHandle };
		EditorUI::EditorUIService::SelectOption(m_ShapeColorType);

		if (component.ShaderSpecification.ColorInput == Rendering::ColorInputType::None) { return; }

		if (component.ShaderSpecification.ColorInput == Rendering::ColorInputType::FlatColor)
		{
			m_ShapeColor.CurrentVec4 = *(Rendering::Shader::GetInputLocation<Math::vec4>("a_Color", component.ShaderData, component.Shader));
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
		m_ShapeAddTexture.CurrentBoolean = component.ShaderSpecification.TextureInput == Rendering::TextureInputType::ColorTexture ? true : false;
		EditorUI::EditorUIService::Checkbox(m_ShapeAddTexture);
		if (m_ShapeAddTexture.CurrentBoolean)
		{
			if (component.TextureHandle == Assets::EmptyHandle)
			{
				m_ShapeSetTexture.CurrentOption = { "None", Assets::EmptyHandle };
			}
			else
			{
				m_ShapeSetTexture.CurrentOption =
				{
					Assets::AssetService::GetTexture2DRegistry().at(component.TextureHandle).Data.FileLocation.string(),
					component.TextureHandle
				};
			}
			EditorUI::EditorUIService::SelectOption(m_ShapeSetTexture);

			m_ShapeTilingFactor.CurrentFloat = *Rendering::Shader::GetInputLocation<float>("a_TilingFactor", component.ShaderData, component.Shader);
			EditorUI::EditorUIService::EditFloat(m_ShapeTilingFactor);
		}
	}
	void SceneEditorPanel::DrawShapeComponentCircle()
	{
		ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
		m_ShapeAddCircle.CurrentBoolean = component.ShaderSpecification.AddCircleShape;
		EditorUI::EditorUIService::Checkbox(m_ShapeAddCircle);
		if (component.ShaderSpecification.AddCircleShape)
		{
			m_ShapeCircleThickness.CurrentFloat = *Rendering::Shader::GetInputLocation<float>("a_Thickness", component.ShaderData, component.Shader);
			EditorUI::EditorUIService::EditFloat(m_ShapeCircleThickness);

			m_ShapeCircleFade.CurrentFloat = *Rendering::Shader::GetInputLocation<float>("a_Fade", component.ShaderData, component.Shader);
			EditorUI::EditorUIService::EditFloat(m_ShapeCircleFade);
		}
	}
	void SceneEditorPanel::DrawShapeComponentProjection()
	{
		ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
		m_ShapeAddProjection.CurrentBoolean = component.ShaderSpecification.AddProjectionMatrix;
		EditorUI::EditorUIService::Checkbox(m_ShapeAddProjection);
	}
	void SceneEditorPanel::DrawShapeComponentEntityID()
	{
		ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
		m_ShapeAddEntityID.CurrentBoolean = component.ShaderSpecification.AddEntityID;
		EditorUI::EditorUIService::Checkbox(m_ShapeAddEntityID);
	}
}
