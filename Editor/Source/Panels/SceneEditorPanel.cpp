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
			componentEntry.Label = "Tag Component";
			componentEntry.ProvidedData = CreateRef<uint16_t>((uint16_t)ECS::ComponentType::Tag);
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
			componentEntry.Label = "Transform Component";
			componentEntry.ProvidedData = CreateRef<uint16_t>((uint16_t)ECS::ComponentType::Transform);
			componentEntry.IconHandle = EditorUI::EditorUIService::s_IconTransform;
			componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
			{
				ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
				s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
				s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::Transform);
			};
			newEntry.SubEntries.push_back(componentEntry);
		}
		if (entity.HasComponent<ECS::ClassInstanceComponent>())
		{
			componentEntry.Label = "Class Instance Component";
			componentEntry.ProvidedData = CreateRef<uint16_t>((uint16_t)ECS::ComponentType::ClassInstance);
			componentEntry.IconHandle = EditorUI::EditorUIService::s_IconClassInstance;
			componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
			{
				ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
				s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
				s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::ClassInstance);
			};
			newEntry.SubEntries.push_back(componentEntry);
		}

		if (entity.HasComponent<ECS::Rigidbody2DComponent>())
		{
			componentEntry.Label = "Rigid Body 2D Component";
			componentEntry.ProvidedData = CreateRef<uint16_t>((uint16_t)ECS::ComponentType::Rigidbody2D);
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
			componentEntry.Label = "Box Collider 2D Component";
			componentEntry.ProvidedData = CreateRef<uint16_t>((uint16_t)ECS::ComponentType::BoxCollider2D);
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
			componentEntry.Label = "Circle Collider 2D Component";
			componentEntry.ProvidedData = CreateRef<uint16_t>((uint16_t)ECS::ComponentType::CircleCollider2D);
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
			componentEntry.Label = "Camera Component";
			componentEntry.ProvidedData = CreateRef<uint16_t>((uint16_t)ECS::ComponentType::Camera);
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
			componentEntry.Label = "Shape Component";
			componentEntry.ProvidedData = CreateRef<uint16_t>((uint16_t)ECS::ComponentType::Shape);
			componentEntry.IconHandle = EditorUI::EditorUIService::s_IconEntity;
			componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
			{
				ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
				s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
				s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::Shape);
			};
			newEntry.SubEntries.push_back(componentEntry);
		}

		m_SceneHierarchyTree.InsertEntry(newEntry);
	}

	void SceneEditorPanel::InitializeSceneHierarchy()
	{
		m_MainSceneHeader.Label = "No Scene Name";
		m_MainSceneHeader.EditColorActive = false;
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
			if (!entity.HasComponent<ECS::ClassInstanceComponent>())
			{
				m_AddComponent.AddToOptions("New Component", "Class Instance", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				m_AddComponent.AddToOptions("New Component", "Camera", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<ECS::ShapeComponent>())
			{
				m_AddComponent.AddToOptions("New Component", "Shape", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<ECS::Rigidbody2DComponent>())
			{
				m_AddComponent.AddToOptions("New Component", "Rigidbody 2D", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<ECS::BoxCollider2DComponent>())
			{
				m_AddComponent.AddToOptions("New Component", "Box Collider 2D", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<ECS::CircleCollider2DComponent>())
			{
				m_AddComponent.AddToOptions("New Component", "Circle Collider 2D", Assets::EmptyHandle);
			}

			m_AddComponent.CurrentOption = { "None", Assets::EmptyHandle };
		};

		m_AddComponent.ConfirmAction = [&](const EditorUI::OptionEntry& option)
		{
			ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity(m_AddComponentEntity));

		 	EditorUI::TreeEntry* currentEntry = m_SceneHierarchyTree.SearchFirstLayer((uint64_t)entity);

			if (!currentEntry)
			{
				KG_WARN("Could not locate entity inside m_SceneHierarchyTree");
				return;
			}

			EditorUI::TreeEntry componentEntry {};
			componentEntry.Handle = (uint64_t)entity;
			
			if (!entity)
			{
				KG_WARN("Attempt to add component to empty entity");
				return;
			}

			if (option.Label == "None")
			{
				return;
			}
			if (option.Label == "Class Instance")
			{
				entity.AddComponent<ECS::ClassInstanceComponent>();
				componentEntry.Label = "Class Instance Component";
				componentEntry.ProvidedData = CreateRef<uint16_t>((uint16_t)ECS::ComponentType::ClassInstance);
				componentEntry.IconHandle = EditorUI::EditorUIService::s_IconClassInstance;
				componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
				{
					ECS::Entity entity = Scenes::SceneService::GetActiveScene()->GetEntityByEnttID(entt::entity((int)entry.Handle));
					s_EditorApp->m_SceneEditorPanel->SetSelectedEntity(entity);
					s_EditorApp->m_SceneEditorPanel->SetDisplayedComponent(ECS::ComponentType::ClassInstance);
				};
				currentEntry->SubEntries.push_back(componentEntry);
				return;
			}
			if (option.Label == "Camera")
			{
				entity.AddComponent<ECS::CameraComponent>();
				componentEntry.Label = "Camera Component";
				componentEntry.ProvidedData = CreateRef<uint16_t>((uint16_t)ECS::ComponentType::Camera);
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
				componentEntry.Label = "Shape Component";
				componentEntry.ProvidedData = CreateRef<uint16_t>((uint16_t)ECS::ComponentType::Shape);
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
				componentEntry.Label = "Rigidbody 2D Component";
				componentEntry.ProvidedData = CreateRef<uint16_t>((uint16_t)ECS::ComponentType::Rigidbody2D);
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
				componentEntry.Label = "Box Collider 2D Component";
				componentEntry.ProvidedData = CreateRef<uint16_t>((uint16_t)ECS::ComponentType::BoxCollider2D);
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
				componentEntry.Label = "Circle Collider 2D Component";
				componentEntry.ProvidedData = CreateRef<uint16_t>((uint16_t)ECS::ComponentType::CircleCollider2D);
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
		m_TagEdit.ConfirmAction = [&]()
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (entity && entity.HasComponent<ECS::TagComponent>())
			{
				auto& component = entity.GetComponent<ECS::TagComponent>();
				component.Tag = m_TagEdit.CurrentOption;
			}
		};
	}

	void SceneEditorPanel::InitializeClassInstanceComponent()
	{
		m_ClassInstanceHeader.Label = "Class Instance";
		m_ClassInstanceHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_ClassInstanceHeader.Expanded = true;
		m_ClassInstanceHeader.AddToSelectionList("Remove Component", [&]()
		{
			EngineService::SubmitToMainThread([&]()
			{
				ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<ECS::ClassInstanceComponent>())
				{
					for (auto& entry : m_SceneHierarchyTree.GetTreeEntries())
					{
						if ((uint32_t)entry.Handle == (uint32_t)entity)
						{
							EditorUI::TreePath newPath {};
							for (auto& subEntry : entry.SubEntries)
							{
								if (*(uint16_t*)subEntry.ProvidedData.get() == (uint16_t)ECS::ComponentType::ClassInstance)
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

					entity.RemoveComponent<ECS::ClassInstanceComponent>();
				}
			});
		});

		m_SelectClassOption.Label = "Class";
		m_SelectClassOption.Flags |= EditorUI::SelectOption_Indented;
		m_SelectClassOption.CurrentOption = { "None", Assets::EmptyHandle };
		m_SelectClassOption.PopupAction = [&]()
		{
			m_SelectClassOption.ClearOptions();
			m_SelectClassOption.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetService::GetEntityClassRegistry())
			{
				m_SelectClassOption.AddToOptions("All Options",
					asset.Data.GetSpecificMetaData<Assets::EntityClassMetaData>()->Name, handle);
			}
		};
		m_SelectClassOption.ConfirmAction = [&](const EditorUI::OptionEntry& entry)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			auto& component = entity.GetComponent<ECS::ClassInstanceComponent>();

			if (entry.Handle == Assets::EmptyHandle)
			{
				component.ClassHandle = Assets::EmptyHandle;
				component.Fields.clear();
				component.ClassReference = nullptr;
				return;
			}

			if (component.ClassHandle == entry.Handle)
			{
				return;
			}
			bool success = component.ChangeClass(entry.Handle);
			if (success)
			{
				m_InstanceFieldsTable.OnRefresh();
			}
		};

		m_InstanceFieldsTable.Label = "Instance Fields";
		m_InstanceFieldsTable.Flags |= EditorUI::Table_Indented;
		m_InstanceFieldsTable.Column1Title = "Field Name";
		m_InstanceFieldsTable.Column2Title = "Field Value";
		m_InstanceFieldsTable.Expanded = true;
		m_InstanceFieldsTable.OnRefresh = [&]()
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();

			if (!entity || !entity.HasComponent<ECS::ClassInstanceComponent>())
			{
				return;
			}

			auto& component = entity.GetComponent<ECS::ClassInstanceComponent>();
			uint32_t iteration{ 0 };
			m_InstanceFieldsTable.ClearTable();
			for(auto& wrappedVar : component.Fields)
			{
				m_InstanceFieldsTable.InsertTableEntry(
				{
					component.ClassReference->GetFields().at(iteration).Name,
					wrappedVar->GetValueAsString(),
					Assets::EmptyHandle,
					[&](EditorUI::TableEntry& optionEntry)
					{
						m_CurrentClassField = optionEntry.Label;
						ECS::Entity currentEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
						auto& comp = currentEntity.GetComponent<ECS::ClassInstanceComponent>();
						m_CurrentClassFieldLocation = comp.ClassReference->GetFieldLocation(m_CurrentClassField);
						if (m_CurrentClassFieldLocation == -1)
						{
							KG_WARN("Could not locate field in class definition");
							return;
						}
						m_EditClassFieldPopup.PopupActive = true;
					},
					nullptr
				});
				iteration++;
			}
		};

		m_EditClassFieldPopup.Label = "Edit Field";
		m_EditClassFieldPopup.PopupWidth = 420.0f;
		m_EditClassFieldPopup.PopupAction = [&]()
		{
			ECS::Entity currentEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			auto& comp = currentEntity.GetComponent<ECS::ClassInstanceComponent>();
			const Ref<WrappedVariable> field = comp.Fields.at(m_CurrentClassFieldLocation);

			if (!field)
			{
				KG_ERROR("Unable to retreive field from current game state object");
				return;
			}

			bool success = Utility::FillBufferWithWrappedVarString(field, m_EditFieldValue.FieldBuffer);
			m_EditFieldValue.VariableType = field->Type();
			if (!success)
			{
				KG_WARN("Unable to complete Popup Action");
				return;
			}
		};
		m_EditClassFieldPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditVariable(m_EditFieldValue);
		};
		m_EditClassFieldPopup.ConfirmAction = [&]()
		{
			ECS::Entity currentEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			auto& comp = currentEntity.GetComponent<ECS::ClassInstanceComponent>();
			const Ref<WrappedVariable> field = comp.Fields.at(m_CurrentClassFieldLocation);
			bool success = Utility::FillWrappedVarWithStringBuffer(field, m_EditFieldValue.FieldBuffer);
			if (!success)
			{
				KG_WARN("Issue completing m_EditClassFieldPopup Confirm Action");
				return;
			}
			m_InstanceFieldsTable.OnRefresh();
		};
		m_EditFieldValue.Label = "Edit Value";
		m_EditFieldValue.AllocateBuffer();

	}

	void SceneEditorPanel::InitializeTransformComponent()
	{
		m_TransformHeader.Label = "Transform";
		m_TransformHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_TransformHeader.Expanded = true;

		m_TransformEditTranslation.Label = "Translation";
		m_TransformEditTranslation.Flags = EditorUI::EditVec3_Indented;
		m_TransformEditTranslation.ConfirmAction = [&]() 
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
		m_TransformEditScale.ConfirmAction = [&]()
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
		m_TransformEditRotation.ConfirmAction = [&]()
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
		m_Rigidbody2DHeader.AddToSelectionList("Remove Component", [&]()
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
								if (*(uint16_t*)subEntry.ProvidedData.get() == (uint16_t)ECS::ComponentType::Rigidbody2D)
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
		m_RigidBody2DFixedRotation.ConfirmAction = [&](bool value)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::Rigidbody2DComponent>())
			{
				KG_ERROR("Attempt to edit entity rigid body 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::Rigidbody2DComponent>();
			component.FixedRotation = value;
		};
	}

	void SceneEditorPanel::InitializeBoxCollider2DComponent()
	{
		m_BoxCollider2DHeader.Label = "Box Collider 2D";
		m_BoxCollider2DHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_BoxCollider2DHeader.Expanded = true;
		m_BoxCollider2DHeader.AddToSelectionList("Remove Component", [&]()
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
								if (*(uint16_t*)subEntry.ProvidedData.get() == (uint16_t)ECS::ComponentType::BoxCollider2D)
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
		m_BoxColliderOffset.ConfirmAction = [&]()
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
		m_BoxColliderSize.ConfirmAction = [&]()
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
		m_BoxColliderDensity.ConfirmAction = [&]()
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
		m_BoxColliderFriction.ConfirmAction = [&]()
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
		m_BoxColliderRestitution.ConfirmAction = [&]()
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
		m_BoxColliderRestitutionThreshold.ConfirmAction = [&]()
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
	}
	void SceneEditorPanel::InitializeCircleCollider2DComponent()
	{
		m_CircleCollider2DHeader.Label = "Circle Collider 2D";
		m_CircleCollider2DHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_CircleCollider2DHeader.Expanded = true;
		m_CircleCollider2DHeader.AddToSelectionList("Remove Component", [&]()
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
								if (*(uint16_t*)subEntry.ProvidedData.get() == (uint16_t)ECS::ComponentType::CircleCollider2D)
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
		m_CircleColliderOffset.ConfirmAction = [&]()
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
		m_CircleColliderRadius.ConfirmAction = [&]()
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
		m_CircleColliderDensity.ConfirmAction = [&]()
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
		m_CircleColliderFriction.ConfirmAction = [&]()
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
		m_CircleColliderRestitution.ConfirmAction = [&]()
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
		m_CircleColliderRestitutionThreshold.ConfirmAction = [&]()
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
	}

	void SceneEditorPanel::InitializeCameraComponent()
	{
		m_CameraHeader.Label = "Camera";
		m_CameraHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_CameraHeader.Expanded = true;
		m_CameraHeader.AddToSelectionList("Remove Component", [&]()
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
								if (*(uint16_t*)subEntry.ProvidedData.get() == (uint16_t)ECS::ComponentType::Camera)
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
		m_CameraPrimary.ConfirmAction = [&](bool value)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<ECS::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<ECS::CameraComponent>();
			component.Primary = value;
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
		m_CameraOrthographicSize.ConfirmAction = [&]()
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
		m_CameraOrthographicNearPlane.ConfirmAction = [&]()
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
		m_CameraOrthographicFarPlane.ConfirmAction = [&]()
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
		m_CameraPerspectiveFOV.ConfirmAction = [&]()
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
		m_CameraPerspectiveNearPlane.ConfirmAction = [&]()
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
		m_CameraPerspectiveFarPlane.ConfirmAction = [&]()
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

	void SceneEditorPanel::InitializeShapeComponent()
	{
		m_ShapeHeader.Label = "Shape";
		m_ShapeHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		m_ShapeHeader.Expanded = true;
		m_ShapeHeader.AddToSelectionList("Remove Component", [&]()
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
								if (*(uint16_t*)subEntry.ProvidedData.get() == (uint16_t)ECS::ComponentType::Shape)
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
			UpdateComponent();
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
				UpdateComponent();
			}
			if (entry.Label == "FlatColor")
			{
				component.ShaderSpecification.ColorInput = Rendering::ColorInputType::FlatColor;
				UpdateComponent();
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
				UpdateComponent();
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
		m_ShapeColor.ConfirmAction = [&]()
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
		m_ShapeAddTexture.ConfirmAction = [&](bool value)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			value ? component.ShaderSpecification.TextureInput = Rendering::TextureInputType::ColorTexture :
				component.ShaderSpecification.TextureInput = Rendering::TextureInputType::None;
			UpdateComponent();
			// Checkbox is switched on
			if (value)
			{
				if (component.CurrentShape == Rendering::ShapeTypes::Cube || component.CurrentShape == Rendering::ShapeTypes::Pyramid)
				{
					component.ShaderSpecification.RenderType = Rendering::RenderingType::DrawTriangle;
					UpdateComponent();
					component.Vertices = CreateRef<std::vector<Math::vec3>>(Utility::ShapeTypeToShape(component.CurrentShape).GetTriangleVertices());
					component.TextureCoordinates = CreateRef<std::vector<Math::vec2>>(Utility::ShapeTypeToShape(component.CurrentShape).GetTriangleTextureCoordinates());
					if (component.VertexColors) { component.VertexColors->resize(component.Vertices->size(), { 1.0f, 1.0f, 1.0f, 1.0f }); }
				}
				Rendering::Shader::SetDataAtInputLocation<float>(1.0f, "a_TilingFactor", component.ShaderData, component.Shader);
			}
			// Checkbox is switched off
			if (!value)
			{
				if (component.CurrentShape == Rendering::ShapeTypes::Cube || component.CurrentShape == Rendering::ShapeTypes::Pyramid)
				{
					component.ShaderSpecification.RenderType = Rendering::RenderingType::DrawIndex;
					UpdateComponent();
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
		m_ShapeTilingFactor.ConfirmAction = [&]()
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
		m_ShapeAddCircle.ConfirmAction = [&](bool value)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			component.ShaderSpecification.AddCircleShape = value;
			UpdateComponent();
			if (value)
			{
				Rendering::Shader::SetDataAtInputLocation<float>(1.0f, "a_Thickness", component.ShaderData, component.Shader);
				Rendering::Shader::SetDataAtInputLocation<float>(0.005f, "a_Fade", component.ShaderData, component.Shader);
			}
		};

		m_ShapeCircleThickness.Label = "Circle Thickness";
		m_ShapeCircleThickness.Flags |= EditorUI::EditFloat_Indented;
		m_ShapeCircleThickness.ConfirmAction = [&]()
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
		m_ShapeCircleFade.ConfirmAction = [&]()
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
		m_ShapeAddProjection.ConfirmAction = [&](bool value)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			component.ShaderSpecification.AddProjectionMatrix = value;
			UpdateComponent();
		};

		// Set Shape Add Entity ID Option
		m_ShapeAddEntityID.Label = "Use Entity ID";
		m_ShapeAddEntityID.Flags |= EditorUI::Checkbox_Indented;
		m_ShapeAddEntityID.ConfirmAction = [&](bool value)
		{
			ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
			component.ShaderSpecification.AddEntityID = value;
			UpdateComponent();
		};
	}

	SceneEditorPanel::SceneEditorPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName, 
			KG_BIND_CLASS_FN(SceneEditorPanel::OnKeyPressedEditor));
		InitializeSceneHierarchy();
		InitializeTagComponent();
		InitializeClassInstanceComponent();
		InitializeTransformComponent();
		InitializeRigidbody2DComponent();
		InitializeBoxCollider2DComponent();
		InitializeCircleCollider2DComponent();
		InitializeCameraComponent();
		InitializeShapeComponent();

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
			if (entity.HasComponent<ECS::ClassInstanceComponent>())
			{
				ECS::ClassInstanceComponent& instanceComp = entity.GetComponent<ECS::ClassInstanceComponent>();
				if (instanceComp.ClassHandle == Assets::EmptyHandle)
				{
					m_SelectClassOption.CurrentOption = { "None", Assets::EmptyHandle };
				}
				else
				{
					m_SelectClassOption.CurrentOption = { instanceComp.ClassReference->GetName(),instanceComp.ClassHandle };
				}
				m_InstanceFieldsTable.OnRefresh();
			}

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
	void SceneEditorPanel::RefreshClassInstanceComponent()
	{
		ECS::Entity currentEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		if (!currentEntity)
		{
			return;
		}
		if (currentEntity.HasComponent<ECS::ClassInstanceComponent>())
		{
			auto& comp = currentEntity.GetComponent<ECS::ClassInstanceComponent>();
			if (!Assets::AssetService::GetEntityClass(comp.ClassHandle))
			{
				comp.ClassHandle = Assets::EmptyHandle;
				comp.Fields.clear();
				comp.ClassReference = nullptr;
				m_SelectClassOption.CurrentOption = {"None", Assets::EmptyHandle};
			}
			m_InstanceFieldsTable.OnRefresh();
		}
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
		DrawClassInstanceComponent(entity);
		DrawRigidbody2DComponent(entity);
		DrawBoxCollider2DComponent(entity);
		DrawCircleCollider2DComponent(entity);
		DrawCameraComponent(entity);
		DrawShapeComponent(entity);
	}
	void SceneEditorPanel::DrawSingleComponent(ECS::Entity entity)
	{
		switch (m_DisplayedComponent)
		{
		case ECS::ComponentType::Tag:
		{
			DrawTagComponent(entity);
			return;
		}
		case ECS::ComponentType::Transform:
		{
			DrawTransformComponent(entity);
			return;
		}
		case ECS::ComponentType::Rigidbody2D:
		{
			DrawRigidbody2DComponent(entity);
			return;
		}
		case ECS::ComponentType::BoxCollider2D:
		{
			DrawBoxCollider2DComponent(entity);
			return;
		}
		case ECS::ComponentType::CircleCollider2D:
		{
			DrawCircleCollider2DComponent(entity);
			return;
		}
		case ECS::ComponentType::Camera:
		{
			DrawCameraComponent(entity);
			return;
		}
		case ECS::ComponentType::Shape:
		{
			DrawShapeComponent(entity);
			return;
		}
		case ECS::ComponentType::ClassInstance:
		{
			DrawClassInstanceComponent(entity);
			return;
		}

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
	void SceneEditorPanel::DrawClassInstanceComponent(ECS::Entity entity)
	{
		if (!entity.HasComponent<ECS::ClassInstanceComponent>())
		{
			return;
		}
		ECS::ClassInstanceComponent& component = entity.GetComponent<ECS::ClassInstanceComponent>();
		EditorUI::EditorUIService::CollapsingHeader(m_ClassInstanceHeader);
		if (m_ClassInstanceHeader.Expanded)
		{
			EditorUI::EditorUIService::SelectOption(m_SelectClassOption);
			if (component.ClassHandle != Assets::EmptyHandle)
			{
				EditorUI::EditorUIService::Table(m_InstanceFieldsTable);
				EditorUI::EditorUIService::GenericPopup(m_EditClassFieldPopup);
			}
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
			m_RigidBody2DFixedRotation.ToggleBoolean = component.FixedRotation;
			EditorUI::EditorUIService::Checkbox(m_RigidBody2DFixedRotation);
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
			m_CameraPrimary.ToggleBoolean = component.Primary;
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
				AddColorSection();
				AddTextureSection();
				AddCircleShapeSection();
				AddProjectionMatrixSection();
				AddEntityIDSection();

			}
			if (component.CurrentShape == Rendering::ShapeTypes::Cube || component.CurrentShape == Rendering::ShapeTypes::Pyramid)
			{
				AddColorSection();
				AddTextureSection();
				AddProjectionMatrixSection();
				AddEntityIDSection();
			}
		}
	}
	void SceneEditorPanel::UpdateComponent()
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
	void SceneEditorPanel::AddColorSection()
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
	void SceneEditorPanel::AddTextureSection()
	{
		ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
		m_ShapeAddTexture.ToggleBoolean = component.ShaderSpecification.TextureInput == Rendering::TextureInputType::ColorTexture ? true : false;
		EditorUI::EditorUIService::Checkbox(m_ShapeAddTexture);
		if (m_ShapeAddTexture.ToggleBoolean)
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
	void SceneEditorPanel::AddCircleShapeSection()
	{
		ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
		m_ShapeAddCircle.ToggleBoolean = component.ShaderSpecification.AddCircleShape;
		EditorUI::EditorUIService::Checkbox(m_ShapeAddCircle);
		if (component.ShaderSpecification.AddCircleShape)
		{
			m_ShapeCircleThickness.CurrentFloat = *Rendering::Shader::GetInputLocation<float>("a_Thickness", component.ShaderData, component.Shader);
			EditorUI::EditorUIService::EditFloat(m_ShapeCircleThickness);

			m_ShapeCircleFade.CurrentFloat = *Rendering::Shader::GetInputLocation<float>("a_Fade", component.ShaderData, component.Shader);
			EditorUI::EditorUIService::EditFloat(m_ShapeCircleFade);
		}
	}
	void SceneEditorPanel::AddProjectionMatrixSection()
	{
		ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
		m_ShapeAddProjection.ToggleBoolean = component.ShaderSpecification.AddProjectionMatrix;
		EditorUI::EditorUIService::Checkbox(m_ShapeAddProjection);
	}
	void SceneEditorPanel::AddEntityIDSection()
	{
		ECS::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		ECS::ShapeComponent& component = entity.GetComponent<ECS::ShapeComponent>();
		m_ShapeAddEntityID.ToggleBoolean = component.ShaderSpecification.AddEntityID;
		EditorUI::EditorUIService::Checkbox(m_ShapeAddEntityID);
	}
}
