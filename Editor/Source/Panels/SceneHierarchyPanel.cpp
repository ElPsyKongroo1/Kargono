#include "kgpch.h"

#include "Panels/SceneHierarchyPanel.h"

#include "EditorApp.h"

#include "API/EditorUI/ImGuiBackendAPI.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{
	// Main Header & Tag Component
	static EditorUI::PanelHeaderSpec s_MainHeader{};
	static EditorUI::TextInputSpec s_TagEdit{};
	static EditorUI::SelectOptionSpec s_AddComponent{};

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

	// TODO: Unsorted widgets
	static EditorUI::CheckboxSpec s_PrimaryCameraCheckboxSpec {};
	static EditorUI::CheckboxSpec s_ShapeAddTextureCheckboxSpec {};
	static EditorUI::CheckboxSpec s_ShapeAddCircleSpec {};
	static EditorUI::CheckboxSpec s_ShapeAddProjectionSpec {};
	static EditorUI::CheckboxSpec s_ShapeAddEntityIDSpec {};

	static void InitializeMainHeader()
	{
		s_MainHeader.Label = "Empty Tag";
		s_MainHeader.EditColorActive = false;
		s_MainHeader.AddToSelectionList("Edit Entity Tag", [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (entity && entity.HasComponent<Scenes::TagComponent>())
			{
				auto& component = entity.GetComponent<Scenes::TagComponent>();
				s_TagEdit.CurrentOption = component.Tag;
				s_TagEdit.StartPopup = true;
			}
		});
		s_MainHeader.AddToSelectionList("Add Component", [&]()
		{
			s_AddComponent.PopupActive = true;
		});

		s_TagEdit.Flags = EditorUI::TextInput_PopupOnly;
		s_TagEdit.Label = "Edit Tag";
		s_TagEdit.ConfirmAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
			if (entity && entity.HasComponent<Scenes::TagComponent>())
			{
				auto& component = entity.GetComponent<Scenes::TagComponent>();
				component.Tag = s_TagEdit.CurrentOption;
				s_MainHeader.Label = "Entity Tag: " + component.Tag;
			}
		};

		s_AddComponent.Label = "Add Component";
		s_AddComponent.Flags = EditorUI::SelectOption_PopupOnly;
		s_AddComponent.PopupAction = [&]()
		{
			Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
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
			if (option.Label == "None")
			{
				return;
			}
			if (option.Label == "Class Instance")
			{
				(*Scenes::SceneService::GetActiveScene()->GetSelectedEntity()).AddComponent<Scenes::ClassInstanceComponent>();
				return;
			}
			if (option.Label == "Camera")
			{
				(*Scenes::SceneService::GetActiveScene()->GetSelectedEntity()).AddComponent<Scenes::CameraComponent>();
				return;
			}
			if (option.Label == "Shape")
			{
				(*Scenes::SceneService::GetActiveScene()->GetSelectedEntity()).AddComponent<Scenes::ShapeComponent>();
				return;
			}
			if (option.Label == "Rigidbody 2D")
			{
				(*Scenes::SceneService::GetActiveScene()->GetSelectedEntity()).AddComponent<Scenes::Rigidbody2DComponent>();
				return;
			}
			if (option.Label == "Box Collider 2D")
			{
				(*Scenes::SceneService::GetActiveScene()->GetSelectedEntity()).AddComponent<Scenes::BoxCollider2DComponent>();
				return;
			}
			if (option.Label == "Circle Collider 2D")
			{
				(*Scenes::SceneService::GetActiveScene()->GetSelectedEntity()).AddComponent<Scenes::CircleCollider2DComponent>();
				return;
			}

			KG_ERROR("Invalid option selected to add as component!");
			
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
		s_TransformHeader.AddToSelectionList("Remove Component", [&]()
		{
			EngineService::SubmitToMainThread([&]()
			{
				Scenes::Entity entity = *Scenes::SceneService::GetActiveScene()->GetSelectedEntity();
				if (entity.HasComponent<Scenes::TransformComponent>())
				{
					entity.RemoveComponent<Scenes::TransformComponent>();
				}
			});
		});

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
	SceneHierarchyPanel::SceneHierarchyPanel()
	{
		s_EditorApp = EditorApp::GetCurrentApp();
		s_EditorApp->m_PanelToKeyboardInput.insert_or_assign(m_PanelName, 
			KG_BIND_CLASS_FN(SceneHierarchyPanel::OnKeyPressedEditor));

		// Set Primary Camera Checkbox
		s_PrimaryCameraCheckboxSpec.Label = "Set Primary";

		// Set Shape Add Texture Checkbox
		s_ShapeAddTextureCheckboxSpec.Label = "Use Texture";

		// Set Shape Circle Option
		s_ShapeAddCircleSpec.Label = "Use Circle Shape";

		// Set Shape Add Projection Option
		s_ShapeAddProjectionSpec.Label = "Use Projection Matrix";

		// Set Shape Add Entity ID Option
		s_ShapeAddEntityIDSpec.Label = "Use Entity ID";

		InitializeClassInstanceComponent();
		InitializeTransformComponent();
		InitializeMainHeader();
		InitializeRigidbody2DComponent();
		InitializeBoxCollider2DComponent();
		InitializeCircleCollider2DComponent();

	}
	void SceneHierarchyPanel::OnEditorUIRender()
	{
		KG_PROFILE_FUNCTION();
		EditorUI::EditorUIService::StartWindow(m_PanelName, &s_EditorApp->m_ShowSceneHierarchy);

		if (Scenes::SceneService::GetActiveScene())
		{
			Scenes::SceneService::GetActiveScene()->m_Registry.each([&](auto entityID)
			{
				Scenes::Entity entity{ entityID, Scenes::SceneService::GetActiveScene().get() };
				DrawEntityNode(entity);
			});

			// Right-click on blank space
			if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems))
			{
				if (ImGui::MenuItem("Create Empty Entity")) { Scenes::SceneService::GetActiveScene()->CreateEntity("Empty Entity"); }
				ImGui::EndPopup();
			}
		}
		EditorUI::EditorUIService::EndWindow();
	}
	bool SceneHierarchyPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
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
	void SceneHierarchyPanel::SetSelectedEntity(Scenes::Entity entity)
	{
		*Scenes::SceneService::GetActiveScene()->GetSelectedEntity() = entity;
		if (entity)
		{
			if (entity.HasComponent<Scenes::TagComponent>())
			{
				auto& tagComp = entity.GetComponent<Scenes::TagComponent>();
				s_MainHeader.Label = "Entity Tag: " + tagComp.Tag;
			}

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

		RefreshTransformComponent();
		s_EditorApp->m_ShowProperties = true;
		EditorUI::EditorUIService::BringWindowToFront(s_EditorApp->m_PropertiesPanel->m_PanelName);
		
	}
	void SceneHierarchyPanel::RefreshClassInstanceComponent()
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
	void SceneHierarchyPanel::RefreshTransformComponent()
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
	void SceneHierarchyPanel::DrawEntityNode(Scenes::Entity entity)
	{
		auto& tag = entity.GetComponent<Scenes::TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = ((*Scenes::SceneService::GetActiveScene()->GetSelectedEntity() == entity) ? ImGuiTreeNodeFlags_Selected : 0) |
			ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked())
		{
			SetSelectedEntity(entity);
		}
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			auto& editorCamera = EditorApp::GetCurrentApp()->m_ViewportPanel->m_EditorCamera;
			auto& transformComponent = entity.GetComponent<Scenes::TransformComponent>();
			editorCamera.SetFocalPoint(transformComponent.Translation);
			editorCamera.SetDistance(std::max({ transformComponent.Scale.x, transformComponent.Scale.y, transformComponent.Scale.z }) * 2.5f);
			editorCamera.SetMovementType(Rendering::EditorCamera::MovementType::ModelView);
		}
		bool entityDeleted = false;
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity")) { entityDeleted = true; }
			ImGui::EndPopup();
		}

		if (opened)
		{
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
			bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());
			if (opened)
				ImGui::TreePop();
			ImGui::TreePop();
		}

		if (entityDeleted)
		{
			Scenes::SceneService::GetActiveScene()->DestroyEntity(entity);
			if (*Scenes::SceneService::GetActiveScene()->GetSelectedEntity() == entity)
			{
				*Scenes::SceneService::GetActiveScene()->GetSelectedEntity() = {};
			}
		}
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Scenes::Entity entity, UIFunction uiFunction)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | 
			ImGuiTreeNodeFlags_AllowItemOverlap | 
			ImGuiTreeNodeFlags_Framed | 
			ImGuiTreeNodeFlags_SpanAvailWidth | 
			ImGuiTreeNodeFlags_FramePadding;
		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();

			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar();
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight}))
			{
				ImGui::OpenPopup("ComponentSettings");
			}
			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeComponent = true;
				}
				ImGui::EndPopup();
			}
			if (open)
			{
				//ImGui::ColorEdit4("Color", Math::value_ptr(src.Color));
				uiFunction(component);
				ImGui::TreePop();
			}
		}
	}

	void SceneHierarchyPanel::DrawComponents(Scenes::Entity entity)
	{
		if (entity.HasComponent<Scenes::TagComponent>())
		{
			EditorUI::EditorUIService::PanelHeader(s_MainHeader);
			EditorUI::EditorUIService::TextInputPopup(s_TagEdit);
			EditorUI::EditorUIService::SelectOption(s_AddComponent);
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

		DrawComponent<Scenes::CameraComponent>("Camera", entity, [](auto& component)
		{
			auto& camera = component.Camera;

			// Set Primary Camera Checkbox
			s_PrimaryCameraCheckboxSpec.ToggleBoolean = component.Primary;
			s_PrimaryCameraCheckboxSpec.ConfirmAction = [&](bool value)
			{
				component.Primary = value;
			};
			EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);
			EditorUI::EditorUIService::Checkbox(s_PrimaryCameraCheckboxSpec);
			EditorUI::EditorUIService::Spacing(EditorUI::SpacingAmount::Small);


			const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
			const char* currentProjectionTypeString = projectionTypeStrings[(int)camera.GetProjectionType()];
			if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
			{
				for (int i = 0; i < 2; i++)
				{
					bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
					if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
					{
						currentProjectionTypeString = projectionTypeStrings[i];
						camera.SetProjectionType((Scenes::SceneCamera::ProjectionType)i);
					}
					if (isSelected) { ImGui::SetItemDefaultFocus(); }
				}
				ImGui::EndCombo();
			}

			if (camera.GetProjectionType() == Scenes::SceneCamera::ProjectionType::Perspective)
			{
				float verticalFOV = glm::degrees(camera.GetPerspectiveVerticalFOV());
				if (ImGui::DragFloat("Vertical FOV", &verticalFOV, 1, 0, 10000)) { camera.SetPerspectiveVerticalFOV(glm::radians(verticalFOV)); }
				float perspectiveNear = camera.GetPerspectiveNearClip();
				if (ImGui::DragFloat("Near Plane", &perspectiveNear, 1, 0, 10000)) { camera.SetPerspectiveNearClip(perspectiveNear); }
				float perspectiveFar = camera.GetPerspectiveFarClip();
				if (ImGui::DragFloat("Far Plane", &perspectiveFar, 1, 0, 10000)) { camera.SetPerspectiveFarClip(perspectiveFar); }
			}

			if (camera.GetProjectionType() == Scenes::SceneCamera::ProjectionType::Orthographic)
			{
				float orthoSize = camera.GetOrthographicSize();
				if (ImGui::DragFloat("Size", &orthoSize, 1, 0, 10000)) { camera.SetOrthographicSize(orthoSize); }
				float orthoNear = camera.GetOrthographicNearClip();
				if (ImGui::DragFloat("Near Plane", &orthoNear, 1, 0, 10000)) { camera.SetOrthographicNearClip(orthoNear); }
				float orthoFar = camera.GetOrthographicFarClip();
				if (ImGui::DragFloat("Far Plane", &orthoFar, 1, 0, 10000)) { camera.SetOrthographicFarClip(orthoFar); }
			}
		});

		DrawComponent<Scenes::ShapeComponent>("Shape", entity, [](auto& component)
		{
			//=========================
			// Lambda Functions for Later Use (Scroll down to find main function body)
			//=========================

			// This Lambda updates the shader after a checkbox is clicked that changes the component.ShaderSpecification
			// Another function of this lambda is to ensure data inside the old buffer is transferred to the new buffer
			// if they represent the same input
			auto updateComponent = [&]()
			{
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
				if (oldBuffer) { oldBuffer.Release(); }
			};
			//=========================
			// More Lambdas: ShaderSpecification UI Code
			//=========================

			// These lambdas provide UI for user manipulating of shader specifications and input values
			auto AddFlatColorSection = [&]()
			{
				if (ImGui::Button("Select Color Input")) { ImGui::OpenPopup("Color Type Selection"); }
				ImGui::SameLine();
				ImGui::TextUnformatted(Utility::ColorInputTypeToString(component.ShaderSpecification.ColorInput).c_str());
				if (ImGui::BeginPopup("Color Type Selection"))
				{
					if (ImGui::Selectable("No Color"))
					{
						component.ShaderSpecification.ColorInput = Rendering::ColorInputType::None;
						updateComponent();
					}

					if (ImGui::Selectable("Flat Color"))
					{
						component.ShaderSpecification.ColorInput = Rendering::ColorInputType::FlatColor;
						updateComponent();
						Rendering::Shader::SetDataAtInputLocation<Math::vec4>({ 1.0f, 1.0f, 1.0f, 1.0f }, "a_Color", component.ShaderData, component.Shader);
					}
					if (ImGui::Selectable("Vertex Color"))
					{
						Math::vec4 transferColor {1.0f, 1.0f, 1.0f, 1.0f};
						if (component.ShaderSpecification.ColorInput == Rendering::ColorInputType::FlatColor)
						{
							transferColor = *Rendering::Shader::GetInputLocation<Math::vec4>("a_Color", component.ShaderData, component.Shader);
						}
						component.ShaderSpecification.ColorInput = Rendering::ColorInputType::VertexColor;
						updateComponent();
						if (component.VertexColors) { component.VertexColors->clear(); }
						component.VertexColors = CreateRef<std::vector<Math::vec4>>();
						for (uint32_t iterator{0}; iterator < component.Vertices->size(); iterator++)
						{
							component.VertexColors->push_back(transferColor);
						}
					}
					
					ImGui::EndPopup();
				}

				if (component.ShaderSpecification.ColorInput == Rendering::ColorInputType::None) { return; }

				if (component.ShaderSpecification.ColorInput == Rendering::ColorInputType::FlatColor)
				{
					Math::vec4* color = Rendering::Shader::GetInputLocation<Math::vec4>("a_Color", component.ShaderData, component.Shader);
					ImGui::ColorEdit4("Color", glm::value_ptr(*color));
				}
				if (component.ShaderSpecification.ColorInput == Rendering::ColorInputType::VertexColor)
				{
					uint32_t iterator{ 1 };
					for (auto& color : *component.VertexColors)
					{
						ImGui::ColorEdit4(("Vertex " + std::to_string(iterator)).c_str(), glm::value_ptr(color));
						iterator++;
					}
				}
			};

			auto AddTextureSection = [&]()
			{
				s_ShapeAddTextureCheckboxSpec.ToggleBoolean = component.ShaderSpecification.TextureInput == Rendering::TextureInputType::ColorTexture ? true : false;
				EditorUI::EditorUIService::Checkbox(s_ShapeAddTextureCheckboxSpec);
				s_ShapeAddTextureCheckboxSpec.ConfirmAction = [&](bool value)
				{
					value ? component.ShaderSpecification.TextureInput = Rendering::TextureInputType::ColorTexture :
						component.ShaderSpecification.TextureInput = Rendering::TextureInputType::None;
					updateComponent();
					// Checkbox is switched on
					if (value)
					{
						if (component.CurrentShape == Rendering::ShapeTypes::Cube || component.CurrentShape == Rendering::ShapeTypes::Pyramid)
						{
							component.ShaderSpecification.RenderType = Rendering::RenderingType::DrawTriangle;
							updateComponent();
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
							updateComponent();
							component.Vertices = CreateRef<std::vector<Math::vec3>>(Utility::ShapeTypeToShape(component.CurrentShape).GetIndexVertices());
							component.Indices = CreateRef<std::vector<uint32_t>>(Utility::ShapeTypeToShape(component.CurrentShape).GetIndices());
							component.TextureCoordinates = CreateRef<std::vector<Math::vec2>>(Utility::ShapeTypeToShape(component.CurrentShape).GetIndexTextureCoordinates());
							if (component.VertexColors) { component.VertexColors->resize(component.Vertices->size(), { 1.0f, 1.0f, 1.0f, 1.0f }); }
						}
					}
				};
				if (s_ShapeAddTextureCheckboxSpec.ToggleBoolean)
				{
					ImGui::Button("Texture", ImVec2(100.0f, 0.0f));
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_IMAGE"))
						{
							const wchar_t* path = (const wchar_t*)payload->Data;
							std::filesystem::path texturePath(path);
							Assets::AssetHandle currentHandle = Assets::AssetManager::ImportNewTextureFromFile(texturePath);
							component.TextureHandle = currentHandle;
							Ref<Rendering::Texture2D> texture = Assets::AssetManager::GetTexture(currentHandle);
							if (texture)
								component.Texture = texture;
							else
								KG_WARN("Could not load texture {0}", texturePath.filename().string());
						}
						ImGui::EndDragDropTarget();
					}

					float* tilingFactor = Rendering::Shader::GetInputLocation<float>("a_TilingFactor", component.ShaderData, component.Shader);
					ImGui::DragFloat("Tiling Factor", tilingFactor, 0.1f, 0.0f, 100.0f);
				}
			};

			auto AddCircleShapeSection = [&]()
			{
				s_ShapeAddCircleSpec.ConfirmAction = [&](bool value)
				{
					component.ShaderSpecification.AddCircleShape = value;
					updateComponent();
					if (value)
					{
						Rendering::Shader::SetDataAtInputLocation<float>(1.0f, "a_Thickness", component.ShaderData, component.Shader);
						Rendering::Shader::SetDataAtInputLocation<float>(0.005f, "a_Fade", component.ShaderData, component.Shader);
					}
				};
				s_ShapeAddCircleSpec.ToggleBoolean = component.ShaderSpecification.AddCircleShape;
				EditorUI::EditorUIService::Checkbox(s_ShapeAddCircleSpec);
				if (component.ShaderSpecification.AddCircleShape)
				{
					float* thickness = Rendering::Shader::GetInputLocation<float>("a_Thickness", component.ShaderData, component.Shader);
					ImGui::DragFloat("Thickness", thickness, 0.025f, 0.0f, 1.0f);

					float* fade = Rendering::Shader::GetInputLocation<float>("a_Fade", component.ShaderData, component.Shader);
					ImGui::DragFloat("Fade", fade, 0.00025f, 0.0f, 1.0f);
				}
			};

			auto AddProjectionMatrixSection = [&]()
			{
				s_ShapeAddProjectionSpec.ConfirmAction = [&](bool value)
				{
					component.ShaderSpecification.AddProjectionMatrix = value;
					updateComponent();
				};
				s_ShapeAddProjectionSpec.ToggleBoolean = component.ShaderSpecification.AddProjectionMatrix;
				EditorUI::EditorUIService::Checkbox(s_ShapeAddProjectionSpec);
			};

			auto AddEntityIDSection = [&]()
			{
				s_ShapeAddEntityIDSpec.ConfirmAction = [&](bool value)
				{
					component.ShaderSpecification.AddEntityID = value;
					updateComponent();
				};
				s_ShapeAddEntityIDSpec.ToggleBoolean = component.ShaderSpecification.AddEntityID;
				EditorUI::EditorUIService::Checkbox(s_ShapeAddEntityIDSpec);
			};
			

			//=========================
			// Beginning of Main Functionality
			//=========================
			// Display Selection Popup Button to choose Shape
			Rendering::ShapeTypes selectedShape = component.CurrentShape;
			if (ImGui::Button("Select a Shape")) { ImGui::OpenPopup("Shape Selection"); }
			ImGui::SameLine();
			ImGui::TextUnformatted(Utility::ShapeTypeToString(selectedShape).c_str());
			if (ImGui::BeginPopup("Shape Selection"))
			{
				for (const auto& shape : Rendering::Shape::s_AllShapes)
				{
					if (ImGui::Selectable(Utility::ShapeTypeToString(shape->GetShapeType()).c_str()))
					{
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
						updateComponent();
						if (component.VertexColors) { component.VertexColors->resize(component.Vertices->size(), { 1.0f, 1.0f, 1.0f, 1.0f }); }
					}
				}
				ImGui::EndPopup();
			}
			ImGui::Separator();
			ImGui::Text("Shader Specification");

			// This section displays the shader specification options available for the chosen object
			if (selectedShape == Rendering::ShapeTypes::None)
			{
				return;
			}
			if (selectedShape == Rendering::ShapeTypes::Quad)
			{
				AddFlatColorSection();
				AddTextureSection();
				AddCircleShapeSection();
				AddProjectionMatrixSection();
				AddEntityIDSection();
				
			}
			if (selectedShape == Rendering::ShapeTypes::Cube || selectedShape == Rendering::ShapeTypes::Pyramid)
			{
				AddFlatColorSection();
				AddTextureSection();
				AddProjectionMatrixSection();
				AddEntityIDSection();
				
			}
			
		});
	}
}
