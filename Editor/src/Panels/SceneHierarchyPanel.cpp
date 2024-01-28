#include "kgpch.h"

#include "Panels/SceneHierarchyPanel.h"

#include "EditorLayer.h"

#include <imgui.h>
#include "imgui_internal.h"


/* The Microsoft C++ compiler is non-compliant with the C++ standard and needs
 * the following definition to disable a security warning on std::strncpy().
 */
#ifdef _MSVC_LANG
#define _CRT_SECURE_NO_WARNINGS
#endif

namespace Kargono
{
	void SceneHierarchyPanel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");

		if (Scene::GetActiveScene())
		{
			Scene::GetActiveScene()->m_Registry.each([&](auto entityID)
				{
					Entity entity{ entityID, Scene::GetActiveScene().get() };
					DrawEntityNode(entity);
				});

			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) { *Scene::GetActiveScene()->GetSelectedEntity() = {}; }

			// Right-click on blank space
			if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems))
			{
				if (ImGui::MenuItem("Create Empty Entity")) { Scene::GetActiveScene()->CreateEntity("Empty Entity"); }
				ImGui::EndPopup();
			}

		}
		ImGui::End();

		ImGui::Begin("Properties");
		if (*Scene::GetActiveScene()->GetSelectedEntity())
		{
			DrawComponents(*Scene::GetActiveScene()->GetSelectedEntity());
		}

		ImGui::End();
	}
	void SceneHierarchyPanel::SetSelectedEntity(Entity entity)
	{
		*Scene::GetActiveScene()->GetSelectedEntity() = entity;
	}
	void SceneHierarchyPanel::DrawEntityNode(Entity entity)
	{
		auto& tag = entity.GetComponent<TagComponent>().Tag;

		ImGuiTreeNodeFlags flags = ((*Scene::GetActiveScene()->GetSelectedEntity() == entity) ? ImGuiTreeNodeFlags_Selected : 0) |
			ImGuiTreeNodeFlags_OpenOnArrow;
		flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, tag.c_str());
		if (ImGui::IsItemClicked())
		{
			*Scene::GetActiveScene()->GetSelectedEntity() = entity;
		}
		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			auto& editorCamera = EditorLayer::GetCurrentLayer()->GetEditorCamera();
			auto& transformComponent = entity.GetComponent<TransformComponent>();
			editorCamera.SetFocalPoint(transformComponent.Translation);
			editorCamera.SetDistance(std::max({ transformComponent.Scale.x, transformComponent.Scale.y, transformComponent.Scale.z }) * 2.5f);
			editorCamera.SetMovementType(EditorCamera::MovementType::ModelView);
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
			Scene::GetActiveScene()->DestroyEntity(entity);
			if (*Scene::GetActiveScene()->GetSelectedEntity() == entity)
			{
				*Scene::GetActiveScene()->GetSelectedEntity() = {};
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
	static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction)
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
				if (typeid(T) == typeid(AudioComponent) && entity.HasComponent<MultiAudioComponent>())
				{
					entity.RemoveComponent<MultiAudioComponent>();
				}
			}
		}
	}

	void SceneHierarchyPanel::DrawComponents(Entity entity)
	{
		if (entity.HasComponent<TagComponent>())
		{
			auto& tag = entity.GetComponent<TagComponent>().Tag;

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
			DisplayAddComponentEntry<CameraComponent>("Camera");
			DisplayAddComponentEntry<ScriptComponent>("Script");
			DisplayAddComponentEntry<ShapeComponent>("Shape");
			DisplayAddComponentEntry<AudioComponent>("Audio");
			DisplayAddComponentEntry<Rigidbody2DComponent>("Rigidbody 2D");
			DisplayAddComponentEntry<BoxCollider2DComponent>("Box Collider 2D");
			DisplayAddComponentEntry<CircleCollider2DComponent>("Circle Collider 2D");

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
			{
				DrawVec3Control("Translation", component.Translation);
				Math::vec3 rotation = glm::degrees(component.Rotation);
				DrawVec3Control("Rotation", rotation);
				component.Rotation = glm::radians(rotation);
				DrawVec3Control("Scale", component.Scale, 1.0f);
			});

		DrawComponent<CameraComponent>("Camera", entity, [](auto& component)
			{
				auto& camera = component.Camera;

				ImGui::Checkbox("Primary", &component.Primary);


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
							camera.SetProjectionType((SceneCamera::ProjectionType)i);
						}
						if (isSelected) { ImGui::SetItemDefaultFocus(); }
					}
					ImGui::EndCombo();
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Perspective)
				{
					float verticalFOV = glm::degrees(camera.GetPerspectiveVerticalFOV());
					if (ImGui::DragFloat("Vertical FOV", &verticalFOV, 1, 0, 10000)) { camera.SetPerspectiveVerticalFOV(glm::radians(verticalFOV)); }
					float perspectiveNear = camera.GetPerspectiveNearClip();
					if (ImGui::DragFloat("Near Plane", &perspectiveNear, 1, 0, 10000)) { camera.SetPerspectiveNearClip(perspectiveNear); }
					float perspectiveFar = camera.GetPerspectiveFarClip();
					if (ImGui::DragFloat("Far Plane", &perspectiveFar, 1, 0, 10000)) { camera.SetPerspectiveFarClip(perspectiveFar); }
				}

				if (camera.GetProjectionType() == SceneCamera::ProjectionType::Orthographic)
				{
					float orthoSize = camera.GetOrthographicSize();
					if (ImGui::DragFloat("Size", &orthoSize, 1, 0, 10000)) { camera.SetOrthographicSize(orthoSize); }
					float orthoNear = camera.GetOrthographicNearClip();
					if (ImGui::DragFloat("Near Plane", &orthoNear, 1, 0, 10000)) { camera.SetOrthographicNearClip(orthoNear); }
					float orthoFar = camera.GetOrthographicFarClip();
					if (ImGui::DragFloat("Far Plane", &orthoFar, 1, 0, 10000)) { camera.SetOrthographicFarClip(orthoFar); }
				}
			});

		DrawComponent<ScriptComponent>("Script", entity, [entity, this](auto& component) mutable
		{
			// Load in all entity class names
			auto entityClasses = Script::ScriptEngine::GetEntityClasses();
			std::string currentScript{"None"};
			bool scriptClassExists;
			if (scriptClassExists = entityClasses.contains(component.ClassName)){ currentScript = component.ClassName; }
			if (ImGui::BeginCombo("Entity Scripts", currentScript.c_str()))
			{
				if (ImGui::Selectable("None"))
				{
					component.ClassName = "";
					ImGui::SetItemDefaultFocus();
					scriptClassExists = false;
				}
				for (auto& [className, classReference] : entityClasses)
				{
					if (ImGui::Selectable(className.c_str()))
					{
						component.ClassName = className;
						scriptClassExists = true;
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
			//const bool scriptClassExists = ScriptEngine::EntityClassExists(component.ClassName);

			//static char buffer[64];
			//strcpy_s(buffer, component.ClassName.c_str());

			//UI::ScopedStyleColor textColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f), !scriptClassExists);

			/*if (ImGui::InputText("Class", buffer, sizeof(buffer)))
			{
				component.ClassName = buffer;
				return;
			}*/


			// Fields
			bool sceneRunning = Scene::GetActiveScene()->IsRunning();
			if (sceneRunning)
			{
				Ref<Script::ScriptClassEntityInstance> scriptInstance = Script::ScriptEngine::GetEntityScriptInstance(entity.GetUUID());
			   if (scriptInstance)
			   {
				   const auto& fields = scriptInstance->GetScriptClass()->GetFields();
				   for (const auto& [name, field] : fields)
				   {
					   if (field.Type == Script::ScriptFieldType::Float)
					   {
						   float data = scriptInstance->GetFieldValue<float>(name);
						   if (ImGui::DragFloat(name.c_str(), &data))
						   {
							   scriptInstance->SetFieldValue(name, data);
						   }
					   }
				   }
			   }
			}
			else
			{
				if (scriptClassExists)
				{
					Ref<Script::ScriptClass> entityClass = Script::ScriptEngine::GetEntityClass(component.ClassName);
					const auto& fields = entityClass->GetFields();

					auto& entityFields = Script::ScriptEngine::GetScriptFieldMap(entity);
					for (const auto& [name, field] : fields)
					{
							// Field has been set in editor
						if (entityFields.find(name) != entityFields.end())
						{
							Script::ScriptFieldInstance& scriptField = entityFields.at(name);
							// Display control to set it maybe
							if (field.Type == Script::ScriptFieldType::Float)
							{
								float data = scriptField.GetValue<float>();
								if (ImGui::DragFloat(name.c_str(), &data)){scriptField.SetValue<float>(data);}
							}
						}
						else
						{
							// Display control to set it maybe
							if (field.Type == Script::ScriptFieldType::Float)
							{
								float data = 0.0f;
								if (ImGui::DragFloat(name.c_str(), &data))
								{
									Script::ScriptFieldInstance& fieldInstance = entityFields[name];
									fieldInstance.Field = field;
									fieldInstance.SetValue(data);
								}
							}
						}
					}
				}
			}

		});

		DrawComponent<AudioComponent>("Audio", entity, [&](auto& component)
		{
			bool replaceComponent = false;
			bool deleteComponent = false;
			static std::string oldComponentName {};
			AudioComponent replacementComponent{};

			auto AudioTableRow = [&](uint32_t slot, AudioComponent& audioComp) mutable
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
				if (entity.HasComponent<MultiAudioComponent>())
				{
					iterator++;
					for (auto& [title, audioComp] : entity.GetComponent<MultiAudioComponent>().AudioComponents)
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
				if (!entity.HasComponent<MultiAudioComponent>())
				{
					entity.AddComponent<MultiAudioComponent>();
				}
				AudioComponent newComponent = AudioComponent();
				newComponent.Audio = nullptr;
				uint32_t iterator{ 0 };
				std::string temporaryName {"Empty"};
				// TODO: Potential Infinite Loop
				while (entity.GetComponent<MultiAudioComponent>().AudioComponents.
					contains(temporaryName + std::to_string(iterator))) { iterator++; }
				newComponent.Name = temporaryName + std::to_string(iterator);
				entity.GetComponent<MultiAudioComponent>().AudioComponents.insert({ temporaryName + std::to_string(iterator), newComponent });
			}

			// Replace Component outside of for loop to prevent errors
			if (replaceComponent)
			{
				entity.GetComponent<MultiAudioComponent>().AudioComponents.erase(oldComponentName);
				entity.GetComponent<MultiAudioComponent>().AudioComponents.insert({ replacementComponent.Name, replacementComponent });
			}
			if (deleteComponent)
			{
				entity.GetComponent<MultiAudioComponent>().AudioComponents.erase(oldComponentName);
				if (entity.HasComponent<MultiAudioComponent>())
				{
					if (entity.GetComponent<MultiAudioComponent>().AudioComponents.size() <= 0)
					{
						entity.RemoveComponent<MultiAudioComponent>();
					}
				}
			}
		});

		DrawComponent<ShapeComponent>("Shape", entity, [](auto& component)
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
				Ref<Shader> oldShader = component.Shader;
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
						uint8_t* newLocationPointer = Shader::GetInputLocation<uint8_t>(element.Name, newBuffer, newShader);

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
						component.ShaderSpecification.ColorInput = ColorInputType::None;
						updateComponent();
					}

					if (ImGui::Selectable("Flat Color"))
					{
						component.ShaderSpecification.ColorInput = ColorInputType::FlatColor;
						updateComponent();
						Shader::SetDataAtInputLocation<Math::vec4>({ 1.0f, 1.0f, 1.0f, 1.0f }, "a_Color", component.ShaderData, component.Shader);
					}
					if (ImGui::Selectable("Vertex Color"))
					{
						Math::vec4 transferColor {1.0f, 1.0f, 1.0f, 1.0f};
						if (component.ShaderSpecification.ColorInput == ColorInputType::FlatColor)
						{
							transferColor = *Shader::GetInputLocation<Math::vec4>("a_Color", component.ShaderData, component.Shader);
						}
						component.ShaderSpecification.ColorInput = ColorInputType::VertexColor;
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

				if (component.ShaderSpecification.ColorInput == ColorInputType::None) { return; }

				if (component.ShaderSpecification.ColorInput == ColorInputType::FlatColor)
				{
					Math::vec4* color = Shader::GetInputLocation<Math::vec4>("a_Color", component.ShaderData, component.Shader);
					ImGui::ColorEdit4("Color", glm::value_ptr(*color));
				}
				if (component.ShaderSpecification.ColorInput == ColorInputType::VertexColor)
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
				static bool s_CheckBox = component.ShaderSpecification.TextureInput == TextureInputType::ColorTexture ? true : false;
				if (ImGui::Checkbox("Add Texture", &s_CheckBox))
				{
					s_CheckBox ? component.ShaderSpecification.TextureInput = TextureInputType::ColorTexture :
						component.ShaderSpecification.TextureInput = TextureInputType::None;
					updateComponent();
					// Checkbox is switched on
					if (s_CheckBox)
					{
						if (component.CurrentShape == ShapeTypes::Cube || component.CurrentShape == ShapeTypes::Pyramid)
						{
							component.ShaderSpecification.RenderType = RenderingType::DrawTriangle;
							updateComponent();
							component.Vertices = CreateRef<std::vector<Math::vec3>>(Utility::ShapeTypeToShape(component.CurrentShape).GetTriangleVertices());
							component.TextureCoordinates = CreateRef<std::vector<Math::vec2>>(Utility::ShapeTypeToShape(component.CurrentShape).GetTriangleTextureCoordinates());
							if (component.VertexColors) { component.VertexColors->resize(component.Vertices->size(), { 1.0f, 1.0f, 1.0f, 1.0f }); }
						}
						Shader::SetDataAtInputLocation<float>(1.0f, "a_TilingFactor", component.ShaderData, component.Shader);
					}
					// Checkbox is switched off
					if (!s_CheckBox)
					{
						if (component.CurrentShape == ShapeTypes::Cube || component.CurrentShape == ShapeTypes::Pyramid)
						{
							component.ShaderSpecification.RenderType = RenderingType::DrawIndex;
							updateComponent();
							component.Vertices = CreateRef<std::vector<Math::vec3>>(Utility::ShapeTypeToShape(component.CurrentShape).GetIndexVertices());
							component.Indices = CreateRef<std::vector<uint32_t>>(Utility::ShapeTypeToShape(component.CurrentShape).GetIndices());
							component.TextureCoordinates = CreateRef<std::vector<Math::vec2>>(Utility::ShapeTypeToShape(component.CurrentShape).GetIndexTextureCoordinates());
							if (component.VertexColors) { component.VertexColors->resize(component.Vertices->size(), { 1.0f, 1.0f, 1.0f, 1.0f }); }
						}
					}
				}
				if (s_CheckBox)
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
							Ref<Texture2D> texture = Assets::AssetManager::GetTexture(currentHandle);
							if (texture)
								component.Texture = texture;
							else
								KG_WARN("Could not load texture {0}", texturePath.filename().string());
						}
						ImGui::EndDragDropTarget();
					}

					float* tilingFactor = Shader::GetInputLocation<float>("a_TilingFactor", component.ShaderData, component.Shader);
					ImGui::DragFloat("Tiling Factor", tilingFactor, 0.1f, 0.0f, 100.0f);
				}
			};

			auto AddCircleShapeSection = [&]()
			{
				if (ImGui::Checkbox("Add Circle Shape", &component.ShaderSpecification.AddCircleShape))
				{
					updateComponent();
					if (component.ShaderSpecification.AddCircleShape)
					{
						Shader::SetDataAtInputLocation<float>(1.0f, "a_Thickness", component.ShaderData, component.Shader);
						Shader::SetDataAtInputLocation<float>(0.005f, "a_Fade", component.ShaderData, component.Shader);
					}
				}
				if (component.ShaderSpecification.AddCircleShape)
				{
					float* thickness = Shader::GetInputLocation<float>("a_Thickness", component.ShaderData, component.Shader);
					ImGui::DragFloat("Thickness", thickness, 0.025f, 0.0f, 1.0f);

					float* fade = Shader::GetInputLocation<float>("a_Fade", component.ShaderData, component.Shader);
					ImGui::DragFloat("Fade", fade, 0.00025f, 0.0f, 1.0f);
				}
			};

			auto AddProjectionMatrixSection = [&]()
			{
				if (ImGui::Checkbox("Add Projection Matrix", &component.ShaderSpecification.AddProjectionMatrix)) { updateComponent(); }
			};

			auto AddEntityIDSection = [&]()
			{
				if (ImGui::Checkbox("Add Entity ID", &component.ShaderSpecification.AddEntityID)) { updateComponent(); }
			};
			

			//=========================
			// Beginning of Main Functionality
			//=========================
			// Display Selection Popup Button to choose Shape
			ShapeTypes selectedShape = component.CurrentShape;
			if (ImGui::Button("Select a Shape")) { ImGui::OpenPopup("Shape Selection"); }
			ImGui::SameLine();
			ImGui::TextUnformatted(Utility::ShapeTypeToString(selectedShape).c_str());
			if (ImGui::BeginPopup("Shape Selection"))
			{
				for (const auto& shape : Shape::s_AllShapes)
				{
					if (ImGui::Selectable(Utility::ShapeTypeToString(shape->GetShapeType()).c_str()))
					{
						component.CurrentShape = shape->GetShapeType();
						component.ShaderSpecification.RenderType = shape->GetRenderingType();
						if (shape->GetRenderingType() == RenderingType::DrawIndex)
						{
							component.Vertices = CreateRef<std::vector<Math::vec3>>(shape->GetIndexVertices());
							component.Indices = CreateRef<std::vector<uint32_t>>(shape->GetIndices());
							component.TextureCoordinates = CreateRef<std::vector<Math::vec2>>(shape->GetIndexTextureCoordinates());
						}
						if (shape->GetRenderingType() == RenderingType::DrawTriangle)
						{
							component.Vertices = CreateRef<std::vector<Math::vec3>>(shape->GetTriangleVertices());
							component.TextureCoordinates = CreateRef<std::vector<Math::vec2>>(shape->GetTriangleTextureCoordinates());
						}
						if (component.CurrentShape == ShapeTypes::Cube || component.CurrentShape == ShapeTypes::Pyramid)
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
			if (selectedShape == ShapeTypes::None) { return; }
			if (selectedShape == ShapeTypes::Quad)
			{
				AddFlatColorSection();
				AddTextureSection();
				AddCircleShapeSection();
				AddProjectionMatrixSection();
				AddEntityIDSection();
				
			}
			if (selectedShape == ShapeTypes::Cube || selectedShape == ShapeTypes::Pyramid)
			{
				AddFlatColorSection();
				AddTextureSection();
				AddProjectionMatrixSection();
				AddEntityIDSection();
				
			}
			
		});

		DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](auto& component)
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
							component.Type = (Rigidbody2DComponent::BodyType)i;
						}
						if (isSelected) { ImGui::SetItemDefaultFocus(); }
					}
					ImGui::EndCombo();
				}

				ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
			});

		DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto& component)
			{
				ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset), 0.005f);
				ImGui::DragFloat2("Size", glm::value_ptr(component.Size), 0.005f);
				ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
				ImGui::DragFloat("Restitution Threshold", &component.RestitutionThreshold, 0.01f, 0.0f);
			});

		DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, [](auto& component)
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
		if (!(*Scene::GetActiveScene()->GetSelectedEntity()).HasComponent<T>())
		{
			if (ImGui::MenuItem(entryName.c_str()))
			{
				(*Scene::GetActiveScene()->GetSelectedEntity()).AddComponent<T>();
				ImGui::CloseCurrentPopup();
			}
		}
	}
}
