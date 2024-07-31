#include "kgpch.h"

#include "Panels/SceneEditorPanel.h"

#include "EditorApp.h"

#include "API/EditorUI/ImGuiBackendAPI.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{

	// Scene Hierarchy Tree
	static EditorUI::PanelHeaderSpec s_MainSceneHeader{};
	static EditorUI::TreeSpec s_SceneHierarchyTree {};
	static int32_t s_AddComponentEntity {};
	static EditorUI::SelectOptionSpec s_AddComponent{};

	// Tag Component
	static EditorUI::CollapsingHeaderSpec s_TagHeader{};
	static EditorUI::TextInputSpec s_TagEdit{};

	// Transform Component
	static EditorUI::CollapsingHeaderSpec s_TransformHeader{};
	static EditorUI::EditVec3Spec s_TransformEditTranslation{};
	static EditorUI::EditVec3Spec s_TransformEditScale{};
	static EditorUI::EditVec3Spec s_TransformEditRotation{};

	// Class Instance Component
	static EditorUI::CollapsingHeaderSpec s_ClassInstanceHeader{};
	static EditorUI::SelectOptionSpec s_SelectClassOption{};
	static EditorUI::TableSpec s_InstanceFieldsTable{};
	static std::string s_CurrentClassField {};
	static int32_t s_CurrentClassFieldLocation {};
	static EditorUI::GenericPopupSpec s_EditClassFieldPopup{};
	static EditorUI::EditVariableSpec s_EditFieldValue {};

	// Rigid Body 2D Component
	static EditorUI::CollapsingHeaderSpec s_Rigidbody2DHeader{};
	static EditorUI::RadioSelectorSpec s_Rigidbody2DType {};
	static EditorUI::CheckboxSpec s_RigidBody2DFixedRotation {};

	// Box Collider 2D Component
	static EditorUI::CollapsingHeaderSpec s_BoxCollider2DHeader{};
	static EditorUI::EditVec2Spec s_BoxColliderOffset{};
	static EditorUI::EditVec2Spec s_BoxColliderSize{};
	static EditorUI::EditFloatSpec s_BoxColliderDensity{};
	static EditorUI::EditFloatSpec s_BoxColliderFriction{};
	static EditorUI::EditFloatSpec s_BoxColliderRestitution{};
	static EditorUI::EditFloatSpec s_BoxColliderRestitutionThreshold{};

	// Circle Collider 2D Component
	static EditorUI::CollapsingHeaderSpec s_CircleCollider2DHeader{};
	static EditorUI::EditVec2Spec s_CircleColliderOffset{};
	static EditorUI::EditFloatSpec s_CircleColliderRadius{};
	static EditorUI::EditFloatSpec s_CircleColliderDensity{};
	static EditorUI::EditFloatSpec s_CircleColliderFriction{};
	static EditorUI::EditFloatSpec s_CircleColliderRestitution{};
	static EditorUI::EditFloatSpec s_CircleColliderRestitutionThreshold{};

	// Camera Component
	static EditorUI::CollapsingHeaderSpec s_CameraHeader{};
	static EditorUI::CheckboxSpec s_CameraPrimary {};
	static EditorUI::RadioSelectorSpec s_CameraProjection {};
	static EditorUI::EditFloatSpec s_CameraPerspectiveFOV{};
	static EditorUI::EditFloatSpec s_CameraPerspectiveNearPlane{};
	static EditorUI::EditFloatSpec s_CameraPerspectiveFarPlane{};
	static EditorUI::EditFloatSpec s_CameraOrthographicSize{};
	static EditorUI::EditFloatSpec s_CameraOrthographicNearPlane{};
	static EditorUI::EditFloatSpec s_CameraOrthographicFarPlane{};

	// Shape Component
	static EditorUI::CollapsingHeaderSpec s_ShapeHeader{};
	static std::function<void()> s_UpdateComponent { nullptr };
	static std::function<void()> s_AddColorSection { nullptr };
	static std::function<void()> s_AddTextureSection { nullptr };
	static std::function<void()> s_AddCircleShapeSection { nullptr };
	static std::function<void()> s_AddProjectionMatrixSection { nullptr };
	static std::function<void()> s_AddEntityIDSection { nullptr };

	static EditorUI::SelectOptionSpec s_ShapeSelect {};
	static EditorUI::SelectOptionSpec s_ShapeColorType {};
	static EditorUI::CheckboxSpec s_ShapeAddTexture {};
	static EditorUI::SelectOptionSpec s_ShapeSetTexture {};
	static EditorUI::EditFloatSpec s_ShapeTilingFactor{};
	static EditorUI::CheckboxSpec s_ShapeAddCircle {};
	static EditorUI::EditFloatSpec s_ShapeCircleThickness{};
	static EditorUI::EditFloatSpec s_ShapeCircleFade{};
	static EditorUI::CheckboxSpec s_ShapeAddProjection {};
	static EditorUI::CheckboxSpec s_ShapeAddEntityID {};

	static void InitializeSceneHierarchy()
	{
		s_MainSceneHeader.Label = "No Scene Name";
		s_MainSceneHeader.EditColorActive = false;
		s_MainSceneHeader.AddToSelectionList("Create Entity", []()
		{
			Scenes::SceneService::GetActiveScene()->CreateEntity("Empty Entity");
		});

		s_SceneHierarchyTree.Label = "Scene Hierarchy";
		s_SceneHierarchyTree.OnRefresh = [&]()
		{
			if (Scenes::SceneService::GetActiveScene())
			{
				s_SceneHierarchyTree.ClearTree();
				Scenes::SceneService::GetActiveScene()->m_Registry.each([&](auto entityID)
				{
					Scenes::Entity entity{ entityID, Scenes::SceneService::GetActiveScene().get() };

					EditorUI::TreeEntry newEntry {};
					newEntry.Label = entity.GetComponent<Scenes::TagComponent>().Tag;
					newEntry.IconHandle = EditorUI::EditorUIService::s_IconEntity;
					newEntry.Handle = (uint64_t)entityID;
					newEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
					{
						Scenes::Entity entity{ entt::entity((int)entry.Handle), Scenes::SceneService::GetActiveScene().get() };
						s_EditorApp->m_SceneHierarchyPanel->SetSelectedEntity(entity);
					};
					newEntry.OnDoubleLeftClick = [](EditorUI::TreeEntry& entry)
					{
						Scenes::Entity entity{ entt::entity((int)entry.Handle), Scenes::SceneService::GetActiveScene().get() };
						auto& editorCamera = EditorApp::GetCurrentApp()->m_ViewportPanel->m_EditorCamera;
						auto& transformComponent = entity.GetComponent<Scenes::TransformComponent>();
						editorCamera.SetFocalPoint(transformComponent.Translation);
						editorCamera.SetDistance(std::max({ transformComponent.Scale.x, transformComponent.Scale.y, transformComponent.Scale.z }) * 2.5f);
						editorCamera.SetMovementType(Rendering::EditorCamera::MovementType::ModelView);
					};

					newEntry.OnRightClickSelection.push_back({ "Add Component", [](EditorUI::TreeEntry& entry)
					{
						s_AddComponent.PopupActive = true;
						s_AddComponentEntity = (int32_t)entry.Handle;
					} });

					newEntry.OnRightClickSelection.push_back({ "Delete Entity", [](EditorUI::TreeEntry& entry)
					{
						static Scenes::Entity entityToDelete;
						entityToDelete = { entt::entity((int)entry.Handle), Scenes::SceneService::GetActiveScene().get() };

						EngineService::SubmitToMainThread([&]()
						{
							if (!entityToDelete)
							{
								KG_WARN("Attempt to delete entity that does not exist");
								return;
							}
							Scenes::SceneService::GetActiveScene()->DestroyEntity(entityToDelete);
							if (*Scenes::SceneService::GetActiveScene()->GetSelectedEntity() == entityToDelete)
							{
								*Scenes::SceneService::GetActiveScene()->GetSelectedEntity() = {};
								s_EditorApp->m_SceneHierarchyPanel->SetSelectedEntity({});
							}
						});

					} });

					EditorUI::TreeEntry componentEntry {};
					componentEntry.OnLeftClick = [](EditorUI::TreeEntry& entry)
					{
						
					};
					if (entity.HasComponent<Scenes::TagComponent>())
					{
						componentEntry.Label = "Tag Component";
						componentEntry.IconHandle = EditorUI::EditorUIService::s_IconTag;
						newEntry.SubEntries.push_back(componentEntry);
					}
					if (entity.HasComponent<Scenes::TransformComponent>())
					{
						componentEntry.Label = "Transform Component";
						componentEntry.IconHandle = EditorUI::EditorUIService::s_IconTransform;
						newEntry.SubEntries.push_back(componentEntry);
					}
					if (entity.HasComponent<Scenes::ClassInstanceComponent>())
					{
						componentEntry.Label = "Class Instance Component";
						componentEntry.IconHandle = EditorUI::EditorUIService::s_IconClassInstance;
						newEntry.SubEntries.push_back(componentEntry);
					}

					if (entity.HasComponent<Scenes::Rigidbody2DComponent>())
					{
						componentEntry.Label = "Rigid Body 2D Component";
						componentEntry.IconHandle = EditorUI::EditorUIService::s_IconRigidBody;
						newEntry.SubEntries.push_back(componentEntry);
					}

					if (entity.HasComponent<Scenes::BoxCollider2DComponent>())
					{
						componentEntry.Label = "Box Collider 2D Component";
						componentEntry.IconHandle = EditorUI::EditorUIService::s_IconBoxCollider;
						newEntry.SubEntries.push_back(componentEntry);
					}

					if (entity.HasComponent<Scenes::CircleCollider2DComponent>())
					{
						componentEntry.Label = "Circle Collider 2D Component";
						componentEntry.IconHandle = EditorUI::EditorUIService::s_IconCircleCollider;
						newEntry.SubEntries.push_back(componentEntry);
					}

					if (entity.HasComponent<Scenes::CameraComponent>())
					{
						componentEntry.Label = "Camera Component";
						componentEntry.IconHandle = EditorUI::EditorUIService::s_IconCameraActive;
						newEntry.SubEntries.push_back(componentEntry);
					}

					if (entity.HasComponent<Scenes::ShapeComponent>())
					{
						componentEntry.Label = "Shape Component";
						componentEntry.IconHandle = EditorUI::EditorUIService::s_IconEntity;
						newEntry.SubEntries.push_back(componentEntry);
					}


					s_SceneHierarchyTree.InsertEntry(newEntry);

					//TODO: Remove this last section
					if (entity == *Scenes::SceneService::GetActiveScene()->GetSelectedEntity())
					{
						EditorUI::TreePath path = s_SceneHierarchyTree.GetPathFromEntryReference(&s_SceneHierarchyTree.GetTreeEntries().back());

						if (path)
						{
							s_SceneHierarchyTree.SelectedEntry = path;
						}
					}
				});
			}
		};

		s_AddComponent.Label = "Add Component";
		s_AddComponent.Flags = EditorUI::SelectOption_PopupOnly;
		s_AddComponent.PopupAction = [&]()
		{
			
			Scenes::Entity entity = { entt::entity(s_AddComponentEntity), Scenes::SceneService::GetActiveScene().get() };
			if (!entity)
			{
				KG_WARN("Attempt to add component to empty entity");
				return;
			}
			s_AddComponent.ClearOptions();
			s_AddComponent.AddToOptions("Clear", "None", Assets::EmptyHandle);
			if (!entity.HasComponent<Scenes::ClassInstanceComponent>())
			{
				s_AddComponent.AddToOptions("New Component", "Class Instance", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<Scenes::CameraComponent>())
			{
				s_AddComponent.AddToOptions("New Component", "Camera", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<Scenes::ShapeComponent>())
			{
				s_AddComponent.AddToOptions("New Component", "Shape", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<Scenes::Rigidbody2DComponent>())
			{
				s_AddComponent.AddToOptions("New Component", "Rigidbody 2D", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<Scenes::BoxCollider2DComponent>())
			{
				s_AddComponent.AddToOptions("New Component", "Box Collider 2D", Assets::EmptyHandle);
			}
			if (!entity.HasComponent<Scenes::CircleCollider2DComponent>())
			{
				s_AddComponent.AddToOptions("New Component", "Circle Collider 2D", Assets::EmptyHandle);
			}

			s_AddComponent.CurrentOption = { "None", Assets::EmptyHandle };
		};

		s_AddComponent.ConfirmAction = [&](const EditorUI::OptionEntry& option)
		{
			Scenes::Entity entity = { entt::entity(s_AddComponentEntity), Scenes::SceneService::GetActiveScene().get() };
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
				entity.AddComponent<Scenes::ClassInstanceComponent>();
				return;
			}
			if (option.Label == "Camera")
			{
				entity.AddComponent<Scenes::CameraComponent>();
				return;
			}
			if (option.Label == "Shape")
			{
				entity.AddComponent<Scenes::ShapeComponent>();
				return;
			}
			if (option.Label == "Rigidbody 2D")
			{
				entity.AddComponent<Scenes::Rigidbody2DComponent>();
				return;
			}
			if (option.Label == "Box Collider 2D")
			{
				entity.AddComponent<Scenes::BoxCollider2DComponent>();
				return;
			}
			if (option.Label == "Circle Collider 2D")
			{
				entity.AddComponent<Scenes::CircleCollider2DComponent>();
				return;
			}

			KG_ERROR("Invalid option selected to add as component!");

		};
	}

	static void InitializeTagComponent()
	{
		s_TagHeader.Label = "Tag";
		s_TagHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		s_TagHeader.Expanded = true;

		s_TagEdit.Label = "Tag Label";
		s_TagEdit.Flags |= EditorUI::TextInput_Indented;
		s_TagEdit.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (entity && entity.HasComponent<Scenes::TagComponent>())
			{
				auto& component = entity.GetComponent<Scenes::TagComponent>();
				component.Tag = s_TagEdit.CurrentOption;
			}
		};
	}

	static void InitializeClassInstanceComponent()
	{
		s_ClassInstanceHeader.Label = "Class Instance";
		s_ClassInstanceHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		s_ClassInstanceHeader.Expanded = true;
		s_ClassInstanceHeader.AddToSelectionList("Remove Component", [&]()
		{
			EngineService::SubmitToMainThread([&]()
			{
				Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<Scenes::ClassInstanceComponent>())
				{
					entity.RemoveComponent<Scenes::ClassInstanceComponent>();
				}
			});
		});

		s_SelectClassOption.Label = "Class";
		s_SelectClassOption.Flags |= EditorUI::SelectOption_Indented;
		s_SelectClassOption.CurrentOption = { "None", Assets::EmptyHandle };
		s_SelectClassOption.PopupAction = [&]()
		{
			s_SelectClassOption.ClearOptions();
			s_SelectClassOption.AddToOptions("Clear", "None", Assets::EmptyHandle);
			for (auto& [handle, asset] : Assets::AssetManager::GetEntityClassRegistry())
			{
				s_SelectClassOption.AddToOptions("All Options",
					asset.Data.GetSpecificFileData<Assets::EntityClassMetaData>()->Name, handle);
			}
		};
		s_SelectClassOption.ConfirmAction = [&](const EditorUI::OptionEntry& entry)
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			auto& component = entity.GetComponent<Scenes::ClassInstanceComponent>();

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
				s_InstanceFieldsTable.OnRefresh();
			}
		};

		s_InstanceFieldsTable.Label = "Instance Fields";
		s_InstanceFieldsTable.Flags |= EditorUI::Table_Indented;
		s_InstanceFieldsTable.Column1Title = "Field Name";
		s_InstanceFieldsTable.Column2Title = "Field Value";
		s_InstanceFieldsTable.Expanded = true;
		s_InstanceFieldsTable.OnRefresh = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();

			if (!entity || !entity.HasComponent<Scenes::ClassInstanceComponent>())
			{
				return;
			}

			auto& component = entity.GetComponent<Scenes::ClassInstanceComponent>();
			uint32_t iteration{ 0 };
			s_InstanceFieldsTable.ClearTable();
			for(auto& wrappedVar : component.Fields)
			{
				s_InstanceFieldsTable.InsertTableEntry(
				{
					component.ClassReference->GetFields().at(iteration).Name,
					wrappedVar->GetValueAsString(),
					Assets::EmptyHandle,
					[&](EditorUI::TableEntry& optionEntry)
					{
						s_CurrentClassField = optionEntry.Label;
						Scenes::Entity currentEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
						auto& comp = currentEntity.GetComponent<Scenes::ClassInstanceComponent>();
						s_CurrentClassFieldLocation = comp.ClassReference->GetFieldLocation(s_CurrentClassField);
						if (s_CurrentClassFieldLocation == -1)
						{
							KG_WARN("Could not locate field in class definition");
							return;
						}
						s_EditClassFieldPopup.PopupActive = true;
					},
					nullptr
				});
				iteration++;
			}
		};

		s_EditClassFieldPopup.Label = "Edit Field";
		s_EditClassFieldPopup.PopupWidth = 420.0f;
		s_EditClassFieldPopup.PopupAction = [&]()
		{
			Scenes::Entity currentEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			auto& comp = currentEntity.GetComponent<Scenes::ClassInstanceComponent>();
			const Ref<WrappedVariable> field = comp.Fields.at(s_CurrentClassFieldLocation);

			if (!field)
			{
				KG_ERROR("Unable to retreive field from current game state object");
				return;
			}

			bool success = Utility::FillBufferWithWrappedVarString(field, s_EditFieldValue.FieldBuffer);
			s_EditFieldValue.VariableType = field->Type();
			if (!success)
			{
				KG_WARN("Unable to complete Popup Action");
				return;
			}
		};
		s_EditClassFieldPopup.PopupContents = [&]()
		{
			EditorUI::EditorUIService::EditVariable(s_EditFieldValue);
		};
		s_EditClassFieldPopup.ConfirmAction = [&]()
		{
			Scenes::Entity currentEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			auto& comp = currentEntity.GetComponent<Scenes::ClassInstanceComponent>();
			const Ref<WrappedVariable> field = comp.Fields.at(s_CurrentClassFieldLocation);
			bool success = Utility::FillWrappedVarWithStringBuffer(field, s_EditFieldValue.FieldBuffer);
			if (!success)
			{
				KG_WARN("Issue completing s_EditClassFieldPopup Confirm Action");
				return;
			}
			s_InstanceFieldsTable.OnRefresh();
		};
		s_EditFieldValue.Label = "Edit Value";
		s_EditFieldValue.AllocateBuffer();

	}

	static void InitializeTransformComponent()
	{
		s_TransformHeader.Label = "Transform";
		s_TransformHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		s_TransformHeader.Expanded = true;

		s_TransformEditTranslation.Label = "Translation";
		s_TransformEditTranslation.Flags = EditorUI::EditVec3_Indented;
		s_TransformEditTranslation.ConfirmAction = [&]() 
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::TransformComponent>())
			{
				KG_ERROR("Attempt to edit entity transform component when none exists!");
				return;
			}
			auto& transformComp = entity.GetComponent<Scenes::TransformComponent>();
			transformComp.Translation = s_TransformEditTranslation.CurrentVec3;
		};
		
		s_TransformEditScale.Label = "Scale";
		s_TransformEditScale.Flags = EditorUI::EditVec3_Indented;
		s_TransformEditScale.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::TransformComponent>())
			{
				KG_ERROR("Attempt to edit entity transform component when none exists!");
				return;
			}
			auto& transformComp = entity.GetComponent<Scenes::TransformComponent>();
			transformComp.Scale = s_TransformEditScale.CurrentVec3;
		};
		s_TransformEditRotation.Label = "Rotation";
		s_TransformEditRotation.Flags = EditorUI::EditVec3_Indented;
		s_TransformEditRotation.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::TransformComponent>())
			{
				KG_ERROR("Attempt to edit entity transform component when none exists!");
				return;
			}
			auto& transformComp = entity.GetComponent<Scenes::TransformComponent>();
			transformComp.Rotation = s_TransformEditRotation.CurrentVec3;
		};
		
	}

	static void InitializeRigidbody2DComponent()
	{
		s_Rigidbody2DHeader.Label = "Rigid Body 2D";
		s_Rigidbody2DHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		s_Rigidbody2DHeader.Expanded = true;
		s_Rigidbody2DHeader.AddToSelectionList("Remove Component", [&]()
		{
			EngineService::SubmitToMainThread([&]()
			{
				Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<Scenes::Rigidbody2DComponent>())
				{
					entity.RemoveComponent<Scenes::Rigidbody2DComponent>();
				}
			});
		});

		s_Rigidbody2DType.Label = "Interaction Type";
		s_Rigidbody2DType.FirstOptionLabel = "Static";
		s_Rigidbody2DType.SecondOptionLabel = "Dynamic";
		s_Rigidbody2DType.Flags |= EditorUI::RadioSelector_Indented;
		s_Rigidbody2DType.SelectAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::Rigidbody2DComponent>())
			{
				KG_ERROR("Attempt to edit entity rigid body 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::Rigidbody2DComponent>();

			if (s_Rigidbody2DType.SelectedOption == 0)
			{
				component.Type = Scenes::Rigidbody2DComponent::BodyType::Static;
				return;
			}
			if (s_Rigidbody2DType.SelectedOption == 1)
			{
				component.Type = Scenes::Rigidbody2DComponent::BodyType::Dynamic;
				return;
			}

			KG_ERROR("Invalid SelectedOption");
		};


		// Set Shape Add Fixed Rotation Option
		s_RigidBody2DFixedRotation.Label = "Use Fixed Rotation";
		s_RigidBody2DFixedRotation.Flags |= EditorUI::Checkbox_Indented;
		s_RigidBody2DFixedRotation.ConfirmAction = [&](bool value)
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::Rigidbody2DComponent>())
			{
				KG_ERROR("Attempt to edit entity rigid body 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::Rigidbody2DComponent>();
			component.FixedRotation = value;
		};
	}

	void InitializeBoxCollider2DComponent()
	{
		s_BoxCollider2DHeader.Label = "Box Collider 2D";
		s_BoxCollider2DHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		s_BoxCollider2DHeader.Expanded = true;
		s_BoxCollider2DHeader.AddToSelectionList("Remove Component", [&]()
		{
			EngineService::SubmitToMainThread([&]()
			{
				Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<Scenes::BoxCollider2DComponent>())
				{
					entity.RemoveComponent<Scenes::BoxCollider2DComponent>();
				}
			});
		});

		s_BoxColliderOffset.Label = "Offset";
		s_BoxColliderOffset.Flags |= EditorUI::EditVec2_Indented;
		s_BoxColliderOffset.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::BoxCollider2DComponent>();
			component.Offset = s_BoxColliderOffset.CurrentVec2;
		};

		s_BoxColliderSize.Label = "Size";
		s_BoxColliderSize.Flags |= EditorUI::EditVec2_Indented;
		s_BoxColliderSize.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::BoxCollider2DComponent>();
			component.Size = s_BoxColliderSize.CurrentVec2;
		};

		s_BoxColliderDensity.Label = "Density";
		s_BoxColliderDensity.Flags |= EditorUI::EditFloat_Indented;
		s_BoxColliderDensity.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::BoxCollider2DComponent>();
			component.Density = s_BoxColliderDensity.CurrentFloat;
		};

		s_BoxColliderFriction.Label = "Friction";
		s_BoxColliderFriction.Flags |= EditorUI::EditFloat_Indented;
		s_BoxColliderFriction.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::BoxCollider2DComponent>();
			component.Friction = s_BoxColliderFriction.CurrentFloat;
		};

		s_BoxColliderRestitution.Label = "Restitution";
		s_BoxColliderRestitution.Flags |= EditorUI::EditFloat_Indented;
		s_BoxColliderRestitution.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::BoxCollider2DComponent>();
			component.Restitution = s_BoxColliderRestitution.CurrentFloat;
		};

		s_BoxColliderRestitutionThreshold.Label = "Restitution Threshold";
		s_BoxColliderRestitutionThreshold.Flags |= EditorUI::EditFloat_Indented;
		s_BoxColliderRestitutionThreshold.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::BoxCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity box collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::BoxCollider2DComponent>();
			component.RestitutionThreshold = s_BoxColliderRestitutionThreshold.CurrentFloat;
		};
	}
	void InitializeCircleCollider2DComponent()
	{
		s_CircleCollider2DHeader.Label = "Circle Collider 2D";
		s_CircleCollider2DHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		s_CircleCollider2DHeader.Expanded = true;
		s_CircleCollider2DHeader.AddToSelectionList("Remove Component", [&]()
		{
			EngineService::SubmitToMainThread([&]()
			{
				Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<Scenes::CircleCollider2DComponent>())
				{
					entity.RemoveComponent<Scenes::CircleCollider2DComponent>();
				}
			});
		});

		s_CircleColliderOffset.Label = "Offset";
		s_CircleColliderOffset.Flags |= EditorUI::EditVec2_Indented;
		s_CircleColliderOffset.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::CircleCollider2DComponent>();
			component.Offset = s_CircleColliderOffset.CurrentVec2;
		};

		s_CircleColliderRadius.Label = "Radius";
		s_CircleColliderRadius.Flags |= EditorUI::EditFloat_Indented;
		s_CircleColliderRadius.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::CircleCollider2DComponent>();
			component.Radius = s_CircleColliderRadius.CurrentFloat;
		};

		s_CircleColliderDensity.Label = "Density";
		s_CircleColliderDensity.Flags |= EditorUI::EditFloat_Indented;
		s_CircleColliderDensity.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::CircleCollider2DComponent>();
			component.Density = s_CircleColliderDensity.CurrentFloat;
		};

		s_CircleColliderFriction.Label = "Friction";
		s_CircleColliderFriction.Flags |= EditorUI::EditFloat_Indented;
		s_CircleColliderFriction.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::CircleCollider2DComponent>();
			component.Friction = s_CircleColliderFriction.CurrentFloat;
		};

		s_CircleColliderRestitution.Label = "Restitution";
		s_CircleColliderRestitution.Flags |= EditorUI::EditFloat_Indented;
		s_CircleColliderRestitution.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::CircleCollider2DComponent>();
			component.Restitution = s_CircleColliderRestitution.CurrentFloat;
		};

		s_CircleColliderRestitutionThreshold.Label = "Restitution Threshold";
		s_CircleColliderRestitutionThreshold.Flags |= EditorUI::EditFloat_Indented;
		s_CircleColliderRestitutionThreshold.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::CircleCollider2DComponent>())
			{
				KG_ERROR("Attempt to edit entity circle collider 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::CircleCollider2DComponent>();
			component.RestitutionThreshold = s_CircleColliderRestitutionThreshold.CurrentFloat;
		};
	}

	void InitializeCameraComponent()
	{
		s_CameraHeader.Label = "Camera";
		s_CameraHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		s_CameraHeader.Expanded = true;
		s_CameraHeader.AddToSelectionList("Remove Component", [&]()
		{
			EngineService::SubmitToMainThread([&]()
			{
				Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<Scenes::CameraComponent>())
				{
					entity.RemoveComponent<Scenes::CameraComponent>();
				}
			});
		});

		// Set Primary Camera Checkbox
		s_CameraPrimary.Label = "Primary Camera";
		s_CameraPrimary.Flags |= EditorUI::Checkbox_Indented;
		s_CameraPrimary.ConfirmAction = [&](bool value)
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::CameraComponent>();
			component.Primary = value;
		};

		s_CameraProjection.Label = "Projection Type";
		s_CameraProjection.Flags |= EditorUI::RadioSelector_Indented;
		s_CameraProjection.FirstOptionLabel = "Perspective";
		s_CameraProjection.SecondOptionLabel = "Orthographic";
		s_CameraProjection.SelectAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::CameraComponent>();

			if (s_CameraProjection.SelectedOption == 0)
			{
				component.Camera.SetProjectionType(Scenes::SceneCamera::ProjectionType::Perspective);
				return;
			}
			if (s_CameraProjection.SelectedOption == 1)
			{
				component.Camera.SetProjectionType(Scenes::SceneCamera::ProjectionType::Orthographic);
				return;
			}
			KG_ERROR("Invalid selection option provided");
		};

		s_CameraOrthographicSize.Label = "Size";
		s_CameraOrthographicSize.Flags |= EditorUI::EditFloat_Indented;
		s_CameraOrthographicSize.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::CameraComponent>();
			component.Camera.SetOrthographicSize(s_CameraOrthographicSize.CurrentFloat);
		};

		s_CameraOrthographicNearPlane.Label = "Near Plane";
		s_CameraOrthographicNearPlane.Flags |= EditorUI::EditFloat_Indented;
		s_CameraOrthographicNearPlane.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::CameraComponent>();
			component.Camera.SetOrthographicNearClip(s_CameraOrthographicNearPlane.CurrentFloat);
		};

		s_CameraOrthographicFarPlane.Label = "Far Plane";
		s_CameraOrthographicFarPlane.Flags |= EditorUI::EditFloat_Indented;
		s_CameraOrthographicFarPlane.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::CameraComponent>();
			component.Camera.SetOrthographicFarClip(s_CameraOrthographicFarPlane.CurrentFloat);
		};

		s_CameraPerspectiveFOV.Label = "Vertical FOV";
		s_CameraPerspectiveFOV.Flags |= EditorUI::EditFloat_Indented;
		s_CameraPerspectiveFOV.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::CameraComponent>();
			component.Camera.SetPerspectiveVerticalFOV(s_CameraPerspectiveFOV.CurrentFloat);
		};

		s_CameraPerspectiveNearPlane.Label = "Near Plane";
		s_CameraPerspectiveNearPlane.Flags |= EditorUI::EditFloat_Indented;
		s_CameraPerspectiveNearPlane.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::CameraComponent>();
			component.Camera.SetPerspectiveNearClip(s_CameraPerspectiveNearPlane.CurrentFloat);
		};

		s_CameraPerspectiveFarPlane.Label = "Far Plane";
		s_CameraPerspectiveFarPlane.Flags |= EditorUI::EditFloat_Indented;
		s_CameraPerspectiveFarPlane.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::CameraComponent>())
			{
				KG_ERROR("Attempt to edit entity camera component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::CameraComponent>();
			component.Camera.SetPerspectiveFarClip(s_CameraPerspectiveFarPlane.CurrentFloat);
		};
	}

	static void InitializeShapeComponent()
	{
		//=========================
		// Lambda Functions for Later Use (Scroll down to find main function body)
		//=========================

		// This Lambda updates the shader after a checkbox is clicked that changes the component.ShaderSpecification
		// Another function of this lambda is to ensure data inside the old buffer is transferred to the new buffer
		// if they represent the same input
		s_UpdateComponent = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			Scenes::ShapeComponent& component = entity.GetComponent<Scenes::ShapeComponent>();
			// Get Previous Buffer and Previous Shader
			Buffer oldBuffer = component.ShaderData;
			Ref<Rendering::Shader> oldShader = component.Shader;
			// Get New Shader
			auto [newShaderAssetHandle, newShader] = Assets::AssetManager::GetShader(component.ShaderSpecification);
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
		};
		//=========================
		// More Lambdas: ShaderSpecification UI Code
		//=========================

		// These lambdas provide UI for user manipulating of shader specifications and input values
		s_AddColorSection = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			Scenes::ShapeComponent& component = entity.GetComponent<Scenes::ShapeComponent>();
			s_ShapeColorType.CurrentOption = { Utility::ColorInputTypeToString(component.ShaderSpecification.ColorInput), Assets::EmptyHandle };
			EditorUI::EditorUIService::SelectOption(s_ShapeColorType);

			if (component.ShaderSpecification.ColorInput == Rendering::ColorInputType::None) { return; }

			if (component.ShaderSpecification.ColorInput == Rendering::ColorInputType::FlatColor)
			{
				Math::vec4* color = Rendering::Shader::GetInputLocation<Math::vec4>("a_Color", component.ShaderData, component.Shader);
				ImGui::SetCursorPosX(30.0f);
				ImGui::ColorEdit4("Color", glm::value_ptr(*color));
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
		};

		s_AddTextureSection = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			Scenes::ShapeComponent& component = entity.GetComponent<Scenes::ShapeComponent>();
			s_ShapeAddTexture.ToggleBoolean = component.ShaderSpecification.TextureInput == Rendering::TextureInputType::ColorTexture ? true : false;
			EditorUI::EditorUIService::Checkbox(s_ShapeAddTexture);
			if (s_ShapeAddTexture.ToggleBoolean)
			{
				if (component.TextureHandle == Assets::EmptyHandle)
				{
					s_ShapeSetTexture.CurrentOption = { "None", Assets::EmptyHandle };
				}
				else
				{
					s_ShapeSetTexture.CurrentOption = 
					{
						Assets::AssetManager::GetTextureRegistry().at(component.TextureHandle).Data.GetSpecificFileData<Assets::TextureMetaData>()->InitialFileLocation.string(),
						component.TextureHandle
					};
				}
				EditorUI::EditorUIService::SelectOption(s_ShapeSetTexture);

				s_ShapeTilingFactor.CurrentFloat = *Rendering::Shader::GetInputLocation<float>("a_TilingFactor", component.ShaderData, component.Shader);
				EditorUI::EditorUIService::EditFloat(s_ShapeTilingFactor);
			}
		};

		s_AddCircleShapeSection = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			Scenes::ShapeComponent& component = entity.GetComponent<Scenes::ShapeComponent>();
			s_ShapeAddCircle.ToggleBoolean = component.ShaderSpecification.AddCircleShape;
			EditorUI::EditorUIService::Checkbox(s_ShapeAddCircle);
			if (component.ShaderSpecification.AddCircleShape)
			{
				s_ShapeCircleThickness.CurrentFloat = *Rendering::Shader::GetInputLocation<float>("a_Thickness", component.ShaderData, component.Shader);
				EditorUI::EditorUIService::EditFloat(s_ShapeCircleThickness);

				s_ShapeCircleFade.CurrentFloat = *Rendering::Shader::GetInputLocation<float>("a_Fade", component.ShaderData, component.Shader);
				EditorUI::EditorUIService::EditFloat(s_ShapeCircleFade);
			}
		};

		s_AddProjectionMatrixSection = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			Scenes::ShapeComponent& component = entity.GetComponent<Scenes::ShapeComponent>();
			s_ShapeAddProjection.ToggleBoolean = component.ShaderSpecification.AddProjectionMatrix;
			EditorUI::EditorUIService::Checkbox(s_ShapeAddProjection);
		};

		s_AddEntityIDSection = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			Scenes::ShapeComponent& component = entity.GetComponent<Scenes::ShapeComponent>();
			s_ShapeAddEntityID.ToggleBoolean = component.ShaderSpecification.AddEntityID;
			EditorUI::EditorUIService::Checkbox(s_ShapeAddEntityID);
		};

		s_ShapeHeader.Label = "Shape";
		s_ShapeHeader.Flags |= EditorUI::CollapsingHeader_UnderlineTitle;
		s_ShapeHeader.Expanded = true;
		s_ShapeHeader.AddToSelectionList("Remove Component", [&]()
		{
			EngineService::SubmitToMainThread([&]()
			{
				Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<Scenes::ShapeComponent>())
				{
					entity.RemoveComponent<Scenes::ShapeComponent>();
				}
			});
		});

		s_ShapeSelect.Label = "Mesh";
		s_ShapeSelect.Flags |= EditorUI::SelectOption_Indented;
		s_ShapeSelect.PopupAction = [&]()
		{
			s_ShapeSelect.ClearOptions();
			s_ShapeSelect.AddToOptions("Clear", "None", Assets::EmptyHandle);
			s_ShapeSelect.AddToOptions("All Shapes", "Quad", Assets::EmptyHandle);
			s_ShapeSelect.AddToOptions("All Shapes", "Pyramid", Assets::EmptyHandle);
			s_ShapeSelect.AddToOptions("All Shapes", "Cube", Assets::EmptyHandle);
		};
		s_ShapeSelect.ConfirmAction = [&](const EditorUI::OptionEntry& entry)
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			Scenes::ShapeComponent& component = entity.GetComponent<Scenes::ShapeComponent>();
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
			s_UpdateComponent();
			if (component.VertexColors)
			{
				component.VertexColors->resize(component.Vertices->size(), { 1.0f, 1.0f, 1.0f, 1.0f });
			}
		};

		s_ShapeColorType.Label = "Color Type";
		s_ShapeColorType.Flags |= EditorUI::SelectOption_Indented;
		s_ShapeColorType.PopupAction = [&]()
		{
			s_ShapeColorType.ClearOptions();
			s_ShapeColorType.AddToOptions("Clear", "None", Assets::EmptyHandle);
			s_ShapeColorType.AddToOptions("All Types", "FlatColor", Assets::EmptyHandle);
			s_ShapeColorType.AddToOptions("All Types", "VertexColor", Assets::EmptyHandle);
		};
		s_ShapeColorType.ConfirmAction = [&](const EditorUI::OptionEntry& entry)
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			Scenes::ShapeComponent& component = entity.GetComponent<Scenes::ShapeComponent>();
			if (entry.Label == "None")
			{
				component.ShaderSpecification.ColorInput = Rendering::ColorInputType::None;
				s_UpdateComponent();
			}
			if (entry.Label == "FlatColor")
			{
				component.ShaderSpecification.ColorInput = Rendering::ColorInputType::FlatColor;
				s_UpdateComponent();
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
				s_UpdateComponent();
				if (component.VertexColors) { component.VertexColors->clear(); }
				component.VertexColors = CreateRef<std::vector<Math::vec4>>();
				for (uint32_t iterator{ 0 }; iterator < component.Vertices->size(); iterator++)
				{
					component.VertexColors->push_back(transferColor);
				}
			}
		};

		// Set Shape Add Texture Checkbox
		s_ShapeAddTexture.Label = "Use Texture";
		s_ShapeAddTexture.Flags |= EditorUI::Checkbox_Indented;
		s_ShapeAddTexture.ConfirmAction = [&](bool value)
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			Scenes::ShapeComponent& component = entity.GetComponent<Scenes::ShapeComponent>();
			value ? component.ShaderSpecification.TextureInput = Rendering::TextureInputType::ColorTexture :
				component.ShaderSpecification.TextureInput = Rendering::TextureInputType::None;
			s_UpdateComponent();
			// Checkbox is switched on
			if (value)
			{
				if (component.CurrentShape == Rendering::ShapeTypes::Cube || component.CurrentShape == Rendering::ShapeTypes::Pyramid)
				{
					component.ShaderSpecification.RenderType = Rendering::RenderingType::DrawTriangle;
					s_UpdateComponent();
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
					s_UpdateComponent();
					component.Vertices = CreateRef<std::vector<Math::vec3>>(Utility::ShapeTypeToShape(component.CurrentShape).GetIndexVertices());
					component.Indices = CreateRef<std::vector<uint32_t>>(Utility::ShapeTypeToShape(component.CurrentShape).GetIndices());
					component.TextureCoordinates = CreateRef<std::vector<Math::vec2>>(Utility::ShapeTypeToShape(component.CurrentShape).GetIndexTextureCoordinates());
					if (component.VertexColors) { component.VertexColors->resize(component.Vertices->size(), { 1.0f, 1.0f, 1.0f, 1.0f }); }
				}
			}
		};

		s_ShapeSetTexture.Label = "Select Texture";
		s_ShapeSetTexture.Flags |= EditorUI::SelectOption_Indented;
		s_ShapeSetTexture.PopupAction = [&]()
		{
			s_ShapeSetTexture.ClearOptions();
			for (auto& [handle, asset] : Assets::AssetManager::GetTextureRegistry())
			{
				s_ShapeSetTexture.AddToOptions("All Textures", asset.Data.GetSpecificFileData<Assets::TextureMetaData>()->InitialFileLocation.string(), handle);
			}
		};
		s_ShapeSetTexture.ConfirmAction = [&](const EditorUI::OptionEntry& entry)
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			Scenes::ShapeComponent& component = entity.GetComponent<Scenes::ShapeComponent>();
			if (entry.Handle == Assets::EmptyHandle)
			{
				Buffer textureBuffer{ 4 };
				textureBuffer.SetDataToByte(0xff);
				component.TextureHandle = Assets::AssetManager::ImportNewTextureFromData(textureBuffer, 1, 1, 4);
				component.Texture = Assets::AssetManager::GetTexture(component.TextureHandle);
				textureBuffer.Release();
			}

			if (!Assets::AssetManager::GetTextureRegistry().contains(entry.Handle))
			{
				KG_WARN("Could not locate texture in asset registry!");
				return;
			}

			component.TextureHandle = entry.Handle;
			component.Texture = Assets::AssetManager::GetTexture(entry.Handle);
		};

		s_ShapeTilingFactor.Label = "Tiling Factor";
		s_ShapeTilingFactor.Flags |= EditorUI::EditFloat_Indented;
		s_ShapeTilingFactor.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::ShapeComponent>())
			{
				KG_ERROR("Attempt to edit entity shape 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::ShapeComponent>();
			float* tilingFactor = Rendering::Shader::GetInputLocation<float>("a_TilingFactor", component.ShaderData, component.Shader);
			*tilingFactor = s_ShapeTilingFactor.CurrentFloat;
		};

		// Set Shape Circle Option
		s_ShapeAddCircle.Label = "Use Circle Shape";
		s_ShapeAddCircle.Flags |= EditorUI::Checkbox_Indented;
		s_ShapeAddCircle.ConfirmAction = [&](bool value)
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			Scenes::ShapeComponent& component = entity.GetComponent<Scenes::ShapeComponent>();
			component.ShaderSpecification.AddCircleShape = value;
			s_UpdateComponent();
			if (value)
			{
				Rendering::Shader::SetDataAtInputLocation<float>(1.0f, "a_Thickness", component.ShaderData, component.Shader);
				Rendering::Shader::SetDataAtInputLocation<float>(0.005f, "a_Fade", component.ShaderData, component.Shader);
			}
		};

		s_ShapeCircleThickness.Label = "Circle Thickness";
		s_ShapeCircleThickness.Flags |= EditorUI::EditFloat_Indented;
		s_ShapeCircleThickness.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::ShapeComponent>())
			{
				KG_ERROR("Attempt to edit entity shape 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::ShapeComponent>();
			float* thickness = Rendering::Shader::GetInputLocation<float>("a_Thickness", component.ShaderData, component.Shader);
			*thickness = s_ShapeCircleThickness.CurrentFloat;
		};

		s_ShapeCircleFade.Label = "Circle Fade";
		s_ShapeCircleFade.Flags |= EditorUI::EditFloat_Indented;
		s_ShapeCircleFade.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (!entity.HasComponent<Scenes::ShapeComponent>())
			{
				KG_ERROR("Attempt to edit entity shape 2D component when none exists!");
				return;
			}
			auto& component = entity.GetComponent<Scenes::ShapeComponent>();
			float* fade = Rendering::Shader::GetInputLocation<float>("a_Fade", component.ShaderData, component.Shader);
			*fade = s_ShapeCircleFade.CurrentFloat;
		};

		// Set Shape Add Projection Option
		s_ShapeAddProjection.Label = "Use Projection Matrix";
		s_ShapeAddProjection.Flags |= EditorUI::Checkbox_Indented;
		s_ShapeAddProjection.ConfirmAction = [&](bool value)
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			Scenes::ShapeComponent& component = entity.GetComponent<Scenes::ShapeComponent>();
			component.ShaderSpecification.AddProjectionMatrix = value;
			s_UpdateComponent();
		};

		// Set Shape Add Entity ID Option
		s_ShapeAddEntityID.Label = "Use Entity ID";
		s_ShapeAddEntityID.Flags |= EditorUI::Checkbox_Indented;
		s_ShapeAddEntityID.ConfirmAction = [&](bool value)
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			Scenes::ShapeComponent& component = entity.GetComponent<Scenes::ShapeComponent>();
			component.ShaderSpecification.AddEntityID = value;
			s_UpdateComponent();
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
			s_MainSceneHeader.Label = Assets::AssetManager::GetSceneRegistry().at(
				Scenes::SceneService::GetActiveSceneHandle()).Data.IntermediateLocation.string();
			
			EditorUI::EditorUIService::PanelHeader(s_MainSceneHeader);
			s_SceneHierarchyTree.OnRefresh();
			EditorUI::EditorUIService::Tree(s_SceneHierarchyTree);

			EditorUI::EditorUIService::SelectOption(s_AddComponent);
		}
		EditorUI::EditorUIService::EndWindow();
	}
	bool SceneEditorPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		switch (event.GetKeyCode())
		{
			case Key::Escape:
			{
				s_EditorApp->m_SceneHierarchyPanel->SetSelectedEntity({});
				return true;
			}
			default:
			{
				return false;
			}
		}
	}
	void SceneEditorPanel::SetSelectedEntity(Scenes::Entity entity)
	{
		*Scenes::SceneService::GetActiveScene()->GetSelectedEntity() = entity;
		if (!entity)
		{
			s_SceneHierarchyTree.SelectedEntry = {};
		}
		if (entity)
		{
			if (entity.HasComponent<Scenes::ClassInstanceComponent>())
			{
				Scenes::ClassInstanceComponent& instanceComp = entity.GetComponent<Scenes::ClassInstanceComponent>();
				if (instanceComp.ClassHandle == Assets::EmptyHandle)
				{
					s_SelectClassOption.CurrentOption = { "None", Assets::EmptyHandle };
				}
				else
				{
					s_SelectClassOption.CurrentOption = { instanceComp.ClassReference->GetName(),instanceComp.ClassHandle };
				}
				s_InstanceFieldsTable.OnRefresh();
			}
		}

		//TODO: Add Selected Entry to Tree
		/*EditorUI::TreePath path = s_SceneHierarchyTree.GetPathFromEntryReference(&s_SceneHierarchyTree.GetTreeEntries().back());

		if (path)
		{
			s_SceneHierarchyTree.SelectedEntry = path;
		}*/
		

		RefreshTransformComponent();
		s_EditorApp->m_ShowProperties = true;
		EditorUI::EditorUIService::BringWindowToFront(s_EditorApp->m_PropertiesPanel->m_PanelName);
		
	}
	void SceneEditorPanel::RefreshClassInstanceComponent()
	{
		Scenes::Entity currentEntity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		if (!currentEntity)
		{
			return;
		}
		if (currentEntity.HasComponent<Scenes::ClassInstanceComponent>())
		{
			auto& comp = currentEntity.GetComponent<Scenes::ClassInstanceComponent>();
			if (!Assets::AssetManager::GetEntityClass(comp.ClassHandle))
			{
				comp.ClassHandle = Assets::EmptyHandle;
				comp.Fields.clear();
				comp.ClassReference = nullptr;
				s_SelectClassOption.CurrentOption = {"None", Assets::EmptyHandle};
			}
			s_InstanceFieldsTable.OnRefresh();
		}
	}
	void SceneEditorPanel::RefreshTransformComponent()
	{
		Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
		if (!entity)
		{
			return;
		}
		if (entity.HasComponent<Scenes::TransformComponent>())
		{
			auto& transformComp = entity.GetComponent<Scenes::TransformComponent>();
			s_TransformEditTranslation.CurrentVec3 = transformComp.Translation;
			s_TransformEditRotation.CurrentVec3 = transformComp.Rotation;
			s_TransformEditScale.CurrentVec3 = transformComp.Scale;
		}
	}

	void SceneEditorPanel::DrawComponents(Scenes::Entity entity)
	{
		if (entity.HasComponent<Scenes::TagComponent>())
		{
			Scenes::TagComponent& component = entity.GetComponent<Scenes::TagComponent>();
			EditorUI::EditorUIService::CollapsingHeader(s_TagHeader);
			if (s_TagHeader.Expanded)
			{
				s_TagEdit.CurrentOption = component.Tag;
				EditorUI::EditorUIService::TextInput(s_TagEdit);
			}
		}

		if (entity.HasComponent<Scenes::TransformComponent>())
		{
			Scenes::TransformComponent& component = entity.GetComponent<Scenes::TransformComponent>();
			EditorUI::EditorUIService::CollapsingHeader(s_TransformHeader);
			if (s_TransformHeader.Expanded)
			{
				s_TransformEditTranslation.CurrentVec3 = component.Translation;
				EditorUI::EditorUIService::EditVec3(s_TransformEditTranslation);
				s_TransformEditScale.CurrentVec3 = component.Scale;
				EditorUI::EditorUIService::EditVec3(s_TransformEditScale);
				s_TransformEditRotation.CurrentVec3 = component.Rotation;
				EditorUI::EditorUIService::EditVec3(s_TransformEditRotation);
			}
		}

		if (entity.HasComponent<Scenes::ClassInstanceComponent>())
		{
			Scenes::ClassInstanceComponent& component = entity.GetComponent<Scenes::ClassInstanceComponent>();
			EditorUI::EditorUIService::CollapsingHeader(s_ClassInstanceHeader);
			if (s_ClassInstanceHeader.Expanded)
			{
				EditorUI::EditorUIService::SelectOption(s_SelectClassOption);
				if (component.ClassHandle != Assets::EmptyHandle)
				{
					EditorUI::EditorUIService::Table(s_InstanceFieldsTable);
					EditorUI::EditorUIService::GenericPopup(s_EditClassFieldPopup);
				}
			}
		}

		if (entity.HasComponent<Scenes::Rigidbody2DComponent>())
		{
			Scenes::Rigidbody2DComponent& component = entity.GetComponent<Scenes::Rigidbody2DComponent>();
			EditorUI::EditorUIService::CollapsingHeader(s_Rigidbody2DHeader);
			if (s_Rigidbody2DHeader.Expanded)
			{
				s_Rigidbody2DType.SelectedOption = component.Type == Scenes::Rigidbody2DComponent::BodyType::Static ?
					0 : 1;
				EditorUI::EditorUIService::RadioSelector(s_Rigidbody2DType);
				s_RigidBody2DFixedRotation.ToggleBoolean = component.FixedRotation;
				EditorUI::EditorUIService::Checkbox(s_RigidBody2DFixedRotation);
			}
		}

		if (entity.HasComponent<Scenes::BoxCollider2DComponent>())
		{
			Scenes::BoxCollider2DComponent& component = entity.GetComponent<Scenes::BoxCollider2DComponent>();
			EditorUI::EditorUIService::CollapsingHeader(s_BoxCollider2DHeader);
			if (s_BoxCollider2DHeader.Expanded)
			{
				s_BoxColliderOffset.CurrentVec2 = component.Offset;
				EditorUI::EditorUIService::EditVec2(s_BoxColliderOffset);
				s_BoxColliderSize.CurrentVec2 = component.Size;
				EditorUI::EditorUIService::EditVec2(s_BoxColliderSize);
				s_BoxColliderDensity.CurrentFloat = component.Density;
				EditorUI::EditorUIService::EditFloat(s_BoxColliderDensity);
				s_BoxColliderFriction.CurrentFloat = component.Friction;
				EditorUI::EditorUIService::EditFloat(s_BoxColliderFriction);
				s_BoxColliderRestitution.CurrentFloat = component.Restitution;
				EditorUI::EditorUIService::EditFloat(s_BoxColliderRestitution);
				s_BoxColliderRestitutionThreshold.CurrentFloat = component.RestitutionThreshold;
				EditorUI::EditorUIService::EditFloat(s_BoxColliderRestitutionThreshold);
			}
		}

		if (entity.HasComponent<Scenes::CircleCollider2DComponent>())
		{
			Scenes::CircleCollider2DComponent& component = entity.GetComponent<Scenes::CircleCollider2DComponent>();
			EditorUI::EditorUIService::CollapsingHeader(s_CircleCollider2DHeader);
			if (s_CircleCollider2DHeader.Expanded)
			{
				s_CircleColliderOffset.CurrentVec2 = component.Offset;
				EditorUI::EditorUIService::EditVec2(s_CircleColliderOffset);
				s_CircleColliderRadius.CurrentFloat = component.Radius;
				EditorUI::EditorUIService::EditFloat(s_CircleColliderRadius);
				s_CircleColliderDensity.CurrentFloat = component.Density;
				EditorUI::EditorUIService::EditFloat(s_CircleColliderDensity);
				s_CircleColliderFriction.CurrentFloat = component.Friction;
				EditorUI::EditorUIService::EditFloat(s_CircleColliderFriction);
				s_CircleColliderRestitution.CurrentFloat = component.Restitution;
				EditorUI::EditorUIService::EditFloat(s_CircleColliderRestitution);
				s_CircleColliderRestitutionThreshold.CurrentFloat = component.RestitutionThreshold;
				EditorUI::EditorUIService::EditFloat(s_CircleColliderRestitutionThreshold);
			}
		}

		if (entity.HasComponent<Scenes::CameraComponent>())
		{
			Scenes::CameraComponent& component = entity.GetComponent<Scenes::CameraComponent>();
			EditorUI::EditorUIService::CollapsingHeader(s_CameraHeader);
			if (s_CameraHeader.Expanded)
			{
				s_CameraPrimary.ToggleBoolean = component.Primary;
				EditorUI::EditorUIService::Checkbox(s_CameraPrimary);
				s_CameraProjection.SelectedOption = component.Camera.GetProjectionType() ==
					Scenes::SceneCamera::ProjectionType::Perspective ? 0 : 1;
				EditorUI::EditorUIService::RadioSelector(s_CameraProjection);

				if (component.Camera.GetProjectionType() == Scenes::SceneCamera::ProjectionType::Perspective)
				{
					s_CameraPerspectiveFOV.CurrentFloat = component.Camera.GetPerspectiveVerticalFOV();
					EditorUI::EditorUIService::EditFloat(s_CameraPerspectiveFOV);
					s_CameraPerspectiveNearPlane.CurrentFloat = component.Camera.GetPerspectiveNearClip();
					EditorUI::EditorUIService::EditFloat(s_CameraPerspectiveNearPlane);
					s_CameraPerspectiveFarPlane.CurrentFloat = component.Camera.GetPerspectiveFarClip();
					EditorUI::EditorUIService::EditFloat(s_CameraPerspectiveFarPlane);
				}
				else
				{
					s_CameraOrthographicSize.CurrentFloat = component.Camera.GetOrthographicSize();
					EditorUI::EditorUIService::EditFloat(s_CameraOrthographicSize);
					s_CameraOrthographicNearPlane.CurrentFloat = component.Camera.GetOrthographicNearClip();
					EditorUI::EditorUIService::EditFloat(s_CameraOrthographicNearPlane);
					s_CameraOrthographicFarPlane.CurrentFloat = component.Camera.GetOrthographicFarClip();
					EditorUI::EditorUIService::EditFloat(s_CameraOrthographicFarPlane);
				}
			}
		}

		if (entity.HasComponent<Scenes::ShapeComponent>())
		{
			Scenes::ShapeComponent& component = entity.GetComponent<Scenes::ShapeComponent>();
			EditorUI::EditorUIService::CollapsingHeader(s_ShapeHeader);
			if (s_ShapeHeader.Expanded)
			{
				s_ShapeSelect.CurrentOption = { Utility::ShapeTypeToString(component.CurrentShape), Assets::EmptyHandle };
				EditorUI::EditorUIService::SelectOption(s_ShapeSelect);

				// This section displays the shader specification options available for the chosen object
				if (component.CurrentShape == Rendering::ShapeTypes::None)
				{
					return;
				}
				if (component.CurrentShape == Rendering::ShapeTypes::Quad)
				{
					s_AddColorSection();
					s_AddTextureSection();
					s_AddCircleShapeSection();
					s_AddProjectionMatrixSection();
					s_AddEntityIDSection();

				}
				if (component.CurrentShape == Rendering::ShapeTypes::Cube || component.CurrentShape == Rendering::ShapeTypes::Pyramid)
				{
					s_AddColorSection();
					s_AddTextureSection();
					s_AddProjectionMatrixSection();
					s_AddEntityIDSection();
				}
			}
		}
	}
}
