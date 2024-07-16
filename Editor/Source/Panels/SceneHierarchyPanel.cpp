#include "kgpch.h"

#include "Panels/SceneHierarchyPanel.h"

#include "EditorApp.h"

#include "API/EditorUI/ImGuiBackendAPI.h"

static Kargono::EditorApp* s_EditorApp { nullptr };

namespace Kargono::Panels
{
	static EditorUI::CheckboxSpec s_PrimaryCameraCheckboxSpec {};
	static EditorUI::CheckboxSpec s_ShapeAddTextureCheckboxSpec {};
	static EditorUI::CheckboxSpec s_ShapeAddCircleSpec {};
	static EditorUI::CheckboxSpec s_ShapeAddProjectionSpec {};
	static EditorUI::CheckboxSpec s_ShapeAddEntityIDSpec {};
	static EditorUI::CheckboxSpec s_RigidBodyFixedRotSpec {};

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
				auto& component = entity.GetComponent<Scenes::ClassInstanceComponent>();
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

		// Set Shape Add Fixed Rotation Option
		s_RigidBodyFixedRotSpec.Label = "Use Fixed Rotation";

		InitializeClassInstanceComponent();
		InitializeTransformComponent();


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

			

			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			{
				SetSelectedEntity({});
			}

			// Right-click on blank space
			if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems))
			{
				if (ImGui::MenuItem("Create Empty Entity")) { Scenes::SceneService::GetActiveScene()->CreateEntity("Empty Entity"); }
				ImGui::EndPopup();
			}

		}
		EditorUI::EditorUIService::EndWindow();

		EditorUI::EditorUIService::StartWindow("Properties");
		if (*Scenes::SceneService::GetActiveScene()->GetSelectedEntity())
		{
			DrawComponents(*Scenes::SceneService::GetActiveScene()->GetSelectedEntity());
		}

		EditorUI::EditorUIService::EndWindow();
	}
	bool SceneHierarchyPanel::OnKeyPressedEditor(Events::KeyPressedEvent event)
	{
		return false;
	}
	void SceneHierarchyPanel::SetSelectedEntity(Scenes::Entity entity)
	{
		*Scenes::SceneService::GetActiveScene()->GetSelectedEntity() = entity;
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

			if (entity.HasComponent<Scenes::TransformComponent>())
			{
				auto& transformComp = entity.GetComponent<Scenes::TransformComponent>();
				s_TransformEditTranslation.CurrentVec3 = transformComp.Translation;
				s_TransformEditRotation.CurrentVec3 = transformComp.Rotation;
				s_TransformEditScale.CurrentVec3 = transformComp.Scale;
			}
		}
		
	}
	void SceneHierarchyPanel::RefreshWidgetData()
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

	static void DrawVec3Control(const std::string& label, Math::vec3& values, 
		float resetValue = 0.0f, float columnWidth = 100.0f)
	{

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize)) { values.x = resetValue; }
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize)) { values.y = resetValue; }
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize)) { values.z = resetValue; }
		ImGui::PopFont();
		ImGui::PopStyleColor(3);
		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();

		
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
			if (removeComponent)
			{
				entity.RemoveComponent<T>();
				if (typeid(T) == typeid(Scenes::AudioComponent) && entity.HasComponent<Scenes::MultiAudioComponent>())
				{
					entity.RemoveComponent<Scenes::MultiAudioComponent>();
				}
			}
		}
	}

	void SceneHierarchyPanel::DrawComponents(Scenes::Entity entity)
	{
		if (entity.HasComponent<Scenes::TagComponent>())
		{
			auto& tag = entity.GetComponent<Scenes::TagComponent>().Tag;

			char buffer[256] = {};
			strncpy_s(buffer, tag.c_str(), sizeof(buffer));
			ImGui::SetCursorPosX(30.0f);
			ImGui::Text(tag.c_str());
			if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				ImGui::OpenPopup("UpdateTag");
			}

			if (ImGui::BeginPopup("UpdateTag"))
			{
				ImGui::InputText("##Tag", buffer, sizeof(buffer));
				if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))
				{
					tag = std::string(buffer);
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}

		ImGui::SameLine(ImGui::GetWindowWidth() - 130);
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("AddComponent");
		}

		if (ImGui::BeginPopup("AddComponent"))
		{

			DisplayAddComponentEntry<Scenes::ClassInstanceComponent>("Class Instance");
			DisplayAddComponentEntry<Scenes::CameraComponent>("Camera");
			DisplayAddComponentEntry<Scenes::ShapeComponent>("Shape");
			DisplayAddComponentEntry<Scenes::AudioComponent>("Audio");
			DisplayAddComponentEntry<Scenes::Rigidbody2DComponent>("Rigidbody 2D");
			DisplayAddComponentEntry<Scenes::BoxCollider2DComponent>("Box Collider 2D");
			DisplayAddComponentEntry<Scenes::CircleCollider2DComponent>("Circle Collider 2D");

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		if (entity.HasComponent<Scenes::TransformComponent>())
		{
			Scenes::TransformComponent& component = entity.GetComponent<Scenes::TransformComponent>();
			EditorUI::EditorUIService::CollapsingHeader(s_TransformHeader);
			if (s_TransformHeader.Expanded)
			{
				EditorUI::EditorUIService::EditVec3(s_TransformEditTranslation);
				EditorUI::EditorUIService::EditVec3(s_TransformEditScale);
				EditorUI::EditorUIService::EditVec3(s_TransformEditRotation);
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

		DrawComponent<Scenes::AudioComponent>("Audio", entity, [&](auto& component)
		{
			bool replaceComponent = false;
			bool deleteComponent = false;
			static std::string oldComponentName {};
			Scenes::AudioComponent replacementComponent{};

			auto AudioTableRow = [&](uint32_t slot, Scenes::AudioComponent& audioComp) mutable
			{
				ImGui::TableNextRow();
				// Column One Displays Slot Number
				ImGui::TableSetColumnIndex(0);
				std::string slotTitle = (std::string("Slot ") + std::to_string(slot));
				if (slot == 1) { slotTitle = slotTitle + " (Default)"; }
				ImGui::Text(slotTitle.c_str());
				// Column Two Displays Component Tag
				ImGui::TableSetColumnIndex(1);
				static std::string nameOutput {};
				static char textBuffer[256] = {};
				nameOutput = audioComp.Audio ? audioComp.Name : "[Empty]";
				strncpy_s(textBuffer, nameOutput.c_str(), sizeof(textBuffer));
				ImGui::Text(textBuffer);
				if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					if (audioComp.Audio)
					{
						ImGui::OpenPopup(("UpdateAudio" + std::to_string(slot)).c_str());
					}
					else
					{
						ImGui::OpenPopup(("InvalidSlot" + std::to_string(slot)).c_str());
					}
					
				}

				if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
				{
					if (slot > 1) { ImGui::OpenPopup(("DeleteSlot" + std::to_string(slot)).c_str()); }
				}

				if (ImGui::BeginPopup(("UpdateAudio" + std::to_string(slot)).c_str()))
				{
					ImGui::InputText("##", textBuffer, sizeof(textBuffer));
					if (ImGui::IsWindowFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))
					{
						oldComponentName = audioComp.Name;
						audioComp.Name = std::string(textBuffer);
						if (slot > 1)
						{
							replacementComponent = audioComp;
							replaceComponent = true;
						}
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				if (ImGui::BeginPopup(("InvalidSlot" + std::to_string(slot)).c_str()))
				{
					ImGui::Text("Please Drag an Audio File to this slot!");
					ImGui::EndPopup();
				}

				if (ImGui::BeginPopup(("DeleteSlot" + std::to_string(slot)).c_str()))
				{
					if (ImGui::Selectable("Delete Slot"))
					{
						oldComponentName = audioComp.Name;
						deleteComponent = true;
					}
					ImGui::EndPopup();
				}

				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_AUDIO"))
					{
						const wchar_t* path = (const wchar_t*)payload->Data;
						std::filesystem::path audioPath(path);
						Assets::AssetHandle currentHandle = Assets::AssetManager::ImportNewAudioFromFile(audioPath);
						audioComp.AudioHandle = currentHandle;
						Ref<Audio::AudioBuffer> audio = Assets::AssetManager::GetAudio(currentHandle);
						if (audio)
						{
							oldComponentName = audioComp.Name;
							audioComp.Audio = audio;
							audioComp.Name = audioPath.stem().string();
							if (slot > 1)
							{
								replacementComponent = audioComp;
								replaceComponent = true;
							}
						}

						else { KG_WARN("Could not load audio {0}", audioPath.filename().string()); }
					}
					ImGui::EndDragDropTarget();
				}

			};

			// Main Table that lists audio slots and their corresponding AudioComponents
			static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

			if (ImGui::BeginTable("table", 2, flags))
			{
				ImGui::TableSetupColumn("Slot", ImGuiTableColumnFlags_WidthFixed, 100.0f);
				ImGui::TableSetupColumn("Audio Tag", ImGuiTableColumnFlags_WidthFixed, 200.0f);
				ImGui::TableHeadersRow();

				uint32_t iterator{ 1 };
				// Add Basic Audio Component Slot
				AudioTableRow(iterator, component);

				// Add Multi Audio Component Slots
				if (entity.HasComponent<Scenes::MultiAudioComponent>())
				{
					iterator++;
					for (auto& [title, audioComp] : entity.GetComponent<Scenes::MultiAudioComponent>().AudioComponents)
					{
						AudioTableRow(iterator, audioComp);
						iterator++;
					}
				}
				ImGui::EndTable();
			}

			// Add New Slot Section
			if (ImGui::Button("Add Audio Slot"))
			{
				if (!entity.HasComponent<Scenes::MultiAudioComponent>())
				{
					entity.AddComponent<Scenes::MultiAudioComponent>();
				}
				Scenes::AudioComponent newComponent = Scenes::AudioComponent();
				newComponent.Audio = nullptr;
				uint32_t iterator{ 0 };
				std::string temporaryName {"Empty"};
				// TODO: Potential Infinite Loop
				while (entity.GetComponent<Scenes::MultiAudioComponent>().AudioComponents.
				              contains(temporaryName + std::to_string(iterator))) { iterator++; }
				newComponent.Name = temporaryName + std::to_string(iterator);
				entity.GetComponent<Scenes::MultiAudioComponent>().AudioComponents.insert({ temporaryName + std::to_string(iterator), newComponent });
			}

			// Replace Component outside of for loop to prevent errors
			if (replaceComponent)
			{
				entity.GetComponent<Scenes::MultiAudioComponent>().AudioComponents.erase(oldComponentName);
				entity.GetComponent<Scenes::MultiAudioComponent>().AudioComponents.insert({ replacementComponent.Name, replacementComponent });
			}
			if (deleteComponent)
			{
				entity.GetComponent<Scenes::MultiAudioComponent>().AudioComponents.erase(oldComponentName);
				if (entity.HasComponent<Scenes::MultiAudioComponent>())
				{
					if (entity.GetComponent<Scenes::MultiAudioComponent>().AudioComponents.size() <= 0)
					{
						entity.RemoveComponent<Scenes::MultiAudioComponent>();
					}
				}
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
			if (selectedShape == Rendering::ShapeTypes::None) { return; }
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

		DrawComponent<Scenes::Rigidbody2DComponent>("Rigidbody 2D", entity, [](auto& component)
		{
			const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic"};
			const char* currentBodyTypeString = bodyTypeStrings[(int)component.Type];
			if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
			{
				for (int i = 0; i < 2; i++)
				{
					bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
					if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
					{
						currentBodyTypeString = bodyTypeStrings[i];
						component.Type = (Scenes::Rigidbody2DComponent::BodyType)i;
					}
					if (isSelected) { ImGui::SetItemDefaultFocus(); }
				}
				ImGui::EndCombo();
			}

			s_RigidBodyFixedRotSpec.ConfirmAction = [&](bool value)
			{
				component.FixedRotation = value;
			};
			s_RigidBodyFixedRotSpec.ToggleBoolean = component.FixedRotation;
			EditorUI::EditorUIService::Checkbox(s_RigidBodyFixedRotSpec);
		});

		DrawComponent<Scenes::BoxCollider2DComponent>("Box Collider 2D", entity, [](auto& component)
			{
				ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset), 0.005f);
				ImGui::DragFloat2("Size", glm::value_ptr(component.Size), 0.005f);
				ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
			});

		DrawComponent<Scenes::CircleCollider2DComponent>("Circle Collider 2D", entity, [](auto& component)
			{
				ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset));
				ImGui::DragFloat("Radius", &component.Radius, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
			});
	}

	template<typename T>
	void SceneHierarchyPanel::DisplayAddComponentEntry(const std::string& entryName) {
		if (!(*Scenes::SceneService::GetActiveScene()->GetSelectedEntity()).HasComponent<T>())
		{
			if (ImGui::MenuItem(entryName.c_str()))
			{
				(*Scenes::SceneService::GetActiveScene()->GetSelectedEntity()).AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
	}
}
